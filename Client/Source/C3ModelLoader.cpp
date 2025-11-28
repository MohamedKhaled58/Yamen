#include "Client/C3ModelLoader.h"
#include "Core/Logging/Logger.h"
#include "Graphics/RHI/Buffer.h"

namespace Yamen::Client {

entt::entity C3ModelLoader::LoadModel(entt::registry &registry,
                                      const std::string &filepath) {
  // Create entity
  auto entity = registry.create();

  // Load PHY file
  auto *phy = new Assets::C3Phy();
  if (!Assets::C3PhyLoader::Load(filepath, *phy)) {
    YAMEN_CORE_ERROR("Failed to load C3 model: {}", filepath);
    delete phy;
    registry.destroy(entity);
    return entt::null;
  }

  // Add mesh component
  auto &meshComp = registry.emplace<ECS::C3MeshComponent>(entity);
  meshComp.phy = phy;
  meshComp.visible = true;

  // Add animation component if model has animation data
  if (phy->motion && phy->motion->boneCount > 0) {
    auto &animComp = registry.emplace<ECS::SkeletalAnimationComponent>(entity);
    animComp.motion = phy->motion; // Reference to motion data in phy
    animComp.currentFrame = 0.0f;
    animComp.playbackSpeed = 30.0f; // 30 FPS default
    animComp.isPlaying = true;
    animComp.loop = true;
    animComp.boneMatrices.resize(phy->motion->boneCount, glm::mat4(1.0f));

    // Copy inverse bind matrices
    if (!phy->invBindMatrices.empty()) {
      animComp.inverseBindMatrices = phy->invBindMatrices;
    } else {
      animComp.inverseBindMatrices.resize(phy->motion->boneCount,
                                          glm::mat4(1.0f));
    }

    YAMEN_CORE_INFO("Loaded animated C3 model: {} ({} bones, {} frames)",
                    filepath, phy->motion->boneCount, phy->motion->frameCount);
  } else {
    YAMEN_CORE_INFO("Loaded static C3 model: {}", filepath);
  }

  return entity;
}

void C3ModelLoader::RenderModel(entt::entity entity, entt::registry &registry,
                                Graphics::C3SkeletalRenderer &renderer,
                                const glm::mat4 &modelViewProj) {
  // Get mesh component
  auto *meshComp = registry.try_get<ECS::C3MeshComponent>(entity);
  if (!meshComp || !meshComp->phy || !meshComp->visible) {
    return;
  }

  auto *phy = meshComp->phy;

  // Set MVP matrix
  renderer.SetModelViewProj(modelViewProj);

  // Set UV animation offset if applicable
  if (phy->uvAnimStep.x != 0.0f || phy->uvAnimStep.y != 0.0f) {
    glm::vec2 uvOffset = phy->uvAnimStep * meshComp->uvAnimTime;
    renderer.SetUVAnimationOffset(uvOffset);
  }

  // Get animation component if present
  auto *animComp = registry.try_get<ECS::SkeletalAnimationComponent>(entity);
  if (animComp && !animComp->boneMatrices.empty()) {
    // Set bone matrices from animation
    renderer.SetBoneMatricesFromMat4(
        animComp->boneMatrices.data(),
        static_cast<uint32_t>(animComp->boneMatrices.size()));
  } else {
    // Use identity matrices for static mesh
    std::vector<glm::mat4> identityBones(1, glm::mat4(1.0f));
    renderer.SetBoneMatricesFromMat4(identityBones.data(), 1);
  }

  // Bind renderer state
  // Bind renderer state
  renderer.Bind();

  // Create buffers if they don't exist
  if (!meshComp->vertexBuffer) {
    meshComp->vertexBuffer = std::make_shared<Graphics::Buffer>(
        renderer.GetDevice(), Graphics::BufferType::Vertex);

    // Create vertex buffer from PHY vertices
    // Note: We use the raw vertex data from C3Phy
    // The shader expects:
    // float3 position : POSITION
    // float4 color : COLOR (we don't have this in PhyVertex, need to handle
    // stride) float2 texCoord : TEXCOORD0 float4 boneIndexWeight : TEXCOORD1

    // Our PhyVertex struct:
    // vec3 pos
    // vec3 normal
    // vec2 uv
    // uint8 indices[4]
    // float weights[4]

    // Total size: 12+12+8+4+16 = 52 bytes

    // We need to verify if the shader input layout matches this.
    // Looking at C3Skin.hlsl:
    // float3 c3_Vertex : POSITION;
    // float4 c3_VertexColor : COLOR;
    // float2 c3_TexCoord0 : TEXCOORD0;
    // float4 c3_BoneIndexWeight : TEXCOORD1;

    // The shader expects a COLOR field which we don't have in PhyVertex.
    // We might need to create a temporary buffer with the correct layout or
    // update the shader. For now, let's try to create the buffer with the
    // existing data and see if we can adjust the stride/offset. Actually, it's
    // better to create a struct that matches the shader input exactly.

    struct RenderVertex {
      glm::vec3 pos;
      glm::vec4 color;
      glm::vec2 uv;
      glm::vec4 boneIndexWeight; // x,y = indices, z,w = weights
    };

    std::vector<RenderVertex> renderVertices;
    renderVertices.reserve(phy->vertices.size());

    for (const auto &v : phy->vertices) {
      RenderVertex rv;
      rv.pos = v.position;
      rv.color = glm::vec4(1.0f); // Default white color
      rv.uv = v.texCoord;

      // Pack indices and weights for shader
      // Shader expects:
      // x = index1 * 3
      // y = index2 * 3
      // z = weight1 * 255
      // w = weight2 * 255

      // But wait, the shader code says:
      // int nIndex1 = int(input.c3_BoneIndexWeight.x * 3.0);
      // float fWeight1 = input.c3_BoneIndexWeight.z * 0.0039215686; // 1/255

      // So we should pass indices as floats? Or bytes?
      // The struct says float4 c3_BoneIndexWeight.

      rv.boneIndexWeight.x = static_cast<float>(v.boneIndices[0]);
      rv.boneIndexWeight.y = static_cast<float>(v.boneIndices[1]);
      rv.boneIndexWeight.z = v.boneWeights[0] * 255.0f;
      rv.boneIndexWeight.w = v.boneWeights[1] * 255.0f;

      renderVertices.push_back(rv);

      // Debug: Log first vertex data
      if (renderVertices.size() == 1) {
        YAMEN_CORE_INFO("Vertex 0: Pos=[{}, {}, {}] Indices=[{}, {}] "
                        "Weights=[{}, {}] ShaderWeights=[{}, {}]",
                        v.position.x, v.position.y, v.position.z,
                        v.boneIndices[0], v.boneIndices[1], v.boneWeights[0],
                        v.boneWeights[1], rv.boneIndexWeight.z,
                        rv.boneIndexWeight.w);
      }
    }

    if (renderVertices.empty()) {
      return;
    }

    meshComp->vertexBuffer->Create(
        renderVertices.data(),
        static_cast<uint32_t>(renderVertices.size() * sizeof(RenderVertex)),
        sizeof(RenderVertex));
  }

  if (!meshComp->indexBuffer) {
    if (phy->indices.empty()) {
      return;
    }

    meshComp->indexBuffer = std::make_shared<Graphics::Buffer>(
        renderer.GetDevice(), Graphics::BufferType::Index);
    meshComp->indexBuffer->Create(
        phy->indices.data(),
        static_cast<uint32_t>(phy->indices.size() * sizeof(uint16_t)),
        sizeof(uint16_t));
  }

  // Bind buffers
  meshComp->vertexBuffer->Bind();
  meshComp->indexBuffer->Bind();

  // Draw
  renderer.GetDevice().GetContext()->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  renderer.GetDevice().GetContext()->DrawIndexed(
      static_cast<UINT>(phy->indices.size()), 0, 0);

  renderer.Unbind();
}

void C3ModelLoader::UnloadModel(entt::entity entity, entt::registry &registry) {
  // Components will be automatically destroyed by registry
  // C3MeshComponent destructor will delete the PHY data
  registry.destroy(entity);
}

} // namespace Yamen::Client

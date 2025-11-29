#include "Client/C3ModelLoader.h"
#include "AssetsC3/C3PhyLoader.h"
#include "Core/Logging/Logger.h"
#include "ECS/Components/SkeletalAnimationComponent.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/Texture/TextureLoader.h"
#include <vector>

namespace Yamen::Client {

using namespace Yamen::Core;

// Vertex format matching C3SkeletalRenderer input layout
struct RenderVertex {
  vec3 pos;      // Offset 0
  vec4 color;    // Offset 12
  vec2 uv;       // Offset 28
  vec4 boneData; // Offset 36 (x,y=indices, z,w=weights)
};

static_assert(sizeof(RenderVertex) == 52, "RenderVertex size mismatch");
static_assert(offsetof(RenderVertex, pos) == 0,
              "RenderVertex pos offset mismatch");
static_assert(offsetof(RenderVertex, color) == 12,
              "RenderVertex color offset mismatch");
static_assert(offsetof(RenderVertex, uv) == 28,
              "RenderVertex uv offset mismatch");
static_assert(offsetof(RenderVertex, boneData) == 36,
              "RenderVertex boneData offset mismatch");

entt::entity C3ModelLoader::LoadModel(entt::registry &registry,
                                      Graphics::GraphicsDevice &device,
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

  // Create Vertex Buffer
  // We must convert PhyVertex (52 bytes, has Normal) to RenderVertex (52 bytes,
  // has Color, different layout)
  std::vector<RenderVertex> renderVertices;
  renderVertices.reserve(phy->vertices.size());

  for (const auto &v : phy->vertices) {
    RenderVertex rv;
    rv.pos = v.position;
    rv.color = phy->color; // Use material color from PHY
    rv.uv = v.texCoord;

    // Pack bone data for shader: xy=indices, zw=weights(0-1)
    rv.boneData.x = (float)v.boneIndices[0];
    rv.boneData.y = (float)v.boneIndices[1];
    rv.boneData.z = v.boneWeights[0];
    rv.boneData.w = v.boneWeights[1];

    // Debug: Log first vertex bone data
    static bool logged = false;
    if (!logged && renderVertices.empty()) {
      YAMEN_CORE_INFO(
          "First Vertex BoneData: Indices=({}, {}), Weights=({}, {})",
          rv.boneData.x, rv.boneData.y, rv.boneData.z, rv.boneData.w);
      logged = true;
    }

    renderVertices.push_back(rv);
  }

  if (!renderVertices.empty()) {
    meshComp.vertexBuffer = std::make_shared<Graphics::Buffer>(
        device, Graphics::BufferType::Vertex);
    meshComp.vertexBuffer->Create(
        renderVertices.data(),
        (uint32_t)(renderVertices.size() * sizeof(RenderVertex)),
        sizeof(RenderVertex));
  }

  // Create Index Buffer
  if (!phy->indices.empty()) {
    meshComp.indexBuffer =
        std::make_shared<Graphics::Buffer>(device, Graphics::BufferType::Index);
    meshComp.indexBuffer->Create(
        phy->indices.data(), (uint32_t)(phy->indices.size() * sizeof(uint16_t)),
        sizeof(uint16_t));
    meshComp.indexCount = (uint32_t)phy->indices.size();
  }

  // Try to load texture (replace .c3 with .dds)
  std::string texturePath = filepath;
  size_t extPos = texturePath.find_last_of('.');
  if (extPos != std::string::npos) {
    texturePath.replace(extPos, 3, ".dds");
  }

  meshComp.texture = Graphics::TextureLoader::LoadFromFile(device, texturePath);
  if (!meshComp.texture) {
    // If failed, try loading from same directory as C3 file but with
    // textureName from PHY (Implementation omitted for brevity, fallback to
    // default white texture handled by renderer)
    YAMEN_CORE_WARN("Failed to load texture: {}", texturePath);
  }

  // Add Animation Component if motion exists
  if (phy->motion) {
    auto &animComp = registry.emplace<ECS::SkeletalAnimationComponent>(entity);
    animComp.motion = phy->motion;
    animComp.currentFrame = 0.0f;
    animComp.playbackSpeed = 30.0f; // 30 FPS default
    animComp.isPlaying = true;
    animComp.loop = true;

    // Initialize bone matrices
    if (phy->motion->boneCount > 0) {
      animComp.boneMatrices.resize(phy->motion->boneCount, mat4(1.0f));
    }

    // Copy Inverse Bind Matrices
    if (!phy->invBindMatrices.empty()) {
      animComp.inverseBindMatrices = phy->invBindMatrices;
    }
  }

  return entity;
}

void C3ModelLoader::RenderModel(entt::entity entity, entt::registry &registry,
                                Graphics::C3SkeletalRenderer &renderer,
                                const mat4 &modelViewProj) {
  if (!registry.valid(entity)) {
    // YAMEN_CORE_WARN("RenderModel: Invalid entity");
    return;
  }

  auto *mesh = registry.try_get<ECS::C3MeshComponent>(entity);
  if (!mesh) {
    // YAMEN_CORE_WARN("RenderModel: No mesh component");
    return;
  }
  if (!mesh->visible) {
    // YAMEN_CORE_WARN("RenderModel: Mesh not visible");
    return;
  }
  if (!mesh->vertexBuffer) {
    static int logCounter = 0;
    if (logCounter++ % 600 == 0) {
        YAMEN_CORE_WARN("RenderModel: No vertex buffer (Entity {}) - likely animation-only file", (uint32_t)entity);
    }
    return;
  }

  // Bind Texture
  if (mesh->texture) {
    renderer.SetTexture(mesh->texture.get());
  } else {
    renderer.SetTexture(nullptr); // Use default
  }

  // Set MVP
  // FIXED: Do NOT transpose here, Renderer handles it
  renderer.SetModelViewProj(modelViewProj);

  // Set Animation Data
  auto *anim = registry.try_get<ECS::SkeletalAnimationComponent>(entity);
  if (anim) {
    if (!anim->finalBoneMatrices.empty()) {
      renderer.SetBoneMatrices(anim->finalBoneMatrices.data(),
                               (uint32_t)anim->finalBoneMatrices.size());
    } else if (!anim->boneMatrices.empty()) {
      // Fallback to Global Transforms if Final not ready
      renderer.SetBoneMatrices(anim->boneMatrices.data(),
                               (uint32_t)anim->boneMatrices.size());
    }
  } else {
    // Reset bones to identity if no animation
    // (Or handle static mesh case)
  }

  // Set UV Animation
  if (mesh->phy) {
    renderer.SetUVAnimationOffset(mesh->phy->uvAnimStep *
                                  (anim ? anim->currentFrame : 0.0f));
  }

  // Bind and Draw
  renderer.Bind();

  mesh->vertexBuffer->Bind();
  auto context = renderer.GetDevice().GetContext();

  if (mesh->indexBuffer) {
    mesh->indexBuffer->Bind();
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(mesh->indexCount, 0, 0);
  } else {
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->Draw(mesh->vertexBuffer->GetCount(), 0);
  }
}

void C3ModelLoader::UnloadModel(entt::entity entity, entt::registry &registry) {
  if (registry.valid(entity)) {
    registry.destroy(entity);
  }
}

} // namespace Yamen::Client

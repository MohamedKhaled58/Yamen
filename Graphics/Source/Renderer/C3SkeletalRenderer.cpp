#include "Graphics/Renderer/C3SkeletalRenderer.h"
#include "Core/Logging/Logger.h"
#include "Graphics/Texture/TextureLoader.h"
#include <cstring>

namespace Yamen::Graphics {

C3SkeletalRenderer::C3SkeletalRenderer(GraphicsDevice &device)
    : m_Device(device), m_PerObjectData{}, m_BoneData{} {}

C3SkeletalRenderer::~C3SkeletalRenderer() = default;

bool C3SkeletalRenderer::Initialize() {
  // Create C3Skin shader
  m_Shader = std::make_unique<Shader>(m_Device);
  if (!m_Shader->CreateFromFiles(
          "C:/dev/C3Renderer/Yamen/Graphics/Shaders/C3Skin.hlsl",
          "C:/dev/C3Renderer/Yamen/Graphics/Shaders/C3Skin.hlsl", "VSMain",
          "PSMain")) {
    YAMEN_CORE_ERROR("Failed to create C3Skin shader");
    return false;
  }

  // Create per-object constant buffer (MVP + UV anim)
  m_PerObjectCB = std::make_unique<Buffer>(m_Device, BufferType::Constant);
  if (!m_PerObjectCB->Create(nullptr, sizeof(PerObjectConstants), 0,
                             BufferUsage::Dynamic)) {
    YAMEN_CORE_ERROR(
        "Failed to create C3SkeletalRenderer per-object constant buffer");
    return false;
  }

  // Create bone matrices constant buffer (210 vec4 = 70 bones * 3 vec4)
  m_BoneMatricesCB = std::make_unique<Buffer>(m_Device, BufferType::Constant);
  if (!m_BoneMatricesCB->Create(nullptr, sizeof(BoneMatricesConstants), 0,
                                BufferUsage::Dynamic)) {
    YAMEN_CORE_ERROR(
        "Failed to create C3SkeletalRenderer bone matrices constant buffer");
    return false;
  }

  // Initialize with identity matrices
  std::memset(&m_BoneData, 0, sizeof(BoneMatricesConstants));
  for (uint32_t i = 0; i < MAX_BONES; ++i) {
    uint32_t offset = i * 3;
    // Identity 3x4 matrix
    m_BoneData.c3_BoneMatrix[offset + 0] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    m_BoneData.c3_BoneMatrix[offset + 1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    m_BoneData.c3_BoneMatrix[offset + 2] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
  }



  m_PerObjectData.c3_ModelViewProj = glm::mat4(1.0f);
  m_PerObjectData.c3_UVAnimStep = glm::vec2(0.0f);

  // Create rasterizer state (Cull None)
  D3D11_RASTERIZER_DESC rasterDesc;
  ZeroMemory(&rasterDesc, sizeof(rasterDesc));
  rasterDesc.FillMode = D3D11_FILL_SOLID;
  rasterDesc.CullMode = D3D11_CULL_NONE; // Disable culling
  rasterDesc.FrontCounterClockwise = false;
  rasterDesc.DepthClipEnable = true;

  if (FAILED(m_Device.GetDevice()->CreateRasterizerState(&rasterDesc,
                                                         &m_RasterizerState))) {
    YAMEN_CORE_ERROR("Failed to create rasterizer state");
    return false;
  }

  // Create input layout
  m_InputLayout = std::make_unique<InputLayout>(m_Device);
  std::vector<InputElement> elements = {
      InputElement(InputSemantic::Position, InputFormat::Float3, 0, 0, 0),
      InputElement(InputSemantic::Color, InputFormat::Float4, 0, 0, 12),
      InputElement(InputSemantic::TexCoord, InputFormat::Float2, 0, 0, 28),
      InputElement(InputSemantic::TexCoord, InputFormat::Float4, 1, 0, 36)};

  if (!m_InputLayout->Create(elements,
                             m_Shader->GetVertexShaderBytecode().data(),
                             m_Shader->GetVertexShaderBytecode().size())) {
    YAMEN_CORE_ERROR("Failed to create C3SkeletalRenderer input layout");
    return false;
  }

  // Create default white texture
  m_DefaultTexture =
      TextureLoader::CreateSolidColor(m_Device, 1, 1, 255, 255, 255, 255);
  if (!m_DefaultTexture) {
    YAMEN_CORE_ERROR("Failed to create default texture");
    return false;
  }

  // Create sampler
  m_Sampler = std::make_unique<Sampler>(m_Device);
  if (!m_Sampler->Create(SamplerFilter::Point, SamplerAddressMode::Wrap, 1)) {
    YAMEN_CORE_ERROR("Failed to create sampler");
    return false;
  }

  YAMEN_CORE_INFO("C3SkeletalRenderer initialized successfully");
  return true;
}

void C3SkeletalRenderer::SetBoneMatrices(const glm::mat4 *matrices,
                                         uint32_t count) {
  if (count > MAX_BONES) {
    count = MAX_BONES;
  }

  // Debug: Log first 5 frames
  static int logCount = 0;
  if (logCount < 5 && count > 0) {
      const glm::mat4& m = matrices[0];
      YAMEN_CORE_INFO("Frame {} - Bone[0]: translation=({:.2f}, {:.2f}, {:.2f})",
          logCount, m[3][0], m[3][1], m[3][2]);
      logCount++;
  }

  // Debug: Log first bone matrix
  static bool logged = false;
  if (!logged && count > 0) {
    const glm::mat4 &m = matrices[0];
    YAMEN_CORE_INFO("Bone[0] Matrix sent to shader:");
    YAMEN_CORE_INFO("  [{:.2f}, {:.2f}, {:.2f}, {:.2f}]", m[0][0], m[0][1],
                    m[0][2], m[0][3]);
    YAMEN_CORE_INFO("  [{:.2f}, {:.2f}, {:.2f}, {:.2f}]", m[1][0], m[1][1],
                    m[1][2], m[1][3]);
    YAMEN_CORE_INFO("  [{:.2f}, {:.2f}, {:.2f}, {:.2f}]", m[2][0], m[2][1],
                    m[2][2], m[2][3]);
    YAMEN_CORE_INFO("  [{:.2f}, {:.2f}, {:.2f}, {:.2f}]", m[3][0], m[3][1],
                    m[3][2], m[3][3]);
    logged = true;
  }

  for (uint32_t i = 0; i < count; ++i) {
    // GLM stores column-major: m[col][row]
    // Shader expects 3 vec4s as rows: [row0, row1, row2]
    // Each row is [x, y, z, translation_component]
    const glm::mat4 &m = matrices[i];

    // Row 0: [m00, m10, m20, m30] (first row with translation.x)
    m_BoneData.c3_BoneMatrix[i * 3 + 0] =
        glm::vec4(m[0][0], m[1][0], m[2][0], m[3][0]);
    // Row 1: [m01, m11, m21, m31] (second row with translation.y)
    m_BoneData.c3_BoneMatrix[i * 3 + 1] =
        glm::vec4(m[0][1], m[1][1], m[2][1], m[3][1]);
    // Row 2: [m02, m12, m22, m32] (third row with translation.z)
    m_BoneData.c3_BoneMatrix[i * 3 + 2] =
        glm::vec4(m[0][2], m[1][2], m[2][2], m[3][2]);
  }
}

void C3SkeletalRenderer::SetUVAnimationOffset(const glm::vec2 &offset) {
  m_PerObjectData.c3_UVAnimStep = offset;
}

void C3SkeletalRenderer::SetModelViewProj(const glm::mat4 &mvp) {
  m_PerObjectData.c3_ModelViewProj = mvp;
}

void C3SkeletalRenderer::SetTexture(Texture2D *texture) {
  m_CustomTexture = texture;
}

void C3SkeletalRenderer::Bind() {
  // Safety check - ensure initialization succeeded
  if (!m_PerObjectCB || !m_BoneMatricesCB || !m_Shader || !m_InputLayout) {
    YAMEN_CORE_ERROR(
        "C3SkeletalRenderer::Bind() called but not properly initialized!");
    if (!m_PerObjectCB)
      YAMEN_CORE_ERROR("  m_PerObjectCB is NULL");
    if (!m_BoneMatricesCB)
      YAMEN_CORE_ERROR("  m_BoneMatricesCB is NULL");
    return;
  }

  // Update constant buffers
  m_PerObjectCB->Update(&m_PerObjectData, sizeof(PerObjectConstants));
  m_BoneMatricesCB->Update(&m_BoneData, sizeof(BoneMatricesConstants));

  // Bind shader
  m_Shader->Bind();

  // Bind input layout
  m_InputLayout->Bind();

  // Bind texture (use custom if set, otherwise default) and sampler
  if (m_CustomTexture) {
    m_CustomTexture->Bind(0); // t0
  } else if (m_DefaultTexture) {
    m_DefaultTexture->Bind(0); // t0
  }
  if (m_Sampler) {
    m_Sampler->Bind(0); // s0
  }

  // Bind constant buffers to vertex shader
  m_PerObjectCB->BindToVertexShader(0); // b0

  if (m_BoneMatricesCB) {
    m_BoneMatricesCB->BindToVertexShader(1); // b1
  } else {
    YAMEN_CORE_ERROR("C3SkeletalRenderer::Bind - m_BoneMatricesCB is NULL!");
  }

  // Set rasterizer state
  if (m_RasterizerState) {
    m_Device.GetContext()->RSSetState(m_RasterizerState);
  }
}

void C3SkeletalRenderer::Unbind() { m_Shader->Unbind(); }

} // namespace Yamen::Graphics

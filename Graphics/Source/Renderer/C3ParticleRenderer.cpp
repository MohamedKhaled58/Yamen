#include "Graphics/Renderer/C3ParticleRenderer.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

C3ParticleRenderer::C3ParticleRenderer(GraphicsDevice &device)
    : m_Device(device), m_CurrentShaderType(ParticleShaderType::Ptcl1),
      m_MaxParticles(0), m_InBatch(false) {}

C3ParticleRenderer::~C3ParticleRenderer() = default;

bool C3ParticleRenderer::Initialize() {
  // Create C3Ptcl1 shader
  m_ShaderPtcl1 = std::make_unique<Shader>(m_Device);
  if (!m_ShaderPtcl1->CreateFromFiles("Graphics/Shaders/C3Ptcl1.hlsl",
                                      "Graphics/Shaders/C3Ptcl1.hlsl", "VSMain",
                                      "PSMain")) {
    YAMEN_CORE_ERROR("Failed to create C3Ptcl1 shader");
    return false;
  }

  // Create C3Ptcl3 shader
  m_ShaderPtcl3 = std::make_unique<Shader>(m_Device);
  if (!m_ShaderPtcl3->CreateFromFiles("Graphics/Shaders/C3Ptcl3.hlsl",
                                      "Graphics/Shaders/C3Ptcl3.hlsl", "VSMain",
                                      "PSMain")) {
    YAMEN_CORE_ERROR("Failed to create C3Ptcl3 shader");
    return false;
  }

  // Create constant buffer
  m_ConstantBuffer = std::make_unique<Buffer>(m_Device, BufferType::Constant);
  if (!m_ConstantBuffer->Create(nullptr, sizeof(ConstantsMVP), 0,
                                BufferUsage::Dynamic)) {
    YAMEN_CORE_ERROR("Failed to create C3ParticleRenderer constant buffer");
    return false;
  }

  m_Constants.c3_ModelViewProj = glm::mat4(1.0f);

  YAMEN_CORE_INFO("C3ParticleRenderer initialized successfully");
  return true;
}

void C3ParticleRenderer::SetShaderType(ParticleShaderType type) {
  m_CurrentShaderType = type;
}

void C3ParticleRenderer::SetProjection(const glm::mat4 &proj) {
  m_Constants.c3_ModelViewProj = proj;
}

void C3ParticleRenderer::SetModelViewProj(const glm::mat4 &mvp) {
  m_Constants.c3_ModelViewProj = mvp;
}

void C3ParticleRenderer::Begin(uint32_t maxParticles) {
  if (m_InBatch) {
    YAMEN_CORE_WARN("C3ParticleRenderer::Begin called while already in batch");
    return;
  }

  m_InBatch = true;
  m_MaxParticles = maxParticles;
  m_Particles.clear();
  m_Particles.reserve(maxParticles);

  // Create or resize vertex buffer if needed
  uint32_t requiredSize = maxParticles * sizeof(VertexParticle);
  if (!m_VertexBuffer || m_VertexBuffer->GetSize() < requiredSize) {
    m_VertexBuffer = std::make_unique<Buffer>(m_Device, BufferType::Vertex);
    if (!m_VertexBuffer->Create(nullptr, requiredSize, sizeof(VertexParticle),
                                BufferUsage::Dynamic)) {
      YAMEN_CORE_ERROR("Failed to create particle vertex buffer");
      m_InBatch = false;
      return;
    }
  }
}

void C3ParticleRenderer::AddParticle(const VertexParticle &particle) {
  if (!m_InBatch) {
    YAMEN_CORE_WARN("C3ParticleRenderer::AddParticle called outside of Begin/End");
    return;
  }

  if (m_Particles.size() >= m_MaxParticles) {
    YAMEN_CORE_WARN("C3ParticleRenderer: Max particles ({}) reached",
                m_MaxParticles);
    return;
  }

  m_Particles.push_back(particle);
}

void C3ParticleRenderer::End() {
  if (!m_InBatch) {
    YAMEN_CORE_WARN("C3ParticleRenderer::End called without Begin");
    return;
  }

  m_InBatch = false;

  if (m_Particles.empty())
    return;

  // Update vertex buffer
  m_VertexBuffer->Update(
      m_Particles.data(),
      static_cast<uint32_t>(m_Particles.size() * sizeof(VertexParticle)));

  // Update constant buffer
  m_ConstantBuffer->Update(&m_Constants, sizeof(ConstantsMVP));

  // Bind shader
  Shader *shader = GetShader();
  if (shader) {
    shader->Bind();
    m_ConstantBuffer->BindToVertexShader(0);

    // TODO: Bind vertex buffer and draw
    // This requires integration with the rendering system's draw call mechanism
    // For now, the setup is complete and ready for rendering

    shader->Unbind();
  }
}

Shader *C3ParticleRenderer::GetShader() const {
  switch (m_CurrentShaderType) {
  case ParticleShaderType::Ptcl1:
    return m_ShaderPtcl1.get();
  case ParticleShaderType::Ptcl3:
    return m_ShaderPtcl3.get();
  default:
    return nullptr;
  }
}

} // namespace Yamen::Graphics

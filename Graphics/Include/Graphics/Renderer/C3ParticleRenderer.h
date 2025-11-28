#pragma once

#include "Graphics/Mesh/Vertex.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/Shader/Shader.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>


namespace Yamen::Graphics {

/**
 * @brief Particle renderer for C3 particle systems
 *
 * Supports both C3Ptcl1 (simple, no color) and C3Ptcl3 (with color modulation).
 */
class C3ParticleRenderer {
public:
  enum class ParticleShaderType {
    Ptcl1, // Simple particles (projection only, no color)
    Ptcl3  // Advanced particles (MVP + color modulation)
  };

  C3ParticleRenderer(GraphicsDevice &device);
  ~C3ParticleRenderer();

  // Non-copyable
  C3ParticleRenderer(const C3ParticleRenderer &) = delete;
  C3ParticleRenderer &operator=(const C3ParticleRenderer &) = delete;

  /**
   * @brief Initialize the renderer
   * @return True if initialization succeeded
   */
  bool Initialize();

  /**
   * @brief Set shader type to use
   */
  void SetShaderType(ParticleShaderType type);

  /**
   * @brief Set projection matrix (for Ptcl1)
   */
  void SetProjection(const glm::mat4 &proj);

  /**
   * @brief Set model-view-projection matrix (for Ptcl3)
   */
  void SetModelViewProj(const glm::mat4 &mvp);

  /**
   * @brief Begin particle batch
   * @param maxParticles Maximum number of particles to render
   */
  void Begin(uint32_t maxParticles = 1000);

  /**
   * @brief Add a particle to the batch
   * @param particle Particle vertex data
   */
  void AddParticle(const VertexParticle &particle);

  /**
   * @brief End batch and render all particles
   */
  void End();

  /**
   * @brief Get current shader
   */
  Shader *GetShader() const;

private:
  struct ConstantsProj {
    glm::mat4 c3_Proj;
  };

  struct ConstantsMVP {
    glm::mat4 c3_ModelViewProj;
  };

  GraphicsDevice &m_Device;
  std::unique_ptr<Shader> m_ShaderPtcl1;
  std::unique_ptr<Shader> m_ShaderPtcl3;
  std::unique_ptr<Buffer> m_VertexBuffer;
  std::unique_ptr<Buffer> m_ConstantBuffer;

  ParticleShaderType m_CurrentShaderType;
  std::vector<VertexParticle> m_Particles;
  ConstantsMVP m_Constants; // Can hold both Proj and MVP (same layout)
  uint32_t m_MaxParticles;
  bool m_InBatch;
};

} // namespace Yamen::Graphics

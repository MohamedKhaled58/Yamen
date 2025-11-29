#include "Graphics/Material/Material.h"
#include "Graphics/RHI/BlendState.h"
#include "Graphics/RHI/DepthStencilState.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/RHI/RasterizerState.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Texture/Texture2D.h"


namespace Yamen::Graphics {

using namespace Yamen::Core;

void Material::SetTexture(const std::string &name, Texture2D *texture) {
  m_Textures[name] = texture;
}

Texture2D *Material::GetTexture(const std::string &name) const {
  auto it = m_Textures.find(name);
  return (it != m_Textures.end()) ? it->second : nullptr;
}

bool Material::HasTexture(const std::string &name) const {
  return m_Textures.find(name) != m_Textures.end();
}

void Material::SetFloat(const std::string &name, float value) {
  m_Floats[name] = value;
}

float Material::GetFloat(const std::string &name, float defaultValue) const {
  auto it = m_Floats.find(name);
  return (it != m_Floats.end()) ? it->second : defaultValue;
}

void Material::SetVector(const std::string &name, const vec4 &value) {
  m_Vectors[name] = value;
}

vec4 Material::GetVector(const std::string &name,
                         const vec4 &defaultValue) const {
  auto it = m_Vectors.find(name);
  return (it != m_Vectors.end()) ? it->second : defaultValue;
}

void Material::SetMatrix(const std::string &name, const mat4 &value) {
  m_Matrices[name] = value;
}

mat4 Material::GetMatrix(const std::string &name,
                         const mat4 &defaultValue) const {
  auto it = m_Matrices.find(name);
  return (it != m_Matrices.end()) ? it->second : defaultValue;
}

void Material::Bind(GraphicsDevice &device) {
  // Bind shader
  if (m_Shader) {
    m_Shader->Bind();
  }

  // Bind textures (t0, t1, t2, ...)
  uint32_t textureSlot = 0;
  for (const auto &[name, texture] : m_Textures) {
    if (texture) {
      texture->Bind(textureSlot++);
    }
  }

  // Bind render states
  if (m_BlendState) {
    m_BlendState->Bind(nullptr,
                       0xFFFFFFFF); // Default blend factor and sample mask
  }

  if (m_DepthState) {
    m_DepthState->Bind(0); // Default stencil ref
  }

  if (m_RasterizerState) {
    m_RasterizerState->Bind();
  }

  // Note: Constant buffer updates for float/vector/matrix properties
  // should be handled by the renderer based on shader requirements
  (void)device; // Unused for now
}

} // namespace Yamen::Graphics

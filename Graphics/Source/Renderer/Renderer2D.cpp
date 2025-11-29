#include "Graphics/Renderer/Renderer2D.h"
#include "Graphics/Texture/TextureLoader.h"
#include <Core/Logging/Logger.h>

namespace Yamen::Graphics {

using namespace Yamen::Core;

Renderer2D::Renderer2D(GraphicsDevice &device)
    : m_Device(device), m_CurrentCamera(nullptr), m_InScene(false) {}

Renderer2D::~Renderer2D() {}

bool Renderer2D::Initialize() {
  // Create sprite batch
  m_SpriteBatch = std::make_unique<SpriteBatch>(m_Device, 10000);
  if (!m_SpriteBatch->Initialize()) {
    YAMEN_CORE_ERROR("Failed to initialize SpriteBatch");
    return false;
  }

  // Create blend state (alpha blending)
  m_BlendState = std::make_unique<BlendState>(m_Device);
  if (!m_BlendState->Create(BlendMode::AlphaBlend)) {
    YAMEN_CORE_ERROR("Failed to create blend state");
    return false;
  }

  // Create sampler
  m_Sampler = std::make_unique<Sampler>(m_Device);
  if (!m_Sampler->Create(SamplerFilter::Linear, SamplerAddressMode::Clamp)) {
    YAMEN_CORE_ERROR("Failed to create sampler");
    return false;
  }

  // Create white texture for solid color rendering
  m_WhiteTexture =
      TextureLoader::CreateSolidColor(m_Device, 1, 1, 255, 255, 255, 255);
  if (!m_WhiteTexture) {
    YAMEN_CORE_ERROR("Failed to create white texture");
    return false;
  }

  // Load Sprite2D shader (used by SpriteBatch)
  // The SpriteBatch has its own shader instance

  YAMEN_CORE_INFO("Renderer2D initialized");
  return true;
}

void Renderer2D::BeginScene(Camera2D *camera) {
  if (m_InScene) {
    YAMEN_CORE_WARN("Renderer2D::BeginScene called while already in scene");
    return;
  }

  m_CurrentCamera = camera;
  m_InScene = true;

  // Set render states
  m_BlendState->Bind();
  m_Sampler->Bind(0);

  // Begin sprite batch
  m_SpriteBatch->Begin(camera);
}

void Renderer2D::EndScene() {
  if (!m_InScene) {
    YAMEN_CORE_WARN("Renderer2D::EndScene called without BeginScene");
    return;
  }

  // End sprite batch (flushes)
  m_SpriteBatch->End();

  m_InScene = false;
  m_CurrentCamera = nullptr;
}

void Renderer2D::DrawSprite(Texture2D *texture, const vec2 &position,
                            const vec2 &size, float rotation, const vec4 &color,
                            const vec2 &origin) {
  if (!m_InScene) {
    YAMEN_CORE_WARN(
        "Renderer2D::DrawSprite called outside BeginScene/EndScene");
    return;
  }

  m_SpriteBatch->DrawSprite(texture, position, size, rotation, color, origin);
}

void Renderer2D::DrawQuad(const vec2 &position, const vec2 &size,
                          const vec4 &color, float rotation) {
  if (!m_InScene) {
    YAMEN_CORE_WARN("Renderer2D::DrawQuad called outside BeginScene/EndScene");
    return;
  }

  // Use white texture with color tint
  m_SpriteBatch->DrawSprite(m_WhiteTexture.get(), position, size, rotation,
                            color, vec2(0.0f));
}

void Renderer2D::SetBlendMode(BlendMode mode) {
  if (!m_BlendState->Create(mode)) {
    YAMEN_CORE_ERROR("Failed to set blend mode");
    return;
  }
  m_BlendState->Bind();
}

} // namespace Yamen::Graphics

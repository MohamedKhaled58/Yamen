#pragma once

#include "Graphics/RHI/BlendState.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/RHI/Sampler.h"
#include "Graphics/Renderer/Camera2D.h"
#include "Graphics/Renderer/SpriteBatch.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Texture/Texture2D.h"
#include <Core/Math/Math.h>
#include <memory>


namespace Yamen::Graphics {

using namespace Yamen::Core;

/**
 * @brief High-level 2D rendering API
 */
class Renderer2D {
public:
  Renderer2D(GraphicsDevice &device);
  ~Renderer2D();

  // Non-copyable
  Renderer2D(const Renderer2D &) = delete;
  Renderer2D &operator=(const Renderer2D &) = delete;

  /**
   * @brief Initialize renderer
   */
  bool Initialize();

  /**
   * @brief Begin 2D scene
   */
  void BeginScene(Camera2D *camera);

  /**
   * @brief End 2D scene
   */
  void EndScene();

  /**
   * @brief Draw sprite
   */
  void DrawSprite(Texture2D *texture, const vec2 &position,
                  const vec2 &size = vec2(1.0f), float rotation = 0.0f,
                  const vec4 &color = vec4(1.0f),
                  const vec2 &origin = vec2(0.0f));

  /**
   * @brief Draw quad (no texture)
   */
  void DrawQuad(const vec2 &position, const vec2 &size, const vec4 &color,
                float rotation = 0.0f);

  /**
   * @brief Set blend mode
   */
  void SetBlendMode(BlendMode mode);

private:
  GraphicsDevice &m_Device;
  std::unique_ptr<SpriteBatch> m_SpriteBatch;
  std::unique_ptr<Shader> m_Shader;
  std::unique_ptr<BlendState> m_BlendState;
  std::unique_ptr<Sampler> m_Sampler;
  std::unique_ptr<Texture2D> m_WhiteTexture;
  Camera2D *m_CurrentCamera;
  bool m_InScene;
};

} // namespace Yamen::Graphics

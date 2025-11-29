#pragma once

#include "Graphics/Lighting/Light.h"
#include "Graphics/Lighting/ShadowMap.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/RHI/BlendState.h"
#include "Graphics/RHI/DepthStencilState.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/RHI/InputLayout.h"
#include "Graphics/RHI/RasterizerState.h"
#include "Graphics/RHI/Sampler.h"
#include "Graphics/Renderer/Camera3D.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Texture/Texture2D.h"
#include <Core/Math/Math.h>
#include <memory>
#include <vector>


namespace Yamen::Graphics {

using namespace Yamen::Core;

/**
 * @brief High-level 3D rendering API
 */
class Renderer3D {
public:
  Renderer3D(GraphicsDevice &device);
  ~Renderer3D();

  // Non-copyable
  Renderer3D(const Renderer3D &) = delete;
  Renderer3D &operator=(const Renderer3D &) = delete;

  /**
   * @brief Initialize renderer
   */
  bool Initialize();

  /**
   * @brief Begin 3D scene
   */
  void BeginScene(Camera3D *camera);

  /**
   * @brief End 3D scene
   */
  void EndScene();

  /**
   * @brief Begin shadow pass
   */
  void BeginShadowPass(ShadowMap *shadowMap, Light *light);

  /**
   * @brief End shadow pass
   */
  void EndShadowPass();

  /**
   * @brief Submit light to scene
   */
  void SubmitLight(const Light &light);

  /**
   * @brief Draw mesh
   */
  void DrawMesh(Mesh *mesh, const mat4 &transform, Texture2D *texture = nullptr,
                const vec4 &color = vec4(1.0f));

  /**
   * @brief Draw mesh with material
   */
  void DrawMesh(Mesh *mesh, const mat4 &transform, Material *material);

  /**
   * @brief Draw mesh with sub-meshes (multi-material)
   */
  void DrawMeshWithSubMeshes(Mesh *mesh, const mat4 &transform);

  /**
   * @brief Set wireframe mode
   */
  void SetWireframe(bool enabled);

private:
  void SetupLighting();

  GraphicsDevice &m_Device;
  std::unique_ptr<Shader> m_Shader;
  std::unique_ptr<RasterizerState> m_RasterizerState;
  std::unique_ptr<RasterizerState> m_WireframeState;
  std::unique_ptr<DepthStencilState> m_DepthState;
  std::unique_ptr<BlendState> m_BlendState;
  std::unique_ptr<Sampler> m_Sampler;
  std::unique_ptr<Texture2D> m_WhiteTexture;
  std::unique_ptr<InputLayout> m_InputLayout; // Vertex3D input layout

  // Constant buffers
  std::unique_ptr<Buffer> m_PerFrameCB;  // Camera + view/projection
  std::unique_ptr<Buffer> m_PerObjectCB; // World matrix + material color
  std::unique_ptr<Buffer> m_LightingCB;  // Lighting data

  Camera3D *m_CurrentCamera;
  std::vector<Light> m_Lights;
  bool m_InScene;
  bool m_InShadowPass;
  bool m_Wireframe;

  ShadowMap *m_CurrentShadowMap;
  Light *m_CurrentShadowLight;
  std::unique_ptr<Shader> m_ShadowShader;
};

} // namespace Yamen::Graphics

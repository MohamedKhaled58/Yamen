#pragma once

#include "Graphics/Mesh/Mesh.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/RHI/InputLayout.h"
#include "Graphics/RHI/Sampler.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Texture/Texture2D.h"
#include <Core/Math/Math.h>
#include <memory>
#include <vector>

namespace Yamen::Graphics {

using namespace Yamen::Core;

/**
 * @brief Renderer for C3 skeletal animated meshes
 *
 * Implements the C3 skinning system with dual bone blending.
 * Supports up to 70 bones with 2 influences per vertex.
 */
class C3SkeletalRenderer {
public:
  static constexpr uint32_t MAX_BONES = 200;
  static constexpr uint32_t BONE_MATRIX_VEC4_COUNT =
      MAX_BONES * 3; // 3 vec4 per bone = 3x4 matrix

  C3SkeletalRenderer(GraphicsDevice &device);
  ~C3SkeletalRenderer();

  // Non-copyable
  C3SkeletalRenderer(const C3SkeletalRenderer &) = delete;
  C3SkeletalRenderer &operator=(const C3SkeletalRenderer &) = delete;

  /**
   * @brief Initialize the renderer
   * @return True if initialization succeeded
   */
  bool Initialize();

  /**
   * @brief Set bone transformation matrices from standard 4x4 matrices
   * @param matrices Array of 4x4 matrices
   * @param count Number of matrices (max 70)
   *
   * Converts standard 4x4 matrices to C3 format (3 vec4 per matrix)
   */
  void SetBoneMatrices(const mat4 *matrices, uint32_t count);

  /**
   * @brief Set UV animation offset
   * @param offset UV offset to add to texture coordinates
   */
  void SetUVAnimationOffset(const vec2 &offset);

  /**
   * @brief Set model-view-projection matrix
   * @param mvp Combined model-view-projection matrix
   */
  void SetModelViewProj(const mat4 &mvp);

  /**
   * @brief Set custom texture (overrides default white texture)
   * @param texture Texture to bind for rendering
   */
  void SetTexture(Texture2D *texture);

  /**
   * @brief Bind renderer state (shaders and constant buffers)
   */
  void Bind();

  /**
   * @brief Unbind renderer state
   */
  void Unbind();

  /**
   * @brief Get the C3Skin shader
   */
  Shader *GetShader() const { return m_Shader.get(); }

  /**
   * @brief Get the graphics device
   */
  GraphicsDevice &GetDevice() const { return m_Device; }

private:
  struct PerObjectConstants {
    mat4 c3_ModelViewProj;
    vec2 c3_UVAnimStep;
    vec2 _padding;
  };

  struct BoneMatricesConstants {
    vec4 c3_BoneMatrix[BONE_MATRIX_VEC4_COUNT]; // 70 bones * 3 vec4 = 210 vec4
  };

  GraphicsDevice &m_Device;
  std::unique_ptr<Shader> m_Shader;
  std::unique_ptr<Buffer> m_PerObjectCB;
  std::unique_ptr<Buffer> m_BoneMatricesCB;
  std::unique_ptr<InputLayout> m_InputLayout;
  std::unique_ptr<Texture2D> m_DefaultTexture;
  std::unique_ptr<Sampler> m_Sampler;
  Texture2D *m_CustomTexture = nullptr; // Optional custom texture

  // Rasterizer state for disabling culling
  struct ID3D11RasterizerState *m_RasterizerState = nullptr;

  PerObjectConstants m_PerObjectData;
  BoneMatricesConstants m_BoneData;
};

} // namespace Yamen::Graphics

#include "Graphics/Renderer/Renderer3D.h"
#include "Graphics/Material/Material.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/InputLayout.h"
#include "Graphics/RHI/Sampler.h"
#include "Graphics/Texture/TextureLoader.h"
#include <Core/Logging/Logger.h>

namespace Yamen::Graphics {

using namespace Yamen::Core;

Renderer3D::Renderer3D(GraphicsDevice &device)
    : m_Device(device), m_CurrentCamera(nullptr), m_InScene(false),
      m_InShadowPass(false), m_Wireframe(false), m_CurrentShadowMap(nullptr),
      m_CurrentShadowLight(nullptr) {}

Renderer3D::~Renderer3D() {}

bool Renderer3D::Initialize() {
  // Create rasterizer states
  m_RasterizerState = std::make_unique<RasterizerState>(m_Device);
  if (!m_RasterizerState->Create(CullMode::Back, FillMode::Solid, false,
                                 false)) {
    YAMEN_CORE_ERROR("Failed to create rasterizer state");
    return false;
  }

  m_WireframeState = std::make_unique<RasterizerState>(m_Device);
  if (!m_WireframeState->Create(CullMode::Back, FillMode::Wireframe, false,
                                false)) {
    YAMEN_CORE_ERROR("Failed to create wireframe rasterizer state");
    return false;
  }

  // Create depth-stencil state
  m_DepthState = std::make_unique<DepthStencilState>(m_Device);
  if (!m_DepthState->Create(true, true, D3D11_COMPARISON_LESS)) {
    YAMEN_CORE_ERROR("Failed to create depth-stencil state");
    return false;
  }

  // Create blend state
  m_BlendState = std::make_unique<BlendState>(m_Device);
  if (!m_BlendState->Create(BlendMode::Opaque, false)) {
    YAMEN_CORE_ERROR("Failed to create blend state");
    return false;
  }

  // Create sampler
  m_Sampler = std::make_unique<Sampler>(m_Device);
  if (!m_Sampler->Create(SamplerFilter::Anisotropic, SamplerAddressMode::Wrap,
                         16)) {
    YAMEN_CORE_ERROR("Failed to create sampler");
    return false;
  }

  // Create white texture for untextured meshes
  m_WhiteTexture =
      TextureLoader::CreateSolidColor(m_Device, 1, 1, 255, 255, 255, 255);
  if (!m_WhiteTexture) {
    YAMEN_CORE_ERROR("Failed to create white texture");
    return false;
  }

  // Load Basic3D shader
  m_Shader = std::make_unique<Shader>(m_Device);
  if (!m_Shader->CreateFromFiles(
          "C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl",
          "C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl", "VSMain",
          "PSMain")) {
    YAMEN_CORE_ERROR("Failed to load Basic3D shader");
    return false;
  }

  // Load Shadow shader (reuse Basic3D VS for now, or a dedicated one)
  // For simplicity, we'll assume a "ShadowMap.hlsl" exists or use a null pixel
  // shader technique But since we don't have ShadowMap.hlsl yet, we'll skip
  // creating it here and assume the user will provide it or we use a simple
  // depth-only pass
  // TODO: Create ShadowMap.hlsl

  // Create constant buffers
  m_PerFrameCB = std::make_unique<Buffer>(m_Device, BufferType::Constant);
  if (!m_PerFrameCB->Create(nullptr, 80, 0, BufferUsage::Dynamic)) {
    YAMEN_CORE_ERROR("Failed to create PerFrame constant buffer");
    return false;
  }

  m_PerObjectCB = std::make_unique<Buffer>(m_Device, BufferType::Constant);
  if (!m_PerObjectCB->Create(nullptr, 80, 0, BufferUsage::Dynamic)) {
    YAMEN_CORE_ERROR("Failed to create PerObject constant buffer");
    return false;
  }

  m_LightingCB = std::make_unique<Buffer>(m_Device, BufferType::Constant);
  if (!m_LightingCB->Create(nullptr, 64, 0, BufferUsage::Dynamic)) {
    YAMEN_CORE_ERROR("Failed to create Lighting constant buffer");
    return false;
  }

  // Create input layout for Vertex3D
  m_InputLayout = std::make_unique<InputLayout>(m_Device);
  std::vector<InputElement> elements = {
      InputElement(InputSemantic::Position, InputFormat::Float3, 0, 0, 0),
      InputElement(InputSemantic::Normal, InputFormat::Float3, 0, 0, 12),
      InputElement(InputSemantic::TexCoord, InputFormat::Float2, 0, 0, 24)};
  if (!m_InputLayout->Create(elements,
                             m_Shader->GetVertexShaderBytecode().data(),
                             m_Shader->GetVertexShaderBytecode().size())) {
    YAMEN_CORE_ERROR("Failed to create Renderer3D input layout");
    return false;
  }

  YAMEN_CORE_INFO("Renderer3D initialized");
  return true;
}

void Renderer3D::BeginScene(Camera3D *camera) {
  if (m_InScene) {
    YAMEN_CORE_WARN("Renderer3D::BeginScene called while already in scene");
    return;
  }

  m_CurrentCamera = camera;
  m_InScene = true;
  m_Lights.clear();

  // Set render states
  if (m_Wireframe) {
    m_WireframeState->Bind();
  } else {
    m_RasterizerState->Bind();
  }
  m_DepthState->Bind();
  m_BlendState->Bind();
  m_Sampler->Bind(0);
}

void Renderer3D::EndScene() {
  if (!m_InScene) {
    YAMEN_CORE_WARN("Renderer3D::EndScene called without BeginScene");
    return;
  }

  m_InScene = false;
  m_CurrentCamera = nullptr;
}

void Renderer3D::BeginShadowPass(ShadowMap *shadowMap, Light *light) {
  if (m_InScene) {
    YAMEN_CORE_WARN("Cannot begin shadow pass while in scene");
    return;
  }

  m_InShadowPass = true;
  m_CurrentShadowMap = shadowMap;
  m_CurrentShadowLight = light;

  // Bind shadow map DSV
  m_CurrentShadowMap->BindDSV();
  m_CurrentShadowMap->Clear();

  // Set rasterizer (front-face culling for shadows to avoid peter-panning)
  // m_RasterizerState->Bind(); // Or a specific shadow rasterizer state
}

void Renderer3D::EndShadowPass() {
  m_InShadowPass = false;
  m_CurrentShadowMap = nullptr;
  m_CurrentShadowLight = nullptr;

  // Restore default render target (back buffer) - will be handled by next
  // BeginScene Note: SwapChain manages the back buffer, not GraphicsDevice
  // ID3D11RenderTargetView* rtv = m_Device.GetBackBufferRTV();
  // ID3D11DepthStencilView* dsv = m_Device.GetDepthStencilView();
  // m_Device.GetContext()->OMSetRenderTargets(1, &rtv, dsv);

  // Restore viewport - will be set by next BeginScene
  D3D11_VIEWPORT viewport;
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = 1280; // TODO: Get from window
  viewport.Height = 720;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  m_Device.GetContext()->RSSetViewports(1, &viewport);
}

void Renderer3D::SubmitLight(const Light &light) {
  if (!m_InScene) {
    YAMEN_CORE_WARN(
        "Renderer3D::SubmitLight called outside BeginScene/EndScene");
    return;
  }

  m_Lights.push_back(light);
}

void Renderer3D::DrawMesh(Mesh *mesh, const mat4 &transform, Texture2D *texture,
                          const vec4 &color) {
  if (!m_InScene && !m_InShadowPass) {
    YAMEN_CORE_WARN("Renderer3D::DrawMesh called outside BeginScene/EndScene "
                    "or ShadowPass");
    return;
  }

  if (!mesh) {
    YAMEN_CORE_WARN("Renderer3D::DrawMesh called with null mesh");
    return;
  }

  // Bind texture or white texture
  if (texture) {
    texture->Bind(0);
  } else {
    m_WhiteTexture->Bind(0);
  }

  // Bind input layout
  m_InputLayout->Bind();

  // Bind shader
  if (m_InShadowPass) {
    // Use shadow shader (vertex only usually)
    // For now, we'll reuse the main shader but we should really have a
    // dedicated one
    m_Shader->Bind();
    // TODO: Unbind pixel shader for shadow pass
    m_Device.GetContext()->PSSetShader(nullptr, nullptr, 0);
  } else {
    m_Shader->Bind();
  }

  // Update PerFrame constant buffer (b0)
  struct PerFrameData {
    mat4 ViewProjection;
    vec3 CameraPosition;
    float _pad0;
  } perFrameData;

  perFrameData.ViewProjection = Math::Transpose(m_CurrentCamera->GetViewProjectionMatrix());
  perFrameData.CameraPosition = m_CurrentCamera->GetPosition();
  perFrameData._pad0 = 0.0f;

  m_PerFrameCB->Update(&perFrameData, sizeof(PerFrameData));
  m_PerFrameCB->BindToVertexShader(0);

  if (!m_InShadowPass) {
    m_PerFrameCB->BindToPixelShader(0);
  }

  // Update PerObject constant buffer (b1)
  struct PerObjectData {
    mat4 World;
    vec4 MaterialColor;
  } perObjectData;

  perObjectData.World = Math::Transpose(transform);
  perObjectData.MaterialColor = color;

  m_PerObjectCB->Update(&perObjectData, sizeof(PerObjectData));
  m_PerObjectCB->BindToVertexShader(1);

  if (!m_InShadowPass) {
    m_PerObjectCB->BindToPixelShader(1);
  }

  // Update Lighting constant buffer (b2)
  struct LightingData {
    vec3 LightDirection;
    float _pad1;
    vec3 LightColor;
    float LightIntensity;
    vec3 AmbientColor = {};
    float _pad2;
  } lightingData;

  // Use first directional light or default
  if (!m_Lights.empty() && m_Lights[0].type == LightType::Directional) {
    lightingData.LightDirection = m_Lights[0].direction;
    lightingData.LightColor = m_Lights[0].color;
    lightingData.LightIntensity = m_Lights[0].intensity;
  } else {
    lightingData.LightDirection = vec3(0.0f, -1.0f, 0.0f);
    lightingData.LightColor = vec3(1.0f);
    lightingData.LightIntensity = 1.0f;
  }
  lightingData.AmbientColor = vec3(0.2f, 0.2f, 0.2f);
  lightingData._pad1 = 0.0f;
  lightingData._pad2 = 0.0f;

  if (!m_InShadowPass) {
    m_LightingCB->Update(&lightingData, sizeof(LightingData));
    m_LightingCB->BindToVertexShader(2);
    m_LightingCB->BindToPixelShader(2);
  }

  // Bind and draw mesh
  mesh->Bind();
  mesh->Draw();
}

void Renderer3D::DrawMesh(Mesh *mesh, const mat4 &transform,
                          Material *material) {
  if (!mesh || !material || !m_InScene) {
    static bool logged = false;
    if (!logged) {
      YAMEN_CORE_WARN("DrawMesh early return: mesh={}, material={}, inScene={}",
                      mesh != nullptr, material != nullptr, m_InScene);
      logged = true;
    }
    return;
  }

  // Debug: Log once
  static bool drawLogged = false;
  if (!drawLogged) {
    YAMEN_CORE_INFO("DrawMesh called: mesh valid, material valid, binding...");
    drawLogged = true;
  }

  // Bind material (shader, textures, render states)
  material->Bind(m_Device);

  // Bind input layout (always needed)
  m_InputLayout->Bind();

  // Update per-frame constant buffer
  struct PerFrameData {
    mat4 ViewProjection;
    vec3 CameraPosition = {};
    float _pad0;
  } perFrameData;

  perFrameData.ViewProjection = Math::Transpose(m_CurrentCamera->GetViewProjectionMatrix());
  perFrameData.CameraPosition = m_CurrentCamera->GetPosition();
  perFrameData._pad0 = 0.0f;

  m_PerFrameCB->Update(&perFrameData, sizeof(PerFrameData));
  m_PerFrameCB->BindToVertexShader(0);
  m_PerFrameCB->BindToPixelShader(0);

  // Update per-object constant buffer
  struct PerObjectData {
    mat4 World;
    vec4 MaterialColor;
  } perObjectData;

  perObjectData.World = Math::Transpose(transform);
  perObjectData.MaterialColor =
      material->GetVector(Material::ALBEDO_COLOR, vec4(1.0f));

  m_PerObjectCB->Update(&perObjectData, sizeof(PerObjectData));
  m_PerObjectCB->BindToVertexShader(1);
  m_PerObjectCB->BindToPixelShader(1);

  // Setup lighting (same as regular DrawMesh)
  struct LightingData {
    vec3 LightDirection;
    float _pad1;
    vec3 LightColor;
    float LightIntensity;
    vec3 AmbientColor;
    float _pad2;
  } lightingData;

  if (!m_Lights.empty() && m_Lights[0].type == LightType::Directional) {
    lightingData.LightDirection = m_Lights[0].direction;
    lightingData.LightColor = m_Lights[0].color;
    lightingData.LightIntensity = m_Lights[0].intensity;
  } else {
    lightingData.LightDirection = vec3(0.0f, -1.0f, 0.0f);
    lightingData.LightColor = vec3(1.0f);
    lightingData.LightIntensity = 1.0f;
  }
  lightingData.AmbientColor = vec3(0.2f, 0.2f, 0.2f);
  lightingData._pad1 = 0.0f;
  lightingData._pad2 = 0.0f;

  m_LightingCB->Update(&lightingData, sizeof(LightingData));
  m_LightingCB->BindToVertexShader(2);
  m_LightingCB->BindToPixelShader(2);

  // Bind and draw mesh
  mesh->Bind();
  mesh->Draw();
}

void Renderer3D::DrawMeshWithSubMeshes(Mesh *mesh, const mat4 &transform) {
  if (!mesh || !m_InScene) {
    return;
  }

  if (!mesh->HasSubMeshes()) {
    // No submeshes, draw as regular mesh
    DrawMesh(mesh, transform);
    return;
  }

  // Draw each submesh with its own material
  const auto &submeshes = mesh->GetSubMeshes();
  for (size_t i = 0; i < submeshes.size(); ++i) {
    const auto &submesh = submeshes[i];

    if (submesh.material) {
      submesh.material->Bind(m_Device);
    } else {
      m_Shader->Bind();
      m_WhiteTexture->Bind(0);
    }

    m_InputLayout->Bind();

    //Update constant buffers (same pattern)
    struct PerFrameData {
      mat4 ViewProjection;
      vec3 CameraPosition;
      float _pad0;
    } perFrameData;

    perFrameData.ViewProjection = Math::Transpose(m_CurrentCamera->GetViewProjectionMatrix());
    perFrameData.CameraPosition = m_CurrentCamera->GetPosition();
    perFrameData._pad0 = 0.0f;

    m_PerFrameCB->Update(&perFrameData, sizeof(PerFrameData));
    m_PerFrameCB->BindToVertexShader(0);
    m_PerFrameCB->BindToPixelShader(0);

    struct PerObjectData {
      mat4 World;
      vec4 MaterialColor;
    } perObjectData;

    perObjectData.World = Math::Transpose(transform);
    if (submesh.material) {
      perObjectData.MaterialColor =
          submesh.material->GetVector(Material::ALBEDO_COLOR, vec4(1.0f));
    } else {
      perObjectData.MaterialColor = vec4(1.0f);
    }

    m_PerObjectCB->Update(&perObjectData, sizeof(PerObjectData));
    m_PerObjectCB->BindToVertexShader(1);
    m_PerObjectCB->BindToPixelShader(1);

    struct LightingData {
      vec3 LightDirection;
      float _pad1;
      vec3 LightColor;
      float LightIntensity;
      vec3 AmbientColor;
      float _pad2;
    } lightingData;

    if (!m_Lights.empty() && m_Lights[0].type == LightType::Directional) {
      lightingData.LightDirection = m_Lights[0].direction;
      lightingData.LightColor = m_Lights[0].color;
      lightingData.LightIntensity = m_Lights[0].intensity;
    } else {
      lightingData.LightDirection = vec3(0.0f, -1.0f, 0.0f);
      lightingData.LightColor = vec3(1.0f);
      lightingData.LightIntensity = 1.0f;
    }
    lightingData.AmbientColor = vec3(0.2f, 0.2f, 0.2f);
    lightingData._pad1 = 0.0f;
    lightingData._pad2 = 0.0f;

    m_LightingCB->Update(&lightingData, sizeof(LightingData));
    m_LightingCB->BindToVertexShader(2);
    m_LightingCB->BindToPixelShader(2);

    // Draw this submesh
    mesh->Bind();
    mesh->DrawSubMesh(static_cast<uint32_t>(i));
  }
}

void Renderer3D::SetWireframe(bool enabled) {
  m_Wireframe = enabled;
  if (m_InScene) {
    if (m_Wireframe) {
      m_WireframeState->Bind();
    } else {
      m_RasterizerState->Bind();
    }
  }
}

void Renderer3D::SetupLighting() {
  // TODO Lighting setup is now done inline in DrawMesh methods
}

} // namespace Yamen::Graphics

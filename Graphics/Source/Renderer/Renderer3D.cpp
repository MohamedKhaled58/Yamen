#include "Graphics/Renderer/Renderer3D.h"
#include "Graphics/Texture/TextureLoader.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    Renderer3D::Renderer3D(GraphicsDevice& device)
        : m_Device(device)
        , m_CurrentCamera(nullptr)
        , m_InScene(false)
        , m_Wireframe(false)
    {
    }

    Renderer3D::~Renderer3D() {
    }

    bool Renderer3D::Initialize() {
        // Create rasterizer states
        m_RasterizerState = std::make_unique<RasterizerState>(m_Device);
        if (!m_RasterizerState->Create(CullMode::Back, FillMode::Solid)) {
            YAMEN_CORE_ERROR("Failed to create rasterizer state");
            return false;
        }

        m_WireframeState = std::make_unique<RasterizerState>(m_Device);
        if (!m_WireframeState->Create(CullMode::Back, FillMode::Wireframe)) {
            YAMEN_CORE_ERROR("Failed to create wireframe state");
            return false;
        }

        // Create depth/stencil state
        m_DepthState = std::make_unique<DepthStencilState>(m_Device);
        if (!m_DepthState->Create(true, true, D3D11_COMPARISON_LESS)) {
            YAMEN_CORE_ERROR("Failed to create depth/stencil state");
            return false;
        }

        // Create blend state (opaque)
        m_BlendState = std::make_unique<BlendState>(m_Device);
        if (!m_BlendState->Create(BlendMode::Opaque)) {
            YAMEN_CORE_ERROR("Failed to create blend state");
            return false;
        }

        // Create sampler
        m_Sampler = std::make_unique<Sampler>(m_Device);
        if (!m_Sampler->Create(SamplerFilter::Anisotropic, SamplerAddressMode::Wrap, 16)) {
            YAMEN_CORE_ERROR("Failed to create sampler");
            return false;
        }

        // Create white texture for untextured meshes
        m_WhiteTexture = TextureLoader::CreateSolidColor(m_Device, 1, 1, 255, 255, 255, 255);
        if (!m_WhiteTexture) {
            YAMEN_CORE_ERROR("Failed to create white texture");
            return false;
        }

        // Load Basic3D shader
        m_Shader = std::make_unique<Shader>(m_Device);
        if (!m_Shader->CreateFromFiles("C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl", "C:/dev/C3Renderer/Yamen/Assets/Shaders/Basic3D.hlsl", "VSMain", "PSMain")) {
            YAMEN_CORE_ERROR("Failed to load Basic3D shader");
            return false;
        }

        // Create constant buffers
        // PerFrame: ViewProjection (mat4) + CameraPosition (vec3) + padding = 80 bytes
        m_PerFrameCB = std::make_unique<Buffer>(m_Device, BufferType::Constant);
        if (!m_PerFrameCB->Create(nullptr, 80, 0, BufferUsage::Dynamic)) {
            YAMEN_CORE_ERROR("Failed to create PerFrame constant buffer");
            return false;
        }

        // PerObject: World (mat4) + MaterialColor (vec4) = 80 bytes
        m_PerObjectCB = std::make_unique<Buffer>(m_Device, BufferType::Constant);
        if (!m_PerObjectCB->Create(nullptr, 80, 0, BufferUsage::Dynamic)) {
            YAMEN_CORE_ERROR("Failed to create PerObject constant buffer");
            return false;
        }

        // Lighting: LightDirection (vec3) + pad + LightColor (vec3) + Intensity (float) + AmbientColor (vec3) + pad = 64 bytes
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
            InputElement(InputSemantic::TexCoord, InputFormat::Float2, 0, 0, 24)
        };
        if (!m_InputLayout->Create(elements, m_Shader->GetVertexShaderBytecode().data(), m_Shader->GetVertexShaderBytecode().size())) {
            YAMEN_CORE_ERROR("Failed to create Renderer3D input layout");
            return false;
        }

        YAMEN_CORE_INFO("Renderer3D initialized");
        return true;
    }

    void Renderer3D::BeginScene(Camera3D* camera) {
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

    void Renderer3D::SubmitLight(const Light& light) {
        if (!m_InScene) {
            YAMEN_CORE_WARN("Renderer3D::SubmitLight called outside BeginScene/EndScene");
            return;
        }

        m_Lights.push_back(light);
    }

    void Renderer3D::DrawMesh(
        Mesh* mesh,
        const glm::mat4& transform,
        Texture2D* texture,
        const glm::vec4& color)
    {
        if (!m_InScene) {
            YAMEN_CORE_WARN("Renderer3D::DrawMesh called outside BeginScene/EndScene");
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
        m_Shader->Bind();

        // Update PerFrame constant buffer (b0)
        struct PerFrameData {
            glm::mat4 ViewProjection;
            glm::vec3 CameraPosition;
            float _pad0;
        } perFrameData;
        
        // IMPORTANT: Transpose matrices for HLSL (GLM is column-major, HLSL expects row-major)
        perFrameData.ViewProjection = glm::transpose(m_CurrentCamera->GetViewProjectionMatrix());
        perFrameData.CameraPosition = m_CurrentCamera->GetPosition();
        perFrameData._pad0 = 0.0f;
        
        m_PerFrameCB->Update(&perFrameData, sizeof(PerFrameData));
        m_PerFrameCB->BindToVertexShader(0);
        m_PerFrameCB->BindToPixelShader(0);

        // Update PerObject constant buffer (b1)
        struct PerObjectData {
            glm::mat4 World;
            glm::vec4 MaterialColor;
        } perObjectData;
        
        // IMPORTANT: Transpose world matrix for HLSL
        perObjectData.World = glm::transpose(transform);
        perObjectData.MaterialColor = color;
        
        m_PerObjectCB->Update(&perObjectData, sizeof(PerObjectData));
        m_PerObjectCB->BindToVertexShader(1);
        m_PerObjectCB->BindToPixelShader(1);

        // Update Lighting constant buffer (b2)
        struct LightingData {
            glm::vec3 LightDirection;
            float _pad1;
            glm::vec3 LightColor;
            float LightIntensity;
            glm::vec3 AmbientColor;
            float _pad2;
        } lightingData;
        
        // Use first directional light or default
        if (!m_Lights.empty() && m_Lights[0].type == LightType::Directional) {
            lightingData.LightDirection = m_Lights[0].direction;
            lightingData.LightColor = m_Lights[0].color;
            lightingData.LightIntensity = m_Lights[0].intensity;
        } else {
            lightingData.LightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
            lightingData.LightColor = glm::vec3(1.0f);
            lightingData.LightIntensity = 1.0f;
        }
        lightingData.AmbientColor = glm::vec3(0.2f, 0.2f, 0.2f);
        lightingData._pad1 = 0.0f;
        lightingData._pad2 = 0.0f;
        
        m_LightingCB->Update(&lightingData, sizeof(LightingData));
        m_LightingCB->BindToVertexShader(2);
        m_LightingCB->BindToPixelShader(2);

        // Bind and draw mesh
        mesh->Bind();
        mesh->Draw();
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
        // TODO: Upload light data to shader constant buffer
        // This will be implemented when we add constant buffer support
    }

} // namespace Yamen::Graphics

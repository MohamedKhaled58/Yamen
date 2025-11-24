#include "Graphics/Shader/Shader.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    Shader::Shader(GraphicsDevice& device)
        : m_Device(device)
    {
    }

    Shader::~Shader() {
        m_VertexShader.Reset();
        m_PixelShader.Reset();
    }

    bool Shader::CreateFromSource(
        const std::string& vsSource,
        const std::string& psSource,
        const std::string& vsEntry,
        const std::string& psEntry)
    {
        // Compile vertex shader
        auto vsResult = ShaderCompiler::CompileFromSource(vsSource, vsEntry, "vs_5_0", true);
        if (!vsResult.success) {
            YAMEN_CORE_ERROR("Vertex shader compilation failed: {}", vsResult.errorMessage);
            return false;
        }

        // Compile pixel shader
        auto psResult = ShaderCompiler::CompileFromSource(psSource, psEntry, "ps_5_0", true);
        if (!psResult.success) {
            YAMEN_CORE_ERROR("Pixel shader compilation failed: {}", psResult.errorMessage);
            return false;
        }

        // Store bytecode
        m_VSBytecode = std::move(vsResult.bytecode);
        m_PSBytecode = std::move(psResult.bytecode);

        // Create vertex shader
        HRESULT hr = m_Device.GetDevice()->CreateVertexShader(
            m_VSBytecode.data(),
            m_VSBytecode.size(),
            nullptr,
            &m_VertexShader
        );

        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create vertex shader: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        // Create pixel shader
        hr = m_Device.GetDevice()->CreatePixelShader(
            m_PSBytecode.data(),
            m_PSBytecode.size(),
            nullptr,
            &m_PixelShader
        );

        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create pixel shader: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        YAMEN_CORE_INFO("Shader created successfully");
        return true;
    }

    bool Shader::CreateFromFiles(
        const std::string& vsPath,
        const std::string& psPath,
        const std::string& vsEntry,
        const std::string& psEntry)
    {
        // Compile vertex shader
        auto vsResult = ShaderCompiler::CompileFromFile(vsPath, vsEntry, "vs_5_0", true);
        if (!vsResult.success) {
            YAMEN_CORE_ERROR("Vertex shader compilation failed: {}", vsResult.errorMessage);
            return false;
        }

        // Compile pixel shader
        auto psResult = ShaderCompiler::CompileFromFile(psPath, psEntry, "ps_5_0", true);
        if (!psResult.success) {
            YAMEN_CORE_ERROR("Pixel shader compilation failed: {}", psResult.errorMessage);
            return false;
        }

        // Store bytecode
        m_VSBytecode = std::move(vsResult.bytecode);
        m_PSBytecode = std::move(psResult.bytecode);

        // Create vertex shader
        HRESULT hr = m_Device.GetDevice()->CreateVertexShader(
            m_VSBytecode.data(),
            m_VSBytecode.size(),
            nullptr,
            &m_VertexShader
        );

        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create vertex shader: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        // Create pixel shader
        hr = m_Device.GetDevice()->CreatePixelShader(
            m_PSBytecode.data(),
            m_PSBytecode.size(),
            nullptr,
            &m_PixelShader
        );

        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create pixel shader: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        YAMEN_CORE_INFO("Shader created from files: {}, {}", vsPath, psPath);
        return true;
    }

    void Shader::Bind() {
        m_Device.GetContext()->VSSetShader(m_VertexShader.Get(), nullptr, 0);
        m_Device.GetContext()->PSSetShader(m_PixelShader.Get(), nullptr, 0);
    }

    void Shader::Unbind() {
        m_Device.GetContext()->VSSetShader(nullptr, nullptr, 0);
        m_Device.GetContext()->PSSetShader(nullptr, nullptr, 0);
    }

} // namespace Yamen::Graphics

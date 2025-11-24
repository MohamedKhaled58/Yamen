#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/Shader/ShaderCompiler.h"
#include <string>
#include <vector>

namespace Yamen::Graphics {

    /**
     * @brief Shader program (vertex + pixel shaders)
     * 
     * Manages compiled shader bytecode and D3D11 shader objects.
     */
    class Shader {
    public:
        Shader(GraphicsDevice& device);
        ~Shader();

        // Non-copyable
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        /**
         * @brief Create shader from source code
         * @param vsSource Vertex shader source
         * @param psSource Pixel shader source
         * @param vsEntry Vertex shader entry point (default: "VSMain")
         * @param psEntry Pixel shader entry point (default: "PSMain")
         * @return True if creation succeeded
         */
        bool CreateFromSource(
            const std::string& vsSource,
            const std::string& psSource,
            const std::string& vsEntry = "VSMain",
            const std::string& psEntry = "PSMain"
        );

        /**
         * @brief Create shader from files
         * @param vsPath Vertex shader file path
         * @param psPath Pixel shader file path
         * @param vsEntry Vertex shader entry point
         * @param psEntry Pixel shader entry point
         * @return True if creation succeeded
         */
        bool CreateFromFiles(
            const std::string& vsPath,
            const std::string& psPath,
            const std::string& vsEntry = "VSMain",
            const std::string& psEntry = "PSMain"
        );

        /**
         * @brief Bind shader to pipeline
         */
        void Bind();

        /**
         * @brief Unbind shader from pipeline
         */
        void Unbind();

        /**
         * @brief Get vertex shader bytecode (for input layout creation)
         */
        const std::vector<uint8_t>& GetVertexShaderBytecode() const { return m_VSBytecode; }

        /**
         * @brief Get shader objects
         */
        ID3D11VertexShader* GetVertexShader() const { return m_VertexShader.Get(); }
        ID3D11PixelShader* GetPixelShader() const { return m_PixelShader.Get(); }

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11VertexShader> m_VertexShader;
        ComPtr<ID3D11PixelShader> m_PixelShader;
        std::vector<uint8_t> m_VSBytecode;
        std::vector<uint8_t> m_PSBytecode;
    };

} // namespace Yamen::Graphics

#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <d3dcompiler.h>
#include <string>
#include <vector>

namespace Yamen::Graphics {

    /**
     * @brief HLSL shader compiler
     * 
     * Compiles HLSL shaders at runtime using D3DCompile.
     */
    class ShaderCompiler {
    public:
        /**
         * @brief Shader compilation result
         */
        struct CompileResult {
            bool success;
            std::vector<uint8_t> bytecode;
            std::string errorMessage;
        };

        /**
         * @brief Compile shader from source code
         * @param source HLSL source code
         * @param entryPoint Entry point function name (e.g., "main", "VSMain")
         * @param target Shader model target (e.g., "vs_5_0", "ps_5_0")
         * @param debugMode Enable debug information
         * @return Compilation result
         */
        static CompileResult CompileFromSource(
            const std::string& source,
            const std::string& entryPoint,
            const std::string& target,
            bool debugMode = false
        );

        /**
         * @brief Compile shader from file
         * @param filepath Path to HLSL file
         * @param entryPoint Entry point function name
         * @param target Shader model target
         * @param debugMode Enable debug information
         * @return Compilation result
         */
        static CompileResult CompileFromFile(
            const std::string& filepath,
            const std::string& entryPoint,
            const std::string& target,
            bool debugMode = false
        );

    private:
        static CompileResult Compile(
            const void* data,
            size_t dataSize,
            const std::string& sourceName,
            const std::string& entryPoint,
            const std::string& target,
            bool debugMode
        );
    };

} // namespace Yamen::Graphics

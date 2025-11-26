#include "Graphics/Shader/ShaderCompiler.h"
#include <Core/Logging/Logger.h>
#include "Core/Utils/FileSystem.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace Yamen::Graphics {

    ShaderCompiler::CompileResult ShaderCompiler::CompileFromSource(
        const std::string& source,
        const std::string& entryPoint,
        const std::string& target,
        bool debugMode)
    {
        return Compile(
            source.c_str(),
            source.length(),
            "ShaderSource",
            entryPoint,
            target,
            debugMode
        );
    }

    ShaderCompiler::CompileResult ShaderCompiler::CompileFromFile(
        const std::string& filepath,
        const std::string& entryPoint,
        const std::string& target,
        bool debugMode)
    {
        // Read file
        std::string source = Core::FileSystem::ReadFileText(filepath);
        if (source.empty()) {
            CompileResult result;
            result.success = false;
            result.errorMessage = "Failed to read shader file: " + filepath;
            return result;
        }

        return Compile(
            source.c_str(),
            source.length(),
            filepath,
            entryPoint,
            target,
            debugMode
        );
    }

    ShaderCompiler::CompileResult ShaderCompiler::Compile(
        const void* data,
        size_t dataSize,
        const std::string& sourceName,
        const std::string& entryPoint,
        const std::string& target,
        bool debugMode)
    {
        CompileResult result;
        result.success = false;

        // Compilation flags
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
        if (debugMode) {
            flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        } else {
            flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
        }

        // Compile shader
        ID3DBlob* shaderBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;

        HRESULT hr = D3DCompile(
            data,
            dataSize,
            sourceName.c_str(),
            nullptr,                // Defines
            D3D_COMPILE_STANDARD_FILE_INCLUDE,  // Include handler
            entryPoint.c_str(),
            target.c_str(),
            flags,
            0,
            &shaderBlob,
            &errorBlob
        );

        if (FAILED(hr)) {
            if (errorBlob) {
                result.errorMessage = std::string(
                    static_cast<const char*>(errorBlob->GetBufferPointer()),
                    errorBlob->GetBufferSize()
                );
                errorBlob->Release();
            } else {
                result.errorMessage = "Unknown compilation error";
            }

            YAMEN_CORE_ERROR("Shader compilation failed: {}", result.errorMessage);
            
            if (shaderBlob) {
                shaderBlob->Release();
            }
            
            return result;
        }

        // Copy bytecode
        result.bytecode.resize(shaderBlob->GetBufferSize());
        memcpy(result.bytecode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
        result.success = true;

        shaderBlob->Release();
        if (errorBlob) {
            errorBlob->Release();
        }

        YAMEN_CORE_TRACE("Shader compiled successfully: {} (entry: {}, target: {})", 
            sourceName, entryPoint, target);

        return result;
    }

} // namespace Yamen::Graphics

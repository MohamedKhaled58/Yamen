#include "Graphics/Shader/ShaderLibrary.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/RHI/GraphicsDevice.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    ShaderLibrary::ShaderLibrary(GraphicsDevice& device)
        : m_Device(device) {
    }

    Shader* ShaderLibrary::Load(const std::string& name, const std::string& vsPath, const std::string& psPath) {
        return LoadWithDefines(name, vsPath, psPath, {});
    }

    Shader* ShaderLibrary::LoadWithDefines(const std::string& name, const std::string& vsPath, const std::string& psPath,
                                          const std::vector<std::string>& defines) {
        // Check if already loaded
        if (Exists(name)) {
            YAMEN_CORE_WARN("Shader '{}' already exists in library, replacing...", name);
            Remove(name);
        }

        // Create new shader
        auto shader = std::make_unique<Shader>(m_Device);
        if (!shader->CreateFromFiles(vsPath, psPath)) {
            YAMEN_CORE_ERROR("Failed to load shader '{}' from files: {}, {}", name, vsPath, psPath);
            return nullptr;
        }

        // Track files for hot-reload
        TrackFile(name, vsPath);
        TrackFile(name, psPath);

        // Store shader
        auto* shaderPtr = shader.get();
        m_Shaders[name] = std::move(shader);

        YAMEN_CORE_INFO("Loaded shader '{}' into library", name);
        
        // Defines are not used in current implementation but kept for future use
        (void)defines;
        
        return shaderPtr;
    }

    Shader* ShaderLibrary::Get(const std::string& name) {
        auto it = m_Shaders.find(name);
        if (it != m_Shaders.end()) {
            return it->second.get();
        }
        
        YAMEN_CORE_WARN("Shader '{}' not found in library", name);
        return nullptr;
    }

    bool ShaderLibrary::Exists(const std::string& name) const {
        return m_Shaders.find(name) != m_Shaders.end();
    }

    void ShaderLibrary::Remove(const std::string& name) {
        m_Shaders.erase(name);
        m_ShaderFiles.erase(name);
    }

    void ShaderLibrary::Clear() {
        m_Shaders.clear();
        m_ShaderFiles.clear();
        m_FileTimes.clear();
    }

    void ShaderLibrary::CheckForChanges() {
        if (!m_HotReloadEnabled) return;

        for (auto& [shaderName, filePaths] : m_ShaderFiles) {
            bool needsReload = false;

            for (const auto& path : filePaths) {
                if (HasFileChanged(path)) {
                    needsReload = true;
                    break;
                }
            }

            if (needsReload && filePaths.size() >= 2) {
                YAMEN_CORE_INFO("Hot-reloading shader '{}'...", shaderName);
                
                // Reload shader (assumes first file is VS, second is PS)
                Load(shaderName, filePaths[0], filePaths[1]);
            }
        }
    }

    void ShaderLibrary::PrecompileDefaults() {
        // Precompile commonly used shaders
        Load("Sprite2D", "Assets/Shaders/Sprite2D.hlsl", "Assets/Shaders/Sprite2D.hlsl");
        Load("Basic3D", "Assets/Shaders/Basic3D.hlsl", "Assets/Shaders/Basic3D.hlsl");
        Load("C3Sprite", "Assets/Shaders/C3Sprite.hlsl", "Assets/Shaders/C3Sprite.hlsl");
        Load("C3PhyMesh", "Assets/Shaders/C3PhyMesh.hlsl", "Assets/Shaders/C3PhyMesh.hlsl");
        Load("C3Skin", "Assets/Shaders/C3Skin.hlsl", "Assets/Shaders/C3Skin.hlsl");

        YAMEN_CORE_INFO("Precompiled default shaders");
    }

    void ShaderLibrary::TrackFile(const std::string& shaderName, const std::string& path) {
        // Add to shader files list
        m_ShaderFiles[shaderName].push_back(path);

        // Store initial file time
        if (std::filesystem::exists(path)) {
            m_FileTimes[path] = std::filesystem::last_write_time(path);
        }
    }

    bool ShaderLibrary::HasFileChanged(const std::string& path) {
        if (!std::filesystem::exists(path)) {
            return false;
        }

        auto currentTime = std::filesystem::last_write_time(path);
        auto it = m_FileTimes.find(path);
        
        if (it != m_FileTimes.end() && it->second != currentTime) {
            m_FileTimes[path] = currentTime;
            return true;
        }

        return false;
    }

} // namespace Yamen::Graphics

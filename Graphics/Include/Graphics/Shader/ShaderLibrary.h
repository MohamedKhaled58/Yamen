#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <filesystem>

namespace Yamen::Graphics {

    // Forward declarations
    class GraphicsDevice;
    class Shader;

    /// <summary>
    /// Centralized shader management with caching and hot-reload
    /// </summary>
    class ShaderLibrary {
    public:
        ShaderLibrary(GraphicsDevice& device);
        ~ShaderLibrary() = default;

        // Load/compile shaders
        Shader* Load(const std::string& name, const std::string& vsPath, const std::string& psPath);
        Shader* LoadWithDefines(const std::string& name, const std::string& vsPath, const std::string& psPath,
                               const std::vector<std::string>& defines);

        // Get cached shader
        Shader* Get(const std::string& name);
        bool Exists(const std::string& name) const;

        // Remove shader from cache
        void Remove(const std::string& name);
        void Clear();

        // Hot-reload support
        void EnableHotReload(bool enable) { m_HotReloadEnabled = enable; }
        bool IsHotReloadEnabled() const { return m_HotReloadEnabled; }
        void CheckForChanges(); // Call per frame to detect file changes

        // Precompile common shaders
        void PrecompileDefaults();

    private:
        void TrackFile(const std::string& shaderName, const std::string& path);
        bool HasFileChanged(const std::string& path);

        GraphicsDevice& m_Device;
        std::unordered_map<std::string, std::unique_ptr<Shader>> m_Shaders;
        std::unordered_map<std::string, std::vector<std::string>> m_ShaderFiles; // Shader name -> file paths
        std::unordered_map<std::string, std::filesystem::file_time_type> m_FileTimes;
        bool m_HotReloadEnabled = false;
    };

} // namespace Yamen::Graphics

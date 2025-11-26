#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

namespace Yamen::Graphics {

    // Forward declarations
    class GraphicsDevice;
    class Shader;
    class Texture2D;
    class BlendState;
    class DepthStencilState;
    class RasterizerState;

    /// <summary>
    /// Material system for rendering with shaders and textures
    /// </summary>
    class Material {
    public:
        Material() = default;
        ~Material() = default;

        // Shader & render states
        void SetShader(Shader* shader) { m_Shader = shader; }
        Shader* GetShader() const { return m_Shader; }

        void SetBlendState(BlendState* state) { m_BlendState = state; }
        BlendState* GetBlendState() const { return m_BlendState; }
        void SetDepthStencilState(DepthStencilState* state) { m_DepthState = state; }
        void SetRasterizerState(RasterizerState* state) { m_RasterizerState = state; }

        // Textures
        void SetTexture(const std::string& name, Texture2D* texture);
        Texture2D* GetTexture(const std::string& name) const;
        bool HasTexture(const std::string& name) const;

        // Float properties
        void SetFloat(const std::string& name, float value);
        float GetFloat(const std::string& name, float defaultValue = 0.0f) const;

        // Vector properties
        void SetVector(const std::string& name, const glm::vec4& value);
        glm::vec4 GetVector(const std::string& name, const glm::vec4& defaultValue = glm::vec4(0.0f)) const;

        // Matrix properties
        void SetMatrix(const std::string& name, const glm::mat4& value);
        glm::mat4 GetMatrix(const std::string& name, const glm::mat4& defaultValue = glm::mat4(1.0f)) const;

        // Bind material to rendering pipeline
        void Bind(GraphicsDevice& device);

        // Common property names
        static constexpr const char* DIFFUSE_TEXTURE = "DiffuseTexture";
        static constexpr const char* NORMAL_TEXTURE = "NormalTexture";
        static constexpr const char* SPECULAR_TEXTURE = "SpecularTexture";
        static constexpr const char* EMISSIVE_TEXTURE = "EmissiveTexture";
        static constexpr const char* OCCLUSION_TEXTURE = "OcclusionTexture";

        static constexpr const char* ALBEDO_COLOR = "AlbedoColor";
        static constexpr const char* EMISSIVE_COLOR = "EmissiveColor";
        static constexpr const char* METALLIC = "Metallic";
        static constexpr const char* ROUGHNESS = "Roughness";

    private:
        Shader* m_Shader = nullptr;

        std::unordered_map<std::string, Texture2D*> m_Textures;
        std::unordered_map<std::string, float> m_Floats;
        std::unordered_map<std::string, glm::vec4> m_Vectors;
        std::unordered_map<std::string, glm::mat4> m_Matrices;

        BlendState* m_BlendState = nullptr;
        DepthStencilState* m_DepthState = nullptr;
        RasterizerState* m_RasterizerState = nullptr;
    };

} // namespace Yamen::Graphics

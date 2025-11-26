#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Renderer/Camera3D.h"
#include "Graphics/Lighting/Light.h"

namespace Yamen::ECS {

    /**
     * @brief 2D Sprite component for sprite rendering
     */
    struct SpriteComponent {
        glm::vec4 Color = glm::vec4(1.0f);
        std::shared_ptr<Graphics::Texture2D> Texture = nullptr;
        float TilingFactor = 1.0f;
        int SortingLayer = 0;
        int OrderInLayer = 0;

        SpriteComponent() = default;
        SpriteComponent(const SpriteComponent&) = default;
        SpriteComponent(const glm::vec4& color) : Color(color) {}
    };

    /**
     * @brief 3D Mesh component for mesh rendering
     */
    struct MeshComponent {
        std::shared_ptr<Graphics::Mesh> Mesh = nullptr;
        std::shared_ptr<Graphics::Material> Material = nullptr;
        bool Visible = true;
        bool CastShadows = true;
        bool ReceiveShadows = true;

        MeshComponent() = default;
        MeshComponent(const MeshComponent&) = default;
        MeshComponent(std::shared_ptr<Graphics::Mesh> mesh) : Mesh(mesh) {}
    };

    /**
     * @brief Light component for lighting
     */
    struct LightComponent {
        Graphics::Light LightData;
        bool Active = true;
        bool CastShadows = false;

        LightComponent() = default;
        LightComponent(const LightComponent&) = default;
    };

    /**
     * @brief Camera component for rendering viewpoint
     */
    struct CameraComponent {
        Graphics::Camera3D Camera;
        bool Primary = true;
        bool FixedAspectRatio = false;
        int RenderOrder = 0; // Lower renders first

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

} // namespace Yamen::ECS

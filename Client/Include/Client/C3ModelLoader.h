#pragma once

#include "AssetsC3/C3PhyLoader.h"
#include "ECS/Components/SkeletalAnimationComponent.h"
#include "ECS/Systems/SkeletalAnimationSystem.h"
#include "Graphics/Renderer/C3SkeletalRenderer.h"
#include <entt/entt.hpp>
#include <string>

namespace Yamen::Client {

/**
 * @brief Helper class for loading and rendering C3 models
 *
 * Example usage:
 *
 * // Load a C3 model
 * auto entity = C3ModelLoader::LoadModel(registry, "models/character.c3");
 *
 * // In your update loop:
 * ECS::SkeletalAnimationSystem::Update(registry, deltaTime);
 *
 * // In your render loop:
 * C3ModelLoader::RenderModel(entity, registry, skeletalRenderer, camera);
 */
class C3ModelLoader {
public:
  /**
   * @brief Load a C3 model file and create an entity
   * @param registry ECS registry
   * @param filepath Path to .c3 file
   * @return Entity handle (or entt::null on failure)
   */
  static entt::entity LoadModel(entt::registry &registry,
                                const std::string &filepath);

  /**
   * @brief Render a C3 model entity
   * @param entity Entity with C3MeshComponent and optionally
   * SkeletalAnimationComponent
   * @param registry ECS registry
   * @param renderer C3SkeletalRenderer instance
   * @param modelViewProj Model-View-Projection matrix
   */
  static void RenderModel(entt::entity entity, entt::registry &registry,
                          Graphics::C3SkeletalRenderer &renderer,
                          const glm::mat4 &modelViewProj);

  /**
   * @brief Unload a C3 model entity
   * @param entity Entity to destroy
   * @param registry ECS registry
   */
  static void UnloadModel(entt::entity entity, entt::registry &registry);
};

} // namespace Yamen::Client

#pragma once

#include "AssetsC3/C3PhyLoader.h"
#include "ECS/Components/SkeletalAnimationComponent.h"
#include <entt/entt.hpp>


namespace Yamen::ECS {

/**
 * @brief System for updating skeletal animations
 *
 * Updates animation playback and interpolates bone matrices each frame
 */
class SkeletalAnimationSystem {
public:
  /**
   * @brief Update all skeletal animations
   * @param registry ECS registry
   * @param deltaTime Time since last frame (seconds)
   */
  static void Update(entt::registry &registry, float deltaTime);

  /**
   * @brief Play animation
   * @param anim Animation component
   * @param fromStart Reset to frame 0
   */
  static void Play(SkeletalAnimationComponent &anim, bool fromStart = false);

  /**
   * @brief Pause animation
   * @param anim Animation component
   */
  static void Pause(SkeletalAnimationComponent &anim);

  /**
   * @brief Stop animation and reset to frame 0
   * @param anim Animation component
   */
  static void Stop(SkeletalAnimationComponent &anim);

  /**
   * @brief Set animation frame
   * @param anim Animation component
   * @param frame Target frame
   */
  static void SetFrame(SkeletalAnimationComponent &anim, float frame);

  /**
   * @brief Set playback speed
   * @param anim Animation component
   * @param fps Frames per second
   */
  static void SetSpeed(SkeletalAnimationComponent &anim, float fps);
};

} // namespace Yamen::ECS

#include "ECS/Systems/SkeletalAnimationSystem.h"
#include "Core/Logging/Logger.h"

namespace Yamen::ECS {

void SkeletalAnimationSystem::Update(entt::registry &registry,
                                     float deltaTime) {
  auto view = registry.view<SkeletalAnimationComponent>();

  for (auto entity : view) {
    auto &anim = view.get<SkeletalAnimationComponent>(entity);

    if (!anim.motion || !anim.isPlaying) {
      continue;
    }

    // Advance frame
    anim.currentFrame += anim.playbackSpeed * deltaTime;

    // Handle looping
    if (anim.currentFrame >= anim.motion->frameCount) {
      if (anim.loop) {
        anim.currentFrame = fmod(anim.currentFrame,
                                 static_cast<float>(anim.motion->frameCount));
      } else {
        anim.currentFrame = static_cast<float>(anim.motion->frameCount - 1);
        anim.isPlaying = false;
      }
    }

    // Interpolate bone matrices for current frame (Global Transforms)
    Assets::C3PhyLoader::InterpolateBones(*anim.motion, anim.currentFrame,
                                          anim.boneMatrices);

    // Ensure finalBoneMatrices is resized
    if (anim.finalBoneMatrices.size() != anim.boneMatrices.size()) {
      anim.finalBoneMatrices.resize(anim.boneMatrices.size());
    }

    // User's working order: InvBind * Global
    // (This was manually tested and worked)
// Apply InvBind: finalMatrix = Global * InvBind
    for (size_t i = 0; i < anim.boneMatrices.size(); ++i) {
        if (i < anim.inverseBindMatrices.size()) {
            anim.finalBoneMatrices[i] = anim.boneMatrices[i] * anim.inverseBindMatrices[i];
        }
        else {
            anim.finalBoneMatrices[i] = anim.boneMatrices[i];
        }
    }
  }
}
void SkeletalAnimationSystem::Play(SkeletalAnimationComponent &anim,
                                   bool fromStart) {
  if (fromStart) {
    anim.currentFrame = 0.0f;
  }
  anim.isPlaying = true;
}

void SkeletalAnimationSystem::Pause(SkeletalAnimationComponent &anim) {
  anim.isPlaying = false;
}

void SkeletalAnimationSystem::Stop(SkeletalAnimationComponent &anim) {
  anim.isPlaying = false;
  anim.currentFrame = 0.0f;
}

void SkeletalAnimationSystem::SetFrame(SkeletalAnimationComponent &anim,
                                       float frame) {
  if (anim.motion) {
    anim.currentFrame =
        (std::max)(0.0f, (std::min)(frame, static_cast<float>(
                                               anim.motion->frameCount - 1)));
  }
}

void SkeletalAnimationSystem::SetSpeed(SkeletalAnimationComponent &anim,
                                       float fps) {
  anim.playbackSpeed = fps;
}

} // namespace Yamen::ECS

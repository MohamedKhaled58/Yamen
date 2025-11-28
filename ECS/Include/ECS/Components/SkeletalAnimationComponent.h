#pragma once

#include "AssetsC3/C3PhyLoader.h"
#include <Graphics/Texture/Texture2D.h>
#include <memory>
#include <string>

namespace Yamen::Graphics {
class Buffer;
}

namespace Yamen::ECS {

/**
 * @brief Component for skeletal animation state
 *
 * Attach to entities that have animated C3 models
 */
struct SkeletalAnimationComponent {
  Assets::C3Motion *motion; // Animation data (owned by C3Phy)

  float currentFrame;  // Current playback frame (can be fractional)
  float playbackSpeed; // Frames per second
  bool isPlaying;      // Playback state
  bool loop;           // Loop animation

  std::vector<glm::mat4> boneMatrices; // Current bone transformations (Global)
  std::vector<glm::mat4> inverseBindMatrices; // Inverse Bind Pose Matrices
  std::vector<glm::mat4>
      finalBoneMatrices; // Final Skinning Matrices (Global * InvBind)

  SkeletalAnimationComponent()
      : motion(nullptr), currentFrame(0.0f),
        playbackSpeed(30.0f) // Default 30 FPS
        ,
        isPlaying(true), loop(true) {}
};

/**
 * @brief Component for C3 mesh rendering
 *
 * Stores the PHY mesh data for rendering
 * NOTE: mesh pointer is not used to avoid incomplete type issues
 * You should create the mesh in your rendering system where you have access to
 * GraphicsDevice
 */

struct C3MeshComponent {
  Assets::C3Phy *phy; // Mesh data (owned by this component)

  // GPU Buffers
  std::shared_ptr<Yamen::Graphics::Buffer> vertexBuffer;
  std::shared_ptr<Yamen::Graphics::Buffer> indexBuffer;
  uint32_t indexCount;

  // Texture
  std::shared_ptr<Yamen::Graphics::Texture2D> texture;

  // Rendering state
  bool visible;
  float uvAnimTime; // UV animation time accumulator

  C3MeshComponent() : phy(nullptr), visible(true), uvAnimTime(0.0f) {}

  ~C3MeshComponent() {
    if (phy) {
      delete phy;
      phy = nullptr;
    }
  }
};

} // namespace Yamen::ECS

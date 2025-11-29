#pragma once

#include "Graphics/Lighting/Light.h"
#include <Core/Math/Math.h>
#include <vector>


namespace Yamen::Graphics {

using namespace Yamen::Core;

/// <summary>
/// Manages multiple lights with culling and sorting
/// </summary>
class LightManager {
public:
  LightManager() = default;
  ~LightManager() = default;

  // Add/remove lights
  void AddLight(Light *light);
  void RemoveLight(Light *light);
  void Clear();

  // Get all lights
  const std::vector<Light *> &GetLights() const { return m_Lights; }
  size_t GetLightCount() const { return m_Lights.size(); }

  // Light culling - get lights affecting an object at position with radius
  std::vector<Light *> GetLightsForObject(const vec3 &position,
                                          float radius) const;

  // Get top N lights sorted by importance (distance and intensity)
  std::vector<Light *> GetTopLights(const vec3 &position,
                                    size_t maxLights) const;

  // Update all lights (for animation, etc.)
  void Update(float deltaTime);

private:
  float CalculateLightImportance(const Light *light,
                                 const vec3 &position) const;

  std::vector<Light *> m_Lights;
};

} // namespace Yamen::Graphics

#include "Graphics/Lighting/LightManager.h"
#include <algorithm>

namespace Yamen::Graphics {

using namespace Yamen::Core;

void LightManager::AddLight(Light *light) {
  if (light) {
    m_Lights.push_back(light);
  }
}

void LightManager::RemoveLight(Light *light) {
  auto it = std::find(m_Lights.begin(), m_Lights.end(), light);
  if (it != m_Lights.end()) {
    m_Lights.erase(it);
  }
}

void LightManager::Clear() { m_Lights.clear(); }

std::vector<Light *> LightManager::GetLightsForObject(const vec3 &position,
                                                      float radius) const {
  std::vector<Light *> result;

  for (auto *light : m_Lights) {
    // Directional lights always affect everything
    if (light->type == LightType::Directional) {
      result.push_back(light);
      continue;
    }

    // Point and Spot lights - check distance
    float distance = Math::Length(light->position - position);
    if (distance <= (light->range + radius)) {
      result.push_back(light);
    }
  }

  return result;
}

std::vector<Light *> LightManager::GetTopLights(const vec3 &position,
                                                size_t maxLights) const {
  // Sort lights by importance
  std::vector<std::pair<Light *, float>> lightsWithImportance;
  for (auto *light : m_Lights) {
    float importance = CalculateLightImportance(light, position);
    lightsWithImportance.push_back({light, importance});
  }

  // Sort by importance (descending)
  std::sort(lightsWithImportance.begin(), lightsWithImportance.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  // Return top N lights
  std::vector<Light *> result;
  size_t count = std::min(maxLights, lightsWithImportance.size());
  for (size_t i = 0; i < count; ++i) {
    result.push_back(lightsWithImportance[i].first);
  }

  return result;
}

void LightManager::Update(float deltaTime) {
  // Could be used for light animation, flicker effects, etc.
  // Currently unused
  (void)deltaTime;
}

float LightManager::CalculateLightImportance(const Light *light,
                                             const vec3 &position) const {
  // Directional lights have constant importance
  if (light->type == LightType::Directional) {
    return light->intensity * 1000.0f; // High importance
  }

  // Point/Spot lights - importance based on distance and intensity
  float distance = Math::Length(light->position - position);
  float range = light->range;

  if (distance > range) {
    return 0.0f; // Out of range
  }

  // Calculate attenuation (inverse square with range cutoff)
  float attenuation = 1.0f - (distance / range);
  attenuation = attenuation * attenuation; // Quadratic falloff

  return light->intensity * attenuation;
}

} // namespace Yamen::Graphics

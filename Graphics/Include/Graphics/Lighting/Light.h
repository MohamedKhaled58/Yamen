#pragma once

#include <Core/Math/Math.h>

namespace Yamen::Graphics {

using namespace Yamen::Core;

/**
 * @brief Light type enumeration
 */
enum class LightType { Directional, Point, Spot };

/**
 * @brief Base light structure
 */
struct Light {
  LightType type;
  vec3 position;  // For Point and Spot lights
  vec3 direction; // For Directional and Spot lights
  vec3 color;
  float intensity;

  // Point light attenuation
  float range;
  float constant;
  float linear;
  float quadratic;

  // Spot light cone
  float innerConeAngle;
  float outerConeAngle;

  bool castsShadows;

  Light()
      : type(LightType::Directional), position(0.0f),
        direction(0.0f, -1.0f, 0.0f), color(1.0f), intensity(1.0f),
        range(10.0f), constant(1.0f), linear(0.09f), quadratic(0.032f),
        innerConeAngle(12.5f), outerConeAngle(17.5f), castsShadows(false) {}

  /**
   * @brief Create directional light (sun)
   */
  static Light CreateDirectional(const vec3 &direction, const vec3 &color,
                                 float intensity = 1.0f) {
    Light light;
    light.type = LightType::Directional;
    light.direction = Math::Normalize(direction);
    light.color = color;
    light.intensity = intensity;
    return light;
  }

  /**
   * @brief Create point light
   */
  static Light CreatePoint(const vec3 &position, const vec3 &color,
                           float intensity = 1.0f, float range = 10.0f) {
    Light light;
    light.type = LightType::Point;
    light.position = position;
    light.color = color;
    light.intensity = intensity;
    light.range = range;
    return light;
  }

  /**
   * @brief Create spot light
   */
  static Light CreateSpot(const vec3 &position, const vec3 &direction,
                          const vec3 &color, float intensity = 1.0f,
                          float innerAngle = 12.5f, float outerAngle = 17.5f) {
    Light light;
    light.type = LightType::Spot;
    light.position = position;
    light.direction = Math::Normalize(direction);
    light.color = color;
    light.intensity = intensity;
    light.innerConeAngle = innerAngle;
    light.outerConeAngle = outerAngle;
    return light;
  }
};

} // namespace Yamen::Graphics

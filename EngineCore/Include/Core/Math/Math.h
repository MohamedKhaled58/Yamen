#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Yamen::Core {

// Re-export GLM types for convenience
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;

// Common math constants
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;
constexpr float HALF_PI = 0.5f * PI;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;

// Utility functions
inline float ToRadians(float degrees) noexcept {
    return degrees * DEG_TO_RAD;
}

inline float ToDegrees(float radians) noexcept {
    return radians * RAD_TO_DEG;
}

inline float Lerp(float a, float b, float t) noexcept {
    return a + (b - a) * t;
}

inline vec3 Lerp(const vec3& a, const vec3& b, float t) noexcept {
    return a + (b - a) * t;
}

inline float Clamp(float value, float min, float max) noexcept {
    return value < min ? min : (value > max ? max : value);
}

// AABB (Axis-Aligned Bounding Box)
struct AABB {
    vec3 min{0.0f};
    vec3 max{0.0f};
    
    AABB() = default;
    AABB(const vec3& min, const vec3& max) : min(min), max(max) {}
    
    vec3 GetCenter() const noexcept {
        return (min + max) * 0.5f;
    }
    
    vec3 GetExtents() const noexcept {
        return (max - min) * 0.5f;
    }
    
    vec3 GetSize() const noexcept {
        return max - min;
    }
    
    bool Contains(const vec3& point) const noexcept {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
    
    bool Intersects(const AABB& other) const noexcept {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }
    
    void Expand(const vec3& point) noexcept {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }
    
    void Expand(const AABB& other) noexcept {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }
};

// Ray
struct Ray {
    vec3 origin{0.0f};
    vec3 direction{0.0f, 0.0f, 1.0f};
    
    Ray() = default;
    Ray(const vec3& origin, const vec3& direction) 
        : origin(origin), direction(glm::normalize(direction)) {}
    
    vec3 GetPoint(float t) const noexcept {
        return origin + direction * t;
    }
    
    bool IntersectsAABB(const AABB& aabb, float& tMin, float& tMax) const noexcept {
        vec3 invDir = 1.0f / direction;
        vec3 t0 = (aabb.min - origin) * invDir;
        vec3 t1 = (aabb.max - origin) * invDir;
        
        vec3 tmin = glm::min(t0, t1);
        vec3 tmax = glm::max(t0, t1);
        
        tMin = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
        tMax = glm::min(glm::min(tmax.x, tmax.y), tmax.z);
        
        return tMax >= tMin && tMax >= 0.0f;
    }
};

// Frustum (for culling)
struct Frustum {
    enum Plane {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far,
        Count
    };
    
    vec4 planes[Count];
    
    void Update(const mat4& viewProjection) noexcept {
        // Extract frustum planes from view-projection matrix
        // Left
        planes[Left] = vec4(
            viewProjection[0][3] + viewProjection[0][0],
            viewProjection[1][3] + viewProjection[1][0],
            viewProjection[2][3] + viewProjection[2][0],
            viewProjection[3][3] + viewProjection[3][0]
        );
        
        // Right
        planes[Right] = vec4(
            viewProjection[0][3] - viewProjection[0][0],
            viewProjection[1][3] - viewProjection[1][0],
            viewProjection[2][3] - viewProjection[2][0],
            viewProjection[3][3] - viewProjection[3][0]
        );
        
        // Bottom
        planes[Bottom] = vec4(
            viewProjection[0][3] + viewProjection[0][1],
            viewProjection[1][3] + viewProjection[1][1],
            viewProjection[2][3] + viewProjection[2][1],
            viewProjection[3][3] + viewProjection[3][1]
        );
        
        // Top
        planes[Top] = vec4(
            viewProjection[0][3] - viewProjection[0][1],
            viewProjection[1][3] - viewProjection[1][1],
            viewProjection[2][3] - viewProjection[2][1],
            viewProjection[3][3] - viewProjection[3][1]
        );
        
        // Near
        planes[Near] = vec4(
            viewProjection[0][3] + viewProjection[0][2],
            viewProjection[1][3] + viewProjection[1][2],
            viewProjection[2][3] + viewProjection[2][2],
            viewProjection[3][3] + viewProjection[3][2]
        );
        
        // Far
        planes[Far] = vec4(
            viewProjection[0][3] - viewProjection[0][2],
            viewProjection[1][3] - viewProjection[1][2],
            viewProjection[2][3] - viewProjection[2][2],
            viewProjection[3][3] - viewProjection[3][2]
        );
        
        // Normalize planes
        for (int i = 0; i < Count; ++i) {
            float length = glm::length(vec3(planes[i]));
            planes[i] /= length;
        }
    }
    
    bool ContainsPoint(const vec3& point) const noexcept {
        for (int i = 0; i < Count; ++i) {
            if (glm::dot(vec3(planes[i]), point) + planes[i].w < 0.0f) {
                return false;
            }
        }
        return true;
    }
    
    bool IntersectsAABB(const AABB& aabb) const noexcept {
        for (int i = 0; i < Count; ++i) {
            vec3 positive = aabb.min;
            if (planes[i].x >= 0) positive.x = aabb.max.x;
            if (planes[i].y >= 0) positive.y = aabb.max.y;
            if (planes[i].z >= 0) positive.z = aabb.max.z;
            
            if (glm::dot(vec3(planes[i]), positive) + planes[i].w < 0.0f) {
                return false;
            }
        }
        return true;
    }
};

} // namespace Yamen::Core

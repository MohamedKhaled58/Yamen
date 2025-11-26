#pragma once

#include <glm/glm.hpp>
#include <variant>

namespace Yamen::ECS {

    enum class BodyType {
        Static,     // Infinite mass, zero velocity, does not move (e.g. walls, ground)
        Dynamic,    // Finite mass, affected by forces and collisions (e.g. player, debris)
        Kinematic   // Infinite mass, moves by script only (e.g. moving platforms)
    };

    struct RigidBodyComponent {
        BodyType Type = BodyType::Dynamic;
        
        float Mass = 1.0f;
        float LinearDrag = 0.01f;
        float AngularDrag = 0.05f;
        bool UseGravity = true;
        bool IsSleeping = false;

        // Dynamics
        glm::vec3 Velocity = glm::vec3(0.0f);
        glm::vec3 AngularVelocity = glm::vec3(0.0f);
        
        // Accumulators (cleared every frame)
        glm::vec3 Force = glm::vec3(0.0f);
        glm::vec3 Torque = glm::vec3(0.0f);

        // Helpers
        void AddForce(const glm::vec3& force) { Force += force; }
        void AddTorque(const glm::vec3& torque) { Torque += torque; }
        float GetInverseMass() const { return (Type == BodyType::Dynamic && Mass > 0.0f) ? 1.0f / Mass : 0.0f; }
    };

    enum class ColliderType {
        Box,
        Sphere,
        Capsule
    };

    struct BoxCollider {
        glm::vec3 HalfExtents = glm::vec3(0.5f); // Half-width, half-height, half-depth
        glm::vec3 Offset = glm::vec3(0.0f);
    };

    struct SphereCollider {
        float Radius = 0.5f;
        glm::vec3 Offset = glm::vec3(0.0f);
    };

    struct CapsuleCollider {
        float Radius = 0.5f;
        float Height = 1.0f;
        glm::vec3 Offset = glm::vec3(0.0f);
    };

    struct ColliderComponent {
        ColliderType Type = ColliderType::Box;
        
        // Variant to hold specific shape data
        std::variant<BoxCollider, SphereCollider, CapsuleCollider> Shape;
        
        // Material properties
        float Friction = 0.5f;
        float Bounciness = 0.3f; // Restitution
        
        bool IsTrigger = false; // If true, generates events but no physical response

        // Constructors for convenience
        ColliderComponent() : Shape(BoxCollider{}) {}
        ColliderComponent(const BoxCollider& box) : Type(ColliderType::Box), Shape(box) {}
        ColliderComponent(const SphereCollider& sphere) : Type(ColliderType::Sphere), Shape(sphere) {}
        ColliderComponent(const CapsuleCollider& capsule) : Type(ColliderType::Capsule), Shape(capsule) {}
    };

} // namespace Yamen::ECS

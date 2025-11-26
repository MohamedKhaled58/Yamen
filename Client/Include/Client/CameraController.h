#pragma once

#include "ECS/ScriptableEntity.h"
#include "ECS/Components.h"
#include "Platform/Input.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Yamen::Client {

    /**
     * @brief Standard FPS Camera Controller (Y-Up)
     * 
     * Uses quaternions for robust rotation and correct coordinate system alignment.
     * Engine Coordinate System (Standard DirectX):
     * - Forward: +Z
     * - Right:   +X
     * - Up:      +Y
     */
    class CameraController : public ECS::ScriptableEntity {
    public:
        float MoveSpeed = 5.0f;
        float FastMoveSpeed = 10.0f;
        float LookSensitivity = 0.002f;
        
        void OnCreate() override {
            m_LastMouseX = 0.0f;
            m_LastMouseY = 0.0f;
            m_FirstMouse = true;
            
            if (HasComponent<ECS::TransformComponent>()) {
                auto& transform = GetComponent<ECS::TransformComponent>();
                // Standard Y-Up: Yaw around Y, Pitch around X
                glm::vec3 euler = glm::eulerAngles(transform.Rotation);
                m_Yaw = euler.y; 
                m_Pitch = euler.x;
            }
        }

        void OnUpdate(float deltaTime) override {
            auto& transform = GetComponent<ECS::TransformComponent>();
            
            // Calculate movement speed
            float speed = Platform::Input::IsKeyPressed(Platform::KeyCode::LeftShift) 
                ? FastMoveSpeed : MoveSpeed;
            speed *= deltaTime;

            // Mouse Look
            if (Platform::Input::IsMouseButtonPressed(Platform::MouseButton::Right)) {
                float mouseX, mouseY;
                Platform::Input::GetMousePosition(mouseX, mouseY);

                if (m_FirstMouse) {
                    m_LastMouseX = mouseX;
                    m_LastMouseY = mouseY;
                    m_FirstMouse = false;
                }

                float deltaX = mouseX - m_LastMouseX;
                float deltaY = mouseY - m_LastMouseY;
                m_LastMouseX = mouseX;
                m_LastMouseY = mouseY;

                // Yaw rotates around World Up (Y)
                // Pitch rotates around Local Right (X)
                m_Yaw -= deltaX * LookSensitivity; 
                m_Pitch -= deltaY * LookSensitivity;

                const float maxPitch = glm::radians(89.0f);
                if (m_Pitch > maxPitch) m_Pitch = maxPitch;
                if (m_Pitch < -maxPitch) m_Pitch = -maxPitch;
            } else {
                m_FirstMouse = true;
            }

            // Calculate Rotation Quaternion
            // 1. Yaw around World Up (+Y)
            glm::quat qYaw = glm::angleAxis(m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            // 2. Pitch around Local Right (+X)
            glm::quat qPitch = glm::angleAxis(m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            
            transform.Rotation = qYaw * qPitch;

            // Calculate Basis Vectors
            // Forward = Rotation * BaseForward (+Z)
            // Right   = Rotation * BaseRight   (+X)
            // Up      = Rotation * BaseUp      (+Y)
            glm::vec3 forward = transform.Rotation * glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 right   = transform.Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f); // World Up is Y

            // Movement Input (Standard WASD)
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::W))
                transform.Translation += forward * speed;
            
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::S))
                transform.Translation -= forward * speed;
            
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::A))
                transform.Translation -= right * speed;
            
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::D))
                transform.Translation += right * speed;
            
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::E))
                transform.Translation += up * speed;
            
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::Q))
                transform.Translation -= up * speed;
        }

    private:
        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;
        float m_Yaw = 0.0f;
        float m_Pitch = 0.0f;
    };

} // namespace Yamen::Client

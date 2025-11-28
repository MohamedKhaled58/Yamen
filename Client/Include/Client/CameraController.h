#pragma once

#include "ECS/ScriptableEntity.h"
#include "ECS/Components.h"
#include "Platform/Input.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Yamen::Client {

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
                glm::vec3 euler = glm::eulerAngles(transform.Rotation);
                m_Yaw = euler.y; 
                m_Pitch = euler.x;
            }
        }

        void OnUpdate(float deltaTime) override {
            // Only block camera when actually typing in a text field
            if (ImGui::GetCurrentContext()) {
                auto& io = ImGui::GetIO();
                if (io.WantTextInput) {
                    m_FirstMouse = true;
                    return;
                }
            }

            auto& transform = GetComponent<ECS::TransformComponent>();
            
            float speed = Platform::Input::IsKeyPressed(Platform::KeyCode::LeftShift) 
                ? FastMoveSpeed : MoveSpeed;

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

                m_Yaw += deltaX * LookSensitivity; 
                m_Pitch -= deltaY * LookSensitivity;

                const float maxPitch = glm::radians(89.0f);
                if (m_Pitch > maxPitch) m_Pitch = maxPitch;
                if (m_Pitch < -maxPitch) m_Pitch = -maxPitch;
            } else {
                m_FirstMouse = true;
            }

            // Calculate forward and right vectors
            glm::vec3 forward = glm::normalize(glm::vec3(
                glm::cos(m_Yaw) * glm::cos(m_Pitch),
                glm::sin(m_Pitch),
                glm::sin(m_Yaw) * glm::cos(m_Pitch)
            ));
            
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
            
            // Movement (W=forward, S=back, A=left, D=right)
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::W))
                transform.Translation += forward * speed * deltaTime;
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::S))
                transform.Translation -= forward * speed * deltaTime;
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::A))
                transform.Translation -= right * speed * deltaTime;
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::D))
                transform.Translation += right * speed * deltaTime;
            
            // Vertical movement
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::Space))
                transform.Translation.y += speed * deltaTime;
            if (Platform::Input::IsKeyPressed(Platform::KeyCode::LeftControl))
                transform.Translation.y -= speed * deltaTime;
            
            // Update rotation
            glm::quat qYaw = glm::angleAxis(m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat qPitch = glm::angleAxis(m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            transform.Rotation = qYaw * qPitch;
        }

    private:
        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;
        float m_Yaw = 0.0f;
        float m_Pitch = 0.0f;
    };

} // namespace Yamen::Client
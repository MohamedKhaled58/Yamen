#pragma once

#include "Core/Math/Math.h"
#include "ECS/Components.h"
#include "ECS/ScriptableEntity.h"
#include "Platform/Input.h"
#include <imgui.h>


namespace Yamen::Client {

using namespace Yamen::Core;

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
      auto &transform = GetComponent<ECS::TransformComponent>();
      vec3 euler = Math::ToEulerAngles(transform.Rotation);
      m_Yaw = euler.y;
      m_Pitch = euler.x;
    }
  }

  void OnUpdate(float deltaTime) override {
    // Only block camera when actually typing in a text field
    if (ImGui::GetCurrentContext()) {
      auto &io = ImGui::GetIO();
      if (io.WantTextInput) {
        m_FirstMouse = true;
        return;
      }
    }

    auto &transform = GetComponent<ECS::TransformComponent>();

    float speed = Platform::Input::IsKeyPressed(Platform::KeyCode::LeftShift)
                      ? FastMoveSpeed
                      : MoveSpeed;

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

      const float maxPitch = Math::Radians(89.0f);
      if (m_Pitch > maxPitch)
        m_Pitch = maxPitch;
      if (m_Pitch < -maxPitch)
        m_Pitch = -maxPitch;
    } else {
      m_FirstMouse = true;
    }

    // Calculate forward and right vectors
    vec3 forward = Math::Normalize(vec3(std::cos(m_Yaw) * std::cos(m_Pitch),
                                        std::sin(m_Pitch),
                                        std::sin(m_Yaw) * std::cos(m_Pitch)));

    vec3 right = Math::Normalize(Math::Cross(forward, vec3(0.0f, 1.0f, 0.0f)));

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
    quat qYaw = Math::AngleAxis(m_Yaw, vec3(0.0f, 1.0f, 0.0f));
    quat qPitch = Math::AngleAxis(m_Pitch, vec3(1.0f, 0.0f, 0.0f));
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
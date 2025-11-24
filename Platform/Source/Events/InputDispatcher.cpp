#include "Platform/Events/InputDispatcher.h"
#include "Platform/Input.h"
#include "Platform/Events/InputEvents.h"
#include <fmt/format.h>

namespace Yamen::Platform {

    InputDispatcher::InputDispatcher(EventDispatcher& dispatcher)
        : m_Dispatcher(dispatcher) {
        // Initialize with first mouse position
        Input::GetMousePosition(m_LastMouseX, m_LastMouseY);
    }

    void InputDispatcher::Update() {
        // --- Keyboard handling ---
        // Iterate over a reasonable range of key codes (0..350) and check state.
        for (int i = 0; i <= 350; ++i) {
            KeyCode key = static_cast<KeyCode>(i);
            bool isPressed = Input::IsKeyPressed(key);

            auto it = m_KeyState.find(key);
            if (it == m_KeyState.end()) {
                // First frame, just store state
                m_KeyState[key] = isPressed;
                continue;
            }

            if (isPressed != it->second) {
                // State changed -> fire event
                if (isPressed) {
                    KeyPressedEvent ev(key);
                    m_Dispatcher.Dispatch(ev);
                }
                else {
                    KeyReleasedEvent ev(key);
                    m_Dispatcher.Dispatch(ev);
                }
                it->second = isPressed;
            }
        }

        // --- Mouse button handling ---
        for (int i = 0; i <= 7; ++i) {
            MouseButton button = static_cast<MouseButton>(i);
            bool isPressed = Input::IsMouseButtonPressed(button);

            auto it = m_MouseState.find(button);
            if (it == m_MouseState.end()) {
                m_MouseState[button] = isPressed;
                continue;
            }

            if (isPressed != it->second) {
                if (isPressed) {
                    MouseButtonPressedEvent ev(button);
                    m_Dispatcher.Dispatch(ev);
                }
                else {
                    MouseButtonReleasedEvent ev(button);
                    m_Dispatcher.Dispatch(ev);
                }
                it->second = isPressed;
            }
        }

        // --- Mouse movement handling ---
        float mouseX = 0.0f;
        float mouseY = 0.0f;
        Input::GetMousePosition(mouseX, mouseY);

        if (mouseX != m_LastMouseX || mouseY != m_LastMouseY) {
            MouseMovedEvent ev(mouseX, mouseY);
            m_Dispatcher.Dispatch(ev);
            m_LastMouseX = mouseX;
            m_LastMouseY = mouseY;
        }
    }

} // namespace Yamen::Platform
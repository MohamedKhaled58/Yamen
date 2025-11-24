#pragma once

#include "Platform/Events/Event.h"
#include "Platform/Input.h"
#include <fmt/format.h>

namespace Yamen::Platform {

    /**
     * @brief Key pressed event
     */
    class KeyPressedEvent : public Event {
    public:
        explicit KeyPressedEvent(KeyCode key) : m_Key(key) {}

        KeyCode GetKey() const noexcept { return m_Key; }

        EventCategory GetCategory() const override { return EventCategory::Keyboard; }
        const char* GetName() const override { return "KeyPressed"; }
        std::string ToString() const override { return fmt::format("KeyPressedEvent: {}", static_cast<int>(m_Key)); }
    private:
        KeyCode m_Key;
    };

    /**
     * @brief Key released event
     */
    class KeyReleasedEvent : public Event {
    public:
        explicit KeyReleasedEvent(KeyCode key) : m_Key(key) {}
        KeyCode GetKey() const noexcept { return m_Key; }
        EventCategory GetCategory() const override { return EventCategory::Keyboard; }
        const char* GetName() const override { return "KeyReleased"; }
        std::string ToString() const override { return fmt::format("KeyReleasedEvent: {}", static_cast<int>(m_Key)); }
    private:
        KeyCode m_Key;
    };

    /**
     * @brief Mouse button pressed event
     */
    class MouseButtonPressedEvent : public Event {
    public:
        explicit MouseButtonPressedEvent(MouseButton button) : m_Button(button) {}
        MouseButton GetButton() const noexcept { return m_Button; }
        EventCategory GetCategory() const override { return EventCategory::MouseButton; }
        const char* GetName() const override { return "MouseButtonPressed"; }
        std::string ToString() const override { return fmt::format("MouseButtonPressedEvent: {}", static_cast<int>(m_Button)); }
    private:
        MouseButton m_Button;
    };

    /**
     * @brief Mouse button released event
     */
    class MouseButtonReleasedEvent : public Event {
    public:
        explicit MouseButtonReleasedEvent(MouseButton button) : m_Button(button) {}
        MouseButton GetButton() const noexcept { return m_Button; }
        EventCategory GetCategory() const override { return EventCategory::MouseButton; }
        const char* GetName() const override { return "MouseButtonReleased"; }
        std::string ToString() const override { return fmt::format("MouseButtonReleasedEvent: {}", static_cast<int>(m_Button)); }
    private:
        MouseButton m_Button;
    };

    /**
     * @brief Mouse moved event
     */
    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(float x, float y) : m_X(x), m_Y(y) {}
        float GetX() const noexcept { return m_X; }
        float GetY() const noexcept { return m_Y; }
        EventCategory GetCategory() const override { return EventCategory::Mouse; }
        const char* GetName() const override { return "MouseMoved"; }
        std::string ToString() const override { return fmt::format("MouseMovedEvent: ({}, {})", m_X, m_Y); }
    private:
        float m_X, m_Y;
    };

    /**
     * @brief Key typed event (for character input)
     */
    class KeyTypedEvent : public Event {
    public:
        explicit KeyTypedEvent(unsigned int keycode) : m_Keycode(keycode) {}
        unsigned int GetKeycode() const noexcept { return m_Keycode; }
        EventCategory GetCategory() const override { return EventCategory::Keyboard; }
        const char* GetName() const override { return "KeyTyped"; }
        std::string ToString() const override { return fmt::format("KeyTypedEvent: {}", m_Keycode); }
    private:
        unsigned int m_Keycode;
    };

    /**
     * @brief Mouse scrolled event
     */
    class MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}
        float GetXOffset() const noexcept { return m_XOffset; }
        float GetYOffset() const noexcept { return m_YOffset; }
        EventCategory GetCategory() const override { return EventCategory::Mouse; }
        const char* GetName() const override { return "MouseScrolled"; }
        std::string ToString() const override { return fmt::format("MouseScrolledEvent: ({}, {})", m_XOffset, m_YOffset); }
    private:
        float m_XOffset, m_YOffset;
    };

} // namespace Yamen::Platform


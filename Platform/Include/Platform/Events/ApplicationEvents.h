#pragma once

#include "Platform/Events/Event.h"
#include <fmt/format.h>

namespace Yamen::Platform {

/**
 * @brief Window resize event
 */
class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height) {}
    
    uint32_t GetWidth() const noexcept { return m_Width; }
    uint32_t GetHeight() const noexcept { return m_Height; }
    
    EventCategory GetCategory() const override { 
        return EventCategory::Window; 
    }
    
    const char* GetName() const override { 
        return "WindowResize"; 
    }
    
    std::string ToString() const override {
        return fmt::format("WindowResizeEvent: {}x{}", m_Width, m_Height);
    }

private:
    uint32_t m_Width, m_Height;
};

/**
 * @brief Window close event
 */
class WindowCloseEvent : public Event {
public:
    EventCategory GetCategory() const override { 
        return EventCategory::Window; 
    }
    
    const char* GetName() const override { 
        return "WindowClose"; 
    }
};

/**
 * @brief Window focus event
 */
class WindowFocusEvent : public Event {
public:
    explicit WindowFocusEvent(bool focused) : m_Focused(focused) {}
    
    bool IsFocused() const noexcept { return m_Focused; }
    
    EventCategory GetCategory() const override { 
        return EventCategory::Window; 
    }
    
    const char* GetName() const override { 
        return "WindowFocus"; 
    }

private:
    bool m_Focused;
};

/**
 * @brief Window move event
 */
class WindowMoveEvent : public Event {
public:
    WindowMoveEvent(int x, int y) : m_X(x), m_Y(y) {}
    
    int GetX() const noexcept { return m_X; }
    int GetY() const noexcept { return m_Y; }
    
    EventCategory GetCategory() const override { 
        return EventCategory::Window; 
    }
    
    const char* GetName() const override { 
        return "WindowMove"; 
    }

private:
    int m_X, m_Y;
};

/**
 * @brief Application tick event
 */
class AppTickEvent : public Event {
public:
    EventCategory GetCategory() const override { 
        return EventCategory::Application; 
    }
    
    const char* GetName() const override { 
        return "AppTick"; 
    }
};

/**
 * @brief Application update event
 */
class AppUpdateEvent : public Event {
public:
    explicit AppUpdateEvent(float deltaTime) : m_DeltaTime(deltaTime) {}
    
    float GetDeltaTime() const noexcept { return m_DeltaTime; }
    
    EventCategory GetCategory() const override { 
        return EventCategory::Application; 
    }
    
    const char* GetName() const override { 
        return "AppUpdate"; 
    }

private:
    float m_DeltaTime;
};

/**
 * @brief Application render event
 */
class AppRenderEvent : public Event {
public:
    EventCategory GetCategory() const override { 
        return EventCategory::Application; 
    }
    
    const char* GetName() const override { 
        return "AppRender"; 
    }
};

} // namespace Yamen::Platform

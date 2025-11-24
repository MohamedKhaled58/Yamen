#pragma once

#include "Platform/Events/Event.h"
#include <Windows.h>
#include <string>
#include <functional>
#include <memory>

namespace Yamen::Platform {

/**
 * @brief Window properties
 */
struct WindowProps {
    std::string title = "Yamen Engine";
    uint32_t width = 1280;
    uint32_t height = 720;
    bool vsync = true;
    bool resizable = true;
    bool fullscreen = false;
};

/**
 * @brief Win32 Window implementation
 * 
 * Manages window creation, message handling, and event dispatch.
 * Integrates with the event system for input and window events.
 */
class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;
    
    explicit Window(const WindowProps& props = WindowProps());
    ~Window();
    
    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    
    /**
     * @brief Update window (process messages)
     */
    void OnUpdate();
    
    /**
     * @brief Get window width
     */
    uint32_t GetWidth() const noexcept { return m_Data.width; }
    
    /**
     * @brief Get window height
     */
    uint32_t GetHeight() const noexcept { return m_Data.height; }
    
    /**
     * @brief Set event callback
     */
    void SetEventCallback(const EventCallbackFn& callback) { m_Data.eventCallback = callback; }
    
    /**
     * @brief Set VSync
     */
    void SetVSync(bool enabled);
    bool IsVSync() const noexcept { return m_Data.vsync; }
    
    /**
     * @brief Get native window handle
     */
    HWND GetNativeWindow() const noexcept { return m_Window; }
    void* GetNativeHandle() const noexcept { return static_cast<void*>(m_Window); }
    
    /**
     * @brief Check if window should close
     */
    bool ShouldClose() const noexcept { return m_ShouldClose; }
    
    /**
     * @brief Show/hide window
     */
    void Show();
    void Hide();
    
    /**
     * @brief Set window title
     */
    void SetTitle(const std::string& title);
    
    /**
     * @brief Set window size
     */
    void SetSize(uint32_t width, uint32_t height);
    
    /**
     * @brief Set fullscreen
     */
    void SetFullscreen(bool fullscreen);
    
    /**
     * @brief Get window position
     */
    void GetPosition(int& x, int& y) const;
    
    /**
     * @brief Set window position
     */
    void SetPosition(int x, int y);

private:
    void Init(const WindowProps& props);
    void Shutdown();
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    HWND m_Window = nullptr;
    HINSTANCE m_Instance = nullptr;
    bool m_ShouldClose = false;
    
    struct WindowData {
        std::string title;
        uint32_t width;
        uint32_t height;
        bool vsync;
        EventCallbackFn eventCallback;
    };
    
    WindowData m_Data;
    
    // Fullscreen state
    WINDOWPLACEMENT m_WindowPlacement{};
    DWORD m_WindowStyle = 0;
    bool m_Fullscreen = false;
};

} // namespace Yamen::Platform

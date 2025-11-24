#include "Platform/Window.h"
#include "Platform/Events/ApplicationEvents.h"
#include "Platform/Events/InputEvents.h"
#include "Core/Logging/Logger.h"
#include <windowsx.h>

namespace Yamen::Platform {

    static uint8_t s_WindowCount = 0;

    Window::Window(const WindowProps& props) {
        Init(props);
    }

    Window::~Window() {
        m_ShouldClose = true;
        Shutdown();
    }

    void Window::Init(const WindowProps& props) {
        m_Data.title = props.title;
        m_Data.width = props.width;
        m_Data.height = props.height;
        m_Data.vsync = props.vsync;

        YAMEN_CORE_INFO("Creating window {} ({}x{})", props.title, props.width, props.height);

        m_Instance = GetModuleHandle(nullptr);

        // Register window class
        if (s_WindowCount == 0) {
            WNDCLASSEXW wc = {};
            wc.cbSize = sizeof(WNDCLASSEXW);
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            wc.lpfnWndProc = WindowProc;
            wc.hInstance = m_Instance;
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
            wc.lpszClassName = L"YamenWindowClass";

            if (!RegisterClassExW(&wc)) {
                YAMEN_CORE_ERROR("Failed to register window class");
                return;
            }
        }

        // Calculate window size including borders
        DWORD style = WS_OVERLAPPEDWINDOW;
        if (!props.resizable) {
            style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        }

        // Save initial style for fullscreen toggle
        m_WindowStyle = style;

        RECT rect = { 0, 0, (LONG)props.width, (LONG)props.height };
        AdjustWindowRect(&rect, style, FALSE);

        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;

        // Center window on screen
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int windowX = (screenWidth - windowWidth) / 2;
        int windowY = (screenHeight - windowHeight) / 2;

        // Convert title to wide string
        int titleLen = MultiByteToWideChar(CP_UTF8, 0, props.title.c_str(), -1, nullptr, 0);
        std::wstring wideTitle(titleLen, 0);
        MultiByteToWideChar(CP_UTF8, 0, props.title.c_str(), -1, &wideTitle[0], titleLen);

        // Create window
        m_Window = CreateWindowExW(
            0,
            L"YamenWindowClass",
            wideTitle.c_str(),
            style,
            windowX, windowY,
            windowWidth, windowHeight,
            nullptr,
            nullptr,
            m_Instance,
            this
        );

        if (!m_Window) {
            YAMEN_CORE_ERROR("Failed to create window");
            return;
        }

        // Store window data pointer
        SetWindowLongPtr(m_Window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&m_Data));

        s_WindowCount++;

        Show();
    }

    void Window::Shutdown() {
        if (m_Window) {
            DestroyWindow(m_Window);
            m_Window = nullptr;
            s_WindowCount--;

            if (s_WindowCount == 0) {
                UnregisterClassW(L"YamenWindowClass", m_Instance);
            }
        }
    }

    void Window::OnUpdate() {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_ShouldClose = true;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void Window::SetVSync(bool enabled) {
        m_Data.vsync = enabled;
    }

    void Window::Show() {
        ShowWindow(m_Window, SW_SHOW);
        UpdateWindow(m_Window);
    }

    void Window::Hide() {
        ShowWindow(m_Window, SW_HIDE);
    }

    void Window::SetTitle(const std::string& title) {
        m_Data.title = title;
        int titleLen = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
        std::wstring wideTitle(titleLen, 0);
        MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wideTitle[0], titleLen);
        SetWindowTextW(m_Window, wideTitle.c_str());
    }

    void Window::SetSize(uint32_t width, uint32_t height) {
        m_Data.width = width;
        m_Data.height = height;

        RECT rect = { 0, 0, (LONG)width, (LONG)height };
        DWORD style = GetWindowLong(m_Window, GWL_STYLE);
        AdjustWindowRect(&rect, style, FALSE);

        SetWindowPos(m_Window, nullptr, 0, 0,
            rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOMOVE | SWP_NOZORDER);
    }

    void Window::SetFullscreen(bool fullscreen) {
        if (m_Fullscreen == fullscreen) {
            return;
        }

        m_Fullscreen = fullscreen;

        if (fullscreen) {
            // Save current window placement
            m_WindowPlacement.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(m_Window, &m_WindowPlacement);

            // Get monitor info
            HMONITOR monitor = MonitorFromWindow(m_Window, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mi = { sizeof(MONITORINFO) };
            GetMonitorInfo(monitor, &mi);

            // Set borderless fullscreen
            SetWindowLong(m_Window, GWL_STYLE, WS_POPUP | WS_VISIBLE);
            SetWindowPos(m_Window, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_FRAMECHANGED);
        }
        else {
            // Restore windowed mode
            SetWindowLong(m_Window, GWL_STYLE, m_WindowStyle);
            SetWindowPlacement(m_Window, &m_WindowPlacement);
            SetWindowPos(m_Window, nullptr, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }
    }

    void Window::GetPosition(int& x, int& y) const {
        RECT rect;
        GetWindowRect(m_Window, &rect);
        x = rect.left;
        y = rect.top;
    }

    void Window::SetPosition(int x, int y) {
        SetWindowPos(m_Window, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        WindowData* data = reinterpret_cast<WindowData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (msg) {
        case WM_CLOSE: {
            if (data && data->eventCallback) {
                WindowCloseEvent event;
                data->eventCallback(event);
            }
            DestroyWindow(hwnd);
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        case WM_SIZE: {
            if (data && data->eventCallback) {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                data->width = width;
                data->height = height;

                WindowResizeEvent event(width, height);
                data->eventCallback(event);
            }
            return 0;
        }

        case WM_SETFOCUS: {
            if (data && data->eventCallback) {
                WindowFocusEvent event(true);
                data->eventCallback(event);
            }
            return 0;
        }

        case WM_KILLFOCUS: {
            if (data && data->eventCallback) {
                WindowFocusEvent event(false);
                data->eventCallback(event);
            }
            return 0;
        }

        case WM_MOVE: {
            if (data && data->eventCallback) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                WindowMoveEvent event(x, y);
                data->eventCallback(event);
            }
            return 0;
        }

                    // ❌ Input events commented out (using polling instead)
                    /*
                    case WM_KEYDOWN:
                    case WM_SYSKEYDOWN: {
                        if (data && data->eventCallback) {
                            KeyCode key = static_cast<KeyCode>(wParam);
                            KeyPressedEvent event(key);
                            data->eventCallback(event);
                        }
                        return 0;
                    }

                    case WM_KEYUP:
                    case WM_SYSKEYUP: {
                        if (data && data->eventCallback) {
                            KeyCode key = static_cast<KeyCode>(wParam);
                            KeyReleasedEvent event(key);
                            data->eventCallback(event);
                        }
                        return 0;
                    }

                    case WM_CHAR: {
                        return 0;
                    }

                    case WM_MOUSEMOVE: {
                        if (data && data->eventCallback) {
                            float x = static_cast<float>(GET_X_LPARAM(lParam));
                            float y = static_cast<float>(GET_Y_LPARAM(lParam));
                            MouseMovedEvent event(x, y);
                            data->eventCallback(event);
                        }
                        return 0;
                    }

                    case WM_MOUSEWHEEL: {
                        if (data && data->eventCallback) {
                            float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
                            // MouseScrolledEvent event(0.0f, delta);
                            // data->eventCallback(event);
                        }
                        return 0;
                    }

                    case WM_LBUTTONDOWN: {
                        if (data && data->eventCallback) {
                            MouseButtonPressedEvent event(MouseButton::Left);
                            data->eventCallback(event);
                        }
                        return 0;
                    }

                    case WM_LBUTTONUP: {
                        if (data && data->eventCallback) {
                            MouseButtonReleasedEvent event(MouseButton::Left);
                            data->eventCallback(event);
                        }
                        return 0;
                    }

                    case WM_RBUTTONDOWN: {
                        if (data && data->eventCallback) {
                            MouseButtonPressedEvent event(MouseButton::Right);
                            data->eventCallback(event);
                        }
                        return 0;
                    }

                    case WM_RBUTTONUP: {
                        if (data && data->eventCallback) {
                            MouseButtonReleasedEvent event(MouseButton::Right);
                            data->eventCallback(event);
                        }
                        return 0;
                    }

                    case WM_MBUTTONDOWN: {
                        if (data && data->eventCallback) {
                            MouseButtonPressedEvent event(MouseButton::Middle);
                            data->eventCallback(event);
                        }
                        return 0;
                    }

                    case WM_MBUTTONUP: {
                        if (data && data->eventCallback) {
                            MouseButtonReleasedEvent event(MouseButton::Middle);
                            data->eventCallback(event);
                        }
                        return 0;
                    }
                    */
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
} // namespace Yamen::Platform
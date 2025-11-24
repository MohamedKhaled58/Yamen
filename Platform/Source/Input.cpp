#include "Platform/Input.h"
#include <Windows.h>

namespace Yamen::Platform {

    bool Input::IsKeyPressed(KeyCode key) {
        // GetAsyncKeyState returns high-order bit if key is down
        return (GetAsyncKeyState(static_cast<int>(key)) & 0x8000) != 0;
    }

    bool Input::IsMouseButtonPressed(MouseButton button) {
        int vkCode = 0;
        switch (button) {
        case MouseButton::Left:   vkCode = VK_LBUTTON; break;
        case MouseButton::Right:  vkCode = VK_RBUTTON; break;
        case MouseButton::Middle: vkCode = VK_MBUTTON; break;
        case MouseButton::Button4: vkCode = VK_XBUTTON1; break;
        case MouseButton::Button5: vkCode = VK_XBUTTON2; break;
        default: return false;
        }

        return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
    }


    void Input::GetMousePosition(float& x, float& y) {
        POINT point;
        GetCursorPos(&point);

        // ✅ Get the active window
        HWND hwnd = GetActiveWindow();
        if (!hwnd) {
            x = 0.0f;
            y = 0.0f;
            return;
        }

        ScreenToClient(hwnd, &point);

        //Get window client area
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        //Check if cursor is inside window
        if (point.x < 0 || point.x > clientRect.right ||
            point.y < 0 || point.y > clientRect.bottom) {
            // Cursor is outside window - return last known position or (0,0)
            x = 0.0f;
            y = 0.0f;
            return;
        }

        x = static_cast<float>(point.x);
        y = static_cast<float>(point.y);
    }
    void Input::SetMousePosition(float x, float y)
    {
        POINT point;
        point.x = static_cast<LONG>(x);
        point.y = static_cast<LONG>(y);
        // Get the active window
        HWND hwnd = GetActiveWindow();
        if (!hwnd) {
            return;
        }
        // Convert client coordinates to screen coordinates
        ClientToScreen(hwnd, &point);
		SetCursorPos(point.x, point.y);
    }
    float Input::GetMouseX() {
        float x, y;
        GetMousePosition(x, y);
        return x;
    }

    float Input::GetMouseY() {
        float x, y;
        GetMousePosition(x, y);
        return y;
    }

} // namespace Yamen::Platform
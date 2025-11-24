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
        x = static_cast<float>(point.x);
        y = static_cast<float>(point.y);
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
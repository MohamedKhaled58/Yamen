#pragma once

#include <cstdint>
#include <fmt/format.h>

namespace Yamen::Platform {

    /**
     * @brief Key codes (matches Windows VK_ codes for convenience)
     */
    enum class KeyCode : uint16_t {
        // Letters
        A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45, F = 0x46,
        G = 0x47, H = 0x48, I = 0x49, J = 0x4A, K = 0x4B, L = 0x4C,
        M = 0x4D, N = 0x4E, O = 0x4F, P = 0x50, Q = 0x51, R = 0x52,
        S = 0x53, T = 0x54, U = 0x55, V = 0x56, W = 0x57, X = 0x58,
        Y = 0x59, Z = 0x5A,

        // Numbers
        Num0 = 0x30, Num1 = 0x31, Num2 = 0x32, Num3 = 0x33, Num4 = 0x34,
        Num5 = 0x35, Num6 = 0x36, Num7 = 0x37, Num8 = 0x38, Num9 = 0x39,

        // Function keys
        F1 = 0x70, F2 = 0x71, F3 = 0x72, F4 = 0x73, F5 = 0x74, F6 = 0x75,
        F7 = 0x76, F8 = 0x77, F9 = 0x78, F10 = 0x79, F11 = 0x7A, F12 = 0x7B,

        // Special keys
        Escape = 0x1B,
        Tab = 0x09,
        CapsLock = 0x14,
        LeftShift = 0xA0,
        RightShift = 0xA1,
        LeftControl = 0xA2,
        RightControl = 0xA3,
        LeftAlt = 0xA4,
        RightAlt = 0xA5,
        Space = 0x20,
        Enter = 0x0D,
        Backspace = 0x08,
        Delete = 0x2E,
        Insert = 0x2D,
        Home = 0x24,
        End = 0x23,
        PageUp = 0x21,
        PageDown = 0x22,

        // Arrow keys
        Left = 0x25,
        Up = 0x26,
        Right = 0x27,
        Down = 0x28,

        // Numpad
        Numpad0 = 0x60, Numpad1 = 0x61, Numpad2 = 0x62, Numpad3 = 0x63, Numpad4 = 0x64,
        Numpad5 = 0x65, Numpad6 = 0x66, Numpad7 = 0x67, Numpad8 = 0x68, Numpad9 = 0x69,
        NumpadMultiply = 0x6A,
        NumpadAdd = 0x6B,
        NumpadSubtract = 0x6D,
        NumpadDecimal = 0x6E,
        NumpadDivide = 0x6F,

        // Symbols
        Semicolon = 0xBA,
        Equal = 0xBB,
        Comma = 0xBC,
        Minus = 0xBD,
        Period = 0xBE,
        Slash = 0xBF,
        Grave = 0xC0,
        LeftBracket = 0xDB,
        Backslash = 0xDC,
        RightBracket = 0xDD,
        Apostrophe = 0xDE
    };

    /**
     * @brief Mouse button codes
     */
    enum class MouseButton : uint8_t {
        Left = 0,
        Right = 1,
        Middle = 2,
        Button4 = 3,
        Button5 = 4
    };

    /**
     * @brief Input state query (polling-based)
     */
    class Input {
    public:
        /**
         * @brief Check if key is currently pressed
         */
        static bool IsKeyPressed(KeyCode key);

        /**
         * @brief Check if mouse button is currently pressed
         */
        static bool IsMouseButtonPressed(MouseButton button);

        /**
         * @brief Get current mouse position
         * @param x Output parameter for X position
         * @param y Output parameter for Y position
         */
        static void GetMousePosition(float& x, float& y);
        static void SetMousePosition(float x, float y);


        /**
         * @brief Get mouse X position
         */
        static float GetMouseX();

        /**
         * @brief Get mouse Y position
         */
        static float GetMouseY();
    };




} // namespace Yamen::Platform

template <>
struct fmt::formatter<Yamen::Platform::KeyCode> : fmt::formatter<int> {
    auto format(Yamen::Platform::KeyCode key, format_context& ctx) const {
        return fmt::formatter<int>::format(static_cast<int>(key), ctx);
    }
};
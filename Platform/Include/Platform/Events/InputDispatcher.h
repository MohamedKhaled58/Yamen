#pragma once
#include "Platform/Events/Event.h"
#include "Platform/Events/InputEvents.h"
#include "Platform/Input.h"

#include <unordered_map>

namespace Yamen::Platform {

    /**
     * @brief Dispatches input polling results as events.
     *
     * Call `InputDispatcher::Update()` once per frame (e.g. at the start of the main loop).
     * It will compare the current input state with the previous frame and fire
     *   * KeyPressedEvent / KeyReleasedEvent
     *   * MouseButtonPressedEvent / MouseButtonReleasedEvent
     *   * MouseMovedEvent
     *
     * The dispatcher uses the global `EventDispatcher` instance (you can pass your own
     * dispatcher if you need a separate one).
     */
    class InputDispatcher {
    public:
        explicit InputDispatcher(EventDispatcher& dispatcher);
        ~InputDispatcher() = default;

        /**
         * @brief Poll input and fire events for any state changes.
         */
        void Update();

    private:
        EventDispatcher& m_Dispatcher;
        std::unordered_map<KeyCode, bool> m_KeyState;
        std::unordered_map<MouseButton, bool> m_MouseState;
        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
    };

} // namespace Yamen::Platform

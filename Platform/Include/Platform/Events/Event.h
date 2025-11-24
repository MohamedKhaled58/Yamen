#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <memory>

namespace Yamen::Platform {

/**
 * @brief Event categories for filtering
 */
enum class EventCategory : uint32_t {
    None        = 0,
    Application = 1 << 0,
    Input       = 1 << 1,
    Keyboard    = 1 << 2,
    Mouse       = 1 << 3,
    MouseButton = 1 << 4,
    Window      = 1 << 5,
    Network     = 1 << 6,
    Gameplay    = 1 << 7,
    UI          = 1 << 8,
    Custom      = 1 << 9
};

/**
 * @brief Event phases (like DOM events)
 */
enum class EventPhase {
    Capturing,  // Top-down
    AtTarget,   // At the target
    Bubbling    // Bottom-up
};

/**
 * @brief Base event class
 * 
 * All events derive from this class.
 * Events are type-safe and support priority-based dispatch.
 */
class Event {
public:
    virtual ~Event() = default;
    
    /**
     * @brief Get event category
     */
    virtual EventCategory GetCategory() const = 0;
    
    /**
     * @brief Get event name (for debugging)
     */
    virtual const char* GetName() const = 0;
    
    /**
     * @brief Get string representation
     */
    virtual std::string ToString() const { return GetName(); }
    
    /**
     * @brief Check if event has been handled
     */
    bool IsHandled() const noexcept { return m_Handled; }
    
    /**
     * @brief Mark event as handled
     */
    void SetHandled(bool handled = true) noexcept { m_Handled = handled; }
    
    /**
     * @brief Get current event phase
     */
    EventPhase GetPhase() const noexcept { return m_Phase; }
    
    /**
     * @brief Set event phase (used by dispatcher)
     */
    void SetPhase(EventPhase phase) noexcept { m_Phase = phase; }
    
    /**
     * @brief Get event priority
     */
    int GetPriority() const noexcept { return m_Priority; }
    
    /**
     * @brief Set event priority
     */
    void SetPriority(int priority) noexcept { m_Priority = priority; }
    
    /**
     * @brief Cast to derived event type
     */
    template<typename T>
    T& As() { return static_cast<T&>(*this); }
    
    /**
     * @brief Cast to derived event type (const)
     */
    template<typename T>
    const T& As() const { return static_cast<const T&>(*this); }

private:
    bool m_Handled = false;
    EventPhase m_Phase = EventPhase::AtTarget;
    int m_Priority = 0;
};

/**
 * @brief Event dispatcher with priority queues
 * 
 * Manages event listeners and dispatches events.
 * Thread-safe and supports priority-based dispatch.
 */
class EventDispatcher {
public:
    using EventCallback = std::function<bool(Event&)>;
    using EventFilter = std::function<bool(const Event&)>;
    
    /**
     * @brief Handle to a registered listener
     */
    struct ListenerHandle {
        uint64_t id;
        EventCategory category;
        int priority;
        
        bool operator==(const ListenerHandle& other) const noexcept {
            return id == other.id;
        }
    };
    
    EventDispatcher();
    ~EventDispatcher();
    
    // Non-copyable
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;
    
    /**
     * @brief Register event listener with priority
     * @param category Event category to listen for
     * @param callback Function to call when event is dispatched
     * @param priority Higher priority listeners are called first
     * @param filter Optional filter function
     * @return Handle to the listener (for removal)
     */
    ListenerHandle AddListener(
        EventCategory category,
        EventCallback callback,
        int priority = 0,
        EventFilter filter = nullptr
    );
    
    /**
     * @brief Remove event listener
     * @param handle Handle returned from AddListener
     */
    void RemoveListener(ListenerHandle handle);
    
    /**
     * @brief Dispatch event immediately
     * @param event Event to dispatch
     */
    void Dispatch(Event& event);
    
    /**
     * @brief Queue event for later dispatch
     * @param event Event to queue (takes ownership)
     */
    void QueueEvent(std::unique_ptr<Event> event);
    
    /**
     * @brief Process all queued events
     */
    void ProcessEvents();
    
    /**
     * @brief Clear all listeners
     */
    void Clear();
    
    /**
     * @brief Get number of registered listeners
     */
    size_t GetListenerCount() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};

/**
 * @brief Scoped event listener (RAII)
 * 
 * Automatically removes listener when destroyed.
 * Perfect for temporary event handling.
 */
class ScopedEventListener {
public:
    ScopedEventListener(
        EventDispatcher& dispatcher,
        EventCategory category,
        EventDispatcher::EventCallback callback,
        int priority = 0
    );
    
    ~ScopedEventListener();
    
    // Non-copyable
    ScopedEventListener(const ScopedEventListener&) = delete;
    ScopedEventListener& operator=(const ScopedEventListener&) = delete;
    
    // Movable
    ScopedEventListener(ScopedEventListener&& other) noexcept;
    ScopedEventListener& operator=(ScopedEventListener&& other) noexcept;

private:
    EventDispatcher* m_Dispatcher = nullptr;
    EventDispatcher::ListenerHandle m_Handle{};
};

} // namespace Yamen::Platform

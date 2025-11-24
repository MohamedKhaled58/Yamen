#include "Platform/Events/Event.h"
#include "Core/Logging/Logger.h"
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <algorithm>

namespace Yamen::Platform {

// EventDispatcher implementation
struct EventDispatcher::Impl {
    struct Listener {
        uint64_t id;
        EventCallback callback;
        EventFilter filter;
        int priority;
        bool active = true;
        
        bool operator<(const Listener& other) const {
            return priority > other.priority; // Higher priority first
        }
    };
    
    std::unordered_map<EventCategory, std::vector<Listener>> listeners;
    std::priority_queue<std::unique_ptr<Event>> eventQueue;
    std::mutex mutex;
    uint64_t nextId = 1;
};

EventDispatcher::EventDispatcher()
    : m_Impl(std::make_unique<Impl>()) {
}

EventDispatcher::~EventDispatcher() = default;

EventDispatcher::ListenerHandle EventDispatcher::AddListener(
    EventCategory category,
    EventCallback callback,
    int priority,
    EventFilter filter
) {
    std::lock_guard lock(m_Impl->mutex);
    
    Impl::Listener listener{};
    listener.id = m_Impl->nextId++;
    listener.callback = std::move(callback);
    listener.filter = std::move(filter);
    listener.priority = priority;
    listener.active = true;
    
    auto& categoryListeners = m_Impl->listeners[category];
    categoryListeners.push_back(std::move(listener));
    
    // Sort by priority
    std::sort(categoryListeners.begin(), categoryListeners.end());
    
    return ListenerHandle{ listener.id, category, priority };
}

void EventDispatcher::RemoveListener(ListenerHandle handle) {
    std::lock_guard lock(m_Impl->mutex);
    
    auto it = m_Impl->listeners.find(handle.category);
    if (it == m_Impl->listeners.end()) {
        return;
    }
    
    auto& categoryListeners = it->second;
    categoryListeners.erase(
        std::remove_if(categoryListeners.begin(), categoryListeners.end(),
            [id = handle.id](const Impl::Listener& listener) {
                return listener.id == id;
            }),
        categoryListeners.end()
    );
}

void EventDispatcher::Dispatch(Event& event) {
    std::lock_guard lock(m_Impl->mutex);
    
    auto it = m_Impl->listeners.find(event.GetCategory());
    if (it == m_Impl->listeners.end()) {
        return;
    }
    
    for (auto& listener : it->second) {
        if (!listener.active) {
            continue;
        }
        
        // Apply filter if present
        if (listener.filter && !listener.filter(event)) {
            continue;
        }
        
        // Call callback
        if (listener.callback(event)) {
            event.SetHandled(true);
            break; // Event was handled, stop propagation
        }
        
        // Check if event was handled by callback
        if (event.IsHandled()) {
            break;
        }
    }
}

void EventDispatcher::QueueEvent(std::unique_ptr<Event> event) {
    std::lock_guard lock(m_Impl->mutex);
    m_Impl->eventQueue.push(std::move(event));
}

void EventDispatcher::ProcessEvents() {
    std::lock_guard lock(m_Impl->mutex);
    
    while (!m_Impl->eventQueue.empty()) {
        auto event = std::move(const_cast<std::unique_ptr<Event>&>(m_Impl->eventQueue.top()));
        m_Impl->eventQueue.pop();
        
        // Dispatch without lock (we already have it)
        auto it = m_Impl->listeners.find(event->GetCategory());
        if (it != m_Impl->listeners.end()) {
            for (auto& listener : it->second) {
                if (!listener.active) {
                    continue;
                }
                
                if (listener.filter && !listener.filter(*event)) {
                    continue;
                }
                
                if (listener.callback(*event)) {
                    event->SetHandled(true);
                    break;
                }
                
                if (event->IsHandled()) {
                    break;
                }
            }
        }
    }
}

void EventDispatcher::Clear() {
    std::lock_guard lock(m_Impl->mutex);
    m_Impl->listeners.clear();
}

size_t EventDispatcher::GetListenerCount() const noexcept {
    std::lock_guard lock(m_Impl->mutex);
    size_t count = 0;
    for (const auto& [category, listeners] : m_Impl->listeners) {
        count += listeners.size();
    }
    return count;
}

// ScopedEventListener implementation
ScopedEventListener::ScopedEventListener(
    EventDispatcher& dispatcher,
    EventCategory category,
    EventDispatcher::EventCallback callback,
    int priority
) : m_Dispatcher(&dispatcher) {
    m_Handle = m_Dispatcher->AddListener(category, std::move(callback), priority);
}

ScopedEventListener::~ScopedEventListener() {
    if (m_Dispatcher) {
        m_Dispatcher->RemoveListener(m_Handle);
    }
}

ScopedEventListener::ScopedEventListener(ScopedEventListener&& other) noexcept
    : m_Dispatcher(other.m_Dispatcher)
    , m_Handle(other.m_Handle) {
    other.m_Dispatcher = nullptr;
}

ScopedEventListener& ScopedEventListener::operator=(ScopedEventListener&& other) noexcept {
    if (this != &other) {
        if (m_Dispatcher) {
            m_Dispatcher->RemoveListener(m_Handle);
        }
        
        m_Dispatcher = other.m_Dispatcher;
        m_Handle = other.m_Handle;
        
        other.m_Dispatcher = nullptr;
    }
    return *this;
}

} // namespace Yamen::Platform

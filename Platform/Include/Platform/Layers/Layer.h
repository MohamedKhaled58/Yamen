#pragma once

#include "Platform/Events/Event.h"
#include <string>
#include <memory>

namespace Yamen::Platform {

/**
 * @brief Layer base class
 * 
 * Layers are isolated execution contexts with their own update/render cycles.
 * They can be enabled/disabled independently and have priority-based ordering.
 * No state collision between layers.
 */
class Layer {
public:
    explicit Layer(std::string name = "Layer");
    virtual ~Layer() = default;
    
    // Lifecycle
    virtual void OnAttach() {}
    virtual void OnDetach() {}
    
    // Update cycle
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnFixedUpdate(float fixedDeltaTime) {}
    virtual void OnLateUpdate(float deltaTime) {}
    
    // Render cycle
    virtual void OnRender() {}
    virtual void OnImGuiRender() {}
    
    // Event handling
    virtual void OnEvent(Event& event) {}
    
    // State
    bool IsEnabled() const noexcept { return m_Enabled; }
    void SetEnabled(bool enabled) noexcept { m_Enabled = enabled; }
    
    const std::string& GetName() const noexcept { return m_Name; }
    
    int GetPriority() const noexcept { return m_Priority; }
    void SetPriority(int priority) noexcept { m_Priority = priority; }

protected:
    std::string m_Name;
    bool m_Enabled = true;
    int m_Priority = 0;
};

/**
 * @brief Layer stack manages all layers
 * 
 * Maintains ordered list of layers and overlays.
 * Handles update/render cycles and event propagation.
 */
class LayerStack {
public:
    LayerStack();
    ~LayerStack();
    
    /**
     * @brief Push layer to the stack
     * Layers are inserted before overlays
     */
    void PushLayer(std::unique_ptr<Layer> layer);
    
    /**
     * @brief Push overlay to the stack
     * Overlays are always on top
     */
    void PushOverlay(std::unique_ptr<Layer> overlay);
    
    /**
     * @brief Pop layer from the stack
     */
    void PopLayer(Layer* layer);
    
    /**
     * @brief Pop overlay from the stack
     */
    void PopOverlay(Layer* overlay);
    
    /**
     * @brief Get layer by name
     */
    Layer* GetLayer(const std::string& name);
    const Layer* GetLayer(const std::string& name) const;
    
    /**
     * @brief Update all enabled layers
     */
    void OnUpdate(float deltaTime);
    void OnFixedUpdate(float fixedDeltaTime);
    void OnLateUpdate(float deltaTime);
    
    /**
     * @brief Render all enabled layers
     */
    void OnRender();
    void OnImGuiRender();
    
    /**
     * @brief Propagate event through layers
     * Events propagate from top to bottom, stopping if handled
     */
    void OnEvent(Event& event);
    
    /**
     * @brief Get number of layers
     */
    size_t GetLayerCount() const noexcept;
    
    // Iteration
    auto begin() { return m_Layers.begin(); }
    auto end() { return m_Layers.end(); }
    auto rbegin() { return m_Layers.rbegin(); }
    auto rend() { return m_Layers.rend(); }

private:
    std::vector<std::unique_ptr<Layer>> m_Layers;
    size_t m_LayerInsertIndex = 0;
};

} // namespace Yamen::Platform

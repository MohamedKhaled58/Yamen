#include "Platform/Layers/Layer.h"
#include "Core/Logging/Logger.h"
#include <algorithm>

namespace Yamen::Platform {

// Layer implementation
Layer::Layer(std::string name)
    : m_Name(std::move(name)) {
}

// LayerStack implementation
LayerStack::LayerStack() = default;

LayerStack::~LayerStack() {
    for (auto& layer : m_Layers) {
        layer->OnDetach();
    }
    m_Layers.clear();
}

void LayerStack::PushLayer(std::unique_ptr<Layer> layer) {
    YAMEN_CORE_INFO("Pushing layer: {}", layer->GetName());
    
    layer->OnAttach();
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, std::move(layer));
    m_LayerInsertIndex++;
}

void LayerStack::PushOverlay(std::unique_ptr<Layer> overlay) {
    YAMEN_CORE_INFO("Pushing overlay: {}", overlay->GetName());
    
    overlay->OnAttach();
    m_Layers.emplace_back(std::move(overlay));
}

void LayerStack::PopLayer(Layer* layer) {
    auto it = std::find_if(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex,
        [layer](const std::unique_ptr<Layer>& l) { return l.get() == layer; });
    
    if (it != m_Layers.begin() + m_LayerInsertIndex) {
        YAMEN_CORE_INFO("Popping layer: {}", (*it)->GetName());
        (*it)->OnDetach();
        m_Layers.erase(it);
        m_LayerInsertIndex--;
    }
}

void LayerStack::PopOverlay(Layer* overlay) {
    auto it = std::find_if(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(),
        [overlay](const std::unique_ptr<Layer>& l) { return l.get() == overlay; });
    
    if (it != m_Layers.end()) {
        YAMEN_CORE_INFO("Popping overlay: {}", (*it)->GetName());
        (*it)->OnDetach();
        m_Layers.erase(it);
    }
}

Layer* LayerStack::GetLayer(const std::string& name) {
    auto it = std::find_if(m_Layers.begin(), m_Layers.end(),
        [&name](const std::unique_ptr<Layer>& layer) {
            return layer->GetName() == name;
        });
    
    return it != m_Layers.end() ? it->get() : nullptr;
}

const Layer* LayerStack::GetLayer(const std::string& name) const {
    auto it = std::find_if(m_Layers.begin(), m_Layers.end(),
        [&name](const std::unique_ptr<Layer>& layer) {
            return layer->GetName() == name;
        });
    
    return it != m_Layers.end() ? it->get() : nullptr;
}

void LayerStack::OnUpdate(float deltaTime) {
    for (auto& layer : m_Layers) {
        if (layer->IsEnabled()) {
            layer->OnUpdate(deltaTime);
        }
    }
}

void LayerStack::OnFixedUpdate(float fixedDeltaTime) {
    for (auto& layer : m_Layers) {
        if (layer->IsEnabled()) {
            layer->OnFixedUpdate(fixedDeltaTime);
        }
    }
}

void LayerStack::OnLateUpdate(float deltaTime) {
    for (auto& layer : m_Layers) {
        if (layer->IsEnabled()) {
            layer->OnLateUpdate(deltaTime);
        }
    }
}

void LayerStack::OnRender() {
    for (auto& layer : m_Layers) {
        if (layer->IsEnabled()) {
            layer->OnRender();
        }
    }
}

void LayerStack::OnImGuiRender() {
    for (auto& layer : m_Layers) {
        if (layer->IsEnabled()) {
            layer->OnImGuiRender();
        }
    }
}

void LayerStack::OnEvent(Event& event) {
    // Propagate from top to bottom (reverse order)
    for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
        if (event.IsHandled()) {
            break;
        }
        
        if ((*it)->IsEnabled()) {
            (*it)->OnEvent(event);
        }
    }
}

size_t LayerStack::GetLayerCount() const noexcept {
    return m_Layers.size();
}

} // namespace Yamen::Platform

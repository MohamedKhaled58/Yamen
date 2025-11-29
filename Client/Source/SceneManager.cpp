#include "Client/SceneManager.h"
#include "Client/Application.h"

namespace Yamen::Client {

    SceneManager::SceneManager(Graphics::GraphicsDevice& device)
        : m_Device(device)
    {
        YAMEN_CLIENT_INFO("SceneManager initialized");
    }

    SceneManager::~SceneManager() {
        if (m_ActiveScene) {
            m_ActiveScene->OnDeactivate();
        }
        YAMEN_CLIENT_INFO("SceneManager shutdown");
    }

    void SceneManager::RegisterScene(const std::string& name, SceneFactory factory) {
        m_SceneFactories[name] = factory;
        YAMEN_CLIENT_INFO("Registered scene: {}", name);
    }

    bool SceneManager::LoadScene(const std::string& name) {
        auto it = m_SceneFactories.find(name);
        if (it == m_SceneFactories.end()) {
            YAMEN_CLIENT_ERROR("Scene '{}' not found in registry", name);
            return false;
        }

        // Deactivate current scene
        if (m_ActiveScene) {
            YAMEN_CLIENT_INFO("Deactivating scene: {}", m_ActiveSceneName);
            m_ActiveScene->OnDeactivate();
            m_ActiveScene.reset();
        }

        // Create and initialize new scene
        YAMEN_CLIENT_INFO("Loading scene: {}", name);
        m_ActiveScene = it->second();
        
        if (!m_ActiveScene->Initialize()) {
            YAMEN_CLIENT_ERROR("Failed to initialize scene: {}", name);
            m_ActiveScene.reset();
            return false;
        }

        m_ActiveScene->OnActivate();
        m_ActiveSceneName = name;
        YAMEN_CLIENT_INFO("Scene '{}' loaded successfully", name);

        // Dispatch SceneChangedEvent
        //SceneChangedEvent event(name);
        //Application::Get().GetEventDispatcher().Dispatch(event);

        return true;
    }

    void SceneManager::Update(float deltaTime) {
        if (m_ActiveScene) {
            m_ActiveScene->Update(deltaTime);
        }
    }

    void SceneManager::Render() {
        if (m_ActiveScene) {
            m_ActiveScene->Render();
        }
    }

    void SceneManager::RenderImGui() {
        if (m_ActiveScene) {
            m_ActiveScene->RenderImGui();
        }
    }

} // namespace Yamen::Client

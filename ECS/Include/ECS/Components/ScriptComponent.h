#pragma once

#include "ECS/ScriptableEntity.h"
#include <functional>

namespace Yamen::ECS {

    /**
     * @brief Native script component for C++ gameplay logic
     * 
     * Allows attaching C++ scripts to entities for custom behavior.
     * Scripts are instantiated and destroyed automatically by the ScriptSystem.
     */
    struct NativeScriptComponent {
        ScriptableEntity* Instance = nullptr;

        std::function<void()> InstantiateScript;
        std::function<void()> DestroyScript;
        std::function<void(ScriptableEntity*)> OnCreateFunc;
        std::function<void(ScriptableEntity*, float)> OnUpdateFunc;
        std::function<void(ScriptableEntity*)> OnDestroyFunc;

        template<typename T>
        void Bind() {
            static_assert(std::is_base_of<ScriptableEntity, T>::value, "T must derive from ScriptableEntity");
            
            InstantiateScript = [this]() { 
                Instance = new T(); 
            };
            
            DestroyScript = [this]() { 
                delete Instance; 
                Instance = nullptr; 
            };
            
            OnCreateFunc = [](ScriptableEntity* instance) {
                static_cast<T*>(instance)->OnCreate();
            };
            
            OnUpdateFunc = [](ScriptableEntity* instance, float dt) {
                static_cast<T*>(instance)->OnUpdate(dt);
            };
            
            OnDestroyFunc = [](ScriptableEntity* instance) {
                static_cast<T*>(instance)->OnDestroy();
            };
        }
    };

} // namespace Yamen::ECS

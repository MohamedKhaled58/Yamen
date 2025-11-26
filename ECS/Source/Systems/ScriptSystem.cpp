#include "ECS/Systems/ScriptSystem.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include <Core/Logging/Logger.h>
#include <entt/entt.hpp>

namespace Yamen::ECS {

    void ScriptSystem::OnInit(Scene* scene) {
        if (!scene) return;

        // Instantiate all scripts
        auto view = scene->Registry().view<NativeScriptComponent>();
        for (auto entity : view) {
            auto& script = view.get<NativeScriptComponent>(entity);
            
            if (!script.Instance && script.InstantiateScript) {
                script.InstantiateScript();
                
                if (script.Instance) {
                    script.Instance->m_Entity = Entity{ entity, scene };
                    
                    if (script.OnCreateFunc) {
                        script.OnCreateFunc(script.Instance);
                    }
                }
            }
        }

        YAMEN_CORE_INFO("ScriptSystem initialized");
    }

    void ScriptSystem::OnUpdate(Scene* scene, float deltaTime) {
        if (!scene) return;

        auto view = scene->Registry().view<NativeScriptComponent>();
        
        for (auto entity : view) {
            auto& script = view.get<NativeScriptComponent>(entity);
            
            // Instantiate if not already
            if (!script.Instance && script.InstantiateScript) {
                script.InstantiateScript();
                
                if (script.Instance) {
                    script.Instance->m_Entity = Entity{ entity, scene };
                    
                    if (script.OnCreateFunc) {
                        script.OnCreateFunc(script.Instance);
                    }
                }
            }
            
            // Update script
            if (script.Instance && script.OnUpdateFunc) {
                script.OnUpdateFunc(script.Instance, deltaTime);
            }
        }
    }

    void ScriptSystem::OnShutdown(Scene* scene) {
        if (!scene) return;

        // Destroy all scripts
        auto view = scene->Registry().view<NativeScriptComponent>();
        for (auto entity : view) {
            auto& script = view.get<NativeScriptComponent>(entity);
            
            if (script.Instance) {
                if (script.OnDestroyFunc) {
                    script.OnDestroyFunc(script.Instance);
                }
                
                if (script.DestroyScript) {
                    script.DestroyScript();
                }
            }
        }

        YAMEN_CORE_INFO("ScriptSystem shutdown");
    }

} // namespace Yamen::ECS

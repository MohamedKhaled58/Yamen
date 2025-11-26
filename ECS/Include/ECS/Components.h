#pragma once

// Core components
#include "ECS/Components/CoreComponents.h"

// Rendering components
#include "ECS/Components/RenderingComponents.h"

// Script components
#include "ECS/Components/ScriptComponent.h"

// Forward declarations for entt
#include <entt/entt.hpp>

/**
 * @file Components.h
 * @brief Central include for all ECS components
 * 
 * Components are organized by domain:
 * - CoreComponents: Transform, Tag, Hierarchy
 * - RenderingComponents: Mesh, Sprite, Light, Camera
 * - ScriptComponent: Native script support
 * 
 * Future additions:
 * - PhysicsComponents: Rigidbody, Collider
 * - GameplayComponents: Player, NPC, Skill
 * - NetworkComponents: Network, Replication
 * - AnimationComponents: C3Animation, Skeleton
 */

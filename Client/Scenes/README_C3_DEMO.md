# C3 Animation Demo Scene

Complete example scene demonstrating skeletal animation with C3 models from Conquer Online.

## Overview

This demo loads and displays 8 different ghost character animations from `Assets/C3/ghost/085/`:

| File | Animation | Action Code |
|------|-----------|-------------|
| 1.c3 | Base Model | - |
| 100.c3 | **Standby** | _ACTION_STANDBY |
| 101.c3 | **Rest** | _ACTION_REST1 |
| 110.c3 | **Walk Left** | _ACTION_WALKL |
| 111.c3 | **Walk Right** | _ACTION_WALKR |
| 120.c3 | **Run Left** | _ACTION_RUNL |
| 121.c3 | **Run Right** | _ACTION_RUNR |
| 350.c3 | **Attack** | _ACTION_ATTACK0 |

## Features

✅ **Load multiple C3 files** with different animations  
✅ **Switch between animations** in real-time  
✅ **Play/Pause control** with adjustable speed  
✅ **Orbit camera** with zoom and rotation  
✅ **ImGui interface** for easy control  
✅ **Animation info display** (frames, bones, keyframes, format)

## Controls

### Keyboard

- **1-8**: Switch between animations
- **Space**: Pause/Play animation
- **Arrow Keys**: Rotate camera (Left/Right) and Zoom (Up/Down)

### ImGui Panel

- **Animation List**: Click to switch animations
- **Speed Slider**: Adjust playback speed (1-120 FPS)
- **Camera Controls**: Fine-tune distance, angle, and height

## Usage

### 1. Add to Your Application

```cpp
#include "Client/Scenes/C3AnimationDemoScene.h"

// In your application initialization:
auto demoScene = std::make_shared<C3AnimationDemoScene>();
sceneManager->PushScene(demoScene);
```

### 2. Integrate with Scene System

If you have a scene manager:

```cpp
// In GameLayer.cpp or similar
void GameLayer::OnAttach() {
    m_SceneManager = std::make_unique<SceneManager>();
    m_SceneManager->PushScene(std::make_shared<C3AnimationDemoScene>());
}

void GameLayer::OnUpdate(float deltaTime) {
    m_SceneManager->OnUpdate(deltaTime);
}

void GameLayer::OnRender() {
    m_SceneManager->OnRender();
}

void GameLayer::OnImGuiRender() {
    m_SceneManager->OnImGuiRender();
}
```

### 3. Standalone Usage

```cpp
// Create scene
C3AnimationDemoScene scene;

// Initialize
scene.OnAttach();

// Game loop
while (running) {
    float deltaTime = timer.GetDeltaTime();
    
    scene.OnUpdate(deltaTime);
    scene.OnRender();
    scene.OnImGuiRender();
}

// Cleanup
scene.OnDetach();
```

## What It Demonstrates

### C3 File Loading

```cpp
// Loads all 8 ghost animations
for (auto& model : m_GhostModels) {
    model.entity = C3ModelLoader::LoadModel(m_Registry, model.filepath);
}
```

### Animation Playback

```cpp
// Update all animations each frame
ECS::SkeletalAnimationSystem::Update(m_Registry, deltaTime);
```

### Animation Control

```cpp
// Switch animations
ECS::SkeletalAnimationSystem::Play(*anim, true);
ECS::SkeletalAnimationSystem::SetSpeed(*anim, 30.0f);
ECS::SkeletalAnimationSystem::Pause(*anim);
```

### Rendering

```cpp
// Render with skeletal renderer
glm::mat4 mvp = projection * view * model;
C3ModelLoader::RenderModel(entity, m_Registry, *m_SkeletalRenderer, mvp);
```

## Technical Details

### Animation Formats Supported

- **KKEY**: Full 4×4 matrices
- **XKEY**: Compressed 3×4 matrices
- **ZKEY**: Quaternion + translation
- **Legacy**: Per-frame matrices

### ECS Architecture

- **Components**: `SkeletalAnimationComponent`, `C3MeshComponent`
- **Systems**: `SkeletalAnimationSystem`
- **Registry**: `entt::registry`

### Rendering Pipeline

1. Load C3 file → Parse binary format
2. Create entity → Add components
3. Update system → Interpolate bones
4. Render → Apply bone matrices to shader

## Expected Output

When running the demo, you should see:

1. **3D View**: Animated ghost character
2. **ImGui Panel**:
   - List of 8 animations
   - Current animation info (frames, bones, format)
   - Playback controls
   - Camera controls
3. **Console**: Loading status and animation info

## Troubleshooting

### Models don't load

- Check file paths: `Assets/C3/ghost/085/*.c3`
- Verify files exist and are readable
- Check console for error messages

### No animation

- Ensure `SkeletalAnimationSystem::Update()` is called each frame
- Check if animation is paused (press Space)
- Verify animation speed is > 0

### Black screen

- Check if `C3SkeletalRenderer` is initialized
- Verify camera position and matrices
- Ensure shaders are compiled

## Next Steps

1. **Add texture loading** for `1.dds`
2. **Create vertex/index buffers** from PHY data
3. **Implement actual draw calls** in `RenderModel()`
4. **Add lighting** for better visuals
5. **Test with other C3 models** from Conquer Online

## File Structure

```
Client/
├── Include/Client/Scenes/
│   └── C3AnimationDemoScene.h
└── Source/Scenes/
    └── C3AnimationDemoScene.cpp

Assets/C3/ghost/085/
├── 1.c3        # Base model
├── 1.dds       # Texture
├── 100.c3      # Standby
├── 101.c3      # Rest
├── 110.c3      # Walk Left
├── 111.c3      # Walk Right
├── 120.c3      # Run Left
├── 121.c3      # Run Right
└── 350.c3      # Attack
```

## References

- **GUIDE**: `GUIDE/YAMEN_ENGINE_COMPLETE_GUIDE.md` - Section 5.5 (Action Types)
- **C3PhyLoader**: `AssetsC3/Include/AssetsC3/C3PhyLoader.h`
- **Animation System**: `ECS/Include/ECS/Systems/SkeletalAnimationSystem.h`
- **Model Loader**: `Client/Include/Client/C3ModelLoader.h`

---

**Status**: Ready to run! Build the project and launch the demo scene. 🎮

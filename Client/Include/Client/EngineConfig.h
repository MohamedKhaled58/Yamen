#pragma once

#include <string>
#include <vector>

namespace Yamen::Client {

    struct EngineConfig {
        // Window Settings
        std::string WindowTitle = "Yamen Engine";
        uint32_t WindowWidth = 1280;
        uint32_t WindowHeight = 720;
        bool VSync = true;
        bool Fullscreen = false;

        // Renderer Settings
        bool EnableMSAA = true;
        uint32_t MSAASamples = 4;

        // Asset Settings
        std::string AssetRoot = "Assets";

        // Scene Settings
        std::string StartScene = "ECS Scene";

        // Debug Settings
        bool EnableImGui = true;
        bool ShowConsole = true;
    };

} // namespace Yamen::Client

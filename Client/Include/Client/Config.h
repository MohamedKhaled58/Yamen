#pragma once

/**
 * @file Config.h
 * @brief Engine configuration and feature toggles
 */

namespace Yamen::Config {

    // ========================================
    // DEMO/TEST FEATURES
    // ========================================

    /**
     * @brief Enable demo scene for testing graphics features
     * 
     * Set to 0 to disable all demo/test code.
     * Set to 1 to enable demo scene in GameLayer.
     */
    #define ENABLE_DEMO_SCENE 1

    // ========================================
    // GRAPHICS SETTINGS
    // ========================================

    constexpr bool EnableVSync = true;
    constexpr bool EnableMSAA = false;
    constexpr int MSAASamples = 4;

    // ========================================
    // DEBUG SETTINGS
    // ========================================

    #ifdef _DEBUG
        constexpr bool EnableImGui = true;
        constexpr bool EnableGraphicsDebug = true;
    #else
        constexpr bool EnableImGui = false;
        constexpr bool EnableGraphicsDebug = false;
    #endif

} // namespace Yamen::Config

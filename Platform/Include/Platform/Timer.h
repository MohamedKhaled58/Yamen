#pragma once

#include <Windows.h>
#include <cstdint>

namespace Yamen::Platform {

/**
 * @brief High-resolution timer for accurate timing
 * 
 * Uses QueryPerformanceCounter for microsecond precision.
 */
class Timer {
public:
    Timer();
    
    /**
     * @brief Reset timer to zero
     */
    void Reset();
    
    /**
     * @brief Get elapsed time in seconds
     */
    float GetElapsedSeconds() const;
    
    /**
     * @brief Get elapsed time in milliseconds
     */
    float GetElapsedMilliseconds() const;
    
    /**
     * @brief Get elapsed time in microseconds
     */
    uint64_t GetElapsedMicroseconds() const;

private:
    LARGE_INTEGER m_StartTime;
    LARGE_INTEGER m_Frequency;
};

/**
 * @brief Frame timer for delta time calculation
 */
class FrameTimer {
public:
    FrameTimer();
    
    /**
     * @brief Update timer (call once per frame)
     * @return Delta time in seconds
     */
    float Update();
    
    /**
     * @brief Get last frame's delta time
     */
    float GetDeltaTime() const noexcept { return m_DeltaTime; }
    
    /**
     * @brief Get frames per second
     */
    float GetFPS() const noexcept { return m_FPS; }
    
    /**
     * @brief Get total elapsed time
     */
    float GetTotalTime() const noexcept { return m_TotalTime; }
    
    /**
     * @brief Get frame count
     */
    uint64_t GetFrameCount() const noexcept { return m_FrameCount; }

private:
    Timer m_Timer;
    float m_DeltaTime = 0.0f;
    float m_TotalTime = 0.0f;
    float m_FPS = 0.0f;
    uint64_t m_FrameCount = 0;
    float m_FPSTimer = 0.0f;
    uint32_t m_FPSFrameCount = 0;
};

} // namespace Yamen::Platform

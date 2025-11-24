#include "Platform/Timer.h"

namespace Yamen::Platform {

// Timer implementation
Timer::Timer() {
    QueryPerformanceFrequency(&m_Frequency);
    Reset();
}

void Timer::Reset() {
    QueryPerformanceCounter(&m_StartTime);
}

float Timer::GetElapsedSeconds() const {
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    
    uint64_t elapsed = currentTime.QuadPart - m_StartTime.QuadPart;
    return static_cast<float>(elapsed) / static_cast<float>(m_Frequency.QuadPart);
}

float Timer::GetElapsedMilliseconds() const {
    return GetElapsedSeconds() * 1000.0f;
}

uint64_t Timer::GetElapsedMicroseconds() const {
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    
    uint64_t elapsed = currentTime.QuadPart - m_StartTime.QuadPart;
    return (elapsed * 1000000) / m_Frequency.QuadPart;
}

// FrameTimer implementation
FrameTimer::FrameTimer() {
    m_Timer.Reset();
}

float FrameTimer::Update() {
    float currentTime = m_Timer.GetElapsedSeconds();
    m_DeltaTime = currentTime - m_TotalTime;
    m_TotalTime = currentTime;
    m_FrameCount++;
    
    // Calculate FPS
    m_FPSTimer += m_DeltaTime;
    m_FPSFrameCount++;
    
    if (m_FPSTimer >= 1.0f) {
        m_FPS = static_cast<float>(m_FPSFrameCount) / m_FPSTimer;
        m_FPSTimer = 0.0f;
        m_FPSFrameCount = 0;
    }
    
    return m_DeltaTime;
}

} // namespace Yamen::Platform

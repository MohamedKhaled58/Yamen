#include "Core/Threading/ThreadPool.h"
#include "Core/Logging/Logger.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#endif

namespace Yamen::Core {

    ThreadPool::ThreadPool(size_t numThreads, bool enableWorkStealing)
        : m_EnableWorkStealing(enableWorkStealing) {

        if (numThreads == 0) {
            numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0) numThreads = 4; // Fallback
        }

        YAMEN_CORE_INFO("ThreadPool: Starting {} worker threads (work stealing: {})",
            numThreads, enableWorkStealing);

        for (size_t i = 0; i < numThreads; ++i) {
            m_Workers.emplace_back([this, i] {
                WorkerThread(i);
                });
        }
    }

    ThreadPool::~ThreadPool() {
        YAMEN_CORE_INFO("ThreadPool: Shutting down...");

        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Stop = true;
        }

        m_Condition.notify_all();

        for (std::thread& worker : m_Workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }

        YAMEN_CORE_INFO("ThreadPool: Shutdown complete. Stats:");
        YAMEN_CORE_INFO("  - Tasks completed: {}", m_Stats.tasksCompleted.load());
        YAMEN_CORE_INFO("  - Tasks failed: {}", m_Stats.tasksFailed.load());
        YAMEN_CORE_INFO("  - Uptime: {:.2f}s", m_Stats.GetUptime());
        YAMEN_CORE_INFO("  - Avg tasks/sec: {:.2f}", m_Stats.GetTasksPerSecond());
    }

    void ThreadPool::WorkerThread(size_t threadId) {
        // Set thread name for debugging
        SetThreadName("Worker-" + std::to_string(threadId));

        YAMEN_CORE_TRACE("Worker thread {} started", threadId);

        for (;;) {
            TaskWrapper taskWrapper;

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);

                m_Condition.wait(lock, [this] {
                    return m_Stop || (!m_Paused && !m_Tasks.empty());
                    });

                if (m_Stop && m_Tasks.empty()) {
                    YAMEN_CORE_TRACE("Worker thread {} exiting", threadId);
                    return;
                }

                if (m_Tasks.empty()) {
                    continue;
                }

                taskWrapper = std::move(const_cast<TaskWrapper&>(m_Tasks.top()));
                m_Tasks.pop();

                m_ActiveTasks++;
                m_Stats.activeThreads++;
            }

            // Calculate queue wait time
            auto now = std::chrono::steady_clock::now();
            auto waitTime = std::chrono::duration<double, std::milli>(
                now - taskWrapper.enqueueTime
            ).count();

            if (waitTime > 100.0) { // Log if task waited >100ms
                YAMEN_CORE_WARN("Task waited {:.2f}ms in queue (priority: {})",
                    waitTime, static_cast<int>(taskWrapper.priority));
            }

            // Execute task with exception safety
            try {
                taskWrapper.task();
                m_Stats.tasksCompleted++;
            }
            catch (const std::exception& e) {
                YAMEN_CORE_ERROR("Worker {}: Task threw exception: {}", threadId, e.what());
                m_Stats.tasksFailed++;
            }
            catch (...) {
                YAMEN_CORE_ERROR("Worker {}: Task threw unknown exception", threadId);
                m_Stats.tasksFailed++;
            }

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_ActiveTasks--;
                m_Stats.activeThreads--;
            }

            m_WaitCondition.notify_all();
        }
    }

    void ThreadPool::SetThreadName(const std::string& name) {
#ifdef _WIN32
        // Windows: SetThreadDescription (Windows 10+ only)
        HANDLE handle = GetCurrentThread();
        std::wstring wname(name.begin(), name.end());
        SetThreadDescription(handle, wname.c_str());
#elif defined(__linux__)
        // Linux: pthread_setname_np
        pthread_setname_np(pthread_self(), name.c_str());
#elif defined(__APPLE__)
        // macOS: pthread_setname_np (different signature)
        pthread_setname_np(name.c_str());
#endif
    }

    bool ThreadPool::WaitForAll(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(m_QueueMutex);

        if (timeout.count() == 0) {
            // Wait indefinitely
            m_WaitCondition.wait(lock, [this] {
                return m_Tasks.empty() && m_ActiveTasks == 0;
                });
            return true;
        }
        else {
            // Wait with timeout
            return m_WaitCondition.wait_for(lock, timeout, [this] {
                return m_Tasks.empty() && m_ActiveTasks == 0;
                });
        }
    }

    size_t ThreadPool::GetPendingTaskCount() const {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        return m_Tasks.size();
    }

    void ThreadPool::ClearPendingTasks() {
        std::unique_lock<std::mutex> lock(m_QueueMutex);

        size_t cleared = m_Tasks.size();
        while (!m_Tasks.empty()) {
            m_Tasks.pop();
        }

        YAMEN_CORE_INFO("ThreadPool: Cleared {} pending tasks", cleared);
    }

    void ThreadPool::Pause() {
        m_Paused = true;
        YAMEN_CORE_INFO("ThreadPool: Paused");
    }

    void ThreadPool::Resume() {
        m_Paused = false;
        m_Condition.notify_all();
        YAMEN_CORE_INFO("ThreadPool: Resumed");
    }

} // namespace Yamen::Core
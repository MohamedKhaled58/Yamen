#include "Core/Threading/ThreadPool.h"
#include <Core/Logging/Logger.h>

namespace Yamen::Core {

    ThreadPool::ThreadPool(size_t numThreads)
        : m_Stop(false) {

        if (numThreads == 0) {
            numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0) numThreads = 4; // Fallback
        }

        for (size_t i = 0; i < numThreads; ++i) {
            m_Workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->m_QueueMutex);
                        this->m_Condition.wait(lock, [this] {
                            return this->m_Stop || !this->m_Tasks.empty();
                            });

                        if (this->m_Stop && this->m_Tasks.empty())
                            return;

                        task = std::move(this->m_Tasks.front());
                        this->m_Tasks.pop();
                    }

                    // Execute task with exception safety
                    // This prevents exceptions from killing the worker thread
                    try {
                        task();
                    }
                    catch (const std::exception& e) {
                        // Log error but keep thread alive
                        // Uncomment if you have logging:
                        YAMEN_CORE_ERROR("Task threw exception: {}", e.what());
                        (void)e;  // Silence unused variable warning
                    }
                    catch (...) {
                        // Catch all other exceptions
                        // Uncomment if you have logging:
                         YAMEN_CORE_ERROR("Task threw unknown exception");
                    }
                }
                });
        }
    }

    ThreadPool::~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Stop = true;
        }

        m_Condition.notify_all();

        for (std::thread& worker : m_Workers) {
            if (worker.joinable())
                worker.join();
        }
    }

} // namespace Yamen::Core
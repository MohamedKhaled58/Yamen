#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <chrono>
#include <memory>

namespace Yamen::Core {

    enum class TaskPriority {
        Low = 0,
        Normal = 1,
        High = 2,
        Critical = 3
    };

    class ThreadPool {
    public:
        struct Stats {
            std::atomic<uint64_t> tasksCompleted{ 0 };
            std::atomic<uint64_t> tasksEnqueued{ 0 };
            std::atomic<uint64_t> tasksFailed{ 0 };
            std::atomic<uint32_t> activeThreads{ 0 };
            std::chrono::steady_clock::time_point startTime;

            Stats() : startTime(std::chrono::steady_clock::now()) {}

            double GetUptime() const {
                auto now = std::chrono::steady_clock::now();
                return std::chrono::duration<double>(now - startTime).count();
            }

            double GetTasksPerSecond() const {
                double uptime = GetUptime();
                return uptime > 0 ? tasksCompleted.load() / uptime : 0.0;
            }
        };

        /**
         * @brief Create thread pool with specified number of threads
         * @param numThreads Number of threads (0 = hardware concurrency)
         * @param enableWorkStealing Enable work stealing optimization
         */
        explicit ThreadPool(size_t numThreads = 0, bool enableWorkStealing = false);
        ~ThreadPool();

        // Delete copy/move
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;

        /**
         * @brief Enqueue a task with priority
         * @tparam F Function type
         * @tparam Args Argument types
         * @param priority Task priority
         * @param f Function to execute
         * @param args Arguments
         * @return Future for the result
         */
        template<class F, class... Args>
        auto Enqueue(TaskPriority priority, F&& f, Args&&... args)
            -> std::future<typename std::invoke_result<F, Args...>::type> {

            using return_type = typename std::invoke_result<F, Args...>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<return_type> res = task->get_future();

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);

                if (m_Stop) {
                    throw std::runtime_error("Enqueue on stopped ThreadPool");
                }

                m_Tasks.push(TaskWrapper{
                    priority,
                    [task]() { (*task)(); },
                    std::chrono::steady_clock::now()
                    });

                m_Stats.tasksEnqueued++;
            }

            m_Condition.notify_one();
            return res;
        }

        /**
         * @brief Enqueue a task with normal priority (convenience)
         */
        template<class F, class... Args>
        auto Enqueue(F&& f, Args&&... args)
            -> std::future<typename std::invoke_result<F, Args...>::type> {
            return Enqueue(TaskPriority::Normal, std::forward<F>(f), std::forward<Args>(args)...);
        }

        /**
         * @brief Enqueue a fire-and-forget task (no return value)
         */
        template<class F, class... Args>
        void EnqueueDetached(TaskPriority priority, F&& f, Args&&... args) {
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);

                if (m_Stop) {
                    throw std::runtime_error("Enqueue on stopped ThreadPool");
                }

                m_Tasks.push(TaskWrapper{
                    priority,
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...),
                    std::chrono::steady_clock::now()
                    });

                m_Stats.tasksEnqueued++;
            }

            m_Condition.notify_one();
        }

        /**
         * @brief Wait for all tasks to complete
         * @param timeout Maximum time to wait (0 = infinite)
         */
        bool WaitForAll(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

        /**
         * @brief Get number of pending tasks
         */
        size_t GetPendingTaskCount() const;

        /**
         * @brief Get number of worker threads
         */
        size_t GetThreadCount() const { return m_Workers.size(); }

        /**
         * @brief Get thread pool statistics
         */
        const Stats& GetStats() const { return m_Stats; }

        /**
         * @brief Clear all pending tasks (does not cancel running tasks)
         */
        void ClearPendingTasks();

        /**
         * @brief Pause task execution (running tasks continue)
         */
        void Pause();

        /**
         * @brief Resume task execution
         */
        void Resume();

        /**
         * @brief Check if thread pool is paused
         */
        bool IsPaused() const { return m_Paused.load(); }

    private:
        struct TaskWrapper {
            TaskPriority priority;
            std::function<void()> task;
            std::chrono::steady_clock::time_point enqueueTime;

            bool operator<(const TaskWrapper& other) const {
                // Higher priority = lower value in priority_queue (reverse)
                return priority < other.priority;
            }
        };

        void WorkerThread(size_t threadId);
        void SetThreadName(const std::string& name);

        std::vector<std::thread> m_Workers;
        std::priority_queue<TaskWrapper> m_Tasks;

        mutable std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        std::condition_variable m_WaitCondition;

        std::atomic<bool> m_Stop{ false };
        std::atomic<bool> m_Paused{ false };
        std::atomic<size_t> m_ActiveTasks{ 0 };

        bool m_EnableWorkStealing;
        Stats m_Stats;
    };

} // namespace Yamen::Core
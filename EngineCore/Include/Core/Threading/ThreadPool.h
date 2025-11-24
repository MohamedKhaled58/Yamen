#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

namespace Yamen::Core {

    class ThreadPool {
    public:
        /**
         * @brief Create thread pool with specified number of threads
         * @param numThreads Number of threads (0 = hardware concurrency)
         */
        explicit ThreadPool(size_t numThreads = 0);
        ~ThreadPool();

        /**
         * @brief Enqueue a task to be executed
         * @tparam F Function type
         * @tparam Args Argument types
         * @param f Function to execute
         * @param args Arguments
         * @return Future for the result
         */
        template<class F, class... Args>
        auto Enqueue(F&& f, Args&&... args) 
            -> std::future<typename std::invoke_result<F, Args...>::type> {
            
            using return_type = typename std::invoke_result<F, Args...>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
            
            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                if (m_Stop) {
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                }

                m_Tasks.emplace([task]() { (*task)(); });
            }
            
            m_Condition.notify_one();
            return res;
        }

    private:
        std::vector<std::thread> m_Workers;
        std::queue<std::function<void()>> m_Tasks;
        
        std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        bool m_Stop;
    };

} // namespace Yamen::Core

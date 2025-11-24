#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <string>

namespace Yamen::Core {

/**
 * @brief Centralized logging system
 * 
 * Provides categorized logging with multiple sinks (console, file, custom).
 * Thread-safe and high-performance using spdlog.
 */
class Logger {
public:
    enum class Level {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical
    };

    /**
     * @brief Initialize the logging system
     * @param logFilePath Path to log file (optional)
     */
    static void Initialize(const std::string& logFilePath = "Yamen.log");

    /**
     * @brief Shutdown the logging system
     */
    static void Shutdown();

    /**
     * @brief Get logger for a specific category
     */
    static std::shared_ptr<spdlog::logger> GetLogger(const std::string& category = "Core");

    /**
     * @brief Set global log level
     */
    static void SetLevel(Level level);

    /**
     * @brief Flush all loggers
     */
    static void Flush();

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
    static std::shared_ptr<spdlog::logger> s_ServerLogger;
    static bool s_Initialized;
};

} // namespace Yamen::Core

// Convenience macros
#define YAMEN_CORE_TRACE(...)    ::Yamen::Core::Logger::GetLogger("Core")->trace(__VA_ARGS__)
#define YAMEN_CORE_DEBUG(...)    ::Yamen::Core::Logger::GetLogger("Core")->debug(__VA_ARGS__)
#define YAMEN_CORE_INFO(...)     ::Yamen::Core::Logger::GetLogger("Core")->info(__VA_ARGS__)
#define YAMEN_CORE_WARN(...)     ::Yamen::Core::Logger::GetLogger("Core")->warn(__VA_ARGS__)
#define YAMEN_CORE_ERROR(...)    ::Yamen::Core::Logger::GetLogger("Core")->error(__VA_ARGS__)
#define YAMEN_CORE_CRITICAL(...) ::Yamen::Core::Logger::GetLogger("Core")->critical(__VA_ARGS__)

#define YAMEN_CLIENT_TRACE(...)    ::Yamen::Core::Logger::GetLogger("Client")->trace(__VA_ARGS__)
#define YAMEN_CLIENT_DEBUG(...)    ::Yamen::Core::Logger::GetLogger("Client")->debug(__VA_ARGS__)
#define YAMEN_CLIENT_INFO(...)     ::Yamen::Core::Logger::GetLogger("Client")->info(__VA_ARGS__)
#define YAMEN_CLIENT_WARN(...)     ::Yamen::Core::Logger::GetLogger("Client")->warn(__VA_ARGS__)
#define YAMEN_CLIENT_ERROR(...)    ::Yamen::Core::Logger::GetLogger("Client")->error(__VA_ARGS__)
#define YAMEN_CLIENT_CRITICAL(...) ::Yamen::Core::Logger::GetLogger("Client")->critical(__VA_ARGS__)

#define YAMEN_SERVER_TRACE(...)    ::Yamen::Core::Logger::GetLogger("Server")->trace(__VA_ARGS__)
#define YAMEN_SERVER_DEBUG(...)    ::Yamen::Core::Logger::GetLogger("Server")->debug(__VA_ARGS__)
#define YAMEN_SERVER_INFO(...)     ::Yamen::Core::Logger::GetLogger("Server")->info(__VA_ARGS__)
#define YAMEN_SERVER_WARN(...)     ::Yamen::Core::Logger::GetLogger("Server")->warn(__VA_ARGS__)
#define YAMEN_SERVER_ERROR(...)    ::Yamen::Core::Logger::GetLogger("Server")->error(__VA_ARGS__)
#define YAMEN_SERVER_CRITICAL(...) ::Yamen::Core::Logger::GetLogger("Server")->critical(__VA_ARGS__)

#include "Core/Logging/Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vector>

namespace Yamen::Core {

std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;
std::shared_ptr<spdlog::logger> Logger::s_ServerLogger;
bool Logger::s_Initialized = false;

void Logger::Initialize(const std::string& logFilePath) {
    if (s_Initialized) {
        return;
    }

    // Create sinks
    std::vector<spdlog::sink_ptr> sinks;
    
    // Console sink with colors
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("%^[%T] [%n] %v%$");
    sinks.push_back(consoleSink);
    
    // File sink
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, true);
    fileSink->set_pattern("[%T] [%l] [%n] %v");
    sinks.push_back(fileSink);
    
    // Create loggers
    s_CoreLogger = std::make_shared<spdlog::logger>("Core", sinks.begin(), sinks.end());
    s_CoreLogger->set_level(spdlog::level::trace);
    s_CoreLogger->flush_on(spdlog::level::trace);
    spdlog::register_logger(s_CoreLogger);
    
    s_ClientLogger = std::make_shared<spdlog::logger>("Client", sinks.begin(), sinks.end());
    s_ClientLogger->set_level(spdlog::level::trace);
    s_ClientLogger->flush_on(spdlog::level::trace);
    spdlog::register_logger(s_ClientLogger);
    
    s_ServerLogger = std::make_shared<spdlog::logger>("Server", sinks.begin(), sinks.end());
    s_ServerLogger->set_level(spdlog::level::trace);
    s_ServerLogger->flush_on(spdlog::level::trace);
    spdlog::register_logger(s_ServerLogger);
    
    s_Initialized = true;
    
    YAMEN_CORE_INFO("Logger initialized");
}

void Logger::Shutdown() {
    if (!s_Initialized) {
        return;
    }
    
    YAMEN_CORE_INFO("Logger shutting down");
    
    spdlog::shutdown();
    s_Initialized = false;
}

std::shared_ptr<spdlog::logger> Logger::GetLogger(const std::string& category) {
    if (!s_Initialized) {
        Initialize();
    }
    
    if (category == "Core") {
        return s_CoreLogger;
    } else if (category == "Client") {
        return s_ClientLogger;
    } else if (category == "Server") {
        return s_ServerLogger;
    }
    
    // Return or create custom logger
    auto logger = spdlog::get(category);
    if (!logger) {
        logger = spdlog::stdout_color_mt(category);
    }
    return logger;
}

void Logger::SetLevel(Level level) {
    spdlog::level::level_enum spdLevel;
    switch (level) {
        case Level::Trace:    spdLevel = spdlog::level::trace; break;
        case Level::Debug:    spdLevel = spdlog::level::debug; break;
        case Level::Info:     spdLevel = spdlog::level::info; break;
        case Level::Warn:     spdLevel = spdlog::level::warn; break;
        case Level::Error:    spdLevel = spdlog::level::err; break;
        case Level::Critical: spdLevel = spdlog::level::critical; break;
        default:              spdLevel = spdlog::level::info; break;
    }
    
    spdlog::set_level(spdLevel);
}

void Logger::Flush() {
    if (s_CoreLogger) s_CoreLogger->flush();
    if (s_ClientLogger) s_ClientLogger->flush();
    if (s_ServerLogger) s_ServerLogger->flush();
}

} // namespace Yamen::Core

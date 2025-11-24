#pragma once

#include "Core/Utils/FileSystem.h"
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace Yamen::Core {

    /**
     * @brief Simple singleton configuration loader.
     *
     * Usage:
     *   Config::Instance().Load("config.json");
     *   int maxThreads = Config::Instance().Get<int>("thread_pool.max_threads", 4);
     */
    class Config {
    public:
        static Config& Instance();
        void Load(const std::filesystem::path& path);
        template<typename T>
        T Get(const std::string& key, const T& defaultValue = T{}) const {
            auto it = m_Values.find(key);
            if (it != m_Values.end()) {
                try { return it->second.get<T>(); } catch (...) { return defaultValue; }
            }
            return defaultValue;
        }
    private:
        Config() = default;
        nlohmann::json m_Values;
    };

} // namespace Yamen::Core

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace Yamen::Core {

/**
 * @brief String utility functions
 */
class StringUtils {
public:
    /**
     * @brief Convert string to lowercase
     */
    static std::string ToLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    /**
     * @brief Convert string to uppercase
     */
    static std::string ToUpper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::toupper(c); });
        return str;
    }

    /**
     * @brief Trim whitespace from start
     */
    static std::string TrimStart(std::string str) {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(),
            [](unsigned char c) { return !std::isspace(c); }));
        return str;
    }

    /**
     * @brief Trim whitespace from end
     */
    static std::string TrimEnd(std::string str) {
        str.erase(std::find_if(str.rbegin(), str.rend(),
            [](unsigned char c) { return !std::isspace(c); }).base(), str.end());
        return str;
    }

    /**
     * @brief Trim whitespace from both ends
     */
    static std::string Trim(std::string str) {
        return TrimStart(TrimEnd(std::move(str)));
    }

    /**
     * @brief Split string by delimiter
     */
    static std::vector<std::string> Split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(delimiter);
        
        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find(delimiter, start);
        }
        
        tokens.push_back(str.substr(start));
        return tokens;
    }

    /**
     * @brief Check if string starts with prefix
     */
    static bool StartsWith(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() &&
               str.compare(0, prefix.size(), prefix) == 0;
    }

    /**
     * @brief Check if string ends with suffix
     */
    static bool EndsWith(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() &&
               str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    /**
     * @brief Replace all occurrences
     */
    static std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != std::string::npos) {
            str.replace(pos, from.length(), to);
            pos += to.length();
        }
        return str;
    }
};

} // namespace Yamen::Core

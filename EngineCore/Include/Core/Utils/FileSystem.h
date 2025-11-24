#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

namespace Yamen::Core {

    /**
     * @brief Cross-platform file system utilities
     * 
     * Note: Method names avoid Windows.h conflicts (e.g., CreateDir instead of CreateDirectory)
     */
    class FileSystem {
    public:
        // ===== File Reading =====
        
        /**
         * @brief Read entire file into a binary buffer
         * @param path Path to the file
         * @return Vector of bytes containing file content, empty on error
         */
        static std::vector<uint8_t> ReadFile(const std::filesystem::path& path);

        /**
         * @brief Read entire file into a string
         * @param path Path to the file
         * @return String containing file content, empty on error
         */
        static std::string ReadFileText(const std::filesystem::path& path);

        // ===== File Writing =====
        
        /**
         * @brief Write binary buffer to file
         * @param path Path to the file
         * @param data Data to write
         * @return True if successful
         * @note Creates parent directories if needed
         */
        static bool WriteFile(const std::filesystem::path& path, const std::vector<uint8_t>& data);

        /**
         * @brief Write string to file
         * @param path Path to the file
         * @param text Text to write
         * @return True if successful
         * @note Creates parent directories if needed
         */
        static bool WriteFileText(const std::filesystem::path& path, const std::string& text);

        // ===== File/Directory Information =====
        
        /**
         * @brief Check if path exists
         * @param path Path to check
         * @return True if exists, false otherwise
         */
        static bool PathExists(const std::filesystem::path& path);

        /**
         * @brief Check if path is a directory
         * @param path Path to check
         * @return True if directory, false otherwise
         */
        static bool IsDirectory(const std::filesystem::path& path);

        /**
         * @brief Check if path is a regular file
         * @param path Path to check
         * @return True if regular file, false otherwise
         */
        static bool IsFile(const std::filesystem::path& path);

        /**
         * @brief Get size of file in bytes
         * @param path Path to file
         * @return File size in bytes, 0 on error
         */
        static uint64_t GetFileSize(const std::filesystem::path& path);

        // ===== Directory Operations =====
        
        /**
         * @brief Create directory (and parent directories if needed)
         * @param path Directory path to create
         * @return True if created or already exists
         */
        static bool CreateDir(const std::filesystem::path& path);

        /**
         * @brief List all entries in a directory
         * @param path Directory to list
         * @param recursive If true, recursively list subdirectories
         * @return Vector of paths in the directory
         */
        static std::vector<std::filesystem::path> ListDirectory(
            const std::filesystem::path& path, 
            bool recursive = false);

        // ===== File/Directory Manipulation =====
        
        /**
         * @brief Remove a file
         * @param path Path to file
         * @return True if removed or didn't exist
         */
        static bool RemoveFile(const std::filesystem::path& path);

        /**
         * @brief Remove a directory
         * @param path Path to directory
         * @param recursive If true, remove recursively
         * @return True if removed or didn't exist
         */
        static bool RemoveDir(const std::filesystem::path& path, bool recursive = false);

        /**
         * @brief Copy a file
         * @param from Source path
         * @param to Destination path
         * @param overwrite If true, overwrite existing file
         * @return True if successful
         */
        static bool Copy(
            const std::filesystem::path& from, 
            const std::filesystem::path& to, 
            bool overwrite = false);

        /**
         * @brief Move/rename a file
         * @param from Source path
         * @param to Destination path
         * @return True if successful
         */
        static bool Move(
            const std::filesystem::path& from, 
            const std::filesystem::path& to);

        // ===== Working Directory =====
        
        /**
         * @brief Get current working directory
         * @return Current working directory path
         */
        static std::filesystem::path GetWorkingDirectory();

        /**
         * @brief Set current working directory
         * @param path New working directory
         * @return True if successful
         */
        static bool SetWorkingDirectory(const std::filesystem::path& path);

        // ===== Path Utilities =====
        
        /**
         * @brief Get file extension (including the dot)
         * @param path File path
         * @return Extension string (e.g., ".txt")
         */
        static std::string GetExtension(const std::filesystem::path& path);

        /**
         * @brief Get filename with extension
         * @param path File path
         * @return Filename string (e.g., "file.txt")
         */
        static std::string GetFilename(const std::filesystem::path& path);

        /**
         * @brief Get filename without extension
         * @param path File path
         * @return Filename without extension (e.g., "file")
         */
        static std::string GetStem(const std::filesystem::path& path);

        /**
         * @brief Get parent directory path
         * @param path File or directory path
         * @return Parent directory path
         */
        static std::filesystem::path GetParentPath(const std::filesystem::path& path);

        /**
         * @brief Get absolute path
         * @param path Relative or absolute path
         * @return Absolute path
         */
        static std::filesystem::path GetAbsolutePath(const std::filesystem::path& path);

        /**
         * @brief Get relative path from base
         * @param path Path to make relative
         * @param base Base path
         * @return Relative path from base to path
         */
        static std::filesystem::path GetRelativePath(
            const std::filesystem::path& path, 
            const std::filesystem::path& base);
    };

} // namespace Yamen::Core
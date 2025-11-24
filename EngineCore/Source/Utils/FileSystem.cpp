#include "Core/Utils/FileSystem.h"
#include "Core/Logging/Logger.h"
#include <fstream>

namespace Yamen::Core {

    std::vector<uint8_t> FileSystem::ReadFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            YAMEN_CORE_ERROR("Failed to open file: {}", path.string());
            return {};
        }

        std::streamsize size = file.tellg();
        if (size == -1) {
            YAMEN_CORE_ERROR("Failed to get file size: {}", path.string());
            return {};
        }
        
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(static_cast<size_t>(size));
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            YAMEN_CORE_ERROR("Failed to read file: {}", path.string());
            return {};
        }

        return buffer;
    }

    std::string FileSystem::ReadFileText(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file) {
            YAMEN_CORE_ERROR("Failed to open file: {}", path.string());
            return "";
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        
        if (file.bad()) {
            YAMEN_CORE_ERROR("Error reading file: {}", path.string());
            return "";
        }
        
        return content;
    }

    bool FileSystem::WriteFile(const std::filesystem::path& path, const std::vector<uint8_t>& data) {
        // Create parent directories if they don't exist
        auto parent = path.parent_path();
        if (!parent.empty() && !PathExists(parent)) {
            if (!CreateDir(parent)) {
                return false;
            }
        }

        std::ofstream file(path, std::ios::binary);
        if (!file) {
            YAMEN_CORE_ERROR("Failed to create file: {}", path.string());
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        
        if (!file.good()) {
            YAMEN_CORE_ERROR("Error writing to file: {}", path.string());
            return false;
        }
        
        return true;
    }

    bool FileSystem::WriteFileText(const std::filesystem::path& path, const std::string& text) {
        // Create parent directories if they don't exist
        auto parent = path.parent_path();
        if (!parent.empty() && !PathExists(parent)) {
            if (!CreateDir(parent)) {
                return false;
            }
        }

        std::ofstream file(path);
        if (!file) {
            YAMEN_CORE_ERROR("Failed to create file: {}", path.string());
            return false;
        }

        file << text;
        
        if (!file.good()) {
            YAMEN_CORE_ERROR("Error writing to file: {}", path.string());
            return false;
        }
        
        return true;
    }

    bool FileSystem::PathExists(const std::filesystem::path& path) {
        try {
            return std::filesystem::exists(path);
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Error checking if path exists {}: {}", path.string(), e.what());
            return false;
        }
    }

    bool FileSystem::CreateDir(const std::filesystem::path& path) {
        try {
            return std::filesystem::create_directories(path);
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to create directory {}: {}", path.string(), e.what());
            return false;
        }
    }

    bool FileSystem::IsDirectory(const std::filesystem::path& path) {
        try {
            return std::filesystem::is_directory(path);
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Error checking if path is directory {}: {}", path.string(), e.what());
            return false;
        }
    }

    bool FileSystem::IsFile(const std::filesystem::path& path) {
        try {
            return std::filesystem::is_regular_file(path);
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Error checking if path is file {}: {}", path.string(), e.what());
            return false;
        }
    }

    uint64_t FileSystem::GetFileSize(const std::filesystem::path& path) {
        try {
            if (!PathExists(path) || !IsFile(path)) {
                return 0;
            }
            return std::filesystem::file_size(path);
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Error getting file size {}: {}", path.string(), e.what());
            return 0;
        }
    }

    bool FileSystem::RemoveFile(const std::filesystem::path& path) {
        try {
            if (!PathExists(path)) {
                return true;  // Already doesn't exist
            }
            return std::filesystem::remove(path);
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to remove file {}: {}", path.string(), e.what());
            return false;
        }
    }

    bool FileSystem::RemoveDir(const std::filesystem::path& path, bool recursive) {
        try {
            if (!PathExists(path)) {
                return true;  // Already doesn't exist
            }
            
            if (recursive) {
                return std::filesystem::remove_all(path) > 0;
            } else {
                return std::filesystem::remove(path);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to remove directory {}: {}", path.string(), e.what());
            return false;
        }
    }

    bool FileSystem::Copy(const std::filesystem::path& from, const std::filesystem::path& to, bool overwrite) {
        try {
            // Create parent directories if they don't exist
            auto parent = to.parent_path();
            if (!parent.empty() && !PathExists(parent)) {
                if (!CreateDir(parent)) {
                    return false;
                }
            }

            auto options = overwrite ? 
                std::filesystem::copy_options::overwrite_existing : 
                std::filesystem::copy_options::none;
                
            std::filesystem::copy_file(from, to, options);
            return true;
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to copy file from {} to {}: {}", 
                from.string(), to.string(), e.what());
            return false;
        }
    }

    bool FileSystem::Move(const std::filesystem::path& from, const std::filesystem::path& to) {
        try {
            // Create parent directories if they don't exist
            auto parent = to.parent_path();
            if (!parent.empty() && !PathExists(parent)) {
                if (!CreateDir(parent)) {
                    return false;
                }
            }

            std::filesystem::rename(from, to);
            return true;
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to move file from {} to {}: {}", 
                from.string(), to.string(), e.what());
            return false;
        }
    }

    std::filesystem::path FileSystem::GetWorkingDirectory() {
        try {
            return std::filesystem::current_path();
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to get working directory: {}", e.what());
            return {};
        }
    }

    bool FileSystem::SetWorkingDirectory(const std::filesystem::path& path) {
        try {
            std::filesystem::current_path(path);
            return true;
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to set working directory to {}: {}", 
                path.string(), e.what());
            return false;
        }
    }

    std::vector<std::filesystem::path> FileSystem::ListDirectory(
        const std::filesystem::path& path, 
        bool recursive) 
    {
        std::vector<std::filesystem::path> entries;
        
        try {
            if (!PathExists(path) || !IsDirectory(path)) {
                return entries;
            }

            if (recursive) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                    entries.push_back(entry.path());
                }
            } else {
                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                    entries.push_back(entry.path());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to list directory {}: {}", path.string(), e.what());
        }
        
        return entries;
    }

    std::string FileSystem::GetExtension(const std::filesystem::path& path) {
        return path.extension().string();
    }

    std::string FileSystem::GetFilename(const std::filesystem::path& path) {
        return path.filename().string();
    }

    std::string FileSystem::GetStem(const std::filesystem::path& path) {
        return path.stem().string();
    }

    std::filesystem::path FileSystem::GetParentPath(const std::filesystem::path& path) {
        return path.parent_path();
    }

    std::filesystem::path FileSystem::GetAbsolutePath(const std::filesystem::path& path) {
        try {
            return std::filesystem::absolute(path);
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to get absolute path for {}: {}", 
                path.string(), e.what());
            return path;
        }
    }

    std::filesystem::path FileSystem::GetRelativePath(
        const std::filesystem::path& path, 
        const std::filesystem::path& base) 
    {
        try {
            return std::filesystem::relative(path, base);
        } catch (const std::filesystem::filesystem_error& e) {
            YAMEN_CORE_ERROR("Failed to get relative path: {}", e.what());
            return path;
        }
    }

} // namespace Yamen::Core
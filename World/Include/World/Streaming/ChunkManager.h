#pragma once

#include "Core/Math/Math.h"
#include "Core/Threading/ThreadPool.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <future>
#include <functional>
#include <mutex>

namespace Yamen::World {

    /**
     * @brief Chunk coordinate for world partitioning
     */
    struct ChunkCoord {
        int x, z;

        bool operator==(const ChunkCoord& other) const noexcept {
            return x == other.x && z == other.z;
        }

        bool operator!=(const ChunkCoord& other) const noexcept {
            return !(*this == other);
        }
    };

    /**
     * @brief Hash function for ChunkCoord (for unordered containers)
     */
    struct ChunkCoordHash {
        std::size_t operator()(const ChunkCoord& c) const noexcept {
            // Better hash combining using boost-style hash_combine
            std::size_t seed = std::hash<int>()(c.x);
            seed ^= std::hash<int>()(c.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    /**
     * @brief Manages async chunk loading/unloading based on viewer position
     *
     * Features:
     * - Async chunk loading using thread pool
     * - Distance-based chunk culling
     * - Callbacks for load/unload operations
     * - Thread-safe operations
     */
    class ChunkManager {
    public:
        /**
         * @brief Construct chunk manager
         * @param chunkSize Size of each chunk in world units
         * @param loadRadius Number of chunks to load around viewer (in each direction)
         * @param threadPool Thread pool for async loading
         */
        ChunkManager(Yamen::Core::ThreadPool& threadPool, float chunkSize = 64.0f, int loadRadius = 2);
        ~ChunkManager();

        // Non-copyable
        ChunkManager(const ChunkManager&) = delete;
        ChunkManager& operator=(const ChunkManager&) = delete;

        /**
         * @brief Update chunk loading based on viewer position
         * @param viewerPosition Current viewer position
         *
         * Call this every frame or when viewer moves significantly
         */
        void Update(const Core::vec3& viewerPosition);

        /**
         * @brief Check if chunk is currently loaded
         */
        bool IsChunkLoaded(const ChunkCoord& coord) const;

        /**
         * @brief Check if chunk is pending load
         */
        bool IsChunkPending(const ChunkCoord& coord) const;

        /**
         * @brief Get number of loaded chunks
         */
        size_t GetLoadedChunkCount() const noexcept { return m_LoadedChunks.size(); }

        /**
         * @brief Get number of pending chunks
         */
        size_t GetPendingChunkCount() const noexcept { return m_PendingChunks.size(); }

        /**
         * @brief Get chunk size
         */
        float GetChunkSize() const noexcept { return m_ChunkSize; }

        /**
         * @brief Get load radius
         */
        int GetLoadRadius() const noexcept { return m_LoadRadius; }

        /**
         * @brief Set load radius
         */
        void SetLoadRadius(int radius) noexcept { m_LoadRadius = radius; }

        /**
         * @brief Callback for loading a chunk
         * @param coord Chunk coordinate to load
         * @return true if load successful, false otherwise
         */
        using LoadCallback = std::function<bool(const ChunkCoord&)>;

        /**
         * @brief Callback for unloading a chunk
         * @param coord Chunk coordinate to unload
         */
        using UnloadCallback = std::function<void(const ChunkCoord&)>;

        /**
         * @brief Set chunk load callback
         */
        void SetLoadCallback(LoadCallback cb) { m_LoadCallback = std::move(cb); }

        /**
         * @brief Set chunk unload callback
         */
        void SetUnloadCallback(UnloadCallback cb) { m_UnloadCallback = std::move(cb); }

        /**
         * @brief Force unload all chunks
         */
        void UnloadAll();

    private:
        /**
         * @brief Convert world position to chunk coordinate
         */
        ChunkCoord GetChunkCoord(const Core::vec3& position) const;

        /**
         * @brief Check if chunk is within load distance
         */
        bool IsChunkInRange(const ChunkCoord& coord, const ChunkCoord& center) const;

    private:
        float m_ChunkSize;
        int m_LoadRadius;
        Yamen::Core::ThreadPool& m_ThreadPool;

        std::unordered_set<ChunkCoord, ChunkCoordHash> m_LoadedChunks;
        std::unordered_set<ChunkCoord, ChunkCoordHash> m_PendingChunks;
        std::unordered_map<ChunkCoord, std::future<bool>, ChunkCoordHash> m_LoadFutures;

        LoadCallback m_LoadCallback;
        UnloadCallback m_UnloadCallback;

        ChunkCoord m_LastCenterChunk = { -9999, -9999 };

        // Optional: Add mutex if Update() can be called from multiple threads
        // mutable std::mutex m_Mutex;
    };

} // namespace Yamen::World
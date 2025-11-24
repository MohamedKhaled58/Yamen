#include "World/Streaming/ChunkManager.h"
#include "Core/Logging/Logger.h"
#include "Core/Threading/ThreadPool.h"
#include <chrono>
#include <algorithm>

namespace Yamen::World {

    ChunkManager::ChunkManager(Yamen::Core::ThreadPool& threadPool, float chunkSize, int loadRadius)
        : m_ChunkSize(chunkSize)
        , m_LoadRadius(loadRadius)
        , m_ThreadPool(threadPool) {

        YAMEN_CORE_INFO("ChunkManager initialized: chunkSize={}, loadRadius={}", chunkSize, loadRadius);
    }

    ChunkManager::~ChunkManager() {
        UnloadAll();
    }

    ChunkCoord ChunkManager::GetChunkCoord(const Core::vec3& position) const {
        return {
            static_cast<int>(std::floor(position.x / m_ChunkSize)),
            static_cast<int>(std::floor(position.z / m_ChunkSize))
        };
    }

    bool ChunkManager::IsChunkLoaded(const ChunkCoord& coord) const {
        return m_LoadedChunks.find(coord) != m_LoadedChunks.end();
    }

    bool ChunkManager::IsChunkPending(const ChunkCoord& coord) const {
        return m_PendingChunks.find(coord) != m_PendingChunks.end();
    }

    bool ChunkManager::IsChunkInRange(const ChunkCoord& coord, const ChunkCoord& center) const {
        int dx = std::abs(coord.x - center.x);
        int dz = std::abs(coord.z - center.z);
        return dx <= m_LoadRadius && dz <= m_LoadRadius;
    }

    void ChunkManager::Update(const Core::vec3& viewerPosition) {
        ChunkCoord centerChunk = GetChunkCoord(viewerPosition);

        // Early exit if viewer hasn't moved to a new chunk
        if (centerChunk == m_LastCenterChunk) {
            // Still check for completed futures even if center hasn't changed
            for (auto it = m_LoadFutures.begin(); it != m_LoadFutures.end(); ) {
                auto& fut = it->second;
                if (fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    bool success = false;
                    try {
                        success = fut.get();
                    }
                    catch (const std::exception& e) {
                        YAMEN_CORE_ERROR("Chunk load future threw exception: {}", e.what());
                    }

                    ChunkCoord coord = it->first;
                    m_PendingChunks.erase(coord);

                    if (success) {
                        m_LoadedChunks.insert(coord);
                        YAMEN_CORE_TRACE("Chunk loaded: ({}, {})", coord.x, coord.z);
                    }
                    else {
                        YAMEN_CORE_WARN("Failed to load chunk: ({}, {})", coord.x, coord.z);
                    }

                    it = m_LoadFutures.erase(it);
                }
                else {
                    ++it;
                }
            }
            return;
        }

        m_LastCenterChunk = centerChunk;

        // Determine chunks that should be loaded
        std::unordered_set<ChunkCoord, ChunkCoordHash> chunksToKeep;

        // Populate chunksToKeep and start async loads
        for (int x = -m_LoadRadius; x <= m_LoadRadius; ++x) {
            for (int z = -m_LoadRadius; z <= m_LoadRadius; ++z) {
                ChunkCoord coord{ centerChunk.x + x, centerChunk.z + z };
                chunksToKeep.insert(coord);

                // Start async load if not already loaded/pending/future
                bool alreadyLoaded = m_LoadedChunks.find(coord) != m_LoadedChunks.end();
                bool alreadyPending = m_PendingChunks.find(coord) != m_PendingChunks.end();
                bool hasFuture = m_LoadFutures.find(coord) != m_LoadFutures.end();

                if (!alreadyLoaded && !alreadyPending && !hasFuture) {
                    if (m_LoadCallback) {
                        try {
                            auto future = m_ThreadPool.Enqueue([this, coord]() -> bool {
                                return m_LoadCallback(coord);
                                });
                            m_LoadFutures.emplace(coord, std::move(future));
                            m_PendingChunks.insert(coord);

                            YAMEN_CORE_TRACE("Started loading chunk: ({}, {})", coord.x, coord.z);
                        }
                        catch (const std::exception& e) {
                            YAMEN_CORE_ERROR("Failed to enqueue chunk load: {}", e.what());
                        }
                    }
                }
            }
        }

        // Poll futures for completed loads
        for (auto it = m_LoadFutures.begin(); it != m_LoadFutures.end(); ) {
            auto& fut = it->second;
            if (fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                bool success = false;
                try {
                    success = fut.get();
                }
                catch (const std::exception& e) {
                    YAMEN_CORE_ERROR("Chunk load future threw exception: {}", e.what());
                }

                ChunkCoord coord = it->first;
                m_PendingChunks.erase(coord);

                if (success) {
                    m_LoadedChunks.insert(coord);
                    YAMEN_CORE_TRACE("Chunk loaded: ({}, {})", coord.x, coord.z);
                }
                else {
                    YAMEN_CORE_WARN("Failed to load chunk: ({}, {})", coord.x, coord.z);
                }

                it = m_LoadFutures.erase(it);
            }
            else {
                ++it;
            }
        }

        // Unload chunks that are no longer needed
        for (auto it = m_LoadedChunks.begin(); it != m_LoadedChunks.end(); ) {
            if (chunksToKeep.find(*it) == chunksToKeep.end()) {
                ChunkCoord coord = *it;

                if (m_UnloadCallback) {
                    try {
                        m_UnloadCallback(coord);
                        YAMEN_CORE_TRACE("Chunk unloaded: ({}, {})", coord.x, coord.z);
                    }
                    catch (const std::exception& e) {
                        YAMEN_CORE_ERROR("Chunk unload callback threw exception: {}", e.what());
                    }
                }

                it = m_LoadedChunks.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void ChunkManager::UnloadAll() {
        YAMEN_CORE_INFO("Unloading all chunks...");

        // Wait for all pending loads to complete
        for (auto& [coord, future] : m_LoadFutures) {
            try {
                future.wait();
            }
            catch (const std::exception& e) {
                YAMEN_CORE_ERROR("Exception while waiting for chunk future: {}", e.what());
            }
        }
        m_LoadFutures.clear();
        m_PendingChunks.clear();

        // Unload all loaded chunks
        for (const auto& coord : m_LoadedChunks) {
            if (m_UnloadCallback) {
                try {
                    m_UnloadCallback(coord);
                }
                catch (const std::exception& e) {
                    YAMEN_CORE_ERROR("Exception during chunk unload: {}", e.what());
                }
            }
        }

        m_LoadedChunks.clear();
        m_LastCenterChunk = { -9999, -9999 };

        YAMEN_CORE_INFO("All chunks unloaded");
    }

} // namespace Yamen::World
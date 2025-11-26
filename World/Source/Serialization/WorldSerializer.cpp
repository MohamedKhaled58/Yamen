#include "World/Serialization/WorldSerializer.h"
#include "Core/Logging/Logger.h"
#include <fstream>
#include <sstream>

namespace Yamen::World {

    bool WorldSerializer::SaveWorld(const std::string& filepath, const std::vector<EntityData>& entities) {
        std::ofstream out(filepath);
        if (!out.is_open()) {
            YAMEN_CORE_ERROR("Failed to open file for saving: {0}", filepath);
            return false;
        }

        out << "YamenWorld v1.0\n";
        out << entities.size() << "\n";

        for (const auto& entity : entities) {
            out << entity.ID << " " << entity.Name << "\n";
            out << entity.Position.x << " " << entity.Position.y << " " << entity.Position.z << "\n";
            out << entity.Rotation.x << " " << entity.Rotation.y << " " << entity.Rotation.z << "\n";
            out << entity.Scale.x << " " << entity.Scale.y << " " << entity.Scale.z << "\n";
        }

        out.close();
        YAMEN_CORE_INFO("World saved to {0}", filepath);
        return true;
    }

    bool WorldSerializer::LoadWorld(const std::string& filepath, std::vector<EntityData>& outEntities) {
        std::ifstream in(filepath);
        if (!in.is_open()) {
            YAMEN_CORE_ERROR("Failed to open file for loading: {0}", filepath);
            return false;
        }

        std::string header;
        std::getline(in, header);
        if (header != "YamenWorld v1.0") {
            YAMEN_CORE_ERROR("Invalid world file format: {0}", filepath);
            return false;
        }

        size_t count;
        in >> count;
        outEntities.clear();
        outEntities.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            EntityData entity;
            in >> entity.ID >> entity.Name;
            in >> entity.Position.x >> entity.Position.y >> entity.Position.z;
            in >> entity.Rotation.x >> entity.Rotation.y >> entity.Rotation.z;
            in >> entity.Scale.x >> entity.Scale.y >> entity.Scale.z;
            outEntities.push_back(entity);
        }

        in.close();
        YAMEN_CORE_INFO("World loaded from {0}", filepath);
        return true;
    }

} // namespace Yamen::World

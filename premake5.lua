-- Yamen Engine & YServer - Root Build Configuration
workspace "YamenSolutions"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "Client"

    filter "system:windows"
        buildoptions { "/utf-8" }
    filter {}
    
    flags {
        "MultiProcessorCompile"
    }
    
    -- Output directories
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    
    -- Third-party include directories
    IncludeDirs = {}
    IncludeDirs["entt"] = path.getabsolute("ThirdParty/entt/single_include")
    IncludeDirs["asio"] = path.getabsolute("ThirdParty/asio/asio/include")
    IncludeDirs["spdlog"] = path.getabsolute("ThirdParty/spdlog/include")
    IncludeDirs["fmt"] = path.getabsolute("ThirdParty/fmt/include")
    IncludeDirs["imgui"] = path.getabsolute("ThirdParty/imgui")
    IncludeDirs["glm"] = path.getabsolute("ThirdParty/glm")
    IncludeDirs["DirectXTex"] = path.getabsolute("ThirdParty/DirectXTex/DirectXTex")
    IncludeDirs["lz4"] = path.getabsolute("ThirdParty/lz4/lib")
    IncludeDirs["xxHash"] = path.getabsolute("ThirdParty/xxHash")
    IncludeDirs["stb"] = path.getabsolute("ThirdParty/stb")
    IncludeDirs["protobuf"] = path.getabsolute("ThirdParty/protobuf/src")
    
    -- Client Solution Projects
    group "Yamen/Engine"
        include "EngineCore"
        include "Platform"
        include "Graphics"
        include "ECS"
        include "Network"
        include "IO"
        include "AssetsC3"
        include "World"
    
    group "Yamen/Application"
        include "Client"
        include "Tools"
    
    -- Server Solution Projects
    group "YServer"
        include "YServer/ServerCore"
        include "YServer/AccountServer"
        include "YServer/GameServer"
        include "YServer/DatabaseServer"
        include "YServer/ChatServer"
    
    group ""

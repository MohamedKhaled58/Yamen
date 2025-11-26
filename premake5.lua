-- ============================================================
-- Yamen Engine & YServer  - Root Build Configuration
-- ============================================================

workspace "YamenEngine"
    architecture "x64"
    startproject "Client"

    configurations {
        "Debug",
        "Release",
        "Dist"
    }

    -- Use UTF-8 for Windows builds
    filter "system:windows"
        buildoptions { "/utf-8" }
    filter {}

    flags { "MultiProcessorCompile" }

    -- Output Folder Pattern: Debug-Windows-x64
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- ============================================================
    -- Include Directories (Absolute Paths)
    -- ============================================================
    IncludeDirs = {}
    IncludeDirs["entt"]       = path.getabsolute("ThirdParty/entt/single_include")
    IncludeDirs["asio"]       = path.getabsolute("ThirdParty/asio/asio/include")
    IncludeDirs["spdlog"]     = path.getabsolute("ThirdParty/spdlog/include")
    IncludeDirs["fmt"]        = path.getabsolute("ThirdParty/fmt/include")
    IncludeDirs["imgui"]      = path.getabsolute("ThirdParty/imgui")
    IncludeDirs["glm"]        = path.getabsolute("ThirdParty/glm")
    IncludeDirs["DirectXTex"] = path.getabsolute("ThirdParty/DirectXTex/DirectXTex")
    IncludeDirs["lz4"]        = path.getabsolute("ThirdParty/lz4/lib")
    IncludeDirs["xxHash"]     = path.getabsolute("ThirdParty/xxHash")
    IncludeDirs["stb"]        = path.getabsolute("ThirdParty/stb")
    IncludeDirs["protobuf"]   = path.getabsolute("ThirdParty/protobuf/src")

    -- Allow projects to use IncludeDirs.*
    include_dirs = IncludeDirs  
    -- You can now use: includedirs { include_dirs.entt, include_dirs.spdlog }

    -- ============================================================
    -- CLIENT SIDE PROJECTS
    -- ============================================================
    group "Yamen/Engine"
        include "EngineCore"
        include "Platform"
        include "Graphics"
        include "ECS"
        include "Network"
        include "IO"
        include "AssetsC3"
        include "World"
    group ""

    group "Yamen/Application"
        include "Client"
        include "Tools"
    group ""

    -- ============================================================
    -- THIRD PARTY BUILD TARGETS
    -- ============================================================
    group "ThirdParty"
        include "ThirdParty"
    group ""

    -- ============================================================
    -- SERVER SIDE PROJECTS
    -- ============================================================
    group "YServer"
        include "YServer/ServerCore"
        include "YServer/AccountServer"
        include "YServer/GameServer"
        include "YServer/DatabaseServer"
        include "YServer/ChatServer"
    group ""

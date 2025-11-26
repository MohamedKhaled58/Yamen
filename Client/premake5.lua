project "Client"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "off"
    
    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")
    
    files {
        "Include/**.h",
        "Source/**.cpp"
    }
    
    includedirs {
        "Include",
        "../EngineCore/Include",
        "../Platform/Include",
        "../Graphics/Include",
        "../ECS/Include",
        "../Network/Include",
        "../IO/Include",
        "../AssetsC3/Include",
        "../World/Include",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.fmt}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.entt}",
        "%{IncludeDirs.imgui}",
        "../ThirdParty/ImGuizmo"
    }
    
    links {
        "EngineCore",
        "Platform",
        "Graphics",
        "ECS",
        "Network",
        "IO",
        "AssetsC3",
        "World",
        "ImGui",
        "ImGuizmo"
    }
    
    filter "system:windows"
        systemversion "latest"
        defines {
            "PLATFORM_WINDOWS",
            "WIN32_LEAN_AND_MEAN",
            "NOMINMAX"
        }
    
    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        runtime "Debug"
        symbols "on"
        optimize "off"
    
    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "on"
        symbols "on"
    
    filter "configurations:Dist"
        defines { "NDEBUG", "DIST" }
        runtime "Release"
        optimize "full"
        symbols "off"

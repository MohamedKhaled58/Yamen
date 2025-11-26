project "ECS"
    kind "StaticLib"
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
        "../World/Include",
        "%{IncludeDirs.entt}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.spdlog}"
    }
    
    links {
        "EngineCore"
    }
    
    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS" }
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "on"

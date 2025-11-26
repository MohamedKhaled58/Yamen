project "EngineCore"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "off"

    defines { "GLM_ENABLE_EXPERIMENTAL" }

    
    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")
    
    files {
        "Include/**.h",
        "Source/**.cpp"
    }
    
    includedirs {
        "Include",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.fmt}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.xxHash}"
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

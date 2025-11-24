project "Graphics"
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
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.DirectXTex}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.stb}"
    }
    
    links {
        "EngineCore",
        "Platform",
        "d3d11",
        "dxgi",
        "d3dcompiler"
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
    
    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "on"
    
    filter "configurations:Dist"
        defines { "NDEBUG", "DIST" }
        runtime "Release"
        optimize "full"

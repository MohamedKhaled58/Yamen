project "Network"
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
        "%{IncludeDirs.asio}"
    }
    
    links {
        "EngineCore"
    }
    
    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS", "_WIN32_WINNT=0x0601", "ASIO_STANDALONE" }
        links { "ws2_32" }
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "on"

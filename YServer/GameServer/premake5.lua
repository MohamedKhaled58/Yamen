project "GameServer"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "off"
    
    targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")
    
    files {
        "Include/**.h",
        "Source/**.cpp"
    }
    
    includedirs {
        "Include",
        "../ServerCore/Include",
        "../../EngineCore/Include",
        "../../World/Include",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.fmt}"
    }
    
    links {
        "ServerCore",
        "EngineCore",
        "World"
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

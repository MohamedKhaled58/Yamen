project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    
    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")
    
    files {
        "imgui/imconfig.h",
        "imgui/imgui.h",
        "imgui/imgui.cpp",
        "imgui/imgui_draw.cpp",
        "imgui/imgui_internal.h",
        "imgui/imgui_tables.cpp",
        "imgui/imgui_widgets.cpp",
        "imgui/imstb_rectpack.h",
        "imgui/imstb_textedit.h",
        "imgui/imstb_truetype.h",
        "imgui/imgui_demo.cpp",
        
        -- Backends
        "imgui/backends/imgui_impl_win32.h",
        "imgui/backends/imgui_impl_win32.cpp",
        "imgui/backends/imgui_impl_dx11.h",
        "imgui/backends/imgui_impl_dx11.cpp"
    }
    
    includedirs {
        "imgui",
        "imgui/backends"
    }
    
    filter "system:windows"
        systemversion "latest"
        defines {
            "WIN32_LEAN_AND_MEAN",
            "NOMINMAX"
        }
    
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        runtime "Release"
        optimize "on"
    
    filter "configurations:Dist"
        runtime "Release"
        optimize "full"

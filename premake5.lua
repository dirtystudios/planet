workspace "planet"
    startproject "planet"
    location "build"
    configurations { "debug", "release" }
    platforms { "x32", "x64" }

    vectorextensions "SSE2"

    filter "platforms:x32"
        architecture "x86"
    filter "platforms:x64"
        architecture "x86_64"

    filter "configurations:debug"
        runtime "Debug"
        optimize "Debug"
        symbols "On"
        targetsuffix "d"
        defines { 
            "_DEBUG"
        }
    filter "configurations:release"
        runtime "Release"
        optimize "On"
		symbols "On"
        defines { "NDEBUG" }

    filter "system:windows"
		defines { "_WINDOWS", "WIN32" }
    filter {}

    -- this should fix xcode till premake alpha11
    xcodebuildsettings {['CLANG_CXX_LANGUAGE_STANDARD'] = 'c++14'}

    -- optimization levels in debug running debug stepping
    filter { "configurations:debug", "system:macosx" }
        xcodebuildsettings {['GCC_OPTIMIZATION_LEVEL'] = 0}

    flags { 
        "C++14",-- this should take care of gcc / clang
    }

    group "external"
        include "external/assimp.lua"
        include "external/libnoise.lua"
        include "external/freetype2.lua"
        include "external/sdl2.lua"
    group ""

project "planet"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin"

    defines {
        -- do we *actually* need these here?
        "GLEW_STATIC",
        "NOISE_STATIC"
    }

    filter "configurations:release"
        flags { "LinkTimeOptimization" }

    filter "system:windows"
        characterset "MBCS" -- oops

    filter {}

    -- files 
    includedirs {
        "src",
        "src/**", --double stars cause we mean buisness

        "external/include",
        "external/include/enum-flags/include",

        "external/assimp/include",
        "external/freetype2/include",
        "external/libnoise/src",
        "external/SDL-mirror/include"
    }

    files {
        "src/**.cpp",
        "src/**.h",
        "assets/shaders/**"
    }

    vpaths { 
        -- move all src files up one 
        ["*"] = "src", 
        -- move shaders into their own
        ["shaders/*"] = "assets/shaders/**", 
    }

    removefiles { "src/**/glfw/**" }

    filter "system:windows"
        removefiles { "src/**/osx/**" }
        removefiles { "src/**/uwp/**" }
        removefiles { "src/**/metal/**"}
    filter "system:macosx"
        files { "src/**.mm" }
        removefiles { "src/**/win32/**" }
        removefiles { "src/**/uwp/**" }
        removefiles { "src/**/dx11/**"}
    filter {}

    -- linkage / build stuff
    links {
        "assimp-zlib",
        "assimp",
        "freetype2",
        "libnoise",
        "SDL2"
    }

    filter { "system:windows" }
        postbuildcommands { '{COPY} "%{wks.location}SDL2/bin/%{cfg.platform}/%{cfg.buildcfg}/SDL2.dll" "%{cfg.buildtarget.directory}SDL2.dll*"' }

        libdirs { "external/winlibs/%{cfg.platform}" }
        links { 
            "opengl32",
            "d3d11",
            "d3dcompiler",
            "DXGI",
			"dxguid",
            "glew32s"
        }

    filter { "system:macosx" }
        buildoptions { "-x objective-c++"}
        links { 
            "Metal.framework",
            "QuartzCore.framework",
            "OpenGL.framework",
            "CoreServices.framework",
            "Cocoa.framework"
        }
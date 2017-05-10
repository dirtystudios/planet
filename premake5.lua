workspace "planet"
    startproject "planet"
    location "build"
    targetdir "bin"
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
    -- optimization levels in debug running debug stepping
    filter { "configurations:debug", "system:macosx" }
        xcodebuildsettings {['GCC_OPTIMIZATION_LEVEL'] = 0}
    filter {}

    cppdialect "C++14"
    xcodebuildsettings {['CLANG_CXX_LANGUAGE_STANDARD'] = 'c++14'}

    group "external"
        include "external/assimp.lua"
        include "external/libnoise.lua"
        include "external/freetype2.lua"
        include "external/sdl2.lua"                
        include "projects/backend/backend.lua"
        include "projects/dstd/dstd.lua"
        include "projects/planet/planet.lua"
    group ""


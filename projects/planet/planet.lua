project "planet"
    kind "ConsoleApp"
    language "C++"

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
    sysincludedirs {
        "../../external/include",
        "../../external/include/enum-flags/include",
        "../../external/assimp/include",
        "../../external/freetype2/include",
        "../../external/libnoise/src",
        "../../external/SDL-mirror/include",        
        "../dstd/src/",
        "../dstd/src/**",                
        "../backend/src/**",                
    }
    
    includedirs {
        "src",
        "src/**" --double stars cause we mean buisness
    }

    files {
        "src/**.cpp",
        "src/**.h",
        "../../assets/shaders/**"
    }

    vpaths { 
        -- move all src files up one 
        ["*"] = "src", 
        -- move shaders into their own
        ["shaders/*"] = "../../assets/shaders/**", 
    }

    removefiles { "src/**/glfw/**" }

    filter "system:windows"
        removefiles { "src/osx/**" }
        removefiles { "src/uwp/**" }
        removefiles { "src/metal/**"}
        removefiles { "src/**/uwp/**" }
        removefiles { "src/**/osx/**" }
        removefiles { "src/**metal/**"}
    filter "system:macosx"
        files { "src/**.mm" }
        removefiles { "src/win32/**" }        
        removefiles { "src/uwp/**" }
        removefiles { "src/dx11/**"}
        removefiles { "src/**dx11/**"}
        removefiles { "src/**/win32/**" }
        removefiles { "src/**/uwp/**" }
    filter {}

    -- linkage / build stuff
    links {        
        "backend",        
        "assimp-zlib",
        "assimp",
        "freetype2",
        "libnoise",
        "SDL2",
        "dstd",
    }

    filter { "system:windows" }
        postbuildcommands { '{COPY} "%{wks.location}SDL2/bin/%{cfg.platform}/%{cfg.buildcfg}/SDL2.dll" "%{cfg.buildtarget.directory}SDL2.dll*"' }

        libdirs { "../../external/winlibs/%{cfg.platform}" }
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
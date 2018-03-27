project "backend"    
    warnings "Off"
    kind "StaticLib"
    language "C++"

    targetname "backend"
    
    flags {
        -- This should speed up initial compile
        "MultiProcessorCompile",
        "NoMinimalRebuild"
    }

    sysincludedirs {
        "../dstd/src/",
        "../dstd/src/**",  
        "../../external/include/",
        "../../external/include/enum-flags/include",
    }

    includedirs {
        "src",
        "src/**" --double stars cause we mean buisness
    }

    files {
        "src/**.cpp",
        "src/**.h",
    }

    links {
        "dstd",
    }

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

    filter { "system:windows" }
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
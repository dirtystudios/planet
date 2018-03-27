project "dstd"    
    warnings "Off"
    kind "StaticLib"
    language "C++"

    targetname "dstd"

    flags {
        -- This should speed up initial compile
        "MultiProcessorCompile",
        "NoMinimalRebuild"
    }

    sysincludedirs {        
        "../external/include/enum-flags/include",
        "../../external/include",
    }

    includedirs {
        "src",    
        "src/**",        
    }

    files {
        "src/**.cpp",
        "src/**.h",
    }

    filter "system:windows"
        removefiles { "src/osx/**" }
        removefiles { "src/uwp/**" }
        removefiles { "src/metal/**"}
        removefiles { "src/**/uwp/**" }
        removefiles { "src/**/osx/**" }
        removefiles { "src/**/metal/**"}
    filter "system:macosx"
        files { "src/**.mm" }
        removefiles { "src/win32/**" }        
        removefiles { "src/uwp/**" }
        removefiles { "src/dx11/**"}
        removefiles { "src/**/dx11/**"}
        removefiles { "src/**/win32/**" }
        removefiles { "src/**/uwp/**" }
    filter {}

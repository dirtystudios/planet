project "assimp"
    location "../build/assimp"
    warnings "Off"
    kind "StaticLib"
    language "C++"

    flags {
        -- This should speed up initial compile
        "MultiProcessorCompile",
        "NoMinimalRebuild"
    }

    links {
        "assimp-zlib"
    }
    defines {
        "ASSIMP_BUILD_NO_C4D_IMPORTER",
        "OPENDDL_NO_USE_CPP11",
        "OPENDDLPARSER_BUILD",
        "ASSIMP_BUILD_NO_EXPORT",
        "_SCL_SECURE_NO_WARNINGS",
        "_CRT_SECURE_NO_WARNINGS"
    }

    -- these are manual otherwise it moves when flamewreath is cast and the raid blows up
    includedirs { 
        "assimp/include",
        "assimp/contrib/zlib",
        "assimp/contrib/rapidjson/include",
        "assimp/contrib/openddlparser/include",
        "assimp/contrib/unzip/include",

        "../build/assimp/inc"
    }
    files {
        "assimp/code/**.cpp",
        "assimp/code/**.h",
        "assimp/code/**.inl",

        "assimp/include/**.h",
        "assimp/include/**.hpp",
        "assimp/include/**.inl",

        "assimp/contrib/**.cpp",
        "assimp/contrib/**.c",
        "assimp/contrib/**.cc",
        "assimp/contrib/**.h",
        "assimp/contrib/**.hpp",
    }
    removefiles { "**/zlib/**" }
    removefiles { "**/gtest/**" }
    removefiles { "**/rapidjson/**" }

    -- k im not figuring out how to code this stupid file for now
    local revStr = [[
        #ifndef ASSIMP_REVISION_H_INC
        #define ASSIMP_REVISION_H_INC

        #define GitVersion 0x1337
        #define GitBranch "master"

        #endif // ASSIMP_REVISION_H_INC
    ]]
    local ok, err = os.mkdir('./../build/assimp/inc')
    if (not ok) then
        printf("error %s", err)
    end

    ok, err = os.writefile_ifnotequal(revStr, "./../build/assimp/inc/revision.h")
    if (not ok) then
        printf("error %s", err)
    end

	
project "assimp-zlib"
    location    "../build/assimp"
    warnings "Off"
    language    "C"
	kind        "StaticLib"
	targetname  "zlibstatic"
	defines     { "NO_FSEEKO", "_CRT_SECURE_NO_DEPRECATE"}
	flags 		{ "StaticRuntime" }

	files {
		"assimp/contrib/zlib/**.h",
		"assimp/contrib/zlib/**.c",
        "../build/assimp/inc/zconf.h"
	}

    includedirs {
        "../build/assimp/inc"
    }
	
	filter "system:not windows"
		defines { 'HAVE_UNISTD_H' }

    local ok, err = os.mkdir('./../build/assimp/inc')
    if (not ok) then
        printf("error %s", err)
    end
    ok, err = os.copyfile("./assimp/contrib/zlib/zconf.h.included", './../build/assimp/inc/zconf.h')
    if (not ok) then
        printf("error %s", err)
    end
project "libnoise"
    location "../build/libnoise"
    warnings "Off"
    kind "StaticLib"
    language "C++"

    targetname "noise"
    
    flags {
        -- This should speed up initial compile
        "MultiProcessorCompile",
        "NoMinimalRebuild"
    }

    defines {
        "NOISE_STATIC"
    }

    includedirs {
        "libnoise/src/noise"
    }

    files {
        "libnoise/src/noisegen.cpp",
        "libnoise/src/latlon.cpp",

        "libnoise/src/model/line.cpp",
        "libnoise/src/model/plane.cpp",
        "libnoise/src/model/sphere.cpp",
        "libnoise/src/model/cylinder.cpp",

        "libnoise/src/module/abs.cpp",
        "libnoise/src/module/add.cpp",
        "libnoise/src/module/billow.cpp",
        "libnoise/src/module/blend.cpp",
        "libnoise/src/module/cache.cpp",
        "libnoise/src/module/checkerboard.cpp",
        "libnoise/src/module/clamp.cpp",
        "libnoise/src/module/const.cpp",
        "libnoise/src/module/curve.cpp",
        "libnoise/src/module/cylinders.cpp",
        "libnoise/src/module/displace.cpp",
        "libnoise/src/module/exponent.cpp",
        "libnoise/src/module/invert.cpp",
        "libnoise/src/module/max.cpp",
        "libnoise/src/module/min.cpp",
        "libnoise/src/module/modulebase.cpp",
        "libnoise/src/module/multiply.cpp",
        "libnoise/src/module/perlin.cpp",
        "libnoise/src/module/power.cpp",
        "libnoise/src/module/ridgedmulti.cpp",
        "libnoise/src/module/rotatepoint.cpp",
        "libnoise/src/module/scalebias.cpp",
        "libnoise/src/module/scalepoint.cpp",
        "libnoise/src/module/select.cpp",
        "libnoise/src/module/spheres.cpp",
        "libnoise/src/module/terrace.cpp",
        "libnoise/src/module/translatepoint.cpp",
        "libnoise/src/module/turbulence.cpp",
        "libnoise/src/module/voronoi.cpp",
    }
project "freetype2"
    location "../build/freetype2"
    warnings "Off"
    targetname "freetype"
    language "C"
    kind "StaticLib"
    
    flags {
        -- This should speed up initial compile
        "MultiProcessorCompile",
        "NoMinimalRebuild"
    }

    defines {
        "FT2_BUILD_LIBRARY",
        "_CRT_SECURE_NO_WARNINGS"
    }
    
    sysincludedirs {
        "freetype2/include"
    }

    files {
        "freetype2/include/ft2build.h",
        "freetype2/include/freetype/*.h",
        "freetype2/include/freetype/config/*.h",
        "freetype2/include/freetype/internal/*.h",

        "freetype2/src/autofit/autofit.c",
        "freetype2/src/base/ftbase.c",
        "freetype2/src/base/ftbbox.c",
        "freetype2/src/base/ftbdf.c",
        "freetype2/src/base/ftbitmap.c",
        "freetype2/src/base/ftcid.c",
        "freetype2/src/base/ftfntfmt.c",
        "freetype2/src/base/ftfstype.c",
        "freetype2/src/base/ftgasp.c",
        "freetype2/src/base/ftglyph.c",
        "freetype2/src/base/ftgxval.c",
        "freetype2/src/base/ftinit.c",
        "freetype2/src/base/ftlcdfil.c",
        "freetype2/src/base/ftmm.c",
        "freetype2/src/base/ftotval.c",
        "freetype2/src/base/ftpatent.c",
        "freetype2/src/base/ftpfr.c",
        "freetype2/src/base/ftstroke.c",
        "freetype2/src/base/ftsynth.c",
        "freetype2/src/base/ftsystem.c",
        "freetype2/src/base/fttype1.c",
        "freetype2/src/base/ftwinfnt.c",
        "freetype2/src/bdf/bdf.c",
        "freetype2/src/bzip2/ftbzip2.c",
        "freetype2/src/cache/ftcache.c",
        "freetype2/src/cff/cff.c",
        "freetype2/src/cid/type1cid.c",
        "freetype2/src/gzip/ftgzip.c",
        "freetype2/src/lzw/ftlzw.c",
        "freetype2/src/pcf/pcf.c",
        "freetype2/src/pfr/pfr.c",
        "freetype2/src/psaux/psaux.c",
        "freetype2/src/pshinter/pshinter.c",
        "freetype2/src/psnames/psnames.c",
        "freetype2/src/raster/raster.c",
        "freetype2/src/sfnt/sfnt.c",
        "freetype2/src/smooth/smooth.c",
        "freetype2/src/truetype/truetype.c",
        "freetype2/src/type1/type1.c",
        "freetype2/src/type42/type42.c",
        "freetype2/src/winfonts/winfnt.c",
        "freetype2/builds/windows/ftdebug.c"
    }

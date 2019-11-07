  
project "enet"
    location "../build/enet"
    kind "StaticLib"
    language "C"

    files { "enet/*.c" }
    includedirs { "enet/include/" }
    
    filter { "system:windows" }
        links {
            "ws2_32", 
            "winmm"
        }
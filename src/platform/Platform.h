#ifndef __platform_h__
#define __platform_h__

#ifdef _WIN32
    #define PLATFORM_WIN
    #define PLATFORM_WIN32
    #ifdef _WIN64
        #define PLATFORM_WIN64
   #endif
#elif __APPLE__
    #define PLATFORM_APPLE
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
         #define PLATFORM_IOS
    #elif TARGET_OS_IPHONE
        #define PLATFORM_IIOS
    #elif TARGET_OS_MAC
        #define PLATFORM_OSX
    #else
        
    #endif
#elif __linux
    #define PLATFORM_LINUX
#elif __unix
    #define PLATFORM_UNIX
#elif __posix
    #define PLATFORM_POSIX
#endif

#endif
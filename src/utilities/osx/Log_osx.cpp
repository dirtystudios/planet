#include "Log.h"
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <cstdio>


std::string GetLevelString(Log::Level level) {
    switch(level) {
        case Log::Level::Debug: return "debug";
        case Log::Level::Warn: return "warn";
        case Log::Level::Error: return "error";
    }
}

FILE* GetLevelStream(Log::Level level) {
    switch(level) {
        case Log::Level::Debug: return stdout;
        case Log::Level::Warn: return stdout;
        case Log::Level::Error: return stderr;
    }
}

std::string GetTimestamp() {
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    
    char buffer [80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
    
    char currentTime[84] = "";
    sprintf(currentTime, "%s:%d", buffer, milli);
    return std::string(currentTime);
}

void Log::msg(Log::Level level,
                const std::string& channel,
                std::string fmt, ...) {
    
    char format[1024];
    snprintf(format, sizeof(format), "%-25s %-7s (%s) %s\n",
             GetTimestamp().c_str(),
             GetLevelString(level).c_str(),
             channel.c_str(),
             fmt.c_str());
    
    va_list args;
    va_start(args, fmt);
    FILE* stream = GetLevelStream(level);
    vfprintf(stream, format, args);
    va_end(args);
}


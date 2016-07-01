#include "Log.h"
#include <string.h>
#include <cstdarg>
#include <chrono>

std::string GetLevelString(Log::Level level) {
    switch (level) {
    case Log::Level::Debug: return "debug";
    case Log::Level::Warn: return "warn";
    case Log::Level::Error:
    default:
        return "error";
    }
}

FILE* GetLevelStream(Log::Level level) {
    switch (level) {
    case Log::Level::Debug: return stdout;
    case Log::Level::Warn: return stdout;
    case Log::Level::Error: 
    default:
        return stderr;
    }
}

std::string GetTimestamp() {
    using namespace std::chrono;

    auto now = system_clock::now();
    
    milliseconds ms = duration_cast<milliseconds>(now.time_since_epoch());
    int milli = ms.count() % 1000;
    
    time_t curTime = system_clock::to_time_t(now);

    char buffer[80];
    struct tm buf;
    localtime_s(&buf, &curTime);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &buf);

    char currentTime[84] = "";
    sprintf_s(currentTime, "%s:%d", buffer, milli);
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

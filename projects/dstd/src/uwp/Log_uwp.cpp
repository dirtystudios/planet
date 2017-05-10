#include "Log.h"

#include <Windows.h>
#include <debugapi.h>
#include <cstdio>
#include <ctime>

void Log::d(int line, string func, string msg, ...) {
    char format[1024];    
    char buffer[2048];
    wchar_t wbuffer[2049];
    snprintf(format, sizeof(format), "[%s](%s:%d)[DEBUG] : %s\n", get_timestamp().c_str(), func.c_str(), line,
        msg.c_str());
    va_list args;
    va_start(args, msg);
    vsprintf_s(buffer, format, args);
    va_end(args);
    size_t outsize;
    mbstowcs_s(&outsize, wbuffer, 1025, buffer, 1024);
    OutputDebugString(wbuffer);
}

void Log::w(int line, string func, string msg, ...) {
    char format[1024];
    char buffer[2048];
    wchar_t wbuffer[2049];
    snprintf(format, sizeof(format), "[%s](%s:%d)[WARN] : %s\n", get_timestamp().c_str(), func.c_str(), line,
        msg.c_str());
    va_list args;
    va_start(args, msg);
    vsprintf_s(buffer, format, args);
    va_end(args);
    size_t outsize;
    mbstowcs_s(&outsize, wbuffer, 1025, buffer, 1024);
    OutputDebugString(wbuffer);
}

void Log::e(int line, string func, string msg, ...) {
    char format[1024];
    char buffer[2048];
    wchar_t wbuffer[2049];
    snprintf(format, sizeof(format), "[%s](%s:%d)[ERROR] : %s\n", get_timestamp().c_str(), func.c_str(), line,
        msg.c_str());
    va_list args;
    va_start(args, msg);
    vsprintf_s(buffer, format, args);
    va_end(args);
    size_t outsize;
    mbstowcs_s(&outsize, wbuffer, 1025, buffer, 1024);
    OutputDebugString(wbuffer);
}

string Log::get_timestamp() {
    std::time_t rawTime;
    std::time(&rawTime);
    struct tm timeinfo;
    if (localtime_s(&timeinfo, &rawTime)) {
        return 0;
    }

    int milli = std::clock() / CLOCKS_PER_SEC;

    char buffer[80];
    strftime(buffer, 80, "%H:%M:%S", &timeinfo);

    char currentTime[84] = "";
    sprintf_s(currentTime, "%s:%d", buffer, milli);
    return (string)currentTime;
}
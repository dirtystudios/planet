#include "Log.h"
#include <stdarg.h>
#include <string.h>
#include <ctime>

void Log::d(int line, string func, string msg, ...) {
    char format[1024];
    snprintf(format, sizeof(format), "[%s](%s:%d)[DEBUG] : %s\n", get_timestamp().c_str(), func.c_str(), line,
        msg.c_str());
    va_list args;
    va_start(args, msg);
    vprintf(format, args);
    va_end(args);
}

void Log::w(int line, string func, string msg, ...) {
    char format[1024];
    snprintf(format, sizeof(format), "[%s](%s:%d)[WARN] : %s\n", get_timestamp().c_str(), func.c_str(), line,
        msg.c_str());
    va_list args;
    va_start(args, msg);
    vfprintf(stdout, format, args);
    va_end(args);
}

void Log::e(int line, string func, string msg, ...) {
    char format[1024];
    snprintf(format, sizeof(format), "[%s](%s:%d)[ERROR] : %s\n", get_timestamp().c_str(), func.c_str(), line,
        msg.c_str());
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, format, args);
    va_end(args);
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
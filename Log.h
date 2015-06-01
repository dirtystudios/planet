#include <iostream>
#include <iomanip>

static void WriteLine(const char* channel, const char* severity, const char* msg, ...) {
    char format[1024];         
    snprintf(format, sizeof(format), "( %s [%s]): %s\n", channel, severity, msg);
    va_list args;
    va_start(args,msg);
    vprintf(format, args);
    va_end(args);    
}

#define LOG_E(channel, msg, ...) LOG(channel, "ERROR", msg, __VA_ARGS__)
#define LOG_D(channel, msg, ...) LOG(channel, "DEBUG", msg, __VA_ARGS__)
#define LOG(channel, severity, msg, ...) WriteLine(channel, severity, msg, ##__VA_ARGS__)
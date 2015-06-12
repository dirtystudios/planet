#include <iostream>
#include <iomanip>
#include <cassert>

static void WriteLine(const char* severity, const char* msg, ...) {
    char format[1024];         
    snprintf(format, sizeof(format), "[%s]: %s\n", severity, msg);
    va_list args;
    va_start(args,msg);
    vprintf(format, args);
    va_end(args);    
}


#define LOG_E(msg, ...) LOG("ERROR", msg, __VA_ARGS__); assert(false);
#define LOG_D(msg, ...) LOG("DEBUG", msg, __VA_ARGS__)
#define LOG(severity, msg, ...) WriteLine(severity, msg, __VA_ARGS__)
/*
#define LOG_ERRNO(channel, msg) LOG_E(channel, msg << " (errno: " << strerror(errno) << ")")
#define LOG_E(channel, msg) LOG(channel, "ERROR", msg)
#define LOG_W(channel, msg) LOG(channel, "WARN", msg)
#define LOG_D(channel, msg) LOG(channel, "DEBUG", msg)
#define LOG_V(channel, msg) LOG(channel, "VERBOSE", msg)
#define LOG_FATAL(channel, msg) LOG(channel, "FATAL", msg)
#define LOG(channel, severity, msg) do { std::cout << "(" << std::setw(7) << std::right << channel << " [" << std::left << std::setw(5) << severity << "]) " << msg << std::endl; } while(false)
*/

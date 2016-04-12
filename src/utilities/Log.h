#ifndef __log_h__
#define __log_h__

#include <cassert>

void WriteLine(const char* severity, const char* msg, ...);

#define LOG_E(msg, ...) { LOG("ERROR", msg, __VA_ARGS__); assert(false); }
#define LOG_W(msg, ...) LOG("WARN", msg, __VA_ARGS__);
#define LOG_D(msg, ...) LOG("DEBUG", msg, __VA_ARGS__)
#define LOG(severity, msg, ...) WriteLine(severity, msg, __VA_ARGS__)

#endif
#include "Log.h"

#include <Windows.h>
#include <debugapi.h>
#include <cstdio>

void WriteLine(const char* severity, const char* msg, ...) {
    char format[1024];
    char buffer[2048];
    wchar_t wbuffer[2049];
    snprintf(format, sizeof(format), "[%s]: %s\n", severity, msg);
    va_list args;
    va_start(args, msg);
    vsprintf_s(buffer, format, args);
    va_end(args);
    size_t outsize;
    mbstowcs_s(&outsize, wbuffer, 1025, buffer, 1024);
    OutputDebugString(wbuffer);
}
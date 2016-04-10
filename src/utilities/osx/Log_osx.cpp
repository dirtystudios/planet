#include "Log.h"

#include <stdio.h>
#include <stdarg.h>

void WriteLine(const char* severity, const char* msg, ...) {
    char format[1024];
    snprintf(format, sizeof(format), "[%s]: %s\n", severity, msg);
    va_list args;
    va_start(args, msg);
    vprintf(format, args);
    va_end(args);
}

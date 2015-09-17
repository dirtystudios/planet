#ifndef __log_h__
#define __log_h__

#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(outBuf, size, format, ap);
	va_end(ap);

	return count;
}

#endif

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

#define SLOG__ERRNO(channel, msg) SLOG_E(channel, msg << " (errno: " << strerror(errno) << ")")
#define SLOG_E(channel, msg) SLOG(channel, "ERROR", msg)
#define SLOG_W(channel, msg) SLOG(channel, "WARN", msg)
#define SLOG_D(channel, msg) SLOG(channel, "DEBUG", msg)
#define SLOG_V(channel, msg) SLOG(channel, "VERBOSE", msg)
#define SLOG_FATAL(channel, msg) SLOG(channel, "FATAL", msg)
#define SLOG(channel, severity, msg) do { std::cout << "(" << std::setw(7) << std::right << channel << " [" << std::left << std::setw(5) << severity << "]) " << msg << std::endl; } while(false)


#endif
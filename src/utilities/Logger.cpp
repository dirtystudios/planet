#include "Log.h"
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

pthread_mutex_t Log::_write_lock = PTHREAD_MUTEX_INITIALIZER;

void Log::d(int line, string func, string msg, ...) {
    char format[1024];
    pthread_mutex_lock(&_write_lock);
    // snprintf(format, sizeof(format), "[%s](%s:%d){Thread:%lu}[DEBUG] : %s\n", get_timestamp().c_str(), func.c_str(),
    // line, (long int)pthread_self(), msg.c_str());
    snprintf(format, sizeof(format), "[%s](%s:%d)[DEBUG] : %s\n", get_timestamp().c_str(), func.c_str(), line,
             msg.c_str());
    va_list args;
    va_start(args, msg);
    vprintf(format, args);
    va_end(args);
    pthread_mutex_unlock(&_write_lock);
}

void Log::w(int line, string func, string msg, ...) {
    char format[1024];
    pthread_mutex_lock(&_write_lock);
    // snprintf(format, sizeof(format), "[%s](%s:%d){Thread:%lu}[ERROR] : %s\n", get_timestamp().c_str(), func.c_str(),
    // line, (long int)pthread_self(), msg.c_str());
    snprintf(format, sizeof(format), "[%s](%s:%d)[WARN] : %s\n", get_timestamp().c_str(), func.c_str(), line,
             msg.c_str());
    va_list args;
    va_start(args, msg);
    vfprintf(stdout, format, args);
    va_end(args);
    pthread_mutex_unlock(&_write_lock);
}

void Log::e(int line, string func, string msg, ...) {
    char format[1024];
    pthread_mutex_lock(&_write_lock);
    // snprintf(format, sizeof(format), "[%s](%s:%d){Thread:%lu}[ERROR] : %s\n", get_timestamp().c_str(), func.c_str(),
    // line, (long int)pthread_self(), msg.c_str());
    snprintf(format, sizeof(format), "[%s](%s:%d)[ERROR] : %s\n", get_timestamp().c_str(), func.c_str(), line,
             msg.c_str());
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, format, args);
    va_end(args);
    pthread_mutex_unlock(&_write_lock);
}

string Log::get_timestamp() {
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;

    char buffer[80];
    strftime(buffer, 80, "%H:%M:%S", localtime(&curTime.tv_sec));

    char currentTime[84] = "";
    sprintf(currentTime, "%s:%d", buffer, milli);
    return (string)currentTime;
}
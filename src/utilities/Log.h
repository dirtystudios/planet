#ifndef _logger_h_
#define _logger_h_

#include <cstdio>
#include <pthread.h>
#include <string>
#include <iostream>
using namespace std;

inline std::string methodName(const std::string& prettyFunction) {
    size_t colons = prettyFunction.find_last_of("::");

    size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
    size_t end   = prettyFunction.rfind("(") - begin;

    return prettyFunction.substr(begin, end) + "()";
}

#define __METHOD_NAME__ methodName(__PRETTY_FUNCTION__)
#define LOG_D(x, ...) Log::d(__LINE__, __METHOD_NAME__, x, ##__VA_ARGS__);
#define LOG_W(x, ...) Log::w(__LINE__, __METHOD_NAME__, x, ##__VA_ARGS__);
#define LOG_E(x, ...) Log::e(__LINE__, __METHOD_NAME__, x, ##__VA_ARGS__);

class Log {
public:
    static void d(int line, string func, string log, ...);
    static void w(int line, string func, string log, ...);
    static void e(int line, string func, string log, ...);

private:
    static string get_timestamp();
    static pthread_mutex_t _write_lock;
};

#endif
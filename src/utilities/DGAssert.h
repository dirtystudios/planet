#pragma once

#include <cassert>
#include <stdarg.h>
#include <string.h>
#include <cstdio>

// just pulled this off stackoverflow, not sure if this works or is necessary
#ifdef _MSC_VER
#define DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP);
#endif

class DGAssert {
public:
    static void assertTrue(bool condition, const char* conditionStr, const char* file, int line, const std::string& fmt,
                           ...) {
        static char _formatBuffer[1024];
        if (!condition) {
            snprintf(_formatBuffer, sizeof(_formatBuffer), "cond:%s, file:%s, line:%d, msg:%s", conditionStr, file,
                     line, fmt.c_str());

            va_list args;
            va_start(args, fmt);
            vfprintf(stderr, _formatBuffer, args);
            va_end(args);

            DEBUG_BREAK();
            assert(condition);
        }
    }

    static void assertTrue(bool condition, const char* conditionStr, const char* file, int line) {
        if (!condition) {
            fprintf(stderr, "cond:%s, file:%s, line:%d", conditionStr, file, line);

            DEBUG_BREAK();
            assert(condition);
        }
    }

    static void assertFail(const char* file, int line, const std::string& fmt, ...) {
        static char _formatBuffer[1024];
        snprintf(_formatBuffer, sizeof(_formatBuffer), "file:%s, line:%d, msg:%s", file, line, fmt.c_str());

        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, _formatBuffer, args);
        va_end(args);

        DEBUG_BREAK();
        assert(false);
    }

    static void assertFail(const char* file, int line) {
        fprintf(stderr, "file:%s, line:%d", file, line);

        DEBUG_BREAK();
        assert(false);
    }
};

#define dg_assert(cond, fmt, ...) DGAssert::assertTrue(cond, #cond, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define dg_assert_nm(cond) DGAssert::assertTrue(cond, #cond, __FILE__, __LINE__)
#define dg_assert_fail(fmt, ...) DGAssert::assertFail(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define dg_assert_fail_nm() DGAssert::assertFail(__FILE__, __LINE__)

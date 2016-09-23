#pragma once

#include <cassert>
#include <stdarg.h>
#include <string.h>
#include <cstdio>
#include <iostream>

// just pulled this off stackoverflow, not sure if this works or is necessary
#ifdef _MSC_VER
#define DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP);
#endif

class DGAssert {
public:
    template <typename T, typename K>
    static void assertEquals(const T& a, const K& b, const char* conditionAStr, const char* conditionBStr,
                             const char* file, int line) {
        if (a != b) {
            std::cerr << "\nAssertion Failure:\ncondition:" << conditionAStr << " == " << conditionBStr
                      << "\nexpected:" << b << " but was:" << a << "\nfile:" << file << ", line:" << line << "\n";
            DEBUG_BREAK();
            assert(a == b);
        }
    }

    static void assertTrue(bool condition, const char* conditionStr, const char* file, int line, const char* fmt, ...) {
        static char _formatBuffer[1024];
        if (!condition) {
            snprintf(_formatBuffer, sizeof(_formatBuffer),
                     "\nAssertion Failure:\ncondition: %s\nfile:%s, line:%d\nmsg:\"%s\"\n", conditionStr, file, line,
                     fmt);

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

    [[noreturn]]
    static void assertFail(const char* file, int line, const char* fmt, ...) {
        static char _formatBuffer[1024];
        snprintf(_formatBuffer, sizeof(_formatBuffer), "file:%s, line:%d, msg:%s", file, line, fmt);
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, _formatBuffer, args);
        va_end(args);

        DEBUG_BREAK();
        assert(false);
    }

    [[noreturn]]
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
#define dg_assert_equals_nm(a, b) DGAssert::assertEquals(a, b, #a, #b, __FILE__, __LINE__)

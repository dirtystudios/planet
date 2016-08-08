#pragma once

#include <cstdio>
#include <string>

class Log {
public:
    enum class Level : uint8_t {
        Debug = 0,
        Warn,
        Error,

    };

    static void msg(Log::Level level, const std::string& channel, std::string fmt, ...);
};

static const std::string kDefaultChannel = "main";

#define LOG_D(fmt, ...) Log::msg(Log::Level::Debug, kDefaultChannel, fmt, ##__VA_ARGS__);
#define LOG_W(fmt, ...) Log::msg(Log::Level::Warn, kDefaultChannel, fmt, ##__VA_ARGS__);
#define LOG_E(fmt, ...) Log::msg(Log::Level::Error, kDefaultChannel, fmt, ##__VA_ARGS__);
#define LOG(level, channel, fmt, ...)                                          \
  Log::msg(level, channel, fmt, ##__VA_ARGS__);

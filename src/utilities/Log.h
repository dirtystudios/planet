#pragma once

#include <cstdio>
#include <string>

#include <fmt/std.h>
#include <spdlog/spdlog.h>

#define LOG_D(...)                                                      \
    do {                                                                \
        spdlog::logger* default_logger = spdlog::default_logger_raw();  \
        if (default_logger->should_log(spdlog::level::debug))           \
            SPDLOG_LOGGER_DEBUG(default_logger, __VA_ARGS__);           \
    } while (0)

#define LOG_W(...)                                                       \
    do {                                                                 \
        spdlog::logger* default_logger = spdlog::default_logger_raw();   \
        if (default_logger->should_log(spdlog::level::warn))             \
            SPDLOG_LOGGER_WARN(default_logger, __VA_ARGS__);             \
    } while (0)

#define LOG_E(...)                                                        \
    do {                                                                  \
        spdlog::logger* default_logger = spdlog::default_logger_raw();    \
        if (default_logger->should_log(spdlog::level::err))               \
            SPDLOG_LOGGER_ERROR(default_logger, __VA_ARGS__);             \
    } while (0)


void initlogging();
#include "Log.h"

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>

static void print_logging_error(const std::string& msg) {
    std::fprintf(stderr, "%s\n", msg.c_str());
}

static void log_error_handler(const std::string& msg) {
    print_logging_error("spdlog exception: " + msg);
    assert(false);
}

void initlogging() {
    constexpr auto default_log_pattern = "%^%T.%e|%t|%L| %19s:%-4# %-30!! %$| %v";
    auto formatter = std::make_unique<spdlog::pattern_formatter>();
    formatter->set_pattern(std::string(default_log_pattern));

    auto def_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    def_sink->set_level(spdlog::level::debug);

    auto default_logger = std::make_shared<spdlog::logger>("default", def_sink);
    spdlog::set_default_logger(default_logger);
}
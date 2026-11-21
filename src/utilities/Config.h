#pragma once
#include "IniParser.h"
#include "File.h"
#include "Log.h"

namespace config {
    class Config {
    private:
        fs::path configPath;
        INI::Parser* p{};
    public:
        static Config& getInstance() {
            static Config instance; 
            return instance;
        }

        std::string GetConfigString(const std::string& key) {
            return p ? p->top()[key] : "";
        }

        std::string GetConfigString(const std::string& section, const std::string& key) {
            return p ? p->top()(section)[key] : "";
        }

    private:
        Config() {
            configPath = fs::GetProcessDirectory() / "planet.ini";
            std::ifstream fin(configPath);

            if (fin.fail()) {
                LOG_E("Failed to open file '%s'\n", configPath.c_str());
                assert(false);
            }
            try {
                p = new INI::Parser(configPath.string().c_str());
            }
            catch (std::runtime_error& e) {
                LOG_E("INI Parsing Error: %s", e.what());
            }
        };

        Config(Config const&) = delete;
        void operator=(Config const&) = delete;
    };
}
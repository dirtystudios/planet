#pragma once
#include "IniParser.h"
#include "File.h"
#include "Log.h"

namespace config {
    class Config {
    private:
        std::string configPath;
        INI::Parser* p;
    public:
        static Config& getInstance() {
            static Config instance; 
            return instance;
        }

        std::string GetConfigString(std::string key) {
            return p->top()[key];
        }

        std::string GetConfigString(std::string section, std::string key) {
            return p->top()(section)[key];
        }

    private:
        Config() {
            configPath = fs::AppendPathProcessDir("/planet.ini");
            std::ifstream fin(configPath);

            if (fin.fail()) {
                LOG_E("Failed to open file '%s'\n", configPath.c_str());
                assert(false);
            }
            try {
                p = new INI::Parser(configPath.c_str());
            }
            catch (std::runtime_error& e) {
                LOG_E("INI Parsing Error: %s", e.what());
            }
        };

        Config(Config const&) = delete;
        void operator=(Config const&) = delete;
    };
}
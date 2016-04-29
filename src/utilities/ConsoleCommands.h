#pragma once

#include "Log.h"
#include "Callback.h"
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iterator>
#include <unordered_map>

namespace config {
    // The string vector contains the 'args' of the function
    // the function will have to parse and verify the args as it see's fit
    // The return is displayed in the console...maybe
    typedef Callback<std::string(const std::vector<std::string> &)> ConsoleCommandCallback;

    // Yey more singletons
    class ConsoleCommands {
    private:
        std::unordered_map<std::string, ConsoleCommandCallback> m_commandMap;

    public:
        static ConsoleCommands& getInstance() {
            static ConsoleCommands instance;
            return instance;
        }

        // Register command and callback, commands are case sensitive
        // Don't include a '/' or anything eugene
        void RegisterCommand(const std::string &command, ConsoleCommandCallback callback) {
            m_commandMap.emplace(command, callback);
        }

        std::string ProcessConsoleString(const std::string &command) {
            if (command.length() <= 1) {
                return "";
            }
            if (command[0] != '/') {
                return "";
            }

            std::istringstream iss(command);
            std::vector<std::string> tokens{ std::istream_iterator<std::string>(iss),
                                             std::istream_iterator<std::string>{} };
            auto it = m_commandMap.find(tokens[0].substr(1));
            if (it != m_commandMap.end()) {
                // possibly shitty way to do this, but w/e
                if (tokens.size() > 1) {
                    std::vector<std::string> temp(&tokens[0], &tokens[tokens.size()]);
                    return it->second(temp);
                }
                else {
                    return it->second(std::vector<std::string>{});
                }
            }
            else {
                return "Unknown Command.";
            }
        }

    private:
        ConsoleCommands() {
        };

        ConsoleCommands(ConsoleCommands const&) = delete;
        void operator=(ConsoleCommands const&) = delete;
    };
}
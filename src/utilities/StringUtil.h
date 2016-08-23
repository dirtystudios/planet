#pragma once

#include <string>

namespace StringUtil {

    const std::string WHITESPACE = " \n\r\t";

    std::string TrimLeft(const std::string& s)
    {
        size_t startpos = s.find_first_not_of(WHITESPACE);
        return (startpos == std::string::npos) ? "" : s.substr(startpos);
    }

    std::string TrimRight(const std::string& s)
    {
        size_t endpos = s.find_last_not_of(WHITESPACE);
        return (endpos == std::string::npos) ? "" : s.substr(0, endpos + 1);
    }

    std::string Trim(const std::string& s)
    {
        return TrimRight(TrimLeft(s));
    }
}
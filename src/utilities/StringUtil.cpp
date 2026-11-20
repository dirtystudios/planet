#include "StringUtil.h"

#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace dutil {

    const std::string WHITESPACE = " \n\r\t";

    std::string TrimLeft(const std::string& s) {
        size_t startpos = s.find_first_not_of(WHITESPACE);
        return (startpos == std::string::npos) ? "" : s.substr(startpos);
    }

    std::string TrimRight(const std::string& s) {
        size_t endpos = s.find_last_not_of(WHITESPACE);
        return (endpos == std::string::npos) ? "" : s.substr(0, endpos + 1);
    }

    std::vector<std::string>& Split(const std::string& s, char delim, std::vector<std::string>& elems) {
        std::stringstream ss(s);
        std::string       item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }

    std::vector<std::string> Split(const std::string& s, char delim) {
        std::vector<std::string> elems;
        Split(s, delim, elems);
        return elems;
    }

#ifdef _WIN32
    // wstring / wchar isnt consistant across platforms, this is only used for interactions with winapi
    // portable or other code should use utf16 or similar
    std::string wstring_to_utf8(std::wstring_view wstr) {
        const auto* ptr = reinterpret_cast<const char16_t*>(wstr.data());
        auto len = simdutf::utf8_length_from_utf16(ptr, wstr.size());
        std::string res(len, '0');
        auto written = simdutf::convert_utf16_to_utf8(ptr, wstr.size(), res.data());
        res.resize(written);
        return res;
    }

    std::wstring utf8_to_wstring(std::string_view str) {
        auto len = simdutf::utf16_length_from_utf8(str.data(), str.size());
        std::wstring res(len, '0');
        auto* ptr = reinterpret_cast<char16_t*>(res.data());
        auto written = simdutf::convert_utf8_to_utf16(str.data(), str.size(), ptr);
        res.resize(written);
        return res;
    }
#endif
}
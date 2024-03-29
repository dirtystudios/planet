#pragma once

#include <sstream>
#include <string>
#include <codecvt>
namespace dutil {

const std::string WHITESPACE = " \n\r\t";

static std::string TrimLeft(const std::string& s) {
    size_t startpos = s.find_first_not_of(WHITESPACE);
    return (startpos == std::string::npos) ? "" : s.substr(startpos);
}

static std::string TrimRight(const std::string& s) {
    size_t endpos = s.find_last_not_of(WHITESPACE);
    return (endpos == std::string::npos) ? "" : s.substr(0, endpos + 1);
}

static std::string Trim(const std::string& s) { return TrimRight(TrimLeft(s)); }

static std::vector<std::string>& Split(const std::string& s, char delim, std::vector<std::string>& elems) {
    std::stringstream ss(s);
    std::string       item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

static std::vector<std::string> Split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    Split(s, delim, elems);
    return elems;
}

#ifdef _WIN32
// wstring / wchar isnt consistant across platforms, this is only used for interactions with winapi
// portable or other code should use utf16 or similar
static std::string wstring_to_utf8(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> myconv;
    return myconv.to_bytes(str);
}

static std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> myconv;
    return myconv.from_bytes(str);
}
#endif
}

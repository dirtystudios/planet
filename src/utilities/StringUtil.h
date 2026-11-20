#pragma once

#include <simdutf.h>

#include <string>
#include <string_view>
#include <vector>

namespace dutil {

std::string TrimLeft(const std::string& s);
std::string TrimRight(const std::string& s);
static std::string Trim(const std::string& s) { return TrimRight(TrimLeft(s)); }
std::vector<std::string>& Split(const std::string& s, char delim, std::vector<std::string>& elems);
std::vector<std::string> Split(const std::string& s, char delim);

#ifdef _WIN32
// wstring / wchar isnt consistant across platforms, this is only used for interactions with winapi
// portable or other code should use utf16 or similar
static_assert(sizeof(wchar_t) == 2);
std::string wstring_to_utf8(std::wstring_view wstr);
std::wstring utf8_to_wstring(std::string_view str);
#endif
}

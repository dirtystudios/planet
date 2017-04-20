#pragma once

#include <sstream>
#include <string>
namespace dutil {

const std::string WHITESPACE = " \n\r\t";

static void ToLowercase(std::string& s) { std::transform(begin(s), end(s), begin(s), ::tolower); }

static std::string ToLowercase(const std::string& s) {
    std::string copied(s);
    ToLowercase(copied);
    return copied;
}
    
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
}

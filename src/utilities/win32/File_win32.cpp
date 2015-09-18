
#include "File.h"

std::string fs::GetProcessDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos).append("\\");;
}

std::string fs::AppendPathProcessDir(const std::string& path) {
    std::string currentDir = fs::GetProcessDirectory();
    currentDir.append(path);
    return currentDir;
}
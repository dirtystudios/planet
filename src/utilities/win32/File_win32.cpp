#pragma comment(lib, "shlwapi")

#include "File.h"
#include <Windows.h>
#include <Shlwapi.h>

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

bool fs::IsPathDirectory(std::string path) {
    return (PathIsDirectory(path.c_str()) > 0);
}
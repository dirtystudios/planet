#include "File.h"
#include <Windows.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi")

std::string fs::GetProcessDirectory() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    std::wstring tempStr = std::wstring(buffer).substr(0, pos).append(L"\\");
    return std::string(tempStr.begin(), tempStr.end());
}

std::string fs::AppendPathProcessDir(const std::string& path) {
    std::string currentDir = fs::GetProcessDirectory();
    currentDir = currentDir.substr(0, currentDir.length() - 1);
    // uwp doesnt add a trailing slash
    currentDir.append("\\" + path);
    return currentDir;
}

bool fs::IsPathDirectory(std::string path) {
    // k, cant use this in apps? bleh, quick hack,
    if (path == "") {
        return false;
    }
    return true;
}
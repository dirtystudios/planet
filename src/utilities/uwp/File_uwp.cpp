#include "File.h"
#include <Windows.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi")

fs::path fs::GetProcessDirectory() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    std::wstring tempStr = std::wstring(buffer).substr(0, pos).append(L"\\");
    return std::string(tempStr.begin(), tempStr.end());
}

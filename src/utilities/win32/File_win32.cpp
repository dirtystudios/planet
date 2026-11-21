#pragma comment(lib, "shlwapi")

#include "File.h"
#include "Log.h"

#include "StringUtil.h"
#include <Windows.h>
#include <Shlwapi.h>

#include <cassert>

namespace fs {
    fs::path fs::GetProcessDirectory() {
        wchar_t buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        auto str = dutil::wstring_to_utf8(buffer);
        auto pos = str.find_last_of("\\/");
        return str.substr(0, pos).append("\\");
    }
}
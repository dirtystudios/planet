#pragma comment(lib, "shlwapi")

#include "File.h"
#include "Log.h"

#include "StringUtil.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>

#include <cassert>

namespace fs {
    std::string fs::GetProcessDirectory() {
        char buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        std::string::size_type pos = std::string(buffer).find_last_of("\\/");
        return std::string(buffer).substr(0, pos).append("\\");;
    }

    bool fs::IsPathDirectory(const std::string& path) {
        return (PathIsDirectory(path.c_str()) > 0);
    }

    std::vector<std::string> fs::ListFilesInDirectory(const std::string& dir) {
        // apparently this is a thing
        assert(dir.length() < (MAX_PATH - 3));

        // cause win32 uses wildcard notation...
        std::string searchDir = dir + "\\*";
        WIN32_FIND_DATA search_data = { 0 };

        std::vector<std::string> files;

        HANDLE handle = FindFirstFile(searchDir.c_str(), &search_data);

        while (handle != INVALID_HANDLE_VALUE) {
            if (!((search_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)) {
                files.emplace_back(search_data.cFileName);
            }
            if (FindNextFile(handle, &search_data) == FALSE)
                break;
        }

        //Close the handle after use or memory/resource leak
        FindClose(handle);
        return files;
    }

    bool exists(const std::string& path) {
        return PathFileExists(path.c_str()) == TRUE;
    }

    bool mkdir(const std::string& path) {
        BOOL rtn = CreateDirectory(path.c_str(), NULL);
        return (rtn == 0 || rtn == ERROR_ALREADY_EXISTS);
    }
}
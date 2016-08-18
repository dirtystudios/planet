#pragma comment(lib, "shlwapi")

#include "File.h"
#include "Log.h"

#include <Windows.h>
#include <Shlwapi.h>

#include <cassert>
#include <iostream>
#include <fstream>

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

bool fs::ReadFileContents(const std::string& fpath, std::string* output) {
    assert(output);
    std::ifstream fin(fpath, std::ios::in | std::ios::binary);

    if (fin.fail()) {
        LOG_E("Failed to open file '%s'\n", fpath.c_str());
        return false;
    }

    output->insert(end(*output), std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());

    return true;
}
#include "stdafx.h"
#include "File.h"

std::string fs::GetProcessDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos).append("\\");;

    // UNTESTED - jake -- glgsgbgs
#elif defined(__APPLE__)
    char path[MAXPATHLEN];
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
    std::string dirPath(dirname(path));
    dirPath.append("/");
    return dirPath;
#else
    char result[PATH_MAX], result2[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    realpath(std::string(result, (count > 0) ? count : 0).c_str(), result2);
    std::string dirPath(dirname(path));
    dirPath.append("/");
    return dirPath;
#endif
}

std::string fs::AppendPathProcessDir(const std::string& path) {
    std::string currentDir = fs::GetProcessDirectory();
    currentDir.append(path);
    return currentDir;
}
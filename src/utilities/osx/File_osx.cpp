#include "File.h"
#include <cassert>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Log.h"
#include "StringUtil.h"
#include "WatchDirManager.h"

std::string fs::GetProcessDirectory() {
    char current_path[FILENAME_MAX];

    if (!getcwd(current_path, sizeof(current_path))) {
        assert(false);
    }

    return std::string(current_path) + "/";
}

// Untested -- jake
bool fs::IsPathDirectory(std::string path) {
    struct stat fileAtt;

    if (stat(path.c_str(), &fileAtt) != 0) {
        LOG_E("Couldnt stat file %s\n", path.c_str());
        return false;
    }

    return S_ISDIR(fileAtt.st_mode);
}

bool fs::mkdir(const std::string& path) {
    struct stat st;
    int         status = 0;

    if (stat(path.c_str(), &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (::mkdir(path.c_str(), 0777) != 0 && errno != EEXIST) {
            return false;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return false;
    }

    return true;
}

bool fs::mkdirs(const std::string& path) {
    if (exists(path))
        return true;

    std::vector<std::string> splits = dutil::Split(path, '/');

    std::string currentPath = "";
    for (std::string& split : splits) {
        currentPath += "/" + split;
        if (!fs::mkdir(currentPath)) {
            return false;
        }
    }
    return true;
}

bool fs::exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

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

fs::path fs::GetProcessDirectory() {
    char current_path[FILENAME_MAX];

    if (!getcwd(current_path, sizeof(current_path))) {
        assert(false);
    }

    return fs::path{ std::string(current_path) + "/" };
}


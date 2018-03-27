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
bool fs::IsPathDirectory(const std::string& path) {
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

bool fs::exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

std::vector<std::string> fs::ListFilesInDirectory(const std::string& path) {
    std::vector<std::string> fnames;
    DIR*                     dirFile = opendir(path.c_str());
    if (dirFile) {
        struct dirent* hFile;
        
        while (( hFile = readdir( dirFile )) != NULL )
        {
            if ( !strcmp( hFile->d_name, "."  )) continue;
            if ( !strcmp( hFile->d_name, ".." )) continue;
            
            // in linux hidden files all start with '.'
            if ( hFile->d_name[0] == '.' ) continue;
            
            // dirFile.name is the name of the file. Do whatever string comparison
            // you want here. Something like:
            //            if ( strstr( hFile->d_name, ".txt" ))
            if (hFile->d_type == DT_REG) {
                fnames.push_back(hFile->d_name);
            }
            //  printf( "%s\n", hFile->d_name);
        }
        closedir(dirFile);
    }
    return fnames;
}

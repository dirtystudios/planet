#include "File.h"
#include "util.h"
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <cassert>
#include <fstream>
#include "Log.h"
#include "WatchDirManager.h"

std::string fs::GetProcessDirectory() {
    char current_path[FILENAME_MAX];

    if (!getcwd(current_path, sizeof(current_path))) {
        assert(false);
    }

    return std::string(current_path);
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

std::string fs::AppendPathProcessDir(const std::string& path) {
    std::string currentDir = fs::GetProcessDirectory();
    currentDir.append(path);
    return currentDir;
}

std::vector<std::string> fs::ListFilesInDirectory(const std::string& path) {
    std::vector<std::string> fnames;
    DIR* dirFile = opendir( path.c_str() );
    if ( dirFile )
    {
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

uint64_t fs::WatchDir(const std::string& path, fs::FileEventDelegate delegate) {
    return fs::WatchDirManager::AddWatcher(path, delegate);
}

bool fs::StopWatching(uint64_t watcherId) { return fs::WatchDirManager::RemoveWatcher(watcherId); }

std::string fs::GetParentDir(const std::string& fpath) {
    size_t pos = fpath.find_last_of("/");
    if(pos == std::string::npos) {
        return "";
    }
    return fpath.substr(0, pos);
}

#include "File.h"
#include "util.h"
#include <dirent.h>
#include <iostream>

std::string fs::GetProcessDirectory() {
    // char path[MAXPATHLEN];
    // uint32_t size = sizeof(path);
    // _NSGetExecutablePath(path, &size);
    // std::string dirPath(dirname(path));
    // dirPath.append("/");
    return "";//dirPath;
}

// Untested -- jake
bool fs::IsPathDirectory(std::string path) {
    struct stat fileAtt;

    if (stat(path.c_str(), &fileAtt) != 0)
        throw errno;

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
            if(hFile->d_type == DT_REG) {                
                fnames.push_back(hFile->d_name);
            }
              //  printf( "%s\n", hFile->d_name);
        }         
        closedir( dirFile );
    }
    return fnames;
}
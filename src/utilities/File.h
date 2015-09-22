#pragma once

#include <string>
#include <vector>

namespace fs {

    /**
    * Returns directory program resides in, this *should* leave the trailing slash
    */
    std::string GetProcessDirectory();

    /**
    * Appends path/filename to process dir
    */
    std::string AppendPathProcessDir(const std::string& path);

    /**
     * Lists all non-directory files in given path
    **/
    std::vector<std::string> ListFilesInDirectory(const std::string& dir);

    bool IsPathDirectory(std::string path);
}
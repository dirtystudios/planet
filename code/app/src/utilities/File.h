#pragma once

namespace fs {

    /**
    * Returns directory program resides in, this *should* leave the trailing slash
    */
    std::string GetProcessDirectory();

    /**
    * Appends path/filename to process dir
    */
    std::string AppendPathProcessDir(const std::string& path);
}
#include "File.h"

#include "Log.h"
#include <cassert>
#include <iostream>
#include <fstream>

namespace fs {
    bool ReadFileContents(const std::string& fpath, std::string* output) {
        assert(output);
        std::ifstream fin(fpath, std::ios::in | std::ios::binary);

        if (fin.fail()) {
            LOG_E("Failed to open file '%s'\n", fpath.c_str());
            return false;
        }

        output->insert(end(*output), std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());

        return true;
    }

    std::string AppendPathProcessDir(const std::string& path) {
        std::string currentDir = GetProcessDirectory();
        currentDir.append(path);
        return currentDir;
    }

    std::string GetParentDir(const std::string& fpath) {
        size_t pos = fpath.find_last_of("/");
        if (pos == std::string::npos) {
            return "";
        }
        return fpath.substr(0, pos);
    }

}

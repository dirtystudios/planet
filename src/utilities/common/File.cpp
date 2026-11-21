#include "File.h"

#include "StringUtil.h"
#include "Log.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <regex>

namespace fs {
    
    bool ReadFileContents(const fs::path& fpath, std::string* output) {
        assert(output);
        std::ifstream fin(fpath, std::ios::in | std::ios::binary);

        if (fin.fail()) {
            LOG_E("Failed to open file '%s'\n", fpath.c_str());
            return false;
        }

        output->insert(end(*output), std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());

        return true;
    }
}

#include "File.h"

#include "StringUtil.h"
#include "Log.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <regex>

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

    std::string SanitizeFilePath(const std::string& fpath) {
        const static std::regex kMatch("\\\\+");
        const static std::string kReplace = "/";
        if (fpath == "") return fpath;

        return std::regex_replace(fpath, kMatch, kReplace);
    }

    std::string FullPathDirName(const std::string& s) {
        size_t pos = s.find_last_of("\\/");
        return (std::string::npos == pos)
            ? ""
            : s.substr(0, pos);
    }

    std::string FileName(const std::string& s) {
        const std::string fp = SanitizeFilePath(s);
        std::string file;

        // grab everything after last '/'
        auto found = fp.find_last_of("/");
        if (found == std::string::npos)
            file = fp;
        else
            file = fp.substr(found + 1);

        // slice off extension
        // also attempt to handle case of 'dotfile'
        found = file.find_last_of(".");
        if (found == std::string::npos)
            return file;
        else if (found > 0)
            return file.substr(0, found);
        else
            return file.substr(1);
    }

    bool mkdirs(const std::string& path) {
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
}

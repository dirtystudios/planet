#pragma once

#include <string>
#include <vector>

namespace fs {
// ------------------
//  Common
// ------------------
/**
* Read entires contents of file and append to provided string paramter
*
* @return Success of file read
*/
bool ReadFileContents(const std::string& fpath, std::string* output);

/**
* Appends path/filename to process dir
*/
std::string AppendPathProcessDir(const std::string& path);


std::string GetParentDir(const std::string& fpath);

// replace all "\\" with "/"
std::string SanitizeFilePath(const std::string& fpath);

// Returns fullpath of the containing directory for a file, should probably sanitize path first
std::string DirName(const std::string& s);

// -------------------------
// Implementation specific
// ------------------------

/**
* Returns directory program resides in, this *should* leave the trailing slash
*/
std::string GetProcessDirectory();

/**
 * Lists all non-directory files in given path
**/
std::vector<std::string> ListFilesInDirectory(const std::string& dir);

bool IsPathDirectory(std::string path);

bool exists(const std::string& path);
bool mkdir(const std::string& path);
bool mkdirs(const std::string& path);
}

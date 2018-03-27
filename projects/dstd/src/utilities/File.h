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
std::string FullPathDirName(const std::string& path);

// Returns name of file, no extension from a path
std::string FileName(const std::string& path);

// Makes directories recursively for full path
bool mkdirs(const std::string& path);

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

bool IsPathDirectory(const std::string& path);

bool exists(const std::string& path);
bool mkdir(const std::string& path);
}

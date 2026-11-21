#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs {
using namespace std::filesystem;
// ------------------
//  Common
// ------------------
/**
* Read entires contents of file and append to provided string paramter
*
* @return Success of file read
*/
bool ReadFileContents(const fs::path& fpath, std::string* output);

// -------------------------
// Implementation specific
// ------------------------

/**
* Returns directory program resides in, this *should* leave the trailing slash
*/
fs::path GetProcessDirectory();
}

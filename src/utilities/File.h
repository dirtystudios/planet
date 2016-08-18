#pragma once

#include <string>
#include <vector>
#include <functional>

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

/**
 * Read entires contents of file and append to provided string paramter
 *
 * @return Success of file read
 */
bool ReadFileContents(const std::string& fpath, std::string* output);

enum class FileEventType : uint8_t {
    FileDeleted = 0,
    FileModified,
    FileCreated,
};

struct FileEvent {
    FileEventType type;
    std::string fpath;
};

using WatcherId         = uint64_t;
using FileEventDelegate = std::function<void(const FileEvent& event)>;

WatcherId WatchDir(const std::string& path, FileEventDelegate delegate);
bool StopWatching(WatcherId watcherId);
    
}

#pragma once
#include "File.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

namespace fs {

// Implementation specific
struct Watcher;

using WatcherId = uint64_t;

enum class FileEventType : uint8_t {
    FileDeleted = 0,
    FileModified,
    FileCreated,
};

struct FileEvent {
    FileEventType type;
    std::string fpath;
};

using FileEventDelegate = std::function<void(const FileEvent& event)>;

class WatchDirManager {
private:
    static std::vector<std::unique_ptr<Watcher>> _watchers;
    static std::unordered_map<std::string, std::vector<Watcher*>> _watchedPaths;
public:
    static WatcherId AddWatcher(const std::string& path, fs::FileEventDelegate delegate);
    static bool RemoveWatcher(WatcherId watcherId);
};
}

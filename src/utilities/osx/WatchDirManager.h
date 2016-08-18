#pragma once

#include "File.h"
#include <unordered_map>
#include <CoreServices/CoreServices.h>

namespace fs {

struct Watcher {
    WatcherId id;
    FSEventStreamRef stream;
    fs::FileEventDelegate delegate;
    std::string path;
};

class WatchDirManager {
private:
    static std::vector<Watcher> _watchers;
    static std::unordered_map<std::string, std::vector<Watcher*>> _watchedPaths;

private:
    static void FsEventDelegate(ConstFSEventStreamRef streamRef, void* clientCallBackInfo, size_t numEvents,
                                void* eventPaths, const FSEventStreamEventFlags eventFlags[],
                                const FSEventStreamEventId eventIds[]);

public:
    static WatcherId AddWatcher(const std::string& path, fs::FileEventDelegate delegate);
    static bool RemoveWatcher(WatcherId watcherId);
};
}

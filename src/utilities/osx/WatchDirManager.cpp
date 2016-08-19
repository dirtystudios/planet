#include "WatchDirManager.h"
#include <CoreServices/CoreServices.h>

namespace fs {

struct Watcher {
    WatcherId id;
    FSEventStreamRef stream;
    fs::FileEventDelegate delegate;
    std::string path;
};

std::vector<Watcher> WatchDirManager::_watchers = std::vector<Watcher>();
std::unordered_map<std::string, std::vector<Watcher*>> WatchDirManager::_watchedPaths =
    std::unordered_map<std::string, std::vector<Watcher*>>();

void WatchDirManager::FsEventDelegate(ConstFSEventStreamRef streamRef, void* clientCallBackInfo, size_t numEvents,
                                      void* eventPaths, const FSEventStreamEventFlags eventFlags[],
                                      const FSEventStreamEventId eventIds[]) {

    char** paths = (char**)eventPaths;

    FSEventStreamEventFlags interest =
        kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemRemoved | kFSEventStreamEventFlagItemModified;
    for (uint32_t i = 0; i < numEvents; i++) {
        FSEventStreamEventFlags flags = eventFlags[i];
        if ((flags & interest) == 0) {
            continue;
        }
        
        std::string eventPath = paths[i];
        std::string parent = GetParentDir(eventPath);
        auto observers = _watchedPaths.find(parent);
        if (observers == end(_watchedPaths)) {
            continue;
        }

        fs::FileEvent event;
        event.fpath = std::string(paths[i]);
        if (flags & kFSEventStreamEventFlagItemCreated) {
            event.type = fs::FileEventType::FileCreated;
        } else if (flags & kFSEventStreamEventFlagItemRemoved) {
            event.type = fs::FileEventType::FileDeleted;
        } else if (flags & kFSEventStreamEventFlagItemModified) {
            event.type = fs::FileEventType::FileModified;
        } else {
            // huh
        }
        
        // TODO: A smart person with do this on another thread since chances are Ill be doing IO in these callbacks
        for (const Watcher* watcher : observers->second) {
            watcher->delegate(event);
        }
    }
}

WatcherId WatchDirManager::AddWatcher(const std::string& path, fs::FileEventDelegate delegate) {
    static uint64_t key     = 1;
    CFStringRef mypath      = CFStringCreateWithCStringNoCopy(NULL, path.c_str(), kCFStringEncodingUTF8, kCFAllocatorNull);
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void**)&mypath, 1, NULL);
    FSEventStreamRef stream;
    CFAbsoluteTime latency = 3.0; // seconds

    stream = FSEventStreamCreate(NULL, &WatchDirManager::FsEventDelegate, NULL, pathsToWatch,
                                 kFSEventStreamEventIdSinceNow, latency, kFSEventStreamCreateFlagFileEvents);

    /* Create the stream before calling this. */
    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);


    std::unique_ptr<Watcher> watcher(new Watcher());

    watcher->id = key++;
    watcher->stream = stream;
    watcher->delegate = delegate;
    watcher->path = path;

    _watchers.push_back(std::move(watcher));

    auto it = _watchedPaths.find(path);
    if (it == end(_watchedPaths)) {
        std::vector<Watcher*> watchers;
        _watchedPaths.insert(std::make_pair(path, watchers));
    }
    _watchedPaths[path].push_back(watcher.get());

    return watcher->id;
}

bool WatchDirManager::RemoveWatcher(WatcherId watcherId) {
    auto result =
        std::find_if(begin(_watchers), end(_watchers), [&](const std::unique_ptr<Watcher>& watcher) { return watcher->id == watcherId; });
    if (result == end(_watchers)) {
        return false;
    }
    FSEventStreamStop(result->get()->stream);
    FSEventStreamInvalidate(result->get()->stream);
    FSEventStreamRelease(result->get()->stream);
    auto pathObservers = _watchedPaths.find(result->get()->path);
    if (pathObservers == end(_watchedPaths)) {
        // this is unexpected, but ok
    } else {
         Watcher* watcher = result->get();
        auto watcherRef = std::find(begin(pathObservers->second), end(pathObservers->second), watcher);
        pathObservers->second.erase(watcherRef);
    }
    _watchers.erase(result);
    return true;
}
}

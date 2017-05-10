#include "WatchDirManager.h"
#include <CoreServices/CoreServices.h>

namespace fs {

struct Watcher {
    WatcherId id;
    FSEventStreamRef stream;
    fs::FileEventDelegate delegate;
    std::string path;
};

std::vector<std::unique_ptr<Watcher>> WatchDirManager::_watchers = std::vector<std::unique_ptr<Watcher>>();
std::unordered_map<std::string, std::vector<Watcher*>> WatchDirManager::_watchedPaths =
    std::unordered_map<std::string, std::vector<Watcher*>>();

void FsEventDelegate(ConstFSEventStreamRef streamRef, void* clientCallBackInfo, size_t numEvents,
                                      void* eventPaths, const FSEventStreamEventFlags eventFlags[],
                                      const FSEventStreamEventId eventIds[]) {

    char** paths = (char**)eventPaths;
    Watcher* watcher = reinterpret_cast<Watcher*>(clientCallBackInfo);
    
    FSEventStreamEventFlags interest =
        kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemRemoved | kFSEventStreamEventFlagItemModified;
    for (uint32_t i = 0; i < numEvents; i++) {
        FSEventStreamEventFlags flags = eventFlags[i];
        if ((flags & interest) == 0) {
            continue;
        }
        
        std::string eventPath = paths[i];
    
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
        
        watcher->delegate(event);
    }
}

WatcherId WatchDirManager::AddWatcher(const std::string& path, fs::FileEventDelegate delegate) {
    static uint64_t key     = 1;
    CFStringRef mypath      = CFStringCreateWithCStringNoCopy(NULL, path.c_str(), kCFStringEncodingUTF8, kCFAllocatorNull);
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void**)&mypath, 1, NULL);
    FSEventStreamRef stream;
    CFAbsoluteTime latency = 3.0; // seconds

    std::unique_ptr<Watcher> watcher(new Watcher());
    FSEventStreamContext context = { 0, reinterpret_cast<void*>(watcher.get()), nullptr, nullptr};
    
    stream = FSEventStreamCreate(NULL, &FsEventDelegate, &context, pathsToWatch,
                                 kFSEventStreamEventIdSinceNow, latency, kFSEventStreamCreateFlagFileEvents);

    /* Create the stream before calling this. */
    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);

    watcher->id = key++;
    watcher->stream = stream;
    watcher->delegate = delegate;
    watcher->path = path;
    
    auto it = _watchedPaths.find(path);
    if (it == end(_watchedPaths)) {
        std::vector<Watcher*> watchers;
        _watchedPaths.insert(std::make_pair(path, watchers));
    }
    _watchedPaths[path].push_back(watcher.get());
    
    WatcherId watcherId = watcher->id;
    _watchers.push_back(std::move(watcher));

    return watcherId;
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

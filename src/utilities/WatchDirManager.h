#pragma once
#include "File.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#else
#include <CoreServices/CoreServices.h>
#endif


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
private:
#ifdef _WIN32
    static void CALLBACK NotificationCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
#else
    static void FsEventDelegate(ConstFSEventStreamRef streamRef, void* clientCallBackInfo, size_t numEvents,
        void* eventPaths, const FSEventStreamEventFlags eventFlags[],
        const FSEventStreamEventId eventIds[]);
#endif
};
}

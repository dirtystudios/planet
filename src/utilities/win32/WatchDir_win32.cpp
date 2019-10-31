#include "WatchDirManager.h"
#include "File.h"
#include "Log.h"

#include <memory>
#include <algorithm>
#include <vector>
#include <cassert>
#include <atlstr.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace fs {

    struct Watcher {
        WatcherId id;
        HANDLE dirHandle;
        FileEventDelegate delegate;
        std::string path;
        std::vector<BYTE> buffer;
        std::vector<BYTE> backBuffer;
        DWORD bytesReturned;
    };

    const DWORD dwNotificationFlags =
        FILE_NOTIFY_CHANGE_LAST_WRITE
        | FILE_NOTIFY_CHANGE_CREATION
        | FILE_NOTIFY_CHANGE_FILE_NAME;

    std::vector<std::unique_ptr<Watcher>> WatchDirManager::_watchers = std::vector<std::unique_ptr<Watcher>>();
    std::unordered_map<std::string, std::vector<Watcher*>> WatchDirManager::_watchedPaths =
        std::unordered_map<std::string, std::vector<Watcher*>>();

    void CALLBACK NotificationCompletion(
        DWORD dwErrorCode,									// completion code
        DWORD dwNumberOfBytesTransfered,					// number of bytes transferred
        LPOVERLAPPED lpOverlapped)							// I/O information buffer
    {
        Watcher* watcher = static_cast<Watcher*>(lpOverlapped->hEvent);

        if (dwErrorCode == ERROR_OPERATION_ABORTED) {
            // log this for now
            LOG_E("WatchDir - Operation Aborted!");
            return;
        }

        // Can't use sizeof(FILE_NOTIFY_INFORMATION) because
        // the structure is padded to 16 bytes.
        assert(dwNumberOfBytesTransfered >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof(WCHAR));

        // This might mean overflow? Not sure.
        if (!dwNumberOfBytesTransfered) {
            LOG_E("WatchDir - Potential overflow from notification");
            return;
        }

        memcpy(watcher->backBuffer.data() , watcher->buffer.data(), dwNumberOfBytesTransfered);

        // Get the new read issued as fast as possible. The documentation
        // says that the original OVERLAPPED structure will not be used
        // again once the completion routine is called.
        OVERLAPPED overlapped{ 0 };
        overlapped.hEvent = watcher;

        BOOL success = ReadDirectoryChangesW(
            watcher->dirHandle,						// handle to directory
            watcher->buffer.data(),             // read results buffer
            watcher->buffer.size(),             // length of buffer
            false,                 // monitoring option
            dwNotificationFlags,                    // filter conditions
            &watcher->bytesReturned,                           // bytes returned
            &overlapped,                      // overlapped buffer
            &NotificationCompletion);           // completion routine

        if (!success) {
            LOG_E("WatchDir - ReadDirChangesW reset Failed, %d", GetLastError());
            return;
        }

        BYTE* pBase = watcher->backBuffer.data();

        while(true) {
            FILE_NOTIFY_INFORMATION& fni = (FILE_NOTIFY_INFORMATION&)*pBase;

            CStringW wstrFilename(fni.FileName, fni.FileNameLength / sizeof(wchar_t));
            std::wstring stemp = std::wstring(watcher->path.begin(), watcher->path.end());
            CStringW sw = stemp.c_str();
            // Handle a trailing backslash, such as for a root directory.
            if (wstrFilename.Right(1) != L"\\")
                wstrFilename = sw + L"\\" + wstrFilename;
            else
                wstrFilename = sw + wstrFilename;

            // If it could be a short filename, expand it.
            LPCWSTR wszFilename = PathFindFileNameW(wstrFilename);
            int len = lstrlenW(wszFilename);
            // The maximum length of an 8.3 filename is twelve, including the dot.
            if (len <= 12 && wcschr(wszFilename, L'~')) {
                // Convert to the long filename form. Unfortunately, this
                // does not work for deletions, so it's an imperfect fix.
                wchar_t wbuf[MAX_PATH];
                if (GetLongPathNameW(wstrFilename, wbuf, _countof(wbuf)) > 0)
                    wstrFilename = wbuf;
            }

            FileEvent fEvent;
            switch (fni.Action) {
            case FILE_ACTION_ADDED :
                fEvent.type = FileEventType::FileCreated;
                break;
            case FILE_ACTION_REMOVED:
                fEvent.type = FileEventType::FileDeleted;
                break;
            case FILE_ACTION_MODIFIED:
                fEvent.type = FileEventType::FileModified;
                break;
            default:
                LOG_E("WatchDir - Invalid Event given in notification");
            }

            fEvent.fpath = CW2A(wstrFilename);

            watcher->delegate(fEvent);

            if (!fni.NextEntryOffset)
                break;
            pBase += fni.NextEntryOffset;
        };
    }


    WatcherId WatchDirManager::AddWatcher(const std::string& path, FileEventDelegate delegate) {
        static uint64_t key = 1;

        std::wstring stemp = std::wstring(path.begin(), path.end());
        LPCWSTR sw = stemp.c_str();

        HANDLE handle = CreateFileW(
            sw,					// pointer to the file name
            FILE_LIST_DIRECTORY,                // access (read/write) mode
            FILE_SHARE_READ						// share mode
            | FILE_SHARE_WRITE
            | FILE_SHARE_DELETE,
            NULL,                               // security descriptor
            OPEN_EXISTING,                      // how to create
            FILE_FLAG_BACKUP_SEMANTICS			// file attributes
            | FILE_FLAG_OVERLAPPED,
            NULL);                              // file with attributes to copy

        if (handle == INVALID_HANDLE_VALUE) {
            LOG_E("WatchDir - Invalid Handle");
            return 0;
        }

        std::unique_ptr<Watcher> watcher(new Watcher());

        watcher->id = key++;
        watcher->dirHandle = handle;
        watcher->delegate = delegate;
        watcher->path = path;
        watcher->buffer.resize(16384);
        watcher->backBuffer.resize(16384);
        watcher->bytesReturned = 0;

        OVERLAPPED overlapped{ 0 };
        overlapped.hEvent = watcher.get();

        BOOL success = ReadDirectoryChangesW(
            handle,						// handle to directory
            watcher->buffer.data(),             // read results buffer
            watcher->buffer.size(),             // length of buffer
            false,                 // monitoring option
            dwNotificationFlags,                    // filter conditions
            &watcher->bytesReturned,                           // bytes returned
            &overlapped,                      // overlapped buffer
            &NotificationCompletion);           // completion routine

        if (!success) {
            LOG_E("WatchDir - ReadDirChangesW Failed, %d", GetLastError());
            return 0;
        }

        auto it = _watchedPaths.find(path);
        if (it == end(_watchedPaths)) {
            std::vector<Watcher*> watchers;
            _watchedPaths.insert(std::make_pair(path, watchers));
        }
        _watchedPaths[path].push_back(watcher.get());
        _watchers.push_back(std::move(watcher));

        return key-1;
    }

    bool WatchDirManager::RemoveWatcher(WatcherId watcherId) {
        auto result =
            std::find_if(begin(_watchers), end(_watchers), [&](const std::unique_ptr<Watcher>& watcher) { return watcher->id == watcherId; });
        if (result == end(_watchers)) {
            return false;
        }

        CancelIo(result->get()->dirHandle);
        CloseHandle(result->get()->dirHandle);

        auto pathObservers = _watchedPaths.find(result->get()->path);
        if (pathObservers == end(_watchedPaths)) {
            LOG_D("WatchDir - Path already removed");
        }
        else {
            Watcher* watcher = result->get();
            auto watcherRef = std::find(begin(pathObservers->second), end(pathObservers->second), watcher);
            pathObservers->second.erase(watcherRef);
        }
        _watchers.erase(result);
        return true;
    }
}
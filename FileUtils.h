#ifndef __file_utils_h__
#define __file_utils_h__



namespace file {
    struct FileHandle {

    };

    struct FileWatcherHandle {

    };

    enum FileEvent {

    };

    bool should_init = true;

    static void __init() {
        if(!should_init) return;

        if ((kq = kqueue()) < 0) {
            fprintf(stderr, "Could not open kernel queue.  Error was %s.\n", strerror(errno));
        }
    }

    static FileHandle OpenFile(const char* fpath) {
        return FileHandle();
    }

    static FileWatcherHandle StartWatching(const char* fpath, std::function<void(const char* directory, const char* file, FileEvent event_type)> callback) {
        __init();

        struct kevent events_to_monitor[1];
        struct kevent event_data[1];
        
        int32_t event_fd = open(fpath, O_EVTONLY);
        if (event_fd <=0) {
            fprintf(stderr, "The file %s could not be opened for monitoring.  Error was %s.\n", path, strerror(errno));
        }

        uint32_t vnode_events = NOTE_DELETE |  NOTE_WRITE | NOTE_EXTEND |                            NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE;
        EV_SET( &events_to_monitor[0], event_fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, vnode_events, 0, user_data);
        return FileWatcherHandle();
    }

    static FileWatcherHandle StopWatching(const char* fpath) {
        __init();
        return FileWatcherHandle();
    }
}

#endif
#pragma once

#include "singleton.h"
#include "utils/libcu.h"
#include "utils/cu_sched.h"
#include "utils/CuLogger.h"
#include <sys/inotify.h>

class FileWatcher : public Singleton<FileWatcher> 
{
    public:
        using WatchNotifier = std::function<void(void)>;

        FileWatcher() : watcherMap_(), inotify_fd_(inotify_init()) 
        {
            if (inotify_fd_ < 0) {
                CU::Logger::Error("Failed to init inotify.");
                CU::Logger::Flush();
                std::exit(0);
            }
            std::thread thread_(std::bind(&FileWatcher::WatcherMain_, this));
            thread_.detach();
        }

        ~FileWatcher()
        {
            close(inotify_fd_);
        }

        void AddWatch(const std::string &path, const WatchNotifier &notifier)
        {
            int wd = inotify_add_watch(inotify_fd_, path.c_str(), IN_MODIFY);
            if (wd >= 0) {
                watcherMap_.emplace(wd, notifier);
            } else {
                CU::Logger::Warn("Failed to watch %s.", path.c_str());
            }
        }

    private:
        std::unordered_map<int, WatchNotifier> watcherMap_;
        int inotify_fd_;

        void WatcherMain_()
        {
            SetThreadName("FileWatcher");
            SetTaskSchedPrio(0, 95);

            auto buffer = new inotify_event[128];
            for (;;) {
                memset(buffer, 0, sizeof(inotify_event) * 128);
                auto len = read(inotify_fd_, buffer, sizeof(inotify_event) * 128);
                if (len >= 0) {
                    off_t offset = 0;
                    while ((sizeof(inotify_event) * offset) < len) {
                        auto event = *(buffer + offset);
                        watcherMap_.at(event.wd)();
                        offset++;
                    }
                } else {
                    CU::Logger::Error("Failed to read inotify fd.");
                    CU::Logger::Flush();
                    std::exit(0);
                }
            }
            delete[] buffer;
        }
};

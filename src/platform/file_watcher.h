#pragma once

#include "singleton.h"
#include "utils/libcu.h"
#include "utils/cu_sched.h"
#include <sys/inotify.h>

class FileWatcher : public Singleton<FileWatcher> 
{
    public:
		using WatchNotifier = std::function<void(void)>;

		FileWatcher() : watcherMap_(), inotify_fd_(inotify_init()) 
		{
    		if (inotify_fd_ < 0) {
        		std::runtime_error("Failed to init inotify.");
    		}
    		std::thread thread_(std::bind(&FileWatcher::WatcherMain_, this));
    		thread_.detach();
		}

		void AddWatch(const std::string &path, const WatchNotifier &notifier)
		{
    		int wd = inotify_add_watch(inotify_fd_, path.c_str(), IN_MODIFY);
    		if (wd >= 0) {
        		watcherMap_.emplace(wd, notifier);
    		}
		}

	private:
		std::unordered_map<int, WatchNotifier> watcherMap_;
		int inotify_fd_;

		void WatcherMain_()
		{
    		SetThreadName("FileWatcher");
    		SetTaskSchedPrio(0, 95);

    		static constexpr size_t buffer_size = sizeof(struct inotify_event) * 4096;
    		char* buffer = new char[buffer_size];
    		for (;;) {
        		memset(buffer, 0, buffer_size);
        		auto len = read(inotify_fd_, buffer, buffer_size);
        		off_t offset = 0;
        		while (offset < len) {
           	 		auto event = reinterpret_cast<struct inotify_event*>(buffer + offset);
            		auto iter = watcherMap_.find(event->wd);
            		if (iter != watcherMap_.end()) {
                		(iter->second)();
            		}
            		offset += sizeof(struct inotify_event) + event->len;
        		}
    		}
    		delete[] buffer;
		}
};

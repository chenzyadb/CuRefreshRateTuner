#pragma once

#include "platform/file_watcher.h"
#include "platform/worker_thread.h"

class Module
{
	public:
		Module() { }
		virtual ~Module() { }
		virtual void Start() = 0;

	protected:
		void FileWatcher_AddWatch(const std::string &path, const FileWatcher::WatchNotifier &wn)
		{
			FileWatcher::GetInstance()->AddWatch(path, wn);
		}

		void WorkerThread_AddWork(const WorkerThread::WorkTask &task) 
		{
			WorkerThread::GetInstance()->AddWork(task);
		}
};

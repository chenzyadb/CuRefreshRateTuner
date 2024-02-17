#pragma once

#include "platform/timer.h"
#include "platform/file_watcher.h"
#include "platform/worker_thread.h"

class Module
{
	public:
		Module() { }
		virtual ~Module() { }
		virtual void Start() = 0;

	protected:
		void Timer_AddTimer(const std::string &name, const Timer::TimerTask &task, const int &intervalMs)
		{
			Timer::GetInstance()->AddTimer(name, task, intervalMs);
		}

		void Timer_DeleteTimer(const std::string &name)
		{
			Timer::GetInstance()->DeleteTimer(name);
		}

		void Timer_PauseTimer(const std::string &name)
		{
			Timer::GetInstance()->PauseTimer(name);
		}

		void Timer_ContinueTimer(const std::string &name)
		{
			Timer::GetInstance()->ContinueTimer(name);
		}

		bool Timer_IsTimerExist(const std::string &name)
		{
			return Timer::GetInstance()->IsTimerExist(name);
		}

		void FileWatcher_AddWatch(const std::string &path, const FileWatcher::WatchNotifier &wn)
		{
			FileWatcher::GetInstance()->AddWatch(path, wn);
		}

		void WorkerThread_AddWork(const WorkerThread::WorkTask &task) 
		{
			WorkerThread::GetInstance()->AddWork(task);
		}
};

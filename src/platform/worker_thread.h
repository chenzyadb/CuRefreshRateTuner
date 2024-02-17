#pragma once

#include "singleton.h"
#include "utils/libcu.h"
#include "utils/cu_sched.h"

class WorkerThread : public Singleton<WorkerThread> 
{
    public:
        using WorkTask = std::function<void(void)>;

        WorkerThread() : mtx_(), cv_(), tasks_()
        {
            std::thread thread_(std::bind(&WorkerThread::Main_, this));
            thread_.detach();
        }   

        void AddWork(const WorkTask &task) 
        {
            std::unique_lock<std::mutex> lck(mtx_);
            tasks_.emplace_back(task);
            cv_.notify_all();
        }

    private:
        std::mutex mtx_;
        std::condition_variable cv_;
        std::vector<WorkTask> tasks_;

        void Main_()
        {
            SetThreadName("WorkerThread");
            SetTaskSchedPrio(0, 95);

            for (;;) {
                std::vector<WorkTask> runTasks{};
                {
                    std::unique_lock<std::mutex> lck(mtx_);
                    while (tasks_.size() == 0) {
                        cv_.wait(lck);
                    }
                    runTasks = tasks_;
                    tasks_.clear();
                }
                for (const auto &task : runTasks) {
                    task();
                }
            }
        }
};

#pragma once

#include "singleton.h"
#include "utils/libcu.h"
#include "utils/cu_sched.h"

class Timer : public Singleton<Timer> 
{
    public:
        using TimerTask = std::function<void(void)>;

        Timer() : timerMap_(), mtx_() { }

        void AddTimer(const std::string &name, const TimerTask &task, const int &intervalMs)
        {
            std::unique_lock<std::mutex> lck(mtx_);
            if (timerMap_.count(name) == 1) {
                return;
            }
            TimerData timerData{};
            timerData.task = task;
            timerData.intervalMs = intervalMs;
            timerData.pause = false;
            timerMap_.emplace(name, timerData);
    
            std::thread timerThread(std::bind(&Timer::TimerThread_, this, name));
            timerThread.detach();
        }

        void DeleteTimer(const std::string &name)
        {
            std::unique_lock<std::mutex> lck(mtx_);
            auto iter = timerMap_.find(name);
            if (iter != timerMap_.end()) {
                timerMap_.erase(iter);
            }
        }

        void PauseTimer(const std::string &name)
        {
            std::unique_lock<std::mutex> lck(mtx_);
            auto iter = timerMap_.find(name);
            if (iter == timerMap_.end()) {
                return;
            }
            iter->second.pause = true;
        }

        void ContinueTimer(const std::string &name)
        {
            std::unique_lock<std::mutex> lck(mtx_);
            auto iter = timerMap_.find(name);
            if (iter == timerMap_.end()) {
                return;
            }
            iter->second.pause = false;
        }

        bool IsTimerExist(const std::string &name)
        {
            std::unique_lock<std::mutex> lck(mtx_);
            return (timerMap_.count(name) == 1);
        }

    private:
        struct TimerData {
            TimerTask task;
            int intervalMs;
            bool pause;
        };
        
        std::unordered_map<std::string, TimerData> timerMap_;
        std::mutex mtx_;

        void TimerThread_(const std::string &name) 
        {
            auto timerName = name;
            SetThreadName(StrMerge("Timer:%s", timerName.c_str()));
            SetTaskSchedPrio(0, 120);

            for (;;) {
                TimerData data{};
                {
                    std::unique_lock<std::mutex> lck(mtx_);
                    auto iter = timerMap_.find(timerName);
                    if (iter == timerMap_.end()) {
                        break;
                    }
                    data = iter->second;
                }
                if (!data.pause) {
                    const auto &task = data.task;
                    task();
                }
                usleep(data.intervalMs * 1000);
            }

            return;
        }
};

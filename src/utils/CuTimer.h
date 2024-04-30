// CuTimer by chenzyadb@github.com
// Based on C++17 STL (MSVC)

#ifndef _CU_TIMER_
#define _CU_TIMER_

#include <chrono>
#include <thread>
#include <mutex>
#include <functional>

#define TIMER_LOOP(timer) while((timer)._Timer_Condition())

namespace CU
{
    class TimerExcept : public std::exception
    {
        public:
            TimerExcept(const std::string &message) : message_(message) { }

            const char* what() const noexcept override
            {
                return message_.c_str();
            }

        private:
            const std::string message_;
    };

    class Timer
    {
        public:
            typedef std::function<void(void)> Task;

            static void SingleShot(time_t interval, const Task &callback)
            {
                std::thread timerThread([interval, callback]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                    callback();
                });
                timerThread.detach();
            }

            Timer() : interval_(0), loop_(), mtx_(), cv_(), started_(false), paused_(false), requestStop_(false) { }

            ~Timer()
            {
                if (started_) {
                    stop();
                }
            }

            bool _Timer_Condition()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_));
                if (paused_) {
                    std::unique_lock<std::mutex> lock(mtx_);
                    while (paused_) {
                        cv_.wait(lock);
                    }
                }
                return !requestStop_;
            }

            void setInterval(time_t interval)
            {
                interval_ = interval;
            }

            void setLoop(const Task &loop)
            {
                loop_ = loop;
            }

            void setTimeOutCallback(const Task &callback)
            {
                setLoop([this, callback]() {
                    while (_Timer_Condition()) { 
                        callback();
                    }
                });
            }

            void start()
            {
                if (started_) {
                    throw TimerExcept("Timer already started.");
                }
                std::thread thread_(std::bind(&CU::Timer::TimerThread_, this));
                thread_.detach();
            }

            void stop()
            {
                if (!started_) {
                    throw TimerExcept("Timer already stoped.");
                }
                requestStop_ = true;
                if (paused_) {
                    continueTimer();
                }
                while (started_) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(interval_ / 2));
                }
                requestStop_ = false;
            }

            void pauseTimer()
            {
                if (!paused_) {
                    paused_ = true;
                }
            }

            void continueTimer()
            {
                if (paused_) {
                    std::unique_lock<std::mutex> lock{};
                    paused_ = false;
                    cv_.notify_all();
                }
            }

        private:
            volatile time_t interval_;
            Task loop_;
            std::mutex mtx_;
            std::condition_variable cv_;
            volatile bool started_;
            volatile bool paused_;
            volatile bool requestStop_;

            void TimerThread_()
            {
                started_ = true;
                if (loop_) {
                    loop_();
                }
                started_ = false;
            }
    };
}

#endif // _CU_TIMER_

#pragma once

#define _GNU_SOURCE 1
#include <vector>
#include <cstring>
#include <memory>
#include <sched.h>
#include <sys/resource.h>

class SchedAffinity {
	public:
		static SchedAffinity FromTask(int pid) noexcept
		{
			cpu_set_t cpuset{};
			sched_getaffinity(pid, sizeof(cpu_set_t), std::addressof(cpuset));
			return SchedAffinity(std::addressof(cpuset));
		}

		SchedAffinity() noexcept : cpuset_() { }

		SchedAffinity(const std::vector<int> &cpuList) noexcept : cpuset_()
		{
			for (const int &cpu : cpuList) {
				if (cpu < CPU_SETSIZE) {
					CPU_SET(cpu, std::addressof(cpuset_));
				}
			}
		}

		SchedAffinity(const cpu_set_t* cpuset) noexcept : cpuset_() {
			memcpy(std::addressof(cpuset_), cpuset, sizeof(cpu_set_t));
		}

		SchedAffinity(const SchedAffinity &other) noexcept : cpuset_() {
			memcpy(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t));
		}

		SchedAffinity(SchedAffinity &&other) noexcept : cpuset_() {
			memcpy(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t));
		}

		SchedAffinity &operator=(const SchedAffinity &other) noexcept
		{
			if (std::addressof(other) != this) {
				memcpy(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t));
			}
			return *this;
		}

		bool operator==(const SchedAffinity &other) const noexcept
		{
			return (memcmp(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t)) == 0);
		}

		bool operator!=(const SchedAffinity &other) const noexcept
		{
			return (memcmp(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t)) != 0);
		}

		void toTask(int pid) const noexcept
		{
			sched_setaffinity(pid, sizeof(cpu_set_t), std::addressof(cpuset_));
		}

		const cpu_set_t* cpuset() const noexcept
		{
			return std::addressof(cpuset_);
		}

	private:
		cpu_set_t cpuset_;
};

inline void SetTaskSchedPrio(int pid, int prio) noexcept
{
	if (prio <= 99 && prio >= 1) {
        struct sched_param schedParam{};
		schedParam.sched_priority = 100 - prio;
		sched_setscheduler(pid, SCHED_FIFO, &schedParam);
	} else if (prio >= 100 && prio <= 139) {
        struct sched_param schedParam{};
		schedParam.sched_priority = 0;
		sched_setscheduler(pid, SCHED_NORMAL, &schedParam);
		setpriority(PRIO_PROCESS, pid, prio - 120);
	}
}

inline int GetTaskSchedPrio(int pid) noexcept
{
	int policy = sched_getscheduler(pid);
    if (policy == SCHED_NORMAL) {
        return (getpriority(PRIO_PROCESS, pid) + 120);
    } else if (policy == SCHED_FIFO) {
		struct sched_param schedParam{};
    	sched_getparam(pid, &schedParam);
    	return (100 - schedParam.sched_priority);
	}
	return -1;
}

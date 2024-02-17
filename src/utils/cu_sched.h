#pragma once

#define _GNU_SOURCE 1
#include "libcu.h"
#include <sched.h>
#include <sys/resource.h>

inline void SetTaskSchedPrio(const pid_t &pid, const int &prio) noexcept
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

inline int GetTaskSchedPrio(const pid_t &pid) noexcept
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

inline cpu_set_t MakeCpuset(const std::vector<int> &cpus) noexcept
{
	cpu_set_t cpuset{};
	CPU_ZERO(&cpuset);
	for (const int &cpu : cpus) {
		CPU_SET(cpu, &cpuset);
	}
	return cpuset;
}

inline void SetTaskSchedAffinity(const pid_t &pid, const cpu_set_t &cpuset) noexcept
{
	sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
}

inline cpu_set_t GetTaskSchedAffinity(const pid_t &pid) noexcept
{
	cpu_set_t cpuset{};
	sched_getaffinity(pid, sizeof(cpu_set_t), &cpuset);
	return cpuset;
}

inline bool CpusetEquals(const cpu_set_t &cpuset1, const cpu_set_t &cpuset2) noexcept
{
	return CPU_EQUAL(&cpuset1, &cpuset2);
}

inline void SetTaskCpuset(const int &pid, const std::string &cpuset)
{
	WriteFile(StrMerge("/dev/cpuset%s/cgroup.procs", cpuset.c_str()), StrMerge("%d\n", pid));
}

inline std::string GetTaskCpuset(const int &pid)
{
	auto cgroup = ReadFile(StrMerge("/proc/%d/cgroup", pid));
	return GetPrevString(GetPostString(cgroup, "cpuset:"), "\n");
}

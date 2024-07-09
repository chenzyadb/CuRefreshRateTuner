// CuSched by chenzyadb@github.com
// Based on C++17 STL (LLVM)

#ifndef _CU_SCHED_
#define _CU_SCHED_

#if defined(__linux__) && defined(__GNUC__)

#include <vector>
#include <string>
#include <memory>
#include <sched.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#define CU_INLINE __attribute__((always_inline)) inline
#define CU_LIKELY(val) (__builtin_expect(!!(val), 1))
#define CU_UNLIKELY(val) (__builtin_expect(!!(val), 0))
#define CU_COMPARE(val1, val2, size) (__builtin_memcmp(val1, val2, size) == 0)
#define CU_MEMSET(dst, ch, size) __builtin_memset(dst, ch, size)
#define CU_MEMCPY(dst, src, size) __builtin_memcpy(dst, src, size)

namespace CU
{
	class SchedAffinity
	{
		public:
			static CU_INLINE SchedAffinity FromTask(int pid) noexcept
			{
				cpu_set_t cpuset{};
				sched_getaffinity(pid, sizeof(cpu_set_t), std::addressof(cpuset));
				return SchedAffinity(std::addressof(cpuset));
			}

			CU_INLINE SchedAffinity() noexcept : cpuset_() { }

			CU_INLINE SchedAffinity(const std::vector<int> &cpuList) noexcept : cpuset_()
			{
				for (const int &cpu : cpuList) {
					if (CU_LIKELY(cpu < CPU_SETSIZE)) {
						CPU_SET(cpu, std::addressof(cpuset_));
					}
				}
			}

			CU_INLINE SchedAffinity(const cpu_set_t* cpuset) noexcept : cpuset_() {
				CU_MEMCPY(std::addressof(cpuset_), cpuset, sizeof(cpu_set_t));
			}

			CU_INLINE SchedAffinity(const SchedAffinity &other) noexcept : cpuset_() {
				CU_MEMCPY(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t));
			}

			CU_INLINE SchedAffinity(SchedAffinity &&other) noexcept : cpuset_() {
				CU_MEMCPY(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t));
			}

			CU_INLINE SchedAffinity &operator=(const SchedAffinity &other) noexcept
			{
				if (CU_LIKELY(std::addressof(other) != this)) {
					CU_MEMCPY(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t));
				}
				return *this;
			}

			CU_INLINE bool operator==(const SchedAffinity &other) const noexcept
			{
				return CU_COMPARE(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t));
			}

			CU_INLINE bool operator!=(const SchedAffinity &other) const noexcept
			{
				return !CU_COMPARE(std::addressof(cpuset_), other.cpuset(), sizeof(cpu_set_t));
			}

			CU_INLINE void toTask(int pid) const noexcept
			{
				sched_setaffinity(pid, sizeof(cpu_set_t), std::addressof(cpuset_));
			}

			CU_INLINE const cpu_set_t* cpuset() const noexcept
			{
				return std::addressof(cpuset_);
			}

		private:
			cpu_set_t cpuset_;
	};

	CU_INLINE void SetTaskSchedPrio(int pid, int prio) noexcept
	{
		if (prio <= 99 && prio >= 1) {
			struct sched_param schedParam{};
			schedParam.sched_priority = 100 - prio;
			sched_setscheduler(pid, SCHED_FIFO, &schedParam);
		} else if (prio >= 100 && prio <= 139) {
			struct sched_param schedParam{};
			sched_setscheduler(pid, SCHED_NORMAL, &schedParam);
			setpriority(PRIO_PROCESS, pid, prio - 120);
		}
	}

	CU_INLINE int GetTaskSchedPrio(int pid) noexcept
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

	CU_INLINE void SetThreadName(const std::string &name) noexcept
	{
		prctl(PR_SET_NAME, name.c_str());
	}
}

#endif // __unix__ && __GNUC__
#endif // _CU_SCHED_

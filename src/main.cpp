#include "CuRefreshRateTuner.h"
#include "utils/CuLogger.h"
#include "utils/libcu.h"

constexpr char DAEMON_NAME[] = "CuRefreshRateTuner";

std::vector<std::string> ParseArgs(int argc, char* argv[])
{
	std::vector<std::string> args{};
	for (int idx = 0; idx < argc; idx++) {
		args.emplace_back(argv[idx]);
	}
	size_t argv_size = 0;
	for (const auto &arg : args) {
		argv_size += arg.size() + 1;
	}
	memset(argv[0], 0, argv_size);
	strncpy(argv[0], DAEMON_NAME, sizeof(DAEMON_NAME));
	return args;
}

void KillOldDaemon(void)
{
	int daemon_pid = getpid();
	auto procs = CU::ListFile("/proc", DT_DIR);
	for (const auto &proc : procs) {
		int pid = CU::StrToInt(proc);
		if (pid > 0 && pid <= INT16_MAX) {
			if (pid != daemon_pid && CU::ReadFile(CU::Format("/proc/{}/cmdline", pid)) == DAEMON_NAME) {
				kill(pid, SIGKILL);
			}
		} 
	}
}

std::string GetTaskTombstonePath(int pid) 
{
	auto tombstonePaths = CU::ListPath("/data/tombstones");
	auto tombstoneSymbol = CU::Format("pid: {}", pid);
	for (const auto &tombstonePath : tombstonePaths) {
		if (CU::StrContains(CU::ReadFile(tombstonePath), tombstoneSymbol)) {
			return tombstonePath;
		}
	}
	return {};
}

void StartDaemonWatchDog(const std::string &logPath)
{
	int daemon_pid = getpid();
	fork();
	if (getpid() != daemon_pid) {
		CU::SetThreadName("WatchDog");
		CU::SetTaskSchedPrio(0, 120);

		CU::Logger::Create(CU::Logger::LogLevel::INFO, logPath);
		for (;;) {
			if (CU::ReadFile(CU::Format("/proc/{}/cmdline", daemon_pid)) != DAEMON_NAME) {
				CU::Logger::Info("Daemon stop running.");
				auto tombstonePath = GetTaskTombstonePath(daemon_pid);
				if (CU::IsPathExists(tombstonePath)) {
					CU::Logger::Error("Daemon Crashed (pid={}).", daemon_pid);
					CU::Logger::Error("Tombstone path: {}.", tombstonePath);
					CU::Logger::Error("-- tombstone start --\n{}", CU::ReadFile(tombstonePath));
					CU::Logger::Error("-- tombstone end --");
					CU::Logger::Flush();
				}
				break;
			}
			CU::SleepMs(1000);
		}
		std::exit(0);
	}
	CU::SleepMs(1000);
}

void StartDaemon(const std::string &configPath)
{	
	CuRefreshRateTuner daemon{};
	daemon.Run(configPath);

	CU::Pause();
}

int main(int argc, char* argv[])
{
	auto args = ParseArgs(argc, argv);
	CU::SetThreadName(DAEMON_NAME);
	CU::SetTaskSchedPrio(0, 120);

	// CuRefreshRate [configPath] [logPath]
	if (args.size() == 3) {
		KillOldDaemon();
		daemon(0, 0);
		StartDaemonWatchDog(args[2]);
		CU::Logger::Create(CU::Logger::LogLevel::DEBUG, args[2]);
		CU::Logger::Info("CuRefreshRateTuner V1 ({}) by chenzyadb.", CU::CompileDateCode());
		StartDaemon(args[1]);
	}

	return 0;
}

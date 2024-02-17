#include "CuRefreshRateTuner.h"
#include "utils/CuLogger.h"
#include "utils/libcu.h"

constexpr char DAEMON_NAME[] = "CuRefreshRateTuner";
constexpr int MIN_KERNEL_VERSION = 318000;
constexpr int MIN_ANDROID_SDK = 29;

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
	strcpy(argv[0], DAEMON_NAME);
	return args;
}

void KillOldDaemon(void)
{
	int myPid = getpid();
	auto dir = opendir("/proc");
	if (dir) {
		for (auto entry = readdir(dir); entry != nullptr; entry = readdir(dir)) {
			if (entry->d_type == DT_DIR) {
				int taskPid = atoi(entry->d_name);
				if (taskPid > 0 && taskPid < (INT16_MAX + 1)) {
					auto taskName = GetTaskName(taskPid);
					if (taskName == DAEMON_NAME && taskPid != myPid) {
						kill(taskPid, SIGINT);
					}
				}
			}
		}
		closedir(dir);
	}
}

void StartDaemon(const std::string &configPath)
{
	if (GetLinuxKernelVersion() < MIN_KERNEL_VERSION) {
		CU::Logger::Warn("Your Linux kernel is out-of-date.");
	}
	if (GetAndroidSDKVersion() < MIN_ANDROID_SDK) {
		CU::Logger::Error("Your Android System is out-of-date.");
		CU::Logger::Flush();
		std::exit(0);
	}
	
	CuRefreshRateTuner daemon{};
	daemon.Run(configPath);

	for (;;) {
		sleep(UINT_MAX);
	}
}

int main(int argc, char* argv[])
{
	auto args = ParseArgs(argc, argv);
	SetThreadName(DAEMON_NAME);
	SetTaskSchedPrio(0, 120);

	// CuRefreshRate [configPath] [logPath]
	if (args.size() == 3) {
		daemon(0, 0);
		CU::Logger::Create(CU::Logger::LogLevel::DEBUG, args[2]);
		CU::Logger::Info("CuRefreshRateTuner V1 (%d) by chenzyadb.", GetCompileDateCode());
		KillOldDaemon();
		StartDaemon(args[1]);
	}

	return 0;
}

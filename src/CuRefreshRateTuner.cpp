#include "CuRefreshRateTuner.h"

CuRefreshRateTuner::CuRefreshRateTuner() : modules_() {}

CuRefreshRateTuner::~CuRefreshRateTuner() 
{
	for (auto &module : modules_) {
		delete module;
		module = nullptr;
	}
}

void CuRefreshRateTuner::Run(const std::string &configPath)
{
	try {
		Init_(configPath);
	} catch (const std::exception &e) {
		CU::Logger::Error("Something went wrong while loading.");
		CU::Logger::Error("Exception Thrown: {}.", e.what());
		CU::Logger::Flush();
		std::exit(0);
	}
}

void CuRefreshRateTuner::Init_(const std::string &configPath)
{
	modules_.emplace_back(new RefreshRateTuner(configPath));
	modules_.emplace_back(new InputListener());
	modules_.emplace_back(new CgroupWatcher());
	modules_.emplace_back(new TopAppMonitor());
	for (const auto &module : modules_) {
		module->Start();
	}

	{
		auto proc = CU::Format("{}\n", getpid());
		CU::WriteFile("/dev/cpuset/system-background/cgroup.procs", proc);
		CU::WriteFile("/dev/cpuctl/cgroup.procs", proc);
		CU::WriteFile("/dev/stune/cgroup.procs", proc);
	}

	CU::Logger::Info("Daemon Running (pid={}).", getpid());
	CU::EventTransfer::Post("Main.InitFinished", 0);
}

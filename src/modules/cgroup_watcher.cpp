#include "cgroup_watcher.h"

CgroupWatcher::CgroupWatcher() : Module(), screenState_(ScreenState::SCREEN_ON) { }

CgroupWatcher::~CgroupWatcher() { }

void CgroupWatcher::Start()
{
	auto cpusetPaths = CU::ListPath("/dev/cpuset", DT_DIR);
	for (const auto &cpusetPath : cpusetPaths) {
		auto tasksPath = cpusetPath + "/tasks";
		if (CU::IsPathExists(tasksPath)) {
			FileWatcher_AddWatch(tasksPath, std::bind(&CgroupWatcher::CgroupModified_, this));
		}
		auto procsPath = cpusetPath + "/cgroup.procs";
		if (CU::IsPathExists(procsPath)) {
			FileWatcher_AddWatch(procsPath, std::bind(&CgroupWatcher::CgroupModified_, this));
		}
	}
	screenState_ = GetScreenState();
	CU::EventTransfer::Post("CgroupWatcher.ScreenStateChanged", screenState_);
}

void CgroupWatcher::CgroupModified_()
{
	CU::EventTransfer::Post("CgroupWatcher.CgroupModified", 0);
	CheckScreenState_();
}

void CgroupWatcher::CheckScreenState_()
{
	auto nowaScreenState = GetScreenState();
	if (screenState_ != nowaScreenState) {
		CU::EventTransfer::Post("CgroupWatcher.ScreenStateChanged", nowaScreenState);
		screenState_ = nowaScreenState;
	}
}

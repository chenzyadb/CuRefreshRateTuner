#include "cgroup_watcher.h"

CgroupWatcher::CgroupWatcher() : Module(), screenState_(ScreenState::SCREEN_OFF) { }

CgroupWatcher::~CgroupWatcher() { }

void CgroupWatcher::Start()
{
	if (GetAndroidSDKVersion() < __ANDROID_API_Q__) {
		screenState_ = GetScreenStateViaWakelock();
	} else {
		screenState_ = GetScreenStateViaCgroup();
	}
	CU::EventTransfer::Post("CgroupWatcher.ScreenStateChanged", screenState_);
	auto cpusetPaths = ListDir("/dev/cpuset");
	for (const auto &cpusetPath : cpusetPaths) {
		auto tasksPath = cpusetPath + "/tasks";
		if (IsPathExist(tasksPath)) {
			FileWatcher_AddWatch(tasksPath, std::bind(&CgroupWatcher::CgroupModified_, this));
		}
		auto procsPath = cpusetPath + "/cgroup.procs";
		if (IsPathExist(procsPath)) {
			FileWatcher_AddWatch(procsPath, std::bind(&CgroupWatcher::CgroupModified_, this));
		}
	}
}

void CgroupWatcher::CgroupModified_()
{
	CU::EventTransfer::Post("CgroupWatcher.CgroupModified", 0);
	CheckScreenState_();
}

void CgroupWatcher::CheckScreenState_()
{
	ScreenState newScreenState = ScreenState::SCREEN_ON;
	if (GetAndroidSDKVersion() < __ANDROID_API_Q__) {
		newScreenState = GetScreenStateViaWakelock();
	} else {
		newScreenState = GetScreenStateViaCgroup();
	}
	if (screenState_ != newScreenState) {
		CU::EventTransfer::Post("CgroupWatcher.ScreenStateChanged", newScreenState);
		screenState_ = newScreenState;
	}
}

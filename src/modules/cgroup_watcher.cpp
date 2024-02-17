#include "cgroup_watcher.h"

CgroupWatcher::CgroupWatcher() : Module(), screenState_(ScreenState::SCREEN_OFF) { }

CgroupWatcher::~CgroupWatcher() { }

void CgroupWatcher::Start()
{
	if (GetAndroidSDKVersion() < 29) {
		screenState_ = GetScreenStateViaWakelock();
	} else {
		screenState_ = GetScreenStateViaCgroup();
	}
	CU::EventTransfer::Post("CgroupWatcher.ScreenStateChanged", screenState_);

	FileWatcher_AddWatch("/dev/cpuset/top-app/tasks", std::bind(&CgroupWatcher::TopAppModified_, this));
	FileWatcher_AddWatch("/dev/cpuset/top-app/cgroup.procs", std::bind(&CgroupWatcher::TopAppModified_, this));
	FileWatcher_AddWatch("/dev/cpuset/foreground/tasks", std::bind(&CgroupWatcher::ForegroundModified_, this));
	FileWatcher_AddWatch("/dev/cpuset/foreground/cgroup.procs", std::bind(&CgroupWatcher::ForegroundModified_, this));
	FileWatcher_AddWatch("/dev/cpuset/background/tasks", std::bind(&CgroupWatcher::BackgroundModified_, this));
	FileWatcher_AddWatch("/dev/cpuset/background/cgroup.procs", std::bind(&CgroupWatcher::BackgroundModified_, this));
	FileWatcher_AddWatch("/dev/cpuset/restricted/tasks", std::bind(&CgroupWatcher::RestrictedModified_, this));
	FileWatcher_AddWatch("/dev/cpuset/restricted/cgroup.procs", std::bind(&CgroupWatcher::RestrictedModified_, this));
}

void CgroupWatcher::TopAppModified_()
{
	CU::EventTransfer::Post("CgroupWatcher.TopAppCgroupModified", 0);
	CheckScreenState_();
}

void CgroupWatcher::ForegroundModified_()
{
	CU::EventTransfer::Post("CgroupWatcher.ForegroundCgroupModified", 0);
	CheckScreenState_();
}

void CgroupWatcher::BackgroundModified_()
{
	CU::EventTransfer::Post("CgroupWatcher.BackgroundCgroupModified", 0);
	CheckScreenState_();
}

void CgroupWatcher::RestrictedModified_()
{
	CU::EventTransfer::Post("CgroupWatcher.RestrictedCgroupModified", 0);
	CheckScreenState_();
}

void CgroupWatcher::CheckScreenState_()
{
	ScreenState newScreenState = ScreenState::SCREEN_ON;
	if (GetAndroidSDKVersion() < 29) {
		newScreenState = GetScreenStateViaWakelock();
	} else {
		newScreenState = GetScreenStateViaCgroup();
	}
	if (screenState_ != newScreenState) {
		CU::EventTransfer::Post("CgroupWatcher.ScreenStateChanged", newScreenState);
		screenState_ = newScreenState;
	}
}

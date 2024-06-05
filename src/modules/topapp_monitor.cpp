#include "topapp_monitor.h"

TopAppMonitor::TopAppMonitor() : Module(), timer_(), cv_(), mtx_(), unblocked_(false), topAppPid_(-1) { }

TopAppMonitor::~TopAppMonitor() { }

void TopAppMonitor::Start() { 
	CU::EventTransfer::Subscribe("CgroupWatcher.CgroupModified", this,
		std::bind(&TopAppMonitor::CgroupModified_, this, std::placeholders::_1));
	std::thread thread_(std::bind(&TopAppMonitor::Main_, this));
	thread_.detach();
	timer_.setTimeOutCallback(std::bind(&TopAppMonitor::TimerTask_, this));
	timer_.setInterval(1000);
	timer_.start();
}

void TopAppMonitor::Main_()
{
	SetThreadName("TopAppMonitor");
	SetTaskSchedPrio(0, 95);

	for (;;) {
		{
			std::unique_lock<std::mutex> lck(mtx_);
			while (!unblocked_) {
				cv_.wait(lck);
			}
			unblocked_ = false;
		}
		{
			int newTopAppPid = -1;
			auto topAppInfo = DumpTopActivityInfo();
			if (StrContains(topAppInfo, "fore")) {
				// Proc # 0: fore   T/A/TOP  trm: 0 4272:xyz.chenzyadb.cu_toolbox/u0a353 (top-activity)
				int pid = StringToInteger(GetPrevString(StrSplitAt(topAppInfo, ' ', 7), ":"));
				if (pid > 0 && pid < (INT16_MAX + 1)) {
					newTopAppPid = pid;
				}
			} else if (StrContains(topAppInfo, "fg")) {
				// Proc # 0: fg     T/A/TOP  LCM  t: 0 4272:xyz.chenzyadb.cu_toolbox/u0a353 (top-activity)
				int pid = StringToInteger(GetPrevString(StrSplitAt(topAppInfo, ' ', 8), ":"));
				if (pid > 0 && pid < (INT16_MAX + 1)) {
					newTopAppPid = pid;
				}
			}
			if (newTopAppPid != -1 && newTopAppPid != topAppPid_) {
				CU::EventTransfer::Post("TopAppMonitor.TopAppChanged", newTopAppPid);
				topAppPid_ = newTopAppPid;
			}
		}
		usleep(500000);
	}
}

void TopAppMonitor::CgroupModified_(const CU::EventTransfer::TransData &transData)
{
	CU_UNUSED(transData);
	std::unique_lock<std::mutex> lck(mtx_);
	unblocked_ = true;
	cv_.notify_all();
}

void TopAppMonitor::TimerTask_()
{
	if (GetTaskCgroupType(topAppPid_, "cpuset") != "/top-app") {
		CgroupModified_(nullptr);
	}
}

void TopAppMonitor::ScreenStateChanged_(const CU::EventTransfer::TransData &transData)
{
	auto screenState = CU::EventTransfer::GetData<ScreenState>(transData);
	if (screenState == ScreenState::SCREEN_ON) {
		timer_.continueTimer();
	} else {
		timer_.pauseTimer();
		topAppPid_ = -1;
	}
}

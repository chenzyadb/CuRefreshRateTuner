#include "topapp_monitor.h"

TopAppMonitor::TopAppMonitor() : Module(), cv_(), mtx_(), unblocked_(false) { }

TopAppMonitor::~TopAppMonitor() { }

void TopAppMonitor::Start() { 
	CU::EventTransfer::Subscribe("CgroupWatcher.TopAppCgroupModified", 
		std::bind(&TopAppMonitor::CgroupModified_, this, std::placeholders::_1));
	CU::EventTransfer::Subscribe("CgroupWatcher.ForegroundCgroupModified", 
		std::bind(&TopAppMonitor::CgroupModified_, this, std::placeholders::_1));
	std::thread thread_(std::bind(&TopAppMonitor::Main_, this));
	thread_.detach();
	CgroupModified_(nullptr);
}

void TopAppMonitor::Main_()
{
	SetThreadName("TopAppMonitor");
	SetTaskSchedPrio(0, 95);

	int topAppPid = -1;
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
			std::string newTopAppName{};
			auto topAppInfo = DumpTopActivityInfo();
			if (StrContains(topAppInfo, "fore")) {
				// Proc # 0: fore   T/A/TOP  trm: 0 4272:xyz.chenzyadb.cu_toolbox/u0a353 (top-activity)
				int pid = StringToInteger(GetPrevString(StrSplitSpaceAt(topAppInfo, 7), ":"));
				if (pid > 0 && pid < (INT16_MAX + 1)) {
					newTopAppPid = pid;
					newTopAppName = GetPrevString(GetPostString(StrSplitSpaceAt(topAppInfo, 7), ":"), "/");
				}
			} else if (StrContains(topAppInfo, "fg")) {
				// Proc # 0: fg     T/A/TOP  LCM  t: 0 4272:xyz.chenzyadb.cu_toolbox/u0a353 (top-activity)
				int pid = StringToInteger(GetPrevString(StrSplitSpaceAt(topAppInfo, 8), ":"));
				if (pid > 0 && pid < (INT16_MAX + 1)) {
					newTopAppPid = pid;
					newTopAppName = GetPrevString(GetPostString(StrSplitSpaceAt(topAppInfo, 8), ":"), "/");
				}
			}
			if (newTopAppPid != -1 && newTopAppPid != topAppPid) {
				CU::EventTransfer::Post("TopAppMonitor.TopAppChanged", newTopAppName);
				topAppPid = newTopAppPid;
			}
		}
		usleep(500000);
	}
}

void TopAppMonitor::CgroupModified_(const void* data)
{
	std::unique_lock<std::mutex> lck(mtx_);
	unblocked_ = true;
	cv_.notify_all();
}

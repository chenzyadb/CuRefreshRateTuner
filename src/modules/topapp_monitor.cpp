#include "topapp_monitor.h"

TopAppMonitor::TopAppMonitor() : Module(), monitor_(), notifier_(), topAppPid_(-1) { }

TopAppMonitor::~TopAppMonitor() { }

void TopAppMonitor::Start() { 
	CU::EventTransfer::Subscribe("CgroupWatcher.CgroupModified", this,
		std::bind(&TopAppMonitor::CgroupModified_, this, std::placeholders::_1));
	monitor_.setLoop(std::bind(&TopAppMonitor::MonitorLoop_, this));
	monitor_.setInterval(500);
	monitor_.start();
	notifier_.setTimeOutCallback(std::bind(&TopAppMonitor::NotifierTask_, this));
	notifier_.setInterval(1000);
	notifier_.start();
}

void TopAppMonitor::MonitorLoop_()
{
	CU::SetThreadName("TopAppMonitor");
	CU::SetTaskSchedPrio(0, 95);

	TIMER_LOOP(monitor_) {
		int newTopAppPid = -1;
		auto topAppInfo = DumpTopActivityInfo();
		if (CU::StrContains(topAppInfo, "fore")) {
			// Proc # 0: fore   T/A/TOP  trm: 0 4272:xyz.chenzyadb.cu_toolbox/u0a353 (top-activity)
			int pid = CU::StrToInt(CU::SubPrevStr(CU::StrSplitAt(topAppInfo, ' ', 7), ':'));
			if (pid > 0 && pid < (INT16_MAX + 1)) {
				newTopAppPid = pid;
			}
		} else if (CU::StrContains(topAppInfo, "fg")) {
			// Proc # 0: fg     T/A/TOP  LCM  t: 0 4272:xyz.chenzyadb.cu_toolbox/u0a353 (top-activity)
			int pid = CU::StrToInt(CU::SubPrevStr(CU::StrSplitAt(topAppInfo, ' ', 8), ':'));
			if (pid > 0 && pid < (INT16_MAX + 1)) {
				newTopAppPid = pid;
			}
		}
		if (newTopAppPid != -1 && newTopAppPid != topAppPid_) {
			CU::EventTransfer::Post("TopAppMonitor.TopAppChanged", newTopAppPid);
			topAppPid_ = newTopAppPid;
		}
		monitor_.pauseTimer();
	}
}

void TopAppMonitor::CgroupModified_(const CU::EventTransfer::TransData &transData)
{
	CU_UNUSED(transData);
	monitor_.continueTimer();
}

void TopAppMonitor::NotifierTask_()
{
	if (!IsTopAppTask(topAppPid_)) {
		CgroupModified_(nullptr);
	}
}

void TopAppMonitor::ScreenStateChanged_(const CU::EventTransfer::TransData &transData)
{
	auto screenState = CU::EventTransfer::GetData<ScreenState>(transData);
	if (screenState == ScreenState::SCREEN_ON) {
		notifier_.continueTimer();
	} else {
		notifier_.pauseTimer();
		topAppPid_ = -1;
	}
}

std::string TopAppMonitor::DumpTopActivityInfo()
{
	auto lines = CU::StrSplit(CU::ExecCommand("dumpsys activity oom 2>/dev/null"), '\n');
	for (const auto &line : lines) {
		if (CU::StrContains(line, "(top-activity)")) {
			return line;
		}
	}
	return {};
}

bool TopAppMonitor::IsTopAppTask(int pid)
{
	auto topAppProcs = CU::StrSplit(CU::ReadFile("/dev/cpuset/top-app/cgroup.procs"), '\n');
	return CU::Contains(topAppProcs, CU::To_String(pid));
}

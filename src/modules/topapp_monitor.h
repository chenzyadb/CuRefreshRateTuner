#pragma once

#include "platform/module.h"
#include "utils/libcu.h"
#include "utils/CuSched.h"
#include "utils/CuLogger.h"
#include "utils/CuEventTransfer.h"
#include "utils/CuTimer.h"
#include "utils/CuFormat.h"
#include "utils/android_platform.h"

class TopAppMonitor : public Module
{
	public:
		TopAppMonitor();
		~TopAppMonitor();
		void Start();
		
	private:
		CU::Timer monitor_;
		CU::Timer notifier_;
		int topAppPid_;

		void MonitorLoop_();
		void NotifierTask_();
		void CgroupModified_(const CU::EventTransfer::TransData &transData);
		void ScreenStateChanged_(const CU::EventTransfer::TransData &transData);

		std::string DumpTopActivityInfo();
		bool IsTopAppTask(int pid);
};

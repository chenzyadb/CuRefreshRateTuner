#pragma once

#include "platform/module.h"
#include "utils/libcu.h"
#include "utils/cu_sched.h"
#include "utils/CuLogger.h"
#include "utils/CuEventTransfer.h"
#include "utils/CuTimer.h"

class TopAppMonitor : public Module
{
	public:
		TopAppMonitor();
		~TopAppMonitor();
		void Start();
		
	private:
		CU::Timer timer_;
		std::condition_variable cv_;
		std::mutex mtx_;
		bool unblocked_;
		int topAppPid_;

		void Main_();
		void CgroupModified_(const CU::EventTransfer::TransData &transData);
		void TimerTask_();
		void ScreenStateChanged_(const CU::EventTransfer::TransData &transData);
};

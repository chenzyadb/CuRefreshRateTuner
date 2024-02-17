#pragma once

#include "platform/module.h"
#include "utils/libcu.h"
#include "utils/cu_sched.h"
#include "utils/CuLogger.h"
#include "utils/CuEventTransfer.h"

class TopAppMonitor : public Module
{
	public:
		TopAppMonitor();
		~TopAppMonitor();
		void Start();
		
	private:
		std::condition_variable cv_;
		std::mutex mtx_;
		bool unblocked_;

		void Main_();
		void CgroupModified_(const void* data);
};

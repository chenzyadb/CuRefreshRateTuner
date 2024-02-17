#pragma once

#include <iostream>
#include <exception>
#include "platform/module.h"
#include "modules/cgroup_watcher.h"
#include "modules/topapp_monitor.h"
#include "modules/input_listener.h"
#include "modules/refresh_rate_tuner.h"
#include "utils/libcu.h"
#include "utils/CuLogger.h"
#include "utils/CuEventTransfer.h"

class CuRefreshRateTuner
{
	public:
		CuRefreshRateTuner();
		~CuRefreshRateTuner();
		void Run(const std::string &configPath);

	private:
		std::vector<Module*> modules_;

		void Init_(const std::string &configPath);
};

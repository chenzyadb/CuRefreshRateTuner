#pragma once

#include "platform/module.h"
#include "utils/libcu.h"
#include "utils/cu_sched.h"
#include "utils/CuLogger.h"
#include "utils/CuEventTransfer.h"

class CgroupWatcher : public Module
{
	public:
		CgroupWatcher();
		~CgroupWatcher();
		void Start();

	private:
		ScreenState screenState_;

		void TopAppModified_();
		void ForegroundModified_();
		void BackgroundModified_();
		void RestrictedModified_();
		void CheckScreenState_();
};
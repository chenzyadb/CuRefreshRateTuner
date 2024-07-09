#pragma once

#include "platform/module.h"
#include "utils/libcu.h"
#include "utils/CuFile.h"
#include "utils/CuSched.h"
#include "utils/CuLogger.h"
#include "utils/CuEventTransfer.h"
#include "utils/android_platform.h"

class CgroupWatcher : public Module
{
	public:
		CgroupWatcher();
		~CgroupWatcher();
		void Start();

	private:
		ScreenState screenState_;

		void CgroupModified_();
		void CheckScreenState_();
};

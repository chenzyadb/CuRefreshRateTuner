#pragma once

#include "platform/module.h"
#include "utils/libcu.h"
#include "utils/CuSched.h"
#include "utils/CuLogger.h"
#include "utils/CuEventTransfer.h"
#include "utils/CuFile.h"
#include <linux/input.h>

class InputListener : public Module
{
	public:
		InputListener();
		~InputListener();
		void Start();

	private:
		void ListenerMain_(std::string eventPath);
};

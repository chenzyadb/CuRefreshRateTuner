#include "input_listener.h"

InputListener::InputListener() : Module() { }

InputListener::~InputListener() { }

void InputListener::Start()
{
	auto dir = opendir("/dev/input");
	if (dir) {
		for (auto entry = readdir(dir); entry != nullptr; entry = readdir(dir)) {
			if (entry->d_type == DT_CHR) {
				auto eventPath = StrMerge("/dev/input/%s", entry->d_name);
				std::thread listenerThread(std::bind(&InputListener::ListenerMain_, this, eventPath));
				listenerThread.detach();
			}
		}
		closedir(dir);
	}
}

void InputListener::ListenerMain_(std::string eventPath)
{
	static const auto checkBit = [](const char* bit, unsigned short mask) -> bool {
		return ((bit[mask / 8] & (1 << (mask % 8))) != 0);
	};

	SetThreadName("InputListener");
	SetTaskSchedPrio(0, 95);

	{
		int fd = open(eventPath.c_str(), O_RDONLY | O_NONBLOCK);
		if (fd < 0) {
			return;
		}

		char inputBit[(ABS_MAX + 1) / 8] = { 0 };
		ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(inputBit)), inputBit);
		if (!(checkBit(inputBit, ABS_MT_POSITION_X) && checkBit(inputBit, ABS_MT_POSITION_Y)) &&
			!(checkBit(inputBit, ABS_X) && checkBit(inputBit, ABS_Y))) 
		{
			return;
		}

		char inputName[32] = { 0 };
		ioctl(fd, EVIOCGNAME(sizeof(inputName)), inputName);
		CU::Logger::Info("Listening \"%s\".", inputName);

		close(fd);
	}

	bool touching = false;
	for (int fd = open(eventPath.c_str(), O_RDONLY); fd >= 0;) {
		struct input_event inputEvent{};
		auto len = read(fd, std::addressof(inputEvent), sizeof(struct input_event));
		if (len == sizeof(struct input_event)) {
			if (inputEvent.type == EV_KEY && (inputEvent.code == BTN_TOUCH || inputEvent.code == BTN_DIGI)) {
				if (!touching && inputEvent.value == 1) {
					CU::EventTransfer::Post("InputListener.KEY_DOWN", 0);
					touching = true;
				} else if (touching && inputEvent.value == 0) {
					CU::EventTransfer::Post("InputListener.KEY_UP", 0);
					touching = false;
				}
			}
		} else if (len < 0) {
			close(fd);
			break;
		}
	}
	CU::Logger::Warn("Failed to listen %s.", eventPath.c_str());
}

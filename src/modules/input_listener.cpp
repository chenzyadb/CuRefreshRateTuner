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
				int fd = open(eventPath.c_str(), O_RDONLY);
				if (fd >= 0) {
					std::thread thread_(std::bind(&InputListener::ListenerMain_, this, fd));
					thread_.detach();
				} else {
					CU::Logger::Warn("Failed to open \"%s\".", eventPath.c_str());
				}
			}
		}
		closedir(dir);
	}
}

void InputListener::ListenerMain_(const int fd)
{
	SetThreadName("InputListener");
	SetTaskSchedPrio(0, 95);

	{
		static const auto checkBit = [](const char* bit, unsigned short mask) -> bool {
			return ((bit[mask / 8] & (1 << (mask % 8))) != 0);
		};
		static const auto checkPosition = [](const struct input_absinfo &absInfo) -> bool {
			return (absInfo.maximum > 0 && (absInfo.maximum - absInfo.minimum) == absInfo.maximum);
		};
		
		struct input_absinfo pos_x_info{}, pos_y_info{};
		char inputBit[(ABS_MAX + 1) / 8] = { 0 };
		ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(inputBit)), inputBit);
		if (checkBit(inputBit, ABS_MT_POSITION_X) && checkBit(inputBit, ABS_MT_POSITION_Y)) {
			ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &pos_x_info);
			ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &pos_y_info);
		} else if (checkBit(inputBit, ABS_X) && checkBit(inputBit, ABS_Y)) {
			ioctl(fd, EVIOCGABS(ABS_X), &pos_x_info);
			ioctl(fd, EVIOCGABS(ABS_Y), &pos_y_info);
		} else {
			return;
		}
		if (!checkPosition(pos_x_info) || !checkPosition(pos_y_info)) {
			return;
		}

		char inputName[32] = { 0 };
		ioctl(fd, EVIOCGNAME(sizeof(inputName)), inputName);
		CU::Logger::Info("Listening \"%s\", position: x=0-%d, y=0-%d.", inputName, pos_x_info.maximum, pos_y_info.maximum);
	}

	for (;;) {
		struct input_event inputEvent{};
		auto len = read(fd, std::addressof(inputEvent), sizeof(struct input_event));
		if (len == sizeof(struct input_event)) {
			if (inputEvent.code == BTN_TOUCH || inputEvent.code == BTN_DIGI) {
				if (inputEvent.value == 1) {
					CU::EventTransfer::Post("InputListener.KEY_DOWN", 0);
				} else {
					CU::EventTransfer::Post("InputListener.KEY_UP", 0);
				}
			}
		}
	}

	close(fd);
	return;
}

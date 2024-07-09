#pragma once

#if defined(__ANDROID_API__)

#include "libcu.h"
#include "CuFile.h"
#include <sys/system_properties.h>

enum class ScreenState : uint8_t {SCREEN_ON, SCREEN_OFF};

inline ScreenState GetScreenState()
{
    static const int android_api_level = android_get_device_api_level();
    if (android_api_level < __ANDROID_API_R__) {
        if (CU::StrContains(CU::ReadFile("/sys/power/wake_unlock"), "PowerManagerService.Display")) {
            return ScreenState::SCREEN_OFF;
        }
    } else {
        if (CU::StrSplit(CU::ReadFile("/dev/cpuset/restricted/tasks"), '\n').size() > 10) {
            return ScreenState::SCREEN_OFF;
        }
    }
    return ScreenState::SCREEN_ON;
}

#endif // __ANDROID_API__

#include "refresh_rate_tuner.h"

RefreshRateTuner::RefreshRateTuner(const std::string &configPath) : 
    Module(), 
    configPath_(configPath),
    displayModes_(), 
    config_(),
    activeDisplayModeIdx_(-1),
    idleDisplayModeIdx_(-1),
    keyDownTime_(0),
    keyUpTime_(0),
    active_(false) {}

RefreshRateTuner::~RefreshRateTuner() {}

void RefreshRateTuner::Start()
{
    Init_();
    LoadConfig_();
    UpdatePolicy_("*");
    ResetRefreshRate_();
    SwitchRefreshRate_();
    CU::EventTransfer::Subscribe("CgroupWatcher.ScreenStateChanged", 
        std::bind(&RefreshRateTuner::ScreenStateChanged_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("TopAppMonitor.TopAppChanged", 
        std::bind(&RefreshRateTuner::TopAppChanged_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("InputListener.KEY_DOWN", 
        std::bind(&RefreshRateTuner::KeyDown_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("InputListener.KEY_UP", 
        std::bind(&RefreshRateTuner::KeyUp_, this, std::placeholders::_1));
    FileWatcher_AddWatch(configPath_, std::bind(&RefreshRateTuner::ConfigModified_, this));
    std::thread thread_(std::bind(&RefreshRateTuner::IdleLoop_, this));
    thread_.detach();
}

void RefreshRateTuner::Init_()
{
    std::vector<std::string> supportedModes{};
    {
        auto lines = StrSplitLine(GetPostString(ExecCommand("dumpsys display"), "mSupportedModes="));
        for (const auto &line : lines) {
            // DisplayModeRecord{mMode={id=1, width=1080, height=1920, fps=60.000004}}
            if (StrContains(line, "DisplayModeRecord{mMode=")) {
                supportedModes.emplace_back(line);
            } else if (!StrEmpty(line)) {
                break;
            }
        }
    }
    for (const auto &modeRecord : supportedModes) {
        CU::JSONObject displayMode{};
        displayMode["id"] = StringToInteger(GetPrevString(GetPostString(modeRecord, "id="), ","));
        displayMode["width"] = StringToInteger(GetPrevString(GetPostString(modeRecord, "width="), ","));
        displayMode["height"] = StringToInteger(GetPrevString(GetPostString(modeRecord, "height="), ","));
        displayMode["fps"] = StringToInteger(GetPrevString(GetPostString(modeRecord, "fps="), "."));
        displayModes_.add(displayMode);
    }

    CU::Logger::Info("Display Modes:");
    for (const auto &item : displayModes_) {
        auto displayMode = item.toObject();
        int id = displayMode.at("id").toInt();
        int width = displayMode.at("width").toInt();
        int height = displayMode.at("height").toInt();
        int fps = displayMode.at("fps").toInt();
        CU::Logger::Info("id=%d, resolution=%dx%d, fps=%d.", id, width, height, fps);
    }
}

void RefreshRateTuner::IdleLoop_()
{
    SetThreadName("IdleLoop");
    SetTaskSchedPrio(0, 95);

    for (;;) {
        int idleDelay = config_.data().at("idleDelay").toInt();
        if (active_ && keyUpTime_ > keyDownTime_) {
            auto keyUpDuration = GetTimeStampMs() - keyUpTime_;
            if (keyUpDuration >= idleDelay) {
                SwitchRefreshRate_();
                usleep(idleDelay * 1000);
            } else {
                usleep((idleDelay - keyUpDuration) * 1000);
            }
        } else {
            usleep(idleDelay * 1000);
        }
    }
}

void RefreshRateTuner::LoadConfig_()
{
    try {
        config_ = CU::JSONObject(ReadFile(configPath_));
        CU::Logger::Info("Config loaded.");
    } catch (const std::exception &e) {
        CU::Logger::Warn("Failed to load config.");
        CU::Logger::Warn("Exception Thrown: %s.", e.what());
    }
}

void RefreshRateTuner::UpdatePolicy_(const std::string &appName)
{
    static const auto findDisplayModeIdx = [this](const int &fps, const int &resolution) -> int {
        for (const auto &item : displayModes_) {
            auto displayMode = item.toObject();
            if (displayMode.at("fps").toInt() == fps) {
                int width = displayMode.at("width").toInt();
                int height = displayMode.at("height").toInt();
                if (width == resolution || height == resolution) {
                    return (displayMode.at("id").toInt() - 1);
                }
            }
        }
        return -1;
    };

    auto config = config_.data();
    auto policy = config.at("*").toObject();
    if (config.contains(appName)) {
        policy = config.at(appName).toObject();
    }
    int activeFps = policy.at("active").toInt();
    int idleFps = policy.at("idle").toInt();
    int resolution = policy.at("resolution").toInt();
    activeDisplayModeIdx_ = findDisplayModeIdx(activeFps, resolution);
    idleDisplayModeIdx_ = findDisplayModeIdx(idleFps, resolution);
    if (activeDisplayModeIdx_ == -1 || idleDisplayModeIdx_ == -1) {
        CU::Logger::Warn("Failed to find target DisplayMode.");
    }
}

void RefreshRateTuner::SwitchRefreshRate_()
{
    if (keyDownTime_ > keyUpTime_) {
        RunCommand(StrMerge("service call SurfaceFlinger 1035 i32 %d", activeDisplayModeIdx_));
        active_ = true;
    } else {
        RunCommand(StrMerge("service call SurfaceFlinger 1035 i32 %d", idleDisplayModeIdx_));
        active_ = false;
    }
}

void RefreshRateTuner::ResetRefreshRate_()
{
    int sdk_ver = GetAndroidSDKVersion();
    if (sdk_ver >= 31 && sdk_ver <= 33) {
        // Inject a hotplug connected event for the primary display.
        RunCommand("service call SurfaceFlinger 1037");
    }
    if (sdk_ver >= 30) {
        // Turn off frame rate flexibility mode.
        RunCommand("service call SurfaceFlinger 1036 i32 0");
    }
    if (sdk_ver >= 29) {
        // Reset debug DisplayMode set by Backdoor.
        RunCommand("service call SurfaceFlinger 1035 i32 -1");
    }
    active_ = false;
}

void RefreshRateTuner::ScreenStateChanged_(const CU::EventTransfer::TransData &transData)
{
    auto screenState = CU::EventTransfer::GetData<ScreenState>(transData);
    if (screenState == ScreenState::SCREEN_OFF) {
        UpdatePolicy_("screenOff");
        WorkerThread_AddWork([this]() {
            SwitchRefreshRate_();
        });
    } else {
        UpdatePolicy_("*");
        WorkerThread_AddWork([this]() {
            ResetRefreshRate_();
            SwitchRefreshRate_();
        });
    }
}

void RefreshRateTuner::TopAppChanged_(const CU::EventTransfer::TransData &transData)
{
    auto appName = CU::EventTransfer::GetData<std::string>(transData);
    UpdatePolicy_(appName);
}

void RefreshRateTuner::KeyDown_(const CU::EventTransfer::TransData &transData)
{
    keyDownTime_ = GetTimeStampMs();
    if (!active_) {
        WorkerThread_AddWork([this]() {
            SwitchRefreshRate_();
        });
    }
}

void RefreshRateTuner::KeyUp_(const CU::EventTransfer::TransData &transData)
{
    keyUpTime_ = GetTimeStampMs();
}

void RefreshRateTuner::ConfigModified_()
{
    WorkerThread_AddWork([this]() {
        LoadConfig_();
    });
}

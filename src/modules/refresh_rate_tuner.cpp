#include "refresh_rate_tuner.h"

RefreshRateTuner::RefreshRateTuner(const std::string &configPath) : 
    Module(), 
    configPath_(configPath),
    displayModes_(), 
    config_(ReadFile(configPath)),
    keyDownTime_(GetTimeStampMs()),
    keyUpTime_(GetTimeStampMs()),
    active_(false) {}

RefreshRateTuner::~RefreshRateTuner() {}

void RefreshRateTuner::Start()
{
    Init_();
    UpdatePolicy_("*");
    ResetRefreshRate_();
    SetActiveRefreshRate_();
    CU::EventTransfer::Subscribe("CgroupWatcher.ScreenStateChanged", 
        std::bind(&RefreshRateTuner::ScreenStateChanged_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("TopAppMonitor.TopAppChanged", 
        std::bind(&RefreshRateTuner::TopAppChanged_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("InputListener.KEY_DOWN", 
        std::bind(&RefreshRateTuner::KeyDown_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("InputListener.KEY_UP", 
        std::bind(&RefreshRateTuner::KeyUp_, this, std::placeholders::_1));
    FileWatcher_AddWatch(configPath_, std::bind(&RefreshRateTuner::ReloadConfig_, this));
    std::thread thread_(std::bind(&RefreshRateTuner::Main_, this));
    thread_.detach();
}

void RefreshRateTuner::Main_()
{
    SetThreadName("RefreshRateTuner");
    SetTaskSchedPrio(0, 95);

    for (;;) {
        int idleDelay = config_.at("idleDelay").toInt();
        if (keyUpTime_ > keyDownTime_ && active_) {
            auto keyUpDuration = GetTimeStampMs() - keyUpTime_;
            if (keyUpDuration >= idleDelay) {
                SetIdleRefreshRate_();
                usleep(idleDelay * 1000);
            } else {
                usleep((idleDelay - keyUpDuration) * 1000);
            }
        } else {
            usleep(idleDelay * 1000);
        }
    }
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

    auto policy = config_.at("*").toObject();
    if (config_.contains(appName)) {
        policy = config_.at(appName).toObject();
    }
    int activeFps = policy.at("active").toInt();
    int idleFps = policy.at("idle").toInt();
    int resolution = policy.at("resolution").toInt();
    activeDisplayModeIdx_ = findDisplayModeIdx(activeFps, resolution);
    idleDisplayModeIdx_ = findDisplayModeIdx(idleFps, resolution);
}

void RefreshRateTuner::SetActiveRefreshRate_()
{
    auto command = StrMerge("service call SurfaceFlinger 1035 i32 %d", activeDisplayModeIdx_);
    system(command.c_str());
    active_ = true;
}

void RefreshRateTuner::SetIdleRefreshRate_()
{
    auto command = StrMerge("service call SurfaceFlinger 1035 i32 %d", idleDisplayModeIdx_);
    system(command.c_str());
    active_ = false;
}

void RefreshRateTuner::ResetRefreshRate_()
{
    if (GetAndroidSDKVersion() >= 30) {
        system("service call SurfaceFlinger 1036 i32 1");
        system("service call SurfaceFlinger 1035 i32 -1");
        system("service call SurfaceFlinger 1036 i32 0");
    }
}

void RefreshRateTuner::ScreenStateChanged_(const CU::EventTransfer::TransData &transData)
{
    auto screenState = CU::EventTransfer::GetData<ScreenState>(transData);
    if (screenState == ScreenState::SCREEN_OFF) {
        UpdatePolicy_("screenOff");
        WorkerThread_AddWork([this]() {
            ResetRefreshRate_();
            SetIdleRefreshRate_();
        });
    } else {
        UpdatePolicy_("*");
        WorkerThread_AddWork([this]() {
            ResetRefreshRate_();
            SetActiveRefreshRate_();
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
    SetActiveRefreshRate_();
}

void RefreshRateTuner::KeyUp_(const CU::EventTransfer::TransData &transData)
{
    keyUpTime_ = GetTimeStampMs();
}

void RefreshRateTuner::ReloadConfig_()
{
    static const auto loadConfig = [this]() {
        CU::JSONObject newConfig(ReadFile(configPath_));
        if (config_ != newConfig) {
            config_ = newConfig;
            CU::Logger::Info("Config reloaded.");
        }
    };
    try {
        loadConfig();
    } catch (const std::exception &e) {
        CU::Logger::Warn("Failed to load new config.");
        CU::Logger::Warn("Exception Thrown: %s.", e.what());
    }
}

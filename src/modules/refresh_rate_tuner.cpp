#include "refresh_rate_tuner.h"

RefreshRateTuner::RefreshRateTuner(const std::string &configPath) : 
    Module(), 
    configPath_(configPath),
    timer_(),
    displayModeMap_(), 
    config_(),
    activeDisplayModeIdx_(-1),
    idleDisplayModeIdx_(-1),
    keyDownTime_(0),
    keyUpTime_(0),
    active_(false) 
{ }

RefreshRateTuner::~RefreshRateTuner() { }

void RefreshRateTuner::Start()
{
    Init_();
    LoadConfig_();
    UpdatePolicy_("*");
    ResetRefreshRate_();
    SetIdleRefreshRate_();
    CU::EventTransfer::Subscribe("CgroupWatcher.ScreenStateChanged", 
        std::bind(&RefreshRateTuner::ScreenStateChanged_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("TopAppMonitor.TopAppChanged", 
        std::bind(&RefreshRateTuner::TopAppChanged_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("InputListener.KEY_DOWN", 
        std::bind(&RefreshRateTuner::KeyDown_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("InputListener.KEY_UP", 
        std::bind(&RefreshRateTuner::KeyUp_, this, std::placeholders::_1));
    FileWatcher_AddWatch(configPath_, std::bind(&RefreshRateTuner::ConfigModified_, this));
    timer_.setLoop(std::bind(&RefreshRateTuner::IdleLoop_, this));
    timer_.setInterval(config_.data().at("idleDelay").toInt());
    timer_.start();
}

void RefreshRateTuner::Init_()
{
    auto displayInfo = ExecCommand("dumpsys display");
    if (StrContains(displayInfo, "mSfDisplayModes=")) {
        std::vector<std::string> supportedModes{};
        auto lines = StrSplitLine(GetPostString(displayInfo, "mSfDisplayModes="));
        for (const auto &line : lines) {
            // DisplayMode{id=0, width=1080, height=2460, xDpi=397.565, yDpi=397.987, refreshRate=144.00002,
            if (StrContains(line, "DisplayMode{")) {
                supportedModes.emplace_back(line);
            } else if (!StrEmpty(line)) {
                break;
            }
        }
        for (const auto &modeRecord : supportedModes) {
            int id = StringToInteger(GetPrevString(GetPostString(modeRecord, "id="), ","));
            int width = StringToInteger(GetPrevString(GetPostString(modeRecord, "width="), ","));
            int height = StringToInteger(GetPrevString(GetPostString(modeRecord, "height="), ","));
            int refreshRate = StringToInteger(GetPrevString(GetPostString(modeRecord, "refreshRate="), "."));
            displayModeMap_[refreshRate][width] = id;
            displayModeMap_[refreshRate][height] = id;
            CU::Logger::Info("id=%d, resolution=%dx%d, refreshRate=%d.", id, width, height, refreshRate);
        }
    } else if (StrContains(displayInfo, "mSupportedModes=")) {
        std::vector<std::string> supportedModes{};
        auto lines = StrSplitLine(GetPostString(displayInfo, "mSupportedModes="));
        for (const auto &line : lines) {
            // DisplayModeRecord{mMode={id=1, width=1080, height=1920, fps=60.000004}}
            if (StrContains(line, "DisplayModeRecord{mMode=")) {
                supportedModes.emplace_back(line);
            } else if (!StrEmpty(line)) {
                break;
            }
        }
        for (const auto &modeRecord : supportedModes) {
            int id = StringToInteger(GetPrevString(GetPostString(modeRecord, "id="), ","));
            int width = StringToInteger(GetPrevString(GetPostString(modeRecord, "width="), ","));
            int height = StringToInteger(GetPrevString(GetPostString(modeRecord, "height="), ","));
            int fps = StringToInteger(GetPrevString(GetPostString(modeRecord, "fps="), "."));
            displayModeMap_[fps][width] = id - 1;
            displayModeMap_[fps][height] = id - 1;
            CU::Logger::Info("id=%d, resolution=%dx%d, fps=%d.", id, width, height, fps);
        }
    } else {
        CU::Logger::Error("Failed to get display modes.");
        CU::Logger::Flush();
        std::exit(0);
    }
}

void RefreshRateTuner::IdleLoop_()
{
    SetThreadName("IdleLoop");
    SetTaskSchedPrio(0, 95);

    TIMER_LOOP(timer_) {
        int idleDelay = config_.data().at("idleDelay").toInt();
        if (active_ && keyUpTime_ > keyDownTime_) {
            auto keyUpDuration = GetTimeStampMs() - keyUpTime_;
            if (keyUpDuration >= idleDelay) {
                SetIdleRefreshRate_();
                timer_.setInterval(idleDelay);
            } else {
                timer_.setInterval(idleDelay - keyUpDuration);
            }
        } else {
            timer_.setInterval(idleDelay);
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
        auto fpsIter = displayModeMap_.find(fps);
        if (fpsIter == displayModeMap_.end()) {
            return -1;
        }
        const auto &resolutionMap = fpsIter->second;
        auto resolutionIter = resolutionMap.find(resolution);
        if (resolutionIter == resolutionMap.end()) {
            return -1;
        }
        return resolutionIter->second;
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

void RefreshRateTuner::SetActiveRefreshRate_()
{
    RunCommand(StrMerge("service call SurfaceFlinger 1035 i32 %d", activeDisplayModeIdx_));
    active_ = true;
}

void RefreshRateTuner::SetIdleRefreshRate_()
{
    RunCommand(StrMerge("service call SurfaceFlinger 1035 i32 %d", idleDisplayModeIdx_));
    active_ = false;
}

void RefreshRateTuner::ResetRefreshRate_()
{
    static const auto isFlymeOS = []() -> bool {
        char buffer[PROP_VALUE_MAX] = { 0 };
        __system_property_get("ro.build.flyme.version", buffer);
        return (atof(buffer) != 0.0);
    };

    int sdk_ver = GetAndroidSDKVersion();
    if (sdk_ver >= 31 && isFlymeOS()) {
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
        timer_.pauseTimer();
        WorkerThread_AddWork([this]() {
            SetIdleRefreshRate_();
        });
    } else {
        UpdatePolicy_("*");
        WorkerThread_AddWork([this]() {
            ResetRefreshRate_();
            SetActiveRefreshRate_();
        });
        timer_.continueTimer();
    }
}

void RefreshRateTuner::TopAppChanged_(const CU::EventTransfer::TransData &transData)
{
    auto appName = CU::EventTransfer::GetData<std::string>(transData);
    UpdatePolicy_(appName);
    WorkerThread_AddWork([this]() {
        SetActiveRefreshRate_();
    });
}

void RefreshRateTuner::KeyDown_(const CU::EventTransfer::TransData &transData)
{
    keyDownTime_ = GetTimeStampMs();
    if (!active_) {
        WorkerThread_AddWork([this]() {
            SetActiveRefreshRate_();
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

#include "refresh_rate_tuner.h"

RefreshRateTuner::RefreshRateTuner(const std::string &configPath) : 
    Module(), 
    configPath_(configPath),
    timer_(),
    displayModeMap_(), 
    config_(),
    activeDisplayModeId_(-1),
    idleDisplayModeId_(-1),
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
    CU::EventTransfer::Subscribe("CgroupWatcher.ScreenStateChanged", this,
        std::bind(&RefreshRateTuner::ScreenStateChanged_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("TopAppMonitor.TopAppChanged", this,
        std::bind(&RefreshRateTuner::TopAppChanged_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("InputListener.KEY_DOWN", this,
        std::bind(&RefreshRateTuner::KeyDown_, this, std::placeholders::_1));
    CU::EventTransfer::Subscribe("InputListener.KEY_UP", this,
        std::bind(&RefreshRateTuner::KeyUp_, this, std::placeholders::_1));
    FileWatcher_AddWatch(configPath_, std::bind(&RefreshRateTuner::ConfigModified_, this));
    timer_.setLoop(std::bind(&RefreshRateTuner::IdleLoop_, this));
    timer_.setInterval(config_.data().at("idleDelay").toInt());
    timer_.start();
}

void RefreshRateTuner::Init_()
{
    auto displayInfo = CU::ExecCommand("dumpsys display");
    if (CU::StrContains(displayInfo, "mSfDisplayModes=")) {
        std::vector<std::string> displayModes{};
        auto lines = CU::StrSplit(displayInfo, '\n');
        for (const auto &line : lines) {
            // DisplayMode{id=0,width=1080,height=2460,xDpi=397.565,yDpi=397.987,refreshRate=144.00002,...
            auto trimedLine = CU::TrimStr(line);
            if (CU::StrStartsWith(trimedLine, "DisplayMode{")) {
                displayModes.emplace_back(trimedLine);
            }
        }
        for (const auto &displayMode : displayModes) {
            int id = CU::StrToInt(CU::SubPrevStr(CU::SubPostStr(displayMode, "id="), ','));
            int width = CU::StrToInt(CU::SubPrevStr(CU::SubPostStr(displayMode, "width="), ','));
            int height = CU::StrToInt(CU::SubPrevStr(CU::SubPostStr(displayMode, "height="), ','));
            int refreshRate = CU::StrToInt(CU::SubPrevStr(CU::SubPostStr(displayMode, "refreshRate="), '.'));
            if (refreshRate == 0) {
                refreshRate = CU::StrToInt(CU::SubPrevStr(CU::SubPostStr(displayMode, "peakRefreshRate="), '.'));
            }
            displayModeMap_[refreshRate][width] = id;
            displayModeMap_[refreshRate][height] = id;
            CU::Logger::Info("id={}, resolution={}x{}, refreshRate={}.", id, width, height, refreshRate);
        }
    } else if (CU::StrContains(displayInfo, "mSupportedModes=")) {
        std::vector<std::string> supportedModes{};
        auto lines = CU::StrSplit(displayInfo, '\n');
        for (const auto &line : lines) {
            // DisplayModeRecord{mMode={id=1,width=1080,height=1920,fps=60.000004}}
            auto trimedLine = CU::TrimStr(line);
            if (CU::StrStartsWith(line, "DisplayModeRecord{mMode={")) {
                supportedModes.emplace_back(trimedLine);
            }
        }
        for (const auto &modeRecord : supportedModes) {
            int id = CU::StrToInt(CU::SubPrevStr(CU::SubPostStr(modeRecord, "id="), ',')) - 1;
            int width = CU::StrToInt(CU::SubPrevStr(CU::SubPostStr(modeRecord, "width="), ','));
            int height = CU::StrToInt(CU::SubPrevStr(CU::SubPostStr(modeRecord, "height="), ','));
            int fps = CU::StrToInt(CU::SubPrevStr(CU::SubPostStr(modeRecord, "fps="), '.'));
            displayModeMap_[fps][width] = id;
            displayModeMap_[fps][height] = id;
            CU::Logger::Info("id={}, resolution={}x{}, fps={}.", id, width, height, fps);
        }
    } else {
        CU::Logger::Error("Failed to get display modes.");
        CU::Logger::Flush();
        std::exit(0);
    }
}

void RefreshRateTuner::IdleLoop_()
{
    CU::SetThreadName("IdleLoop");
    CU::SetTaskSchedPrio(0, 95);

    TIMER_LOOP(timer_) {
        int idleDelay = config_.data().at("idleDelay").toInt();
        if (active_ && keyUpTime_ > keyDownTime_) {
            auto keyUpDuration = CU::TimeStamp() - keyUpTime_;
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
        config_ = CU::JSONObject(CU::ReadFile(configPath_), true);
        CU::Logger::Info("Config loaded.");
    } catch (const std::exception &e) {
        CU::Logger::Warn("Failed to load config.");
        CU::Logger::Warn("Exception Thrown: {}.", e.what());
    }
}

void RefreshRateTuner::UpdatePolicy_(const std::string &appName)
{
    static const auto findDisplayModeId = [this](int fps, int resolution) -> int {
        if (!displayModeMap_.containsKey(fps)) {
            fps = *CU::MaxIter(displayModeMap_.keys());
        }
        if (!displayModeMap_.atKey(fps).containsKey(resolution)) {
            resolution = *CU::MaxIter(displayModeMap_.atKey(fps).keys());
        }
        return displayModeMap_.atKey(fps).atKey(resolution);
    };

    auto config = config_.data();
    auto policy = config.at("*").toObject();
    if (config.contains(appName)) {
        policy = config.at(appName).toObject();
    }
    int resolution = policy.at("resolution").toInt();
    activeDisplayModeId_ = findDisplayModeId(policy.at("active").toInt(), resolution);
    idleDisplayModeId_ = findDisplayModeId(policy.at("idle").toInt(), resolution);
}

void RefreshRateTuner::SetActiveRefreshRate_()
{
    CU::RunCommand(CU::Format("service call SurfaceFlinger 1035 i32 {}", activeDisplayModeId_));
    active_ = true;
}

void RefreshRateTuner::SetIdleRefreshRate_()
{
    CU::RunCommand(CU::Format("service call SurfaceFlinger 1035 i32 {}", idleDisplayModeId_));
    active_ = false;
}

void RefreshRateTuner::ResetRefreshRate_()
{
    static const auto isFlymeOS = []() -> bool {
        char buffer[16] = { 0 };
        __system_property_get("ro.build.flyme.version", buffer);
        return (std::atoi(buffer) != 0);
    };

    static int api_level = android_get_device_api_level();
    if (api_level >= 31 && isFlymeOS()) {
        CU::RunCommand("service call SurfaceFlinger 1037");
    }
    if (api_level >= 30) {
        CU::RunCommand("service call SurfaceFlinger 1036 i32 0");
    }
    if (api_level >= 29) {
        CU::RunCommand("service call SurfaceFlinger 1035 i32 -1");
    }
    active_ = false;
}

void RefreshRateTuner::ScreenStateChanged_(const CU::EventTransfer::TransData &transData)
{
    auto screenState = CU::EventTransfer::GetData<ScreenState>(transData);
    if (screenState == ScreenState::SCREEN_OFF) {
        UpdatePolicy_("screenOff");
        timer_.pauseTimer();
        WorkerThread_AddWork(std::bind(&RefreshRateTuner::SetIdleRefreshRate_, this));
    } else {
        static const auto resetRefreshRate = [this]() {
            ResetRefreshRate_();
            SetActiveRefreshRate_();
        };
        UpdatePolicy_("*");
        WorkerThread_AddWork(resetRefreshRate);
        timer_.continueTimer();
    }
}

void RefreshRateTuner::TopAppChanged_(const CU::EventTransfer::TransData &transData)
{
    int pid = CU::EventTransfer::GetData<int>(transData);
    UpdatePolicy_(CU::ReadFile(CU::Format("/proc/{}/cmdline", pid)));
    WorkerThread_AddWork(std::bind(&RefreshRateTuner::SetActiveRefreshRate_, this));
}

void RefreshRateTuner::KeyDown_(const CU::EventTransfer::TransData &transData)
{
    keyDownTime_ = CU::TimeStamp();
    if (!active_) {
        WorkerThread_AddWork(std::bind(&RefreshRateTuner::SetActiveRefreshRate_, this));
    }
}

void RefreshRateTuner::KeyUp_(const CU::EventTransfer::TransData &transData)
{
    keyUpTime_ = CU::TimeStamp();
}

void RefreshRateTuner::ConfigModified_()
{
    WorkerThread_AddWork(std::bind(&RefreshRateTuner::LoadConfig_, this));
}

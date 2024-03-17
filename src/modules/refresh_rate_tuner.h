#include "platform/module.h"
#include "utils/libcu.h"
#include "utils/cu_sched.h"
#include "utils/CuLogger.h"
#include "utils/CuEventTransfer.h"
#include "utils/CuJSONObject.h"
#include "utils/CuSafeVal.h"

class RefreshRateTuner : public Module {
    public:
        RefreshRateTuner(const std::string &configPath);
        ~RefreshRateTuner();

        void Start();

    private:
        std::string configPath_;
        std::unordered_map<int, std::unordered_map<int, int>> displayModeMap_;
        CU::SafeVal<CU::JSONObject> config_;
        int activeDisplayModeIdx_;
        int idleDisplayModeIdx_;
        uint64_t keyDownTime_;
        uint64_t keyUpTime_;
        bool active_;

        void Init_();
        void IdleLoop_();
        void LoadConfig_();
        void UpdatePolicy_(const std::string &appName);
        void SetActiveRefreshRate_();
        void SetIdleRefreshRate_();
        void ResetRefreshRate_();
        void ScreenStateChanged_(const CU::EventTransfer::TransData &transData);
        void TopAppChanged_(const CU::EventTransfer::TransData &transData);
        void KeyDown_(const CU::EventTransfer::TransData &transData);
        void KeyUp_(const CU::EventTransfer::TransData &transData);
        void ConfigModified_();
};

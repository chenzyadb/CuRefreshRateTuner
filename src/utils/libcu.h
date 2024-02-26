#pragma once

#include <iostream>
#include <stdexcept>
#include <utility>
#include <string>
#include <vector>
#include <array>
#include <thread>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <limits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cstdarg>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cctype>
#include <cinttypes>

inline std::vector<std::string> StrSplit(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> strList{};
    size_t start_pos = 0;
    size_t pos = str.find(delimiter);
    while (pos != std::string::npos) {
        if (pos > start_pos) {
            strList.emplace_back(str.substr(start_pos, pos - start_pos));
        }
        start_pos = pos + delimiter.size();
        pos = str.find(delimiter, start_pos);
    }
    if (start_pos < str.size()) {
        strList.emplace_back(str.substr(start_pos));
    }
    return strList;
}

inline std::string StrSplitAt(const std::string &str, const std::string &delimiter, const int &targetCount) 
{
    int count = 0;
    size_t start_pos = 0;
    size_t pos = str.find(delimiter);
    while (pos != std::string::npos) {
        if (pos > start_pos) {
            if (count == targetCount) {
                return str.substr(start_pos, pos - start_pos);
            }
            count++;
        }
        start_pos = pos + delimiter.size();
        pos = str.find(delimiter, start_pos);
    }
    if (start_pos < str.size() && count == targetCount) {
        return str.substr(start_pos);
    }
    return {};
}

inline std::string StrSplitSpaceAt(const std::string &str, const int &targetCount) 
{
    static const auto findNextSpace = [](const std::string &s, const size_t &p) -> size_t {
        if (p >= s.size()) {
            return std::string::npos;
        }
        for (size_t i = p; i < s.size(); i++) {
            if (s[i] == ' ') {
                return i;
            }
        }
        return std::string::npos;
    };

    int count = 0;
    size_t start_pos = 0;
    size_t pos = findNextSpace(str, 0);
    while (pos != std::string::npos) {
        if (pos > start_pos) {
            if (count == targetCount) {
                return str.substr(start_pos, pos - start_pos);
            }
            count++;
        }
        start_pos = pos + 1;
        pos = findNextSpace(str, start_pos);
    }
    if (start_pos < str.size() && count == targetCount) {
        return str.substr(start_pos);
    }
    return {};
}

inline std::string StrSplitLineAt(const std::string &str, const int &targetCount) 
{
    static const auto findNextLine = [](const std::string &s, const size_t &p) -> size_t {
        if (p >= s.size()) {
            return std::string::npos;
        }
        for (size_t i = p; i < s.size(); i++) {
            if (s[i] == '\n') {
                return i;
            }
        }
        return std::string::npos;
    };

    int count = 0;
    size_t start_pos = 0;
    size_t pos = findNextLine(str, 0);
    while (pos != std::string::npos) {
        if (count == targetCount) {
            return str.substr(start_pos, pos - start_pos);
        }
        count++;
        start_pos = pos + 1;
        pos = findNextLine(str, start_pos);
    }
    if (start_pos < str.size() && count == targetCount) {
        return str.substr(start_pos);
    }
    return {};
}

inline std::vector<std::string> StrSplitLine(const std::string &str) 
{
    std::vector<std::string> lines{};
    size_t start_pos = 0;
    for (size_t pos = 0; pos < str.size(); pos++) {
        if (str[pos] == '\n') {
            lines.emplace_back(str.substr(start_pos, pos - start_pos));
            start_pos = pos + 1;
        }
    }
    if (start_pos < str.size()) {
        lines.emplace_back(str.substr(start_pos));
    }
    return lines;
}

inline std::vector<std::string> StrSplitSpace(const std::string &str) 
{
    std::vector<std::string> sequences{};
    size_t start_pos = 0;
    for (size_t pos = 0; pos < str.size(); pos++) {
        if (str[pos] == ' ') {
            sequences.emplace_back(str.substr(start_pos, pos - start_pos));
            start_pos = pos + 1;
        }
    }
    if (start_pos < str.size()) {
        sequences.emplace_back(str.substr(start_pos));
    }
    return sequences;
}

inline std::string StrMerge(const char* format, ...)
{
    va_list args{};
    va_start(args, format);
    size_t len = vsnprintf(nullptr, 0, format, args);
    va_end(args);
    if (len > 0) {
        auto size = len + 1;
        auto buffer = new char[size];
        memset(buffer, 0, size);
        va_start(args, format);
        vsnprintf(buffer, size, format, args);
        va_end(args);
        std::string mergedStr(buffer);
        delete[] buffer;
        return mergedStr;
    }
    return {};
}

inline std::string GetPrevString(const std::string &str, const std::string &key)
{
    return str.substr(0, str.find(key));
}

inline std::string GetRePrevString(const std::string &str, const std::string &key)
{
    return str.substr(0, str.rfind(key));
}

inline std::string GetPostString(const std::string &str, const std::string &key)
{
    return str.substr(str.find(key) + key.size());
}

inline std::string GetRePostString(const std::string &str, const std::string &key)
{
    return str.substr(str.rfind(key) + key.size());
}

inline bool StrContains(const std::string &str, const std::string &subStr) 
{
    if (str.empty() || subStr.empty()) {
        return false;
    }
    return (str.find(subStr) != std::string::npos);
}

inline bool StrEmpty(const std::string &str) noexcept
{
    for (const auto &ch : str) {
        if (isalnum(ch) != 0) {
            return false;
        }
    }
    return true;
}

inline int StringToInteger(const std::string &str) noexcept
{
    int64_t integer = 0;
    size_t num_start_pos = -1;
    bool negative = false;
    for (size_t pos = 0; pos < str.size(); pos++) {
        if (num_start_pos == -1) {
            if (str[pos] >= '0' && str[pos] <= '9') {
                num_start_pos = pos;
            } else if (str[pos] == '-') {
                negative = true;
                num_start_pos = pos;
                continue;
            } else if (str[pos] == '+') {
                negative = false;
                num_start_pos = pos;
                continue;
            } else if (str[pos] == ' ') {
                continue;
            } else {
                return 0;
            }
        }
        if (num_start_pos != -1) {
            if (str[pos] >= '0' && str[pos] <= '9') {
                integer = integer * 10 + str[pos] - '0';
            } else {
                if (negative) {
                    return -integer;
                }
                return integer;
            }
        }
        if (!negative && integer > INT_MAX) {
            return INT_MAX;
        } else if (negative && -integer < INT_MIN) {
            return INT_MIN;
        }
    }
    if (negative) {
        return -integer;
    }
    return integer;
}

inline uint64_t String16BitToInteger(const std::string &str) noexcept
{
    uint64_t integer = 0;
    for (const char &c : str) {
        if (c >= '0' && c <= '9') {
            integer = integer * 16 + (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            integer = integer * 16 + (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            integer = integer * 16 + (c - 'A' + 10);
        } else {
            break;
        }
    }
    return integer;
}

inline std::string TrimStr(const std::string &str) 
{
    std::string trimedStr{};
    for (const auto &ch : str) {
        if (isalnum(ch) != 0) {
            trimedStr += ch;
        }
    }
    return trimedStr;
}

template <typename T>
inline int RoundNum(const T &num) noexcept
{
    if (num > INT_MAX) {
        return INT_MAX;
    } else if (num < INT_MIN) {
        return INT_MIN;
    }

    int intNum = 0;
    int dec = static_cast<int>(num * 10) % 10;
    if (dec >= 5) {
        intNum = static_cast<int>(num) + 1;
    } else {
        intNum = static_cast<int>(num);
    }
    return intNum;
}

template <typename T>
inline T AbsVal(const T &num) noexcept
{
    if (num < 0) {
        return -num;
    }
    return num;
}

template <typename T>
inline T SquareVal(const T &val) noexcept
{
    return (val * val);
}

template <typename T>
inline T SqrtVal(const T &val) noexcept
{
    if (val == 0) {
        return 0;
    }

    auto high = static_cast<double>(val), low = 0.0;
    if (val < 1.0) {
        high = 1.0;
    }
    while ((high - low) > 0.01) {
        auto mid = (low + high) / 2;
        if ((mid * mid) > val) {
            high = mid;
        } else {
            low = mid;
        }
    }
    return static_cast<T>((low + high) / 2);
}

template <typename T>
inline T VecMaxItem(const std::vector<T> &vec) noexcept
{
    if (vec.size() == 0) {
        return {};
    }

    T maxItem{};
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        if (*iter > maxItem) {
            maxItem = *iter;
        }
    }
    return maxItem;
}

template <typename T>
inline T VecMinItem(const std::vector<T> &vec) noexcept
{
    if (vec.size() == 0) {
        return {};
    }

    T minItem{};
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        if (*iter < minItem) {
            minItem = *iter;
        }
    }
    return minItem;
}

template <typename T>
inline T VecApproxItem(const std::vector<T> &vec, const T &targetVal) 
{
    if (vec.size() == 0) {
        return {};
    }

    T approxItem{};
    T minDiff = std::numeric_limits<T>::max();
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        T diff = std::abs(*iter - targetVal);
        if (diff < minDiff) {
            approxItem = *iter;
            minDiff = diff;
        }
    }
    return approxItem;
}

template <typename T>
inline std::vector<T> UniqueVec(const std::vector<T> &vec) 
{
    std::vector<T> uniquedVec(vec);
    std::sort(uniquedVec.begin(), uniquedVec.end());
    auto iter = std::unique(uniquedVec.begin(), uniquedVec.end());
    uniquedVec.erase(iter, uniquedVec.end());
    return uniquedVec;
}

template <typename T>
inline T SumVec(const std::vector<T> &vec) noexcept
{
    if (vec.size() == 0) {
        return {};
    }

    T sum{};
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        sum += *iter;
    }
    return sum;
}

#if defined(__DATE__)

inline int GetCompileDateCode()
{
    static const std::unordered_map<std::string, int> monthMap{
        {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4}, {"May", 5}, {"Jun", 6},
        {"Jul", 7}, {"Aug", 8}, {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12}
    };
    
    char month[4] = { 0 };
    int year = 0, day = 0;
    sscanf(__DATE__, "%s %d %d", month, &day, &year);
    return (year * 10000 + monthMap.at(month) * 100 + day);
}

#endif // __DATE__

#if defined(__unix) || defined(__unix__)
// POSIX Interface Functions.

#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>

inline void CreateFile(const std::string &filePath, const std::string &content) noexcept
{
    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, content.data(), content.size());
        close(fd);
    }
}

inline void AppendFile(const std::string &filePath, const std::string &content) noexcept
{
    int fd = open(filePath.c_str(), O_WRONLY | O_APPEND | O_NONBLOCK);
    if (fd < 0) {
        chmod(filePath.c_str(), 0666);
        fd = open(filePath.c_str(), O_WRONLY | O_APPEND | O_NONBLOCK);
    }
    if (fd >= 0) {
        write(fd, content.data(), content.size());
        close(fd);
    }
}

inline void WriteFile(const std::string &filePath, const std::string &content) noexcept
{
    int fd = open(filePath.c_str(), O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        chmod(filePath.c_str(), 0666);
        fd = open(filePath.c_str(), O_WRONLY | O_NONBLOCK);
    }
    if (fd >= 0) {
        write(fd, content.data(), content.size());
        close(fd);
    }
}

inline std::string ReadFile(const std::string &filePath) 
{
    int fd = open(filePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        chmod(filePath.c_str(), 0666);
        fd = open(filePath.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (fd >= 0) {
        auto file_size = lseek(fd, 0, SEEK_END);
        if (file_size <= 0) {
            file_size = 4096;
        } else {
            file_size += 1;
        }
        lseek(fd, 0, SEEK_SET);
        auto buffer = new char[file_size];
        memset(buffer, 0, file_size);
        auto len = read(fd, buffer, file_size);
        close(fd);
        if (len >= 0) {
            *(buffer + len) = '\0';
        } else {
            *buffer = '\0';
        }
        std::string content(buffer);
        delete[] buffer;
        return content;
    }
    return {};
}

inline bool IsPathExist(const std::string &path) noexcept
{
    return (access(path.c_str(), F_OK) != -1);
}

inline int GetThreadPid(const int &tid) noexcept
{
    char statusPath[128] = { 0 };
    snprintf(statusPath, sizeof(statusPath), "/proc/%d/status", tid);
    int fd = open(statusPath, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        read(fd, buffer, sizeof(buffer));
        close(fd);
        int pid = -1;
        sscanf(strstr(buffer, "Tgid:"), "Tgid: %d", &pid);
        return pid;
    }
    return -1;
}

enum class TaskType : uint8_t {TASK_OTHER, TASK_FOREGROUND, TASK_VISIBLE, TASK_SERVICE, TASK_SYSTEM, TASK_BACKGROUND};

inline TaskType GetTaskType(const int &pid) noexcept
{
    char oomAdjPath[128] = { 0 };
    snprintf(oomAdjPath, sizeof(oomAdjPath), "/proc/%d/oom_adj", pid);
    int fd = open(oomAdjPath, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        read(fd, buffer, sizeof(buffer));
        close(fd);
        int oom_adj = 16;
        if (sscanf(buffer, "%d", &oom_adj) == 1) {
            if (oom_adj == 0) {
                return TaskType::TASK_FOREGROUND;
            } else if (oom_adj == 1) {
                return TaskType::TASK_VISIBLE;
            } else if (oom_adj >= 2 && oom_adj <= 8) {
                return TaskType::TASK_SERVICE;
            } else if (oom_adj <= -1 && oom_adj >= -17) {
                return TaskType::TASK_SYSTEM;
            } else if (oom_adj >= 9 && oom_adj <= 16) {
                return TaskType::TASK_BACKGROUND;
            }
        }
    }
    return TaskType::TASK_OTHER;
}

inline std::string GetTaskName(const int &pid) 
{
    char cmdlinePath[128] = { 0 };
    snprintf(cmdlinePath, sizeof(cmdlinePath), "/proc/%d/cmdline", pid);
    int fd = open(cmdlinePath, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        auto len = read(fd, buffer, sizeof(buffer));
        close(fd);
        if (len >= 0) {
            buffer[len] = '\0';
        } else {
            buffer[0] = '\0';
        }
        std::string taskName(buffer);
        return taskName; 
    }
    return {};
}

inline std::string GetTaskComm(const int &pid) 
{
    char cmdlinePath[128] = { 0 };
    snprintf(cmdlinePath, sizeof(cmdlinePath), "/proc/%d/comm", pid);
    int fd = open(cmdlinePath, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        auto len = read(fd, buffer, sizeof(buffer));
        close(fd);
        if (len >= 0) {
            buffer[len] = '\0';
        } else {
            buffer[0] = '\0';
        }
        std::string taskComm(buffer);
        return taskComm;
    }
    return {};
}

inline uint64_t GetThreadRuntime(const int &pid, const int &tid) noexcept
{
    char statPath[128] = { 0 };
    snprintf(statPath, sizeof(statPath), "/proc/%d/task/%d/stat", pid, tid);
    int fd = open(statPath, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        read(fd, buffer, sizeof(buffer));
        close(fd);
        uint64_t utime = 0, stime = 0;
        sscanf((strchr(buffer, ')') + 2), "%*c %*lld %*lld %*lld %*lld %*lld %*lld %*lld %*lld %*lld %*lld %" SCNu64 " %" SCNu64,
            &utime, &stime);
        return (utime + stime);
    }
    return 0;
}

inline void SetThreadName(const std::string &name) noexcept
{
    prctl(PR_SET_NAME, name.c_str());
}

inline int FindTaskPid(const std::string &taskName) noexcept
{
    int taskPid = -1;
    struct dirent** entryList = nullptr;
    int len = scandir("/proc", &entryList, nullptr, alphasort);
    if (len < 0) {
        return -1;
    }
    for (int pos = 0; pos < len; pos++) {
        auto entry = *(entryList + pos);
        if (taskPid == -1 && entry->d_type == DT_DIR) {
            int pid = atoi(entry->d_name);
            if (pid > 0 && pid < (INT16_MAX + 1)) {
                char cmdlinePath[128] = { 0 };
                snprintf(cmdlinePath, sizeof(cmdlinePath), "/proc/%d/cmdline", pid);
                int fd = open(cmdlinePath, O_RDONLY | O_NONBLOCK);
                if (fd >= 0) {
                    char cmdline[4096] = { 0 };
                    read(fd, cmdline, sizeof(cmdline));
                    if (strstr(cmdline, taskName.c_str())) {
                        taskPid = pid;
                    }
                }
            }
        }
        free(entry);
    }
    free(entryList);
    return taskPid;
}

inline std::vector<int> GetTaskThreads(const int &pid) 
{
    char taskPath[128] = { 0 };
    snprintf(taskPath, sizeof(taskPath), "/proc/%d/task", pid);
    struct dirent** entryList = nullptr;
    int len = scandir(taskPath, &entryList, nullptr, alphasort);
    if (len < 0) {
        return {};
    }
    std::vector<int> threads{};
    for (int pos = 0; pos < len; pos++) {
        auto entry = *(entryList + pos);
        int tid = atoi(entry->d_name);
        if (tid > 0 && tid < (INT16_MAX + 1)) {
            threads.emplace_back(tid);
        }
        free(entry);
    }
    free(entryList);
    return threads;
}

inline uint64_t GetTimeStampMs() noexcept
{
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

inline int GetLinuxKernelVersion() noexcept
{
    // VersionCode: r.xx.yyy(3.18.140) -> 318140; r.x.yy(5.4.86) -> 504086;
    int version = 0;
    struct utsname uts{};
    if (uname(&uts) != -1) {
        int r = 0, x = 0, y = 0;
        sscanf(uts.release, "%d.%d.%d", &r, &x, &y);
        version = r * 100000 + x * 1000 + y;
    }
    return version;
}

inline std::string ExecCommand(const std::string &command)
{
    std::string ret{};
    auto fp = popen(command.c_str(), "r");
    if (fp) {
        char buffer[4096] = { 0 };
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            ret += buffer;
        }
        pclose(fp);
    }
    return ret;
}

inline int RunCommand(const std::string &command) noexcept
{
    return system(command.c_str());
}


#if defined(__ANDROID_API__)
// Android NDK Interface Functions.

#include <sys/system_properties.h>

inline std::string DumpTopActivityInfo() 
{
    auto fp = popen("dumpsys activity oom 2>/dev/null", "r");
    if (fp) {
        std::string activityInfo{};
        char buffer[1024] = { 0 };
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            if (strstr(buffer, "top-activity")) {
                activityInfo = buffer;
                break;
            }
        }
        pclose(fp);
        return activityInfo;
    }
    return {};
}

enum class ScreenState : uint8_t {SCREEN_OFF, SCREEN_ON};

inline ScreenState GetScreenStateViaCgroup() noexcept
{
    int fd = open("/dev/cpuset/restricted/tasks", O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        auto len = read(fd, buffer, sizeof(buffer));
        close(fd);
        int restrictedTaskNum = 0;
        for (size_t pos = 0; pos < len; pos++) {
            if (buffer[pos] == '\n') {
                restrictedTaskNum++;
                if (restrictedTaskNum > 10) {
                    return ScreenState::SCREEN_OFF;
                }
            }
        }
    }
    return ScreenState::SCREEN_ON;
}

inline ScreenState GetScreenStateViaWakelock() noexcept
{
    int fd = open("/sys/power/wake_lock", O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        read(fd, buffer, sizeof(buffer));
        close(fd);
        if (!strstr(buffer, "PowerManagerService.Display")) {
            return ScreenState::SCREEN_OFF;
        }
    }
    return ScreenState::SCREEN_ON;
}

inline int GetAndroidSDKVersion() noexcept
{
    char buffer[PROP_VALUE_MAX] = { 0 };
    __system_property_get("ro.build.version.sdk", buffer);
    return atoi(buffer);
}

inline std::string GetDeviceSerialNo() 
{
    char buffer[PROP_VALUE_MAX] = { 0 };
    __system_property_get("ro.serialno", buffer);
    std::string serialNo(buffer);
    return serialNo;
}

#endif // __ANDROID_API__
#endif // __unix__

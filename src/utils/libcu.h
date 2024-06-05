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

#define CU_UNUSED(VAL) (void)(VAL)
#define CU_WCHAR(VAL) L##VAL

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

inline std::string StrSplitAt(const std::string &str, const std::string &delimiter, int targetCount) 
{
    int count = 0;
    size_t start_pos = 0;
    size_t pos = str.find(delimiter);
    while (pos != std::string::npos) {
        if (pos > start_pos) {
            if (count == targetCount) {
                break;
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

inline std::string StrSplitAt(const std::string &str, char delimiter, int targetCount) 
{
    int count = 0;
    size_t start_pos = 0;
    size_t pos = str.find(delimiter);
    while (pos != std::string::npos) {
        if (pos > start_pos) {
            if (count == targetCount) {
                break;
            }
            count++;
        }
        start_pos = pos + 1;
        pos = str.find(delimiter, start_pos);
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
            if (start_pos < pos) {
                lines.emplace_back(str.substr(start_pos, pos - start_pos));
            }
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
            if (start_pos < pos) {
                sequences.emplace_back(str.substr(start_pos, pos - start_pos));
            }
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
    std::string content{};
    int len = 0;
    {
        va_list args{};
        va_start(args, format);
        len = vsnprintf(nullptr, 0, format, args) + 1;
        va_end(args);
    }
    if (len > 1) {
        auto buffer = new char[len];
        memset(buffer, 0, len);
        va_list args{};
        va_start(args, format);
        vsnprintf(buffer, len, format, args);
        va_end(args);
        content = buffer;
        delete[] buffer;
    }
    return content;
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

inline std::string GetRePostString(const std::string &str, const std::string &key) noexcept
{
    return str.substr(str.rfind(key) + key.size());
}

inline bool StrContains(const std::string &str, const std::string &subStr) noexcept
{
    return (str.find(subStr) != std::string::npos);
}

inline int StringToInteger(const std::string &str) noexcept
{
    char buffer[32] = { 0 };
    int buffer_offset = 0;
    for (int pos = 0; pos < str.size() && pos < (sizeof(buffer) - 1); pos++) {
        if (str[pos] >= '0' && str[pos] <= '9') {
            buffer[buffer_offset] = str[pos];
            buffer_offset++;
        }
    }
    buffer[buffer_offset] = '\0';
    return atoi(buffer);
}

inline int64_t StringToLong(const std::string &str) noexcept
{
    char buffer[32] = { 0 };
    int buffer_offset = 0;
    for (int pos = 0; pos < str.size() && pos < (sizeof(buffer) - 1); pos++) {
        if (str[pos] >= '0' && str[pos] <= '9') {
            buffer[buffer_offset] = str[pos];
            buffer_offset++;
        }
    }
    buffer[buffer_offset] = '\0';
    return atoll(buffer);
}

inline double StringToDouble(const std::string &str) noexcept
{
    char buffer[32] = { 0 };
    int buffer_offset = 0;
    for (int pos = 0; pos < str.size() && pos < (sizeof(buffer) - 1); pos++) {
        if ((str[pos] >= '0' && str[pos] <= '9') || str[pos] == '.') {
            buffer[buffer_offset] = str[pos];
            buffer_offset++;
        }
    }
    buffer[buffer_offset] = '\0';
    return atof(buffer);
}

inline uint64_t String16BitToInteger(const std::string &str) noexcept
{
    char buffer[32] = { 0 };
    int buffer_offset = 0;
    for (int pos = 0; pos < str.size() && pos < (sizeof(buffer) - 1); pos++) {
        if ((str[pos] >= '0' && str[pos] <= '9') || 
            (str[pos] >= 'a' && str[pos] <= 'f') || 
            (str[pos] >= 'A' && str[pos] <= 'F')
        ) {
            buffer[buffer_offset] = str[pos];
            buffer_offset++;
        }
    }
    buffer[buffer_offset] = '\0';
    return strtoull(buffer, nullptr, 16);
}

inline std::string TrimStr(const std::string &str) 
{
    std::string trimedStr{};
    for (const auto &ch : str) {
        switch (ch) {
            case ' ':
            case '\n':
            case '\t':
            case '\r':
            case '\f':
            case '\a':
            case '\b':
            case '\v':
                break;
            default:
                trimedStr += ch;
                break;
        }
    }
    return trimedStr;
}

template <typename _Ty>
inline size_t GetHash(const _Ty &val)
{
    std::hash<_Ty> hashVal{};
    return hashVal(val);
}

template <typename _Ty>
inline bool GenericCompare(const _Ty &val0, const _Ty &val1) noexcept
{
    const auto val0_addr = std::addressof(val0);
    const auto val1_addr = std::addressof(val1);
    if (val0_addr == val1_addr) {
        return true;
    }
    return (memcmp(val0_addr, val1_addr, sizeof(_Ty)) == 0);
}

template <typename _Ty>
inline int RoundNum(_Ty num) noexcept
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

template <typename _Ty>
inline _Ty AbsVal(_Ty num) noexcept
{
    if (num < 0) {
        return -num;
    }
    return num;
}

template <typename _Ty>
inline _Ty SquareVal(_Ty value) noexcept
{
    return (value * value);
}

template <typename _Ty>
inline _Ty SqrtVal(_Ty value) noexcept
{
    if (value == 0) {
        return 0;
    }
    auto high = static_cast<double>(value), low = 0.0;
    if (value < 1.0) {
        high = 1.0;
    }
    while ((high - low) > 0.01) {
        auto mid = (low + high) / 2;
        if ((mid * mid) > value) {
            high = mid;
        } else {
            low = mid;
        }
    }
    return static_cast<_Ty>((low + high) / 2);
}

template <typename _Ty>
inline const _Ty &VecMaxItem(const std::vector<_Ty> &vec) noexcept
{
    auto maxIter = vec.begin();
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        if (*iter > *maxIter) {
            maxIter = iter;
        }
    }
    return *maxIter;
}

template <typename _Ty>
inline const _Ty &VecMinItem(const std::vector<_Ty> &vec) noexcept
{
    auto minIter = vec.begin();
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        if (*iter < *minIter) {
            minIter = iter;
        }
    }
    return *minIter;
}

template <typename _Ty>
inline const _Ty &VecApproxItem(const std::vector<_Ty> &vec, _Ty targetVal) noexcept
{
    auto approxIter = vec.begin();
    _Ty minDiff = std::numeric_limits<_Ty>::max();
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        _Ty diff = std::abs(*iter - targetVal);
        if (diff < minDiff) {
            approxIter = iter;
            minDiff = diff;
        }
    }
    return *approxIter;
}

template <typename _Ty>
inline const _Ty &VecApproxMinItem(const std::vector<_Ty> &vec, _Ty targetVal) noexcept
{
    auto approxIter = vec.begin();
    _Ty minDiff = std::numeric_limits<_Ty>::max();
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        _Ty diff = *iter - targetVal;
        if (diff < minDiff && diff >= 0) {
            approxIter = iter;
            minDiff = diff;
        }
    }
    return *approxIter;
}

template <typename _Ty>
inline const _Ty &VecApproxMaxItem(const std::vector<_Ty> &vec, _Ty targetVal) noexcept
{
    auto approxIter = vec.begin();
    _Ty minDiff = std::numeric_limits<_Ty>::max();
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        _Ty diff = targetVal - *iter;
        if (diff < minDiff && diff >= 0) {
            approxIter = iter;
            minDiff = diff;
        }
    }
    return *approxIter;
}

template <typename _Ty>
inline _Ty AverageVec(const std::vector<_Ty> &vec) noexcept
{
    _Ty sum{};
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        sum += *iter;
    }
    return (sum / vec.size());
}

template <typename _Ty>
inline _Ty SumVec(const std::vector<_Ty> &vec) noexcept
{
    _Ty sum{};
    for (auto iter = vec.begin(); iter < vec.end(); iter++) {
        sum += *iter;
    }
    return sum;
}

template <typename _Ty>
inline std::vector<_Ty> UniqueVec(const std::vector<_Ty> &vec) 
{
    std::vector<_Ty> uniquedVec(vec);
    std::sort(uniquedVec.begin(), uniquedVec.end());
    auto iter = std::unique(uniquedVec.begin(), uniquedVec.end());
    uniquedVec.erase(iter, uniquedVec.end());
    return uniquedVec;
}

template <typename _Ty>
inline std::vector<_Ty> ShrinkVec(const std::vector<_Ty> &vec, size_t size) 
{
    std::vector<_Ty> trimedVec(vec);
    std::sort(trimedVec.begin(), trimedVec.end());
    auto iter = std::unique(trimedVec.begin(), trimedVec.end());
    trimedVec.erase(iter, trimedVec.end());
    if (trimedVec.size() <= size) {
        return trimedVec;
    }
    std::vector<_Ty> shrinkedVec{};
    _Ty itemDiff = (trimedVec.back() - trimedVec.front()) / (size - 1);
    for (size_t idx = 0; idx < size; idx++) {
        _Ty selectVal = trimedVec.front() + itemDiff * idx;
        auto selectIter = trimedVec.begin();
        _Ty minDiff = trimedVec.back();
        for (auto iter = trimedVec.begin(); iter < trimedVec.end(); iter++) {
            _Ty diff = std::abs(*iter - selectVal);
            if (diff < minDiff) {
                selectIter = iter;
                minDiff = diff;
            } else {
                break;
            }
        }
        shrinkedVec.emplace_back(*selectIter);
    }
    return shrinkedVec;
}

template <typename _Ty>
std::vector<_Ty> ReverseVec(const std::vector<_Ty> &vec) 
{
    std::vector<_Ty> reversedVec(vec);
    std::reverse(reversedVec.begin(), reversedVec.end());
    return reversedVec;
}

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


#if defined(__unix__)

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
    int fd = open(filePath.c_str(), (O_WRONLY | O_CREAT | O_TRUNC), 0644);
    if (fd >= 0) {
        write(fd, content.c_str(), (content.length() + 1));
        close(fd);
    }
}

inline void AppendFile(const std::string &filePath, const std::string &content) noexcept
{
    int fd = open(filePath.c_str(), (O_WRONLY | O_APPEND | O_NONBLOCK));
    if (fd < 0) {
        chmod(filePath.c_str(), 0666);
        fd = open(filePath.c_str(), (O_WRONLY | O_APPEND | O_NONBLOCK));
    }
    if (fd >= 0) {
        write(fd, content.c_str(), (content.length() + 1));
        close(fd);
    }
}

inline void WriteFile(const std::string &filePath, const std::string &content) noexcept
{
    int fd = open(filePath.c_str(), (O_WRONLY | O_NONBLOCK));
    if (fd < 0) {
        chmod(filePath.c_str(), 0666);
        fd = open(filePath.c_str(), (O_WRONLY | O_NONBLOCK));
    }
    if (fd >= 0) {
        write(fd, content.c_str(), (content.length() + 1));
        close(fd);
    }
}

inline std::string ReadFile(const std::string &filePath) 
{
    std::string content{};
    int fd = open(filePath.c_str(), (O_RDONLY | O_NONBLOCK));
    if (fd < 0) {
        chmod(filePath.c_str(), 0666);
        fd = open(filePath.c_str(), (O_RDONLY | O_NONBLOCK));
    }
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        while (read(fd, buffer, (sizeof(buffer) - 1)) > 0) {
            content += buffer;
            memset(buffer, 0, sizeof(buffer));
        }
        close(fd);
    }
    return content;
}

inline bool IsPathExist(const std::string &path) noexcept
{
    return (access(path.c_str(), F_OK) != -1);
}

inline int GetThreadPid(int tid) noexcept
{
    int pid = -1;
    char statusPath[128] = { 0 };
    snprintf(statusPath, sizeof(statusPath), "/proc/%d/status", tid);
    int fd = open(statusPath, (O_RDONLY | O_NONBLOCK));
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        if (read(fd, buffer, (sizeof(buffer) - 1)) > 0) {
            sscanf(strstr(buffer, "Tgid:"), "Tgid: %d", &pid);
        }
        close(fd);
    }
    return pid;
}

inline std::string GetTaskCgroupType(int pid, const std::string &cgroup)
{
    std::string cgroupContent{};
    {
        char cgroupPath[128] = { 0 };
        snprintf(cgroupPath, sizeof(cgroupPath), "/proc/%d/cgroup", pid);
        int fd = open(cgroupPath, (O_RDONLY | O_NONBLOCK));
        if (fd >= 0) {
            char buffer[4096] = { 0 };
            if (read(fd, buffer, (sizeof(buffer) - 1)) > 0) {
                cgroupContent = buffer;
            }
            close(fd);
        }
    }
    if (!cgroupContent.empty()) {
        auto begin_pos = cgroupContent.find(cgroup);
        if (begin_pos != std::string::npos) {
            begin_pos = begin_pos + cgroup.length() + 2;
            auto end_pos = cgroupContent.find('\n', begin_pos);
            if (end_pos == std::string::npos) {
                end_pos = cgroupContent.length();
            }
            return cgroupContent.substr(begin_pos, end_pos - begin_pos);
        }
    }
    return cgroupContent;
}

inline std::string GetTaskName(int pid) 
{
    std::string taskName{};
    char cmdlinePath[128] = { 0 };
    snprintf(cmdlinePath, sizeof(cmdlinePath), "/proc/%d/cmdline", pid);
    int fd = open(cmdlinePath, (O_RDONLY | O_NONBLOCK));
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        if (read(fd, buffer, (sizeof(buffer) - 1)) > 0) {
            taskName = buffer;
        }
        close(fd);
    }
    return taskName;
}

inline std::string GetTaskComm(int pid) 
{
    std::string taskComm{};
    char commPath[128] = { 0 };
    snprintf(commPath, sizeof(commPath), "/proc/%d/cmdline", pid);
    int fd = open(commPath, (O_RDONLY | O_NONBLOCK));
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        if (read(fd, buffer, (sizeof(buffer) - 1)) > 0) {
            taskComm = buffer;
        }
        close(fd);
    }
    return taskComm;
}

inline uint64_t GetThreadRuntime(int pid, int tid) noexcept
{
    uint64_t runtime = 0;
    char statPath[128] = { 0 };
    snprintf(statPath, sizeof(statPath), "/proc/%d/task/%d/stat", pid, tid);
    int fd = open(statPath, (O_RDONLY | O_NONBLOCK));
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        if (read(fd, buffer, (sizeof(buffer) - 1)) > 0) {
            uint64_t utime = 0, stime = 0;
            sscanf(strchr(buffer, ')') + 2, "%*c %*ld %*ld %*ld %*ld %*ld %*ld %*ld %*ld %*ld %*ld %" SCNu64 " %" SCNu64,
            &utime, &stime);
            runtime = utime + stime;
        }
        close(fd);
    }
    return runtime;
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
    if (entryList != nullptr && len > 0) {
        for (int pos = 0; pos < len; pos++) {
            auto entry = *(entryList + pos);
            auto dirType = entry->d_type;
            int pid = atoi(entry->d_name);
            free(entry);
            if (dirType == DT_DIR && pid > 0 && pid <= INT16_MAX) {
                char cmdlinePath[128] = { 0 };
                snprintf(cmdlinePath, sizeof(cmdlinePath), "/proc/%d/cmdline", pid);
                int fd = open(cmdlinePath, (O_RDONLY | O_NONBLOCK));
                if (fd >= 0) {
                    char cmdline[4096] = { 0 };
                    read(fd, cmdline, (sizeof(cmdline) - 1));
                    close(fd);
                    if (strstr(cmdline, taskName.c_str())) {
                        taskPid = pid;
                        break;
                    }
                }
            }
        }
        free(entryList);
    }
    return taskPid;
}

inline std::vector<int> GetTaskThreads(int pid) 
{
    std::vector<int> threads{};
    char taskPath[128] = { 0 };
    snprintf(taskPath, sizeof(taskPath), "/proc/%d/task", pid);
    struct dirent** entryList = nullptr;
    int len = scandir(taskPath, &entryList, nullptr, alphasort);
    if (entryList != nullptr && len > 0) {
        for (int pos = 0; pos < len; pos++) {
            auto entry = *(entryList + pos);
            if (entry->d_type == DT_DIR) {
                int tid = atoi(entry->d_name);
                if (tid > 0 && tid <= INT16_MAX) {
                    threads.emplace_back(tid);
                }
            }
            free(entry);
        }
        free(entryList);
    }
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
    std::string content{};
    auto fp = popen(command.c_str(), "r");
    if (fp) {
        char buffer[4096] = { 0 };
        while (fread(buffer, sizeof(char), (sizeof(buffer) - 1), fp) > 0) {
            content += buffer;
            memset(buffer, 0, sizeof(buffer));
        }
        pclose(fp);
    }
    return content;
}

inline int RunCommand(const std::string &command) noexcept
{
    return system(command.c_str());
}

inline std::string FindPath(const std::string &path, const std::string &symbol)
{
    std::string matchedPath{};
    struct dirent** entryList = nullptr;
    int len = scandir(path.c_str(), &entryList, nullptr, alphasort);
    if (entryList != nullptr && len > 0) {
        for (int pos = 0; pos < len; pos++) {
            auto entry = *(entryList + pos);
            std::string dirName(entry->d_name);
            free(entry);
            if (dirName.find(symbol) != std::string::npos) {
                matchedPath = path + '/' + dirName;
                break;
            }
        }
        free(entryList);
    }
    return matchedPath;
}

inline std::vector<std::string> ListDir(const std::string &path) 
{
    std::vector<std::string> dirPaths{};
    struct dirent** entryList = nullptr;
    int len = scandir(path.c_str(), &entryList, nullptr, alphasort);
    if (entryList != nullptr && len > 0) {
        for (int pos = 0; pos < len; pos++) {
            auto entry = *(entryList + pos);
            if (entry->d_type == DT_DIR) {
                dirPaths.emplace_back(path + '/' + entry->d_name);
            }
            free(entry);
        }
        free(entryList);
    }
    return dirPaths;
}

inline std::vector<std::string> ListFile(const std::string &path)
{
    std::vector<std::string> filePaths{};
    struct dirent** entryList = nullptr;
    int len = scandir(path.c_str(), &entryList, nullptr, alphasort);
    if (entryList != nullptr && len > 0) {
        for (int pos = 0; pos < len; pos++) {
            auto entry = *(entryList + pos);
            if (entry->d_type == DT_REG) {
                filePaths.emplace_back(path + '/' + entry->d_name);
            }
            free(entry);
        }
        free(entryList);
    }
    return filePaths;
}

#if defined(__ANDROID_API__)

#include <sys/system_properties.h>

inline std::string DumpTopActivityInfo() 
{
    std::string activityInfo{};
    auto fp = popen("dumpsys activity oom 2>/dev/null", "r");
    if (fp) {
        char buffer[1024] = { 0 };
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            if (strstr(buffer, "top-activity")) {
                activityInfo = buffer;
                break;
            }
        }
        pclose(fp);
    }
    return activityInfo;
}

enum class ScreenState : uint8_t {SCREEN_OFF, SCREEN_ON};

inline ScreenState GetScreenStateViaCgroup() noexcept
{
    ScreenState state = ScreenState::SCREEN_ON;
    int fd = open("/dev/cpuset/restricted/tasks", (O_RDONLY | O_NONBLOCK));
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        if (read(fd, buffer, (sizeof(buffer) - 1)) > 0) {
            int taskCount = 0;
            for (int pos = 0; pos < sizeof(buffer); pos++) {
                if (buffer[pos] == '\n') {
                    taskCount++;
                } else if (buffer[pos] == 0) {
                    break;
                }
            }
            if (taskCount > 10) {
                state = ScreenState::SCREEN_OFF;
            }
        }
        close(fd);
    }
    return state;
}

inline ScreenState GetScreenStateViaWakelock() noexcept
{
    ScreenState state = ScreenState::SCREEN_ON;
    int fd = open("/sys/power/wake_lock", (O_RDONLY | O_NONBLOCK));
    if (fd >= 0) {
        char buffer[4096] = { 0 };
        if (read(fd, buffer, (sizeof(buffer) - 1)) > 0) {
            if (!strstr(buffer, "PowerManagerService.Display")) {
                state = ScreenState::SCREEN_OFF;
            }
        }
        close(fd);
    }
    return state;
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
    return buffer;
}

inline std::string GetTaskTombstonePath(int pid)
{
    std::string tombstonePath{};
    struct dirent** entryList = nullptr;
    int len = scandir("/data/tombstones", &entryList, nullptr, alphasort);
    if (entryList != nullptr && len > 0) {
        char tombstoneSymbol[16] = { 0 };
        snprintf(tombstoneSymbol, sizeof(tombstoneSymbol), "pid: %d", pid);
        for (int pos = 0; pos < len; pos++) {
            auto entry = *(entryList + pos);
            auto dirType = entry->d_type;
            auto dirPath = std::string("/data/tombstones/") + entry->d_name;
            free(entry);
            if (dirType == DT_REG) {
                int fd = open(dirPath.c_str(), (O_RDONLY | O_NONBLOCK));
                if (fd >= 0) {
                    char buffer[4096] = { 0 };
                    read(fd, buffer, (sizeof(buffer) - 1));
                    close(fd);
                    if (strstr(buffer, tombstoneSymbol)) {
                        tombstonePath = dirPath;
                        break;
                    }
                }
            }
        }
        free(entryList);
    }
    return tombstonePath;
}


#endif // __ANDROID_API__
#endif // __unix__

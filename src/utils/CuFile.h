// CuFile by chenzyadb@github.com
// Based on C++11 STL (LLVM)

#ifndef _CU_FILE_
#define _CU_FILE_

#if defined(__unix__) && defined(__GNUC__)

#include <string>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#define CU_INLINE __attribute__((always_inline)) inline
#define CU_LIKELY(val) (__builtin_expect(!!(val), 1))
#define CU_UNLIKELY(val) (__builtin_expect(!!(val), 0))
#define CU_COMPARE(val1, val2, size) (__builtin_memcmp(val1, val2, size) == 0)
#define CU_MEMSET(dst, ch, size) __builtin_memset(dst, ch, size)
#define CU_MEMCPY(dst, src, size) __builtin_memcpy(dst, src, size)

namespace CU
{
    class File
    {
        public:
            CU_INLINE File() : path_(), fd_(-1), flag_(0) { }

            CU_INLINE File(const std::string &path) : path_(path), fd_(-1), flag_(0) { }

            CU_INLINE File(const std::string &path, mode_t permMode) : path_(path), fd_(-1), flag_(0)
            {
                flag_ = (O_RDONLY | O_NONBLOCK | O_CLOEXEC);
                fd_ = open(path_.c_str(), flag_);
                if (CU_LIKELY(fd_ >= 0)) {
                    fchmod(fd_, permMode);
                }
            }

            CU_INLINE File(const File &other) : path_(other.path()), fd_(-1), flag_(0) { }

            CU_INLINE File(File &&other) noexcept : path_(other.path()), fd_(-1), flag_(0) { }

            CU_INLINE ~File() {
                if (fd_ >= 0) {
                    close(fd_);
                    fd_ = -1;
                }
            }

            CU_INLINE File &operator=(const File &other)
            {
                if (CU_LIKELY(std::addressof(other) != this)) {
                    if (fd_ >= 0) {
                        close(fd_);
                        fd_ = -1;
                    }
                    path_ = other.path();
                    flag_ = 0;
                }
                return *this;
            }

            CU_INLINE bool operator==(const File &other) const noexcept
            {
                return (other.path() == path_);
            }

		    CU_INLINE bool operator!=(const File &other) const noexcept
            {
                return (other.path() != path_);
            }

            CU_INLINE void writeText(const std::string &content) noexcept
            {
                if (fd_ < 0 || (flag_ & O_WRONLY) == 0) {
                    if (fd_ >= 0) {
                        close(fd_);
                    }
                    flag_ = (O_WRONLY | O_NONBLOCK | O_CLOEXEC);
                    fd_ = open(path_.c_str(), flag_);
                }
                if (CU_LIKELY(fd_ >= 0)) {
                    lseek(fd_, 0, SEEK_SET);
                    write(fd_, content.c_str(), (content.length() + 1));
                }
            }

            CU_INLINE std::string readText()
            {
                std::string content{};
                if (fd_ < 0 || (flag_ & O_RDONLY) == 0) {
                    if (fd_ >= 0) {
                        close(fd_);
                    }
                    flag_ = (O_RDONLY | O_NONBLOCK | O_CLOEXEC);
                    fd_ = open(path_.c_str(), flag_);
                }
                if (CU_LIKELY(fd_ >= 0)) {
                    lseek(fd_, 0, SEEK_SET);
                    char buffer[PAGE_SIZE + 1] = { 0 };
                    while (read(fd_, buffer, (sizeof(buffer) - 1)) > 0) {
                        content += buffer;
                        CU_MEMSET(buffer, 0, sizeof(buffer));
                    } 
                }
                return content;
            }

            CU_INLINE void create() const noexcept
            {
                int fd = open(path_.c_str(), (O_WRONLY | O_CREAT | O_TRUNC), 0644);
                if (CU_LIKELY(fd >= 0)) {
                    close(fd);
                }
            }

            CU_INLINE void setPermMode(mode_t permMode) noexcept
            {
                if (fd_ < 0) {
                    flag_ = (O_RDONLY | O_NONBLOCK | O_CLOEXEC);
                    fd_ = open(path_.c_str(), flag_);
                }
                if (CU_LIKELY(fd_ >= 0)) {
                    fchmod(fd_, permMode);
                }
            }

            CU_INLINE bool exists() const noexcept
            {
                struct stat buffer{};
                return (lstat(path_.c_str(), std::addressof(buffer)) == 0);
            }

            CU_INLINE std::string path() const
            {
                return path_;
            }

        private:
            std::string path_;
            int fd_;
            int flag_;
    };

    CU_INLINE void CreateFile(const std::string &filePath, const std::string &content) noexcept
    {
        int fd = open(filePath.c_str(), (O_WRONLY | O_CREAT | O_TRUNC), 0644);
        if (CU_LIKELY(fd >= 0)) {
            write(fd, content.c_str(), (content.length() + 1));
            close(fd);
        }
    }

    CU_INLINE void AppendFile(const std::string &filePath, const std::string &content) noexcept
    {
        int fd = open(filePath.c_str(), (O_WRONLY | O_APPEND | O_NONBLOCK));
        if (fd < 0) {
            chmod(filePath.c_str(), 0666);
            fd = open(filePath.c_str(), (O_WRONLY | O_APPEND | O_NONBLOCK));
        }
        if (CU_LIKELY(fd >= 0)) {
            write(fd, content.c_str(), (content.length() + 1));
            close(fd);
        }
    }

    CU_INLINE void WriteFile(const std::string &filePath, const std::string &content) noexcept
    {
        int fd = open(filePath.c_str(), (O_WRONLY | O_NONBLOCK));
        if (fd < 0) {
            chmod(filePath.c_str(), 0666);
            fd = open(filePath.c_str(), (O_WRONLY | O_NONBLOCK));
        }
        if (CU_LIKELY(fd >= 0)) {
            write(fd, content.c_str(), (content.length() + 1));
            close(fd);
        }
    }

    CU_INLINE std::string ReadFile(const std::string &filePath) 
    {
        std::string content{};
        int fd = open(filePath.c_str(), (O_RDONLY | O_NONBLOCK));
        if (CU_LIKELY(fd >= 0)) {
            char buffer[PAGE_SIZE] = { 0 };
            while (read(fd, buffer, (sizeof(buffer) - 1)) > 0) {
                content += buffer;
                CU_MEMSET(buffer, 0, sizeof(buffer));
            }
            close(fd);
        }
        return content;
    }

    CU_INLINE bool IsPathExists(const std::string &path) noexcept
    {
        struct stat buffer{};
        return (lstat(path.c_str(), std::addressof(buffer)) == 0);
    }

    CU_INLINE std::vector<std::string> ListPath(const std::string &path, uint8_t d_type = DT_REG)
    {
        std::vector<std::string> paths{};
        struct dirent** entries = nullptr;
        int size = scandir(path.c_str(), &entries, nullptr, alphasort);
        if (CU_LIKELY(entries != nullptr)) {
            for (int offset = 0; offset < size; offset++) {
                auto entry = *(entries + offset);
                if (entry->d_type == d_type) {
                    paths.emplace_back(path + '/' + entry->d_name);
                }
                std::free(entry);
            }
            std::free(entries);
        }
        return paths;
    }

    CU_INLINE std::vector<std::string> ListFile(const std::string &path, uint8_t d_type = DT_REG)
    {
        std::vector<std::string> files{};
        struct dirent** entries = nullptr;
        int size = scandir(path.c_str(), &entries, nullptr, alphasort);
        if (CU_LIKELY(entries != nullptr)) {
            for (int offset = 0; offset < size; offset++) {
                auto entry = *(entries + offset);
                if (entry->d_type == d_type) {
                    files.emplace_back(entry->d_name);
                }
                std::free(entry);
            }
            std::free(entries);
        }
        return files;
    }

    CU_INLINE std::string ExecCommand(const std::string &command)
    {
        std::string content{};
        auto fp = popen(command.c_str(), "r");
        if (CU_LIKELY(fp != nullptr)) {
            char buffer[PAGE_SIZE] = { 0 };
            while (std::fread(buffer, sizeof(char), (sizeof(buffer) - 1), fp) > 0) {
                content += buffer;
                CU_MEMSET(buffer, 0, sizeof(buffer));
            }
            pclose(fp);
        }
        return content;
    }
}

#endif // __unix__ && __GNUC__
#endif // _CU_FILE_

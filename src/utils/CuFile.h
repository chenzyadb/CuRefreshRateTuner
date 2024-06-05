#ifndef _CU_FILE_
#define _CU_FILE_

#ifdef __unix__

#include <string>
#include <unistd.h>
#include <fcntl.h>

namespace CU 
{
    class File 
    {
        public:
            File() : path_(), fd_(-1), flag_(0) { }

            File(const std::string &path) : path_(path), fd_(-1), flag_(0) { }

            File(const File &other) : path_(other.path()), fd_(-1), flag_(0) { }

            File(File &&other) noexcept : path_(other.path()), fd_(-1), flag_(0) { }

            ~File() {
                if (fd_ >= 0) {
                    close(fd_);
                    fd_ = -1;
                }
            }

            File &operator=(const File &other)
            {
                if (std::addressof(other) != this) {
                    if (fd_ >= 0) {
                        close(fd_);
                        fd_ = -1;
                    }
                    path_ = other.path();
                    flag_ = 0;
                }
                return *this;
            }

            bool operator==(const File &other) const noexcept
            {
                return (other.path() == path_);
            }

		    bool operator!=(const File &other) const noexcept
            {
                return (other.path() != path_);
            }

            void writeText(const std::string &content) noexcept
            {
                if (fd_ < 0 || (flag_ & O_WRONLY) == 0) {
                    flag_ = (O_WRONLY | O_NONBLOCK | O_CLOEXEC);
                    if (fd_ >= 0) {
                        close(fd_);
                    }
                    fd_ = open(path_.c_str(), flag_);
                }
                if (fd_ >= 0) {
                    lseek(fd_, 0, SEEK_SET);
                    write(fd_, content.c_str(), (content.length() + 1));
                }
            }

            std::string readText()
            {
                std::string content{};
                if (fd_ < 0 || (flag_ & O_RDONLY) == 0) {
                    flag_ = (O_RDONLY | O_NONBLOCK | O_CLOEXEC);
                    if (fd_ >= 0) {
                        close(fd_);
                    }
                    fd_ = open(path_.c_str(), flag_);
                }
                if (fd_ >= 0) {
                    lseek(fd_, 0, SEEK_SET);
                    char buffer[4096] = { 0 };
                    while (read(fd_, buffer, (sizeof(buffer) - 1)) > 0) {
                        content += buffer;
                        memset(buffer, 0, sizeof(buffer));
                    } 
                }
                return content;
            }

            void create() const noexcept
            {
                int fd = open(path_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd >= 0) {
                    close(fd);
                }
            }

            void setPermMode(mode_t permMode) const noexcept
            {
                chmod(path_.c_str(), permMode);
            }

            bool exists() const noexcept
            {
                return (access(path_.c_str(), F_OK) != -1);
            }

            std::string path() const
            {
                return path_;
            }

            size_t hash() const 
            {
                std::hash<std::string> strHash{};
                return strHash(path_);
            }

        private:
            std::string path_;
            int fd_;
            int flag_;
    };
}

namespace std
{
    template<>
	struct hash<CU::File>
	{
		size_t operator()(const CU::File &val) const noexcept
		{
            return val.hash();
		}
	};
}

#endif // __unix__
#endif // _CU_FILE_

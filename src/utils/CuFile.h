#pragma once

#include <string>
#include <memory>
#include <unistd.h>
#include <fcntl.h>

namespace CU 
{
    class File 
    {
        public:
            static constexpr int flagRead = (O_RDONLY | O_NONBLOCK | O_CLOEXEC);
            static constexpr int flagWrite = (O_WRONLY | O_NONBLOCK | O_CLOEXEC);

            File() : path_(), fd_(-1), flag_(-1) {}

            File(const std::string &path) : path_(path), fd_(-1), flag_(-1) {}

            File(const File &other) : path_(), fd_(-1), flag_(-1)
            {
                if (std::addressof(other) != this) {
                    path_ = other._Get_Path();
                }
            }

            File(File &&other) noexcept : path_(), fd_(-1), flag_(-1)
            {
                if (std::addressof(other) != this) {
                    path_ = other._Get_Path();
                }
            }

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
                    }
                    path_ = other._Get_Path();
                    fd_ = -1;
                    flag_ = -1;
                }
                return *this;
            }

            File &operator()(const File &other)
            {
                if (std::addressof(other) != this) {
                    if (fd_ >= 0) {
                        close(fd_);
                    }
                    path_ = other._Get_Path();
                    fd_ = -1;
                    flag_ = -1;
                }
                return *this;
            }

            bool operator==(const File &other) const
            {
                return (other._Get_Path() == path_);
            }

		    bool operator!=(const File &other) const
            {
                return (other._Get_Path() != path_);
            }

            std::string _Get_Path() const
            {
                return path_;
            }

            void writeText(const std::string &content)
            {
                if (fd_ >= 0 && flag_ != flagWrite) {
                    close(fd_);
                    flag_ = flagWrite;
                    fd_ = open(path_.c_str(), flag_);
                } else if (fd_ < 0) {
                    flag_ = flagWrite;
                    fd_ = open(path_.c_str(), flag_);
                }
                if (fd_ >= 0) {
                    lseek(fd_, 0, SEEK_SET);
                    if (write(fd_, content.data(), content.size()) < 0) {
                        close(fd_);
                        fd_ = -1;
                    } 
                }
            }

            std::string readText()
            {
                if (fd_ >= 0 && flag_ != flagRead) {
                    close(fd_);
                    flag_ = flagRead;
                    fd_ = open(path_.c_str(), flag_);
                } else if (fd_ < 0) {
                    flag_ = flagRead;
                    fd_ = open(path_.c_str(), flag_);
                }
                if (fd_ >= 0) {
                    lseek(fd_, 0, SEEK_SET);
                    char buffer[4096] = { 0 };
                    auto len = read(fd_, buffer, sizeof(buffer));
                    if (len >= 0) {
                        buffer[len] = '\0';
                        std::string content(buffer);
                        return content;
                    } else {
                        close(fd_);
                        fd_ = -1;
                    }
                }
                return {};
            }

            void create() const
            {
                int fd = open(path_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd >= 0) {
                    close(fd);
                }
            }

            bool exists() const
            {
                return (access(path_.c_str(), F_OK) != -1);
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
		size_t operator()(const CU::File &val) const
		{
			return reinterpret_cast<size_t>(std::addressof(val));
		}
	};
}

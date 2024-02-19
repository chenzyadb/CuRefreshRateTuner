#pragma once

#include <mutex>

namespace CU
{
    template <typename _Ty>
    class SafeVal 
    {   
        public:
            SafeVal() : val_(), mtx_() { }

            SafeVal(const _Ty &val) : val_(val), mtx_() { }

            SafeVal(_Ty &&val) noexcept : val_(val), mtx_() { }

            SafeVal(const SafeVal &other) : val_(), mtx_() 
            {
                if (std::addressof(other) != this) {
                    val_ = other.data();
                }
            }

            SafeVal(SafeVal &&other) noexcept : val_(), mtx_() 
            {
                if (std::addressof(other) != this) {
                    val_ = other.data();
                }
            }

            SafeVal &operator=(const SafeVal &other)
            {
                std::unique_lock<std::mutex> lock(mtx_);
                if (std::addressof(other) != this) {
                    val_ = other.data();
                }
                return *this;
            }

            _Ty operator=(const _Ty &val)
            {
                std::unique_lock<std::mutex> lock(mtx_);
                val_ = val;
                return val_;
            }

            bool operator==(const SafeVal &other) const
            {
                std::unique_lock<std::mutex> lock(mtx_);
                return (val_ == other.data());
            }

            bool operator!=(const SafeVal &other) const
            {
                std::unique_lock<std::mutex> lock(mtx_);
                return (val_ != other.data());
            }

            bool operator==(const _Ty &other) const
            {
                std::unique_lock<std::mutex> lock(mtx_);
                return (val_ == other);
            }

            bool operator!=(const _Ty &other) const
            {
                std::unique_lock<std::mutex> lock(mtx_);
                return (val_ != other);
            }

            _Ty data() const
            {
                std::unique_lock<std::mutex> lock(mtx_);
                return val_;
            }

            void clear() 
            {
                std::unique_lock<std::mutex> lock(mtx_);
                val_ = _Ty();
            }

        private:
            _Ty val_;
            mutable std::mutex mtx_;
    };
}

// CuEventTransfer V1 by chenzyadb.
// Based on C++14 STL (MSVC).

#ifndef _CU_EVENT_TRANSFER_
#define _CU_EVENT_TRANSFER_

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <memory>

namespace CU
{
	class EventTransfer
	{
		public:
			typedef const void* TransData;
			typedef std::function<void(TransData)> Subscriber;

			template <typename _Ty>
			static const _Ty &GetData(const TransData &transData)
			{
				auto data_ptr = reinterpret_cast<const _Ty*>(transData);
				return *data_ptr;
			}

			template <typename _Ty>
			static void Post(const std::string &event, const _Ty &data)
			{
				auto transData = reinterpret_cast<TransData>(std::addressof(data));
				auto instance = getInstance_();
				instance->postEvent_(event, transData);
			}

			static void Subscribe(const std::string &event, const Subscriber &subscriber) 
			{
				auto instance = getInstance_();
				instance->addSubscriber_(event, subscriber);
			}

		private:
			EventTransfer() : eventSubMap_(), mtx_() { }
			EventTransfer(const EventTransfer &other) = delete;
			EventTransfer(EventTransfer &&other) = delete;
			EventTransfer &operator=(const EventTransfer &other) = delete;

			static EventTransfer* getInstance_()
			{
				static EventTransfer* instance = nullptr;
				if (instance == nullptr) {
					instance = new EventTransfer();
				}
				return instance;
			}

			void postEvent_(const std::string &event, const TransData &transData)
			{
				std::vector<Subscriber> subscribers{};
				{
					std::unique_lock<std::mutex> lck(mtx_);
					subscribers = eventSubMap_[event];
				}
				for (const auto &subscriber : subscribers) {
					subscriber(transData);
				}
			}

			void addSubscriber_(const std::string &event, const Subscriber &subscriber)
			{
				std::unique_lock<std::mutex> lck(mtx_);
				eventSubMap_[event].emplace_back(subscriber);
			}

			std::unordered_map<std::string, std::vector<Subscriber>> eventSubMap_;
			std::mutex mtx_;
	};
}

#endif // ifndef _CU_EVENT_TRANSFER_

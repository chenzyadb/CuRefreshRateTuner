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
			typedef void* Instance;
			typedef std::function<void(TransData)> Subscriber;

			template <typename _Ty>
			static const _Ty &GetData(const TransData &transData)
			{
				auto dataPtr = reinterpret_cast<const _Ty*>(transData);
				return *dataPtr;
			}

			template <typename _Ty>
			static void Post(const std::string &event, const _Ty &data)
			{
				auto transData = reinterpret_cast<TransData>(std::addressof(data));
				GetInstance_()->postEvent_(event, transData);
			}

			static void Subscribe(const std::string &event, const Instance &instance, const Subscriber &subscriber)
			{
				GetInstance_()->addSubscriber_(event, instance, subscriber);
			}

			static void Unsubscribe(const std::string &event, const Instance &instance)
			{
				GetInstance_()->removeSubscriber_(event, instance);
			}

		private:
			EventTransfer() : eventSubscribers_(), mtx_() { }
			EventTransfer(const EventTransfer &other) = delete;
			EventTransfer(EventTransfer &&other) = delete;
			EventTransfer &operator=(const EventTransfer &other) = delete;

			static EventTransfer* GetInstance_()
			{
				static EventTransfer* instance = nullptr;
				if (instance == nullptr) {
					instance = new EventTransfer();
				}
				return instance;
			}

			void postEvent_(const std::string &event, const TransData &transData)
			{
				std::unordered_map<Instance, Subscriber> subscribers{};
				{
					std::unique_lock<std::mutex> lck(mtx_);
					auto subscribersIter = eventSubscribers_.find(event);
					if (subscribersIter == eventSubscribers_.end()) {
						return;
					}
					subscribers = subscribersIter->second;
				}
				for (auto iter = subscribers.begin(); iter != subscribers.end(); iter++) {
					(iter->second)(transData);
				}
			}

			void addSubscriber_(const std::string &event, const Instance &instance, const Subscriber &subscriber)
			{
				std::unique_lock<std::mutex> lck(mtx_);
				eventSubscribers_[event][instance] = subscriber;
			}

			void removeSubscriber_(const std::string &event, const Instance &instance)
			{
				std::unique_lock<std::mutex> lck(mtx_);
				auto subscribersIter = eventSubscribers_.find(event);
				if (subscribersIter == eventSubscribers_.end()) {
					return;
				}
				auto &subscribers = subscribersIter->second;
				auto removeIter = subscribers.find(instance);
				if (removeIter != subscribers.end()) {
					subscribers.erase(removeIter);
				}
			}

			std::unordered_map<std::string, std::unordered_map<Instance, Subscriber>> eventSubscribers_;
			std::mutex mtx_;
	};
}

#endif // ifndef _CU_EVENT_TRANSFER_

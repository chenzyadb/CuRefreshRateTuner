// CuJSONObject by chenzyadb@github.com
// Based on C++17 STL (MSVC)

#if !defined(_CU_JSONOBJECT_)
#define _CU_JSONOBJECT_ 1

#include <unordered_map>
#include <vector>
#include <string>
#include <variant>
#include <memory>
#include <exception>
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include <cstring>

namespace CU
{
	class JSONExcept : public std::exception
	{
		public:
			JSONExcept(const std::string &message) : message_(message) { }

			const char* what() const noexcept override
			{
				return message_.c_str();
			}

		private:
			const std::string message_;
	};

	struct _JSON_String
	{
		char stack_buffer[128];
		char* heap_data;
		size_t length;
		size_t capacity;

		_JSON_String() noexcept : stack_buffer(), heap_data(nullptr), length(0), capacity(sizeof(stack_buffer)) { }

		_JSON_String(const _JSON_String &other) noexcept :
			stack_buffer(),
			heap_data(nullptr),
			length(0),
			capacity(sizeof(stack_buffer))
		{
			append(other);
		}

		_JSON_String(_JSON_String &&other) noexcept :
			stack_buffer(),
			heap_data(other.heap_data),
			length(other.length),
			capacity(other.capacity)
		{
			std::memcpy(stack_buffer, other.stack_buffer, other.length);
			other.heap_data = nullptr;
			other.length = 0;
			other.capacity = sizeof(stack_buffer);
		}

		_JSON_String(const char* src) noexcept :
			stack_buffer(),
			heap_data(nullptr),
			length(0),
			capacity(sizeof(stack_buffer))
		{
			append(src);
		}

		~_JSON_String() noexcept
		{
			if (heap_data != nullptr) {
				delete[] heap_data;
			}
		}

		void append(const _JSON_String &other) noexcept
		{
			auto new_len = length + other.length;
			if ((new_len + 1) > capacity) {
				expand(capacity + new_len);
			}
			std::memcpy((data() + length), other.data(), other.length);
			*(data() + new_len) = '\0';
			length = new_len;
		}

		void append(const char* src) noexcept
		{
			auto src_len = std::strlen(src);
			auto new_len = length + src_len;
			if ((new_len + 1) > capacity) {
				expand(capacity + new_len);
			}
			std::memcpy((data() + length), src, src_len);
			*(data() + new_len) = '\0';
			length = new_len;
		}

		void append(char ch) noexcept
		{
			if ((length + 1) >= capacity) {
				expand(capacity * 2);
			}
			*(data() + length) = ch;
			*(data() + length + 1) = '\0';
			length++;
		}

		bool equals(const _JSON_String &text) const noexcept
		{
			if (text.length != length) {
				return false;
			}
			return (std::memcmp(data(), text.data(), length) == 0);
		}

		bool equals(const char* text) const noexcept
		{
			if (std::strlen(text) != length) {
				return false;
			}
			return (std::memcmp(data(), text, length) == 0);
		}

		void expand(size_t req_capacity) noexcept
		{
			if (req_capacity < capacity) {
				return;
			}
			if (heap_data == nullptr && req_capacity > sizeof(stack_buffer)) {
				heap_data = new char[req_capacity];
				std::memcpy(heap_data, stack_buffer, length);
				*(heap_data + length) = '\0';
			} else if (heap_data != nullptr) {
				auto new_heap_data = new char[req_capacity];
				std::memcpy(new_heap_data, heap_data, length);
				*(new_heap_data + length) = '\0';
				delete[] heap_data;
				heap_data = new_heap_data;
			}
			capacity = req_capacity;
		}

		void shrink(size_t req_length) noexcept
		{
			if (req_length < length) {
				*(data() + req_length) = '\0';
				length = req_length;
			}
		}

		char* data() noexcept
		{
			if (heap_data != nullptr) {
				return heap_data;
			}
			return stack_buffer;
		}

		const char* data() const noexcept
		{
			if (heap_data != nullptr) {
				return heap_data;
			}
			return stack_buffer;
		}

		void clear() noexcept
		{
			if (heap_data != nullptr) {
				delete[] heap_data;
				heap_data = nullptr;
			}
			stack_buffer[0] = '\0';
			length = 0;
			capacity = sizeof(stack_buffer);
		}
	};

	class JSONObject;
	class JSONArray;
	class JSONItem;

	namespace _JSON_Misc
	{
		inline char GetEscapeChar(const char &ch) noexcept
		{
			switch (ch) {
				case 'n':
					return '\n';
				case 'r':
					return '\r';
				case 't':
					return '\t';
				case 'b':
					return '\b';
				case 'f':
					return '\f';
				case 'a':
					return '\a';
				case 'v':
					return '\v';
				default:
					break;
			}
			return ch;
		}

		inline _JSON_String StringToJSONRaw(const std::string &str)
		{
			_JSON_String raw("\"");
			for (const auto &ch : str) {
				switch (ch) {
					case '\\':
						raw.append("\\\\");
						break;
					case '\"':
						raw.append("\\\"");
						break;
					case '\n':
						raw.append("\\n");
						break;
					case '\t':
						raw.append("\\t");
						break;
					case '\r':
						raw.append("\\r");
						break;
					case '\f':
						raw.append("\\f");
						break;
					case '\a':
						raw.append("\\a");
						break;
					case '\b':
						raw.append("\\b");
						break;
					case '\v':
						raw.append("\\v");
						break;
					case '/':
						raw.append("\\/");
						break;
					case '#':
						raw.append("\\#");
						break;
					default:
						raw.append(ch);
						break;
				}
			}
			raw.append('\"');
			return raw;
		}

		constexpr size_t npos = static_cast<size_t>(-1);

		inline size_t FindChar(const char* str, char ch, size_t start_pos = 0) noexcept
		{
			for (auto pos = start_pos; *(str + pos) != '\0'; pos++) {
				if (*(str + pos) == ch) {
					return pos;
				}
			}
			return npos;
		}
	}

	class JSONItem
	{
		public:
			enum class ItemType : uint8_t {ITEM_NULL, BOOLEAN, INTEGER, LONG, DOUBLE, STRING, ARRAY, OBJECT};

			typedef decltype(nullptr) ItemNull;
			typedef std::variant<ItemNull, bool, int, int64_t, double, std::string, JSONArray*, JSONObject*> ItemValue;

			JSONItem();
			JSONItem(const _JSON_String &raw);
			JSONItem(ItemNull _null);
			JSONItem(bool value);
			JSONItem(int value);
			JSONItem(int64_t value);
			JSONItem(double value);
			JSONItem(const char* value);
			JSONItem(const std::string &value);
			JSONItem(const JSONArray &value);
			JSONItem(const JSONObject &value);
			JSONItem(const JSONItem &other);
			JSONItem(JSONItem &&other) noexcept;
			~JSONItem();
			
			JSONItem &operator=(const JSONItem &other);
			JSONItem &operator=(JSONItem &&other) noexcept;
			bool operator==(const JSONItem &other) const;
			bool operator!=(const JSONItem &other) const;

			ItemType type() const;
			ItemValue value() const;
			ItemValue &&value_rv();
			void clear();
			size_t size() const;
			size_t hash() const;

			bool isNull() const;
			bool isBoolean() const;
			bool isInt() const;
			bool isLong() const;
			bool isDouble() const;
			bool isString() const;
			bool isArray() const;
			bool isObject() const;

			bool toBoolean() const;
			int toInt() const;
			int64_t toLong() const;
			double toDouble() const;
			std::string toString() const;
			JSONArray toArray() const;
			JSONObject toObject() const;
			std::string toRaw() const;
			
		private:
			ItemType type_;
			ItemValue value_;
	};

	class JSONArray
	{
		public:
			typedef std::vector<JSONItem>::iterator iterator;
			typedef std::vector<JSONItem>::const_iterator const_iterator;

			JSONArray();
			JSONArray(size_t init_size);
			JSONArray(size_t init_size, const JSONItem &init_value);
			JSONArray(iterator begin_iter, iterator end_iter);
			JSONArray(const std::string &jsonText);
			JSONArray(const char* jsonText);
			JSONArray(const std::vector<JSONItem> &data);
			JSONArray(const std::vector<bool> &list);
			JSONArray(const std::vector<int> &list);
			JSONArray(const std::vector<int64_t> &list);
			JSONArray(const std::vector<double> &list);
			JSONArray(const std::vector<std::string> &list);
			JSONArray(const std::vector<JSONArray> &list);
			JSONArray(const std::vector<JSONObject> &list);
			JSONArray(const JSONArray &other);
			JSONArray(JSONArray &&other) noexcept;
			~JSONArray();
			
			JSONArray &operator()(const JSONArray &other);
			JSONArray &operator=(const JSONArray &other);
			JSONArray &operator+=(const JSONArray &other);
			JSONItem &operator[](size_t pos);
			JSONArray operator+(const JSONArray &other) const;
			bool operator==(const JSONArray &other) const;
			bool operator!=(const JSONArray &other) const;

			std::vector<bool> toListBoolean() const;
			std::vector<int> toListInt() const;
			std::vector<int64_t> toListLong() const;
			std::vector<double> toListDouble() const;
			std::vector<std::string> toListString() const;
			std::vector<JSONArray> toListArray() const;
			std::vector<JSONObject> toListObject() const;

			JSONItem at(size_t pos) const;
			iterator find(const JSONItem &item);
			void add(const JSONItem &item);
			void remove(const JSONItem &item);
			void resize(size_t new_size);
			void clear();
			size_t size() const;
			size_t hash() const;
			bool empty() const;
			const std::vector<JSONItem> &data() const;
			std::vector<JSONItem> &&data_rv();
			std::string toString() const;

			JSONItem &front();
			JSONItem &back();
			iterator begin();
			iterator end();
			const_iterator begin() const;
			const_iterator end() const;
			
		private:
			std::vector<JSONItem> data_;

			void Parse_Impl_(const char* json_text);
	};

	class JSONObject
	{
		public:
			JSONObject();
			JSONObject(const std::string &jsonText, bool enableComments = false);
			JSONObject(const char* jsonText, bool enableComments = false);
			JSONObject(const JSONObject &other);
			JSONObject(JSONObject &&other) noexcept;
			JSONObject(std::unordered_map<std::string, JSONItem> &&data, std::vector<std::string> &&order) noexcept;
			~JSONObject();

			JSONObject &operator=(const JSONObject &other);
			JSONObject &operator+=(const JSONObject &other);
			JSONItem &operator[](const std::string &key);
			JSONObject operator+(const JSONObject &other) const;
			bool operator==(const JSONObject &other) const;
			bool operator!=(const JSONObject &other) const;
			
			bool contains(const std::string &key) const;
			JSONItem at(const std::string &key) const;
			void add(const std::string &key, const JSONItem &value);
			void remove(const std::string &key);
			void clear();
			size_t size() const;
			size_t hash() const;
			bool empty() const;
			const std::unordered_map<std::string, JSONItem> &data() const;
			std::unordered_map<std::string, JSONItem> &&data_rv();
			const std::vector<std::string> &order() const;
			std::vector<std::string> &&order_rv();
			std::string toString() const;
			std::string toFormatedString() const;

			struct JSONPair
			{
				std::string key;
				JSONItem value;
			};
			std::vector<JSONPair> toPairs() const;

		private:
			std::unordered_map<std::string, JSONItem> data_;
			std::vector<std::string> order_;

			void Parse_Impl_(const char* json_text, bool enable_comments);
	};
}

namespace std
{
	template <>
	struct hash<CU::JSONItem>
	{
		size_t operator()(const CU::JSONItem &val) const
		{
			return val.hash();
		}
	};

	template <>
	struct hash<CU::JSONArray>
	{
		size_t operator()(const CU::JSONArray &val) const
		{
			return val.hash();
		}
	};

	template <>
	struct hash<CU::JSONObject>
	{
		size_t operator()(const CU::JSONObject &val) const
		{
			return val.hash();
		}
	};
}

#endif // !defined(_CU_JSONOBJECT_)

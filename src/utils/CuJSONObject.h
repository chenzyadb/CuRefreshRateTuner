// CuJSONObject V1 by chenzyadb@github.com
// Based on C++17 STL (MSVC).

#ifndef _CU_JSONOBJECT_
#define _CU_JSONOBJECT_

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

namespace CU
{
	inline char _GetEscapeChar(const char &ch) noexcept
	{
		switch (ch) {
			case '\\':
				return '\\';
			case '\"':
				return '\"';
			case '\'':
				return '\'';
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
			case '/':
				return '/';
			default:
				break;
		}
		return ch;
	}

	inline std::string _StringToJSONRaw(const std::string &str) 
	{
		std::string JSONRaw("\"");
		for (const auto &ch : str) {
			switch (ch) {
				case '\\':
					JSONRaw += "\\\\";
					break;
				case '\"':
					JSONRaw += "\\\"";
					break;
				case '\'':
					JSONRaw += "\\\'";
					break;
				case '\n':
					JSONRaw += "\\n";
					break;
				case '\t':
					JSONRaw += "\\t";
					break;
				case '\r':
					JSONRaw += "\\r";
					break;
				case '\f':
					JSONRaw += "\\f";
					break;
				case '\a':
					JSONRaw += "\\a";
					break;
				case '\b':
					JSONRaw += "\\b";
					break;
				case '\v':
					JSONRaw += "\\v";
					break;
				case '/':
					JSONRaw += "\\/";
					break;
				default:
					JSONRaw += ch;
					break;
			}
		}
		JSONRaw += '\"';
		return JSONRaw;
	}

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

	class JSONObject;
	class JSONArray;

	enum class ItemType : uint8_t {ITEM_NULL, BOOLEAN, INTEGER, LONG, DOUBLE, STRING, ARRAY, OBJECT};

	typedef char ItemNull;
	typedef std::variant<ItemNull, bool, int, int64_t, double, std::string, JSONArray*, JSONObject*> ItemValue;

	class JSONItem
	{
		public:
			struct _Init_Val
			{
				ItemType type;
				ItemValue value;
			};
			static _Init_Val _To_Init_Val(const std::string &JSONRaw);

			JSONItem();
			JSONItem(const bool &value);
			JSONItem(const int &value);
			JSONItem(const int64_t &value);
			JSONItem(const double &value);
			JSONItem(const char* value);
			JSONItem(const std::string &value);
			JSONItem(const JSONArray &value);
			JSONItem(const JSONObject &value);
			JSONItem(const JSONItem &other);
			JSONItem(JSONItem &&other) noexcept;
			JSONItem(_Init_Val &&initVal) noexcept;
			~JSONItem();
			
			JSONItem &operator()(const JSONItem &other);
			JSONItem &operator=(const JSONItem &other);
			bool operator==(const JSONItem &other) const;
			bool operator!=(const JSONItem &other) const;

			ItemType type() const;
			ItemValue value() const;
			void clear();
			size_t size() const;

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
			typedef std::vector<JSONItem>::iterator Iterator;
			typedef std::vector<JSONItem>::const_iterator ConstIterator;

			JSONArray();
			JSONArray(const size_t &init_size);
			JSONArray(const size_t &init_size, const JSONItem &init_value);
			JSONArray(Iterator begin_iter, Iterator end_iter);
			JSONArray(const std::string &JSONString);
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
			JSONItem &operator[](const size_t &pos);
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

			JSONItem at(const size_t &pos) const;
			Iterator find(const JSONItem &item);
			void add(const JSONItem &item);
			void remove(const JSONItem &item);
			void resize(const size_t &new_size);
			void clear();
			size_t size() const;
			bool empty() const;
			std::vector<JSONItem> data() const;
			std::string toString() const;

			JSONItem &front();
			JSONItem &back();
			Iterator begin();
			Iterator end();
			ConstIterator begin() const;
			ConstIterator end() const;
			
		private:
			std::vector<JSONItem> data_;
	};

	class JSONObject
	{
		public:
			JSONObject();
			JSONObject(const std::string &JSONString);
			JSONObject(const std::unordered_map<std::string, JSONItem> &data, const std::vector<std::string> &order);
			JSONObject(const JSONObject &other);
			JSONObject(JSONObject &&other) noexcept;
			~JSONObject();

			JSONObject &operator()(const JSONObject &other);
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
			bool empty() const;
			std::unordered_map<std::string, JSONItem> data() const;
			std::vector<std::string> order() const;
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
	};
}

namespace std
{
	template <>
	struct hash<CU::JSONItem>
	{
		size_t operator()(const CU::JSONItem &val) const
		{
			return reinterpret_cast<size_t>(std::addressof(val));
		}
	};

	template <>
	struct hash<CU::JSONArray>
	{
		size_t operator()(const CU::JSONArray &val) const
		{
			return reinterpret_cast<size_t>(std::addressof(val));
		}
	};

	template <>
	struct hash<CU::JSONObject>
	{
		size_t operator()(const CU::JSONObject &val) const
		{
			return reinterpret_cast<size_t>(std::addressof(val));
		}
	};
}

#endif // _CU_JSONOBJECT_

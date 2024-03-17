#include "CuJSONObject.h"

CU::JSONItem::_Init_Val CU::JSONItem::_To_Init_Val(const std::string &JSONRaw)
{
	static const auto createInitVal = [](const ItemType &type, const ItemValue &value) -> _Init_Val {
		_Init_Val initVal{};
		initVal.type = type;
		initVal.value = value;
		return initVal;
	};

	switch (JSONRaw.front()) {
		case '{':
			if (JSONRaw.back() == '}') {
				return createInitVal(ItemType::OBJECT, new JSONObject(JSONRaw));
			}
			break;
		case '[':
			if (JSONRaw.back() == ']') {
				return createInitVal(ItemType::ARRAY, new JSONArray(JSONRaw));
			}
			break;
		case '\"':
			if (JSONRaw.back() == '\"') {
				return createInitVal(ItemType::STRING, JSONRaw.substr(1, JSONRaw.size() - 2));
			}
			break;
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				auto num = atof(JSONRaw.c_str());
				if (num != 0) {
					if (num == static_cast<int64_t>(num)) {
						if (num > static_cast<double>(INT_MAX) || num < static_cast<double>(INT_MIN)) {
							return createInitVal(ItemType::LONG, static_cast<int64_t>(num));
						} else {
							return createInitVal(ItemType::INTEGER, static_cast<int>(num));
						}
					} else {
						return createInitVal(ItemType::DOUBLE, num);
					}
				} else if (JSONRaw == "0") {
					return createInitVal(ItemType::INTEGER, 0);
				} else if (JSONRaw == "0.0") {
					return createInitVal(ItemType::DOUBLE, 0.0);
				}
			}
			break;
		case 't':
		case 'f':
			if (JSONRaw == "true") {
				return createInitVal(ItemType::BOOLEAN, true);
			} else if (JSONRaw == "false") {
				return createInitVal(ItemType::BOOLEAN, false);
			}
			break;
		case 'n':
			if (JSONRaw == "null") {
				return createInitVal(ItemType::ITEM_NULL, ItemNull());
			}
			break;
		default:
			break;
	}
	throw JSONExcept("Invalid JSONItem");
}

CU::JSONItem::JSONItem() : 
	type_(ItemType::ITEM_NULL), 
	value_(ItemNull()) 
{ }

CU::JSONItem::JSONItem(const bool &value) :
	type_(ItemType::BOOLEAN),
	value_(value) 
{ }

CU::JSONItem::JSONItem(const int &value) :
	type_(ItemType::INTEGER),
	value_(value) 
{ }

CU::JSONItem::JSONItem(const int64_t &value) :
	type_(ItemType::LONG),
	value_(value)
{ }

CU::JSONItem::JSONItem(const double &value) : 
	type_(ItemType::DOUBLE),
	value_(value)
{ }

CU::JSONItem::JSONItem(const char* value) :
	type_(ItemType::STRING),
	value_(std::string(value))
{ }

CU::JSONItem::JSONItem(const std::string &value) :
	type_(ItemType::STRING),
	value_(value)
{ }

CU::JSONItem::JSONItem(const JSONArray &value) :
	type_(ItemType::ARRAY),
	value_(new JSONArray(value))
{ }

CU::JSONItem::JSONItem(const JSONObject &value) :
	type_(ItemType::OBJECT),
	value_(new JSONObject(value))
{ }

CU::JSONItem::JSONItem(const JSONItem &other) :
	type_(other.type()),
	value_(ItemNull())
{
	if (std::addressof(other) == this) {
		return;
	}
	const auto &other_value = other.value();
	if (type_ == ItemType::ARRAY) {
		const auto &jsonArray = *(std::get<JSONArray*>(other_value));
		value_ = new JSONArray(jsonArray);
	} else if (type_ == ItemType::OBJECT) {
		const auto &jsonObject = *(std::get<JSONObject*>(other_value));
		value_ = new JSONObject(jsonObject);
	} else {
		value_ = other_value;
	}
}

CU::JSONItem::JSONItem(JSONItem &&other) noexcept :
	type_(other.type()),
	value_(ItemNull())
{
	if (std::addressof(other) == this) {
		return;
	}
	const auto &other_value = other.value();
	if (type_ == ItemType::ARRAY) {
		const auto &jsonArray = *(std::get<JSONArray*>(other_value));
		value_ = new JSONArray(jsonArray);
	} else if (type_ == ItemType::OBJECT) {
		const auto &jsonObject = *(std::get<JSONObject*>(other_value));
		value_ = new JSONObject(jsonObject);
	} else {
		value_ = other_value;
	}
}

CU::JSONItem::JSONItem(_Init_Val &&initVal) noexcept :
	type_(initVal.type),
	value_(initVal.value)
{ }

CU::JSONItem::~JSONItem()
{
	clear();
}

CU::JSONItem &CU::JSONItem::operator()(const JSONItem &other)
{
	if (std::addressof(other) != this) {
		clear();
		type_ = other.type();
		const auto &other_value = other.value();
		if (type_ == ItemType::ARRAY) {
			const auto &jsonArray = *(std::get<JSONArray*>(other_value));
			value_ = new JSONArray(jsonArray);
		} else if (type_ == ItemType::OBJECT) {
			const auto &jsonObject = *(std::get<JSONObject*>(other_value));
			value_ = new JSONObject(jsonObject);
		} else {
			value_ = other_value;
		}
	}
	return *this;
}

CU::JSONItem &CU::JSONItem::operator=(const JSONItem &other)
{
	if (std::addressof(other) != this) {
		clear();
		type_ = other.type();
		const auto &other_value = other.value();
		if (type_ == ItemType::ARRAY) {
			const auto &jsonArray = *(std::get<JSONArray*>(other_value));
			value_ = new JSONArray(jsonArray);
		} else if (type_ == ItemType::OBJECT) {
			const auto &jsonObject = *(std::get<JSONObject*>(other_value));
			value_ = new JSONObject(jsonObject);
		} else {
			value_ = other_value;
		}
	}
	return *this;
}

bool CU::JSONItem::operator==(const JSONItem &other) const
{
	return (value_ == other.value());
}

bool CU::JSONItem::operator!=(const JSONItem &other) const
{
	return (value_ != other.value());
}

CU::ItemType CU::JSONItem::type() const
{
	return type_;
}

CU::ItemValue CU::JSONItem::value() const
{
	return value_;
}

void CU::JSONItem::clear()
{
	if (type_ == ItemType::ARRAY) {
		delete std::get<JSONArray*>(value_);
	} else if (type_ == ItemType::OBJECT) {
		delete std::get<JSONObject*>(value_);
	}
	type_ = ItemType::ITEM_NULL;
	value_ = ItemNull();
}

size_t CU::JSONItem::size() const
{
	switch (type_) {
		case ItemType::STRING:
			return std::get<std::string>(value_).size();
		case ItemType::ARRAY:
			return std::get<JSONArray*>(value_)->size();
		case ItemType::OBJECT:
			return std::get<JSONObject*>(value_)->size();
		default:
			break;
	}
	return 1;
}

bool CU::JSONItem::toBoolean() const
{
	if (type_ == ItemType::BOOLEAN) {
		return std::get<bool>(value_);
	}
	return {};
}

int CU::JSONItem::toInt() const
{
	if (type_ == ItemType::INTEGER) {
		return std::get<int>(value_);
	}
	return {};
}

int64_t CU::JSONItem::toLong() const
{
	if (type_ == ItemType::LONG) {
		return std::get<int64_t>(value_);
	}
	return {};
}

double CU::JSONItem::toDouble() const
{
	if (type_ == ItemType::DOUBLE) {
		return std::get<double>(value_);
	}
	return {};
}

std::string CU::JSONItem::toString() const
{
	if (type_ == ItemType::STRING) {
		return std::get<std::string>(value_);
	}
	return {};
}

CU::JSONArray CU::JSONItem::toArray() const
{
	if (type_ == ItemType::ARRAY) {
		return *(std::get<JSONArray*>(value_));
	}
	return {};
}

CU::JSONObject CU::JSONItem::toObject() const
{
	if (type_ == ItemType::OBJECT) {
		return *(std::get<JSONObject*>(value_));
	}
	return {};
}

std::string CU::JSONItem::toRaw() const
{
	std::string JSONRaw{};
	switch (type_) {
		case ItemType::ITEM_NULL:
			JSONRaw = "null";
			break;
		case ItemType::BOOLEAN:
			if (std::get<bool>(value_)) {
				JSONRaw = "true";
			} else {
				JSONRaw = "false";
			}
			break;
		case ItemType::INTEGER:
			JSONRaw = std::to_string(std::get<int>(value_));
			break;
		case ItemType::LONG:
			JSONRaw = std::to_string(std::get<int64_t>(value_));
			break;
		case ItemType::DOUBLE:
			JSONRaw = std::to_string(std::get<double>(value_));
			break;
		case ItemType::STRING:
			JSONRaw = _StringToJSONRaw(std::get<std::string>(value_));
			break;
		case ItemType::ARRAY:
			JSONRaw = std::get<JSONArray*>(value_)->toString();
			break;
		case ItemType::OBJECT:
			JSONRaw = std::get<JSONObject*>(value_)->toString();
			break;
	}
	return JSONRaw;
}

CU::JSONArray::JSONArray() : data_() { }

CU::JSONArray::JSONArray(const size_t &init_size) : data_(init_size) { }

CU::JSONArray::JSONArray(const size_t &init_size, const JSONItem &init_value) : data_(init_size, init_value) { }

CU::JSONArray::JSONArray(Iterator begin_iter, Iterator end_iter) : data_(begin_iter, end_iter) { }

CU::JSONArray::JSONArray(const std::string &JSONString) : data_()
{
	enum class ArrayIdx : uint8_t {NONE, ITEM_FRONT, ITEM_COMMON, ITEM_STRING, ITEM_ARRAY, ITEM_OBJECT, ITEM_BACK};
	size_t pos = 0;
	auto idx = ArrayIdx::NONE;
	uint32_t count = 0;
	std::string content{};
	while (pos < JSONString.size()) {
		const char &ch = JSONString.at(pos);
		switch (ch) {
			case '[':
				if (idx == ArrayIdx::NONE) {
					idx = ArrayIdx::ITEM_FRONT;
				} else if (idx == ArrayIdx::ITEM_FRONT) {
					idx = ArrayIdx::ITEM_ARRAY;
					count = 1;
					content += ch;
				} else if (idx == ArrayIdx::ITEM_ARRAY) {
					count++;
					content += ch;
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_OBJECT) {
					content += ch;
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case ']':
				if (idx == ArrayIdx::ITEM_BACK || idx == ArrayIdx::ITEM_COMMON) {
					idx = ArrayIdx::NONE;
				} else if (idx == ArrayIdx::ITEM_FRONT && data_.empty()) {
					idx = ArrayIdx::NONE;
				} else if (idx == ArrayIdx::ITEM_ARRAY) {
					if (count > 0) {
						count--;
						content += ch;
						if (count == 0) {
							idx = ArrayIdx::ITEM_BACK;
						}
					} else {
						throw JSONExcept("Invalid JSONArray Structure");
					}
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_OBJECT) {
					content += ch;
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case ',':
				if (idx == ArrayIdx::ITEM_BACK || idx == ArrayIdx::ITEM_COMMON) {
					idx = ArrayIdx::ITEM_FRONT;
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_ARRAY || idx == ArrayIdx::ITEM_OBJECT) {
					content += ch;
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case '{':
				if (idx == ArrayIdx::ITEM_FRONT) {
					idx = ArrayIdx::ITEM_OBJECT;
					count = 1;
					content += ch;
				} else if (idx == ArrayIdx::ITEM_OBJECT) {
					count++;
					content += ch;
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_ARRAY) {
					content += ch;
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case '}':
				if (idx == ArrayIdx::ITEM_OBJECT) {
					if (count > 0) {
						count--;
						content += ch;
						if (count == 0) {
							idx = ArrayIdx::ITEM_BACK;
						}
					} else {
						throw JSONExcept("Invalid JSONArray Structure");
					}
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_ARRAY) {
					content += ch;
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case '\"':
				if (idx == ArrayIdx::ITEM_FRONT) {
					idx = ArrayIdx::ITEM_STRING;
					content += ch;
				} else if (idx == ArrayIdx::ITEM_STRING) {
					idx = ArrayIdx::ITEM_BACK;
					content += ch;
				} else if (idx == ArrayIdx::ITEM_ARRAY || idx == ArrayIdx::ITEM_OBJECT) {
					content += ch;
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case ' ':
				if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_ARRAY || idx == ArrayIdx::ITEM_OBJECT) {
					content += ch;
				}
				break;
			case '\n':
			case '\t':
			case '\r':
			case '\f':
			case '\a':
			case '\b':
			case '\v':
				break;
			case '\\':
				if (idx == ArrayIdx::ITEM_STRING) {
					pos++;
					content += _GetEscapeChar(JSONString.at(pos));
				} else if (idx == ArrayIdx::ITEM_ARRAY || idx == ArrayIdx::ITEM_OBJECT) {
					content += ch;
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			default:
				if (idx == ArrayIdx::ITEM_FRONT) {
					idx = ArrayIdx::ITEM_COMMON;
					content += ch;
				} else if (idx != ArrayIdx::NONE && idx != ArrayIdx::ITEM_BACK) {
					content += ch;
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
		}
		if ((idx == ArrayIdx::ITEM_FRONT || idx == ArrayIdx::NONE) && !content.empty()) {
			data_.emplace_back(JSONItem::_To_Init_Val(content));
			content.clear();
		}
		pos++;
	}
	if (idx != ArrayIdx::NONE) {
		throw JSONExcept("Invalid JSONArray Structure");
	}
}

CU::JSONArray::JSONArray(const std::vector<JSONItem> &data) : data_(data) { }

CU::JSONArray::JSONArray(const std::vector<bool> &list) : data_() { 
	for (auto iter = list.begin(); iter < list.end(); iter++) {
		data_.emplace_back(*iter);
	}
}

CU::JSONArray::JSONArray(const std::vector<int> &list) : data_()
{
	for (auto iter = list.begin(); iter < list.end(); iter++) {
		data_.emplace_back(*iter);
	}
}

CU::JSONArray::JSONArray(const std::vector<int64_t> &list) : data_()
{
	for (auto iter = list.begin(); iter < list.end(); iter++) {
		data_.emplace_back(*iter);
	}
}

CU::JSONArray::JSONArray(const std::vector<double> &list) : data_()
{
	for (auto iter = list.begin(); iter < list.end(); iter++) {
		data_.emplace_back(*iter);
	}
}

CU::JSONArray::JSONArray(const std::vector<std::string> &list) : data_()
{
	for (auto iter = list.begin(); iter < list.end(); iter++) {
		data_.emplace_back(*iter);
	}
}

CU::JSONArray::JSONArray(const std::vector<JSONArray> &list) : data_()
{
	for (auto iter = list.begin(); iter < list.end(); iter++) {
		data_.emplace_back(*iter);
	}
}

CU::JSONArray::JSONArray(const std::vector<JSONObject> &list) : data_()
{
	for (auto iter = list.begin(); iter < list.end(); iter++) {
		data_.emplace_back(*iter);
	}
}

CU::JSONArray::JSONArray(const JSONArray &other) : data_()
{
	if (std::addressof(other) != this) {
		data_ = other.data();
	}
}

CU::JSONArray::JSONArray(JSONArray &&other) noexcept : data_()
{
	if (std::addressof(other) != this) {
		data_ = other.data();
	}
}

CU::JSONArray::~JSONArray() { }

CU::JSONArray &CU::JSONArray::operator()(const JSONArray &other)
{
	if (std::addressof(other) != this) {
		data_ = other.data();
	}
	return *this;
}

CU::JSONArray &CU::JSONArray::operator=(const JSONArray &other)
{
	if (std::addressof(other) != this) {
		data_ = other.data();
	}
	return *this;
}

CU::JSONArray &CU::JSONArray::operator+=(const JSONArray &other)
{
	if (std::addressof(other) != this) {
		const auto &other_data = other.data();
		for (auto iter = other_data.begin(); iter < other_data.end(); iter++) {
			data_.emplace_back(*iter);
		}
	}
	return *this;
}

CU::JSONItem &CU::JSONArray::operator[](const size_t &pos)
{
	if (pos >= data_.size()) {
		data_.resize(pos + 1);
	}
	return data_.at(pos);
}

CU::JSONArray CU::JSONArray::operator+(const JSONArray &other) const
{
	auto merged_data = data_;
	if (std::addressof(other) != this) {
		const auto &other_data = other.data();
		for (auto iter = other_data.begin(); iter < other_data.end(); iter++) {
			merged_data.emplace_back(*iter);
		}
	}
	return JSONArray(merged_data);
}

bool CU::JSONArray::operator==(const JSONArray &other) const
{
	return (data_ == other.data());
}

bool CU::JSONArray::operator!=(const JSONArray &other) const
{
	return (data_ != other.data());
}

std::vector<bool> CU::JSONArray::toListBoolean() const
{
	std::vector<bool> listBoolean{};
	for (auto iter = data_.begin(); iter < data_.end(); iter++) {
		listBoolean.emplace_back(iter->toBoolean());
	}
	return listBoolean;
}

std::vector<int> CU::JSONArray::toListInt() const
{
	std::vector<int> listInt{};
	for (auto iter = data_.begin(); iter < data_.end(); iter++) {
		listInt.emplace_back(iter->toInt());
	}
	return listInt;
}

std::vector<int64_t> CU::JSONArray::toListLong() const
{
	std::vector<int64_t> listLong{};
	for (auto iter = data_.begin(); iter < data_.end(); iter++) {
		listLong.emplace_back(iter->toLong());
	}
	return listLong;
}

std::vector<double> CU::JSONArray::toListDouble() const
{
	std::vector<double> listDouble{};
	for (auto iter = data_.begin(); iter < data_.end(); iter++) {
		listDouble.emplace_back(iter->toDouble());
	}
	return listDouble;
}

std::vector<std::string> CU::JSONArray::toListString() const
{
	std::vector<std::string> listString{};
	for (auto iter = data_.begin(); iter < data_.end(); iter++) {
		listString.emplace_back(iter->toString());
	}
	return listString;
}

std::vector<CU::JSONArray> CU::JSONArray::toListArray() const
{
	std::vector<JSONArray> listArray{};
	for (auto iter = data_.begin(); iter < data_.end(); iter++) {
		listArray.emplace_back(iter->toArray());
	}
	return listArray;
}

std::vector<CU::JSONObject> CU::JSONArray::toListObject() const
{
	std::vector<JSONObject> listObject{};
	for (auto iter = data_.begin(); iter < data_.end(); iter++) {
		listObject.emplace_back(iter->toObject());
	}
	return listObject;
}

CU::JSONItem CU::JSONArray::at(const size_t &pos) const
{
	if (pos >= data_.size()) {
		throw JSONExcept("Position out of bound");
	}
	return data_.at(pos);
}

CU::JSONArray::Iterator CU::JSONArray::find(const JSONItem &item)
{
	if (data_.begin() == data_.end()) {
		return data_.end();
	}
	return std::find(data_.begin(), data_.end(), item);
}

void CU::JSONArray::add(const JSONItem &item)
{
	data_.emplace_back(item);
}

void CU::JSONArray::remove(const JSONItem &item)
{
	auto iter = std::find(data_.begin(), data_.end(), item);
	if (iter == data_.end()) {
		throw JSONExcept("Item not found");
	}
	data_.erase(iter);
}

void CU::JSONArray::resize(const size_t &new_size)
{
	data_.resize(new_size);
}

void CU::JSONArray::clear()
{
	data_.clear();
}

size_t CU::JSONArray::size() const
{
	return data_.size();
}

bool CU::JSONArray::empty() const
{
	return (data_.begin() == data_.end());
}

std::vector<CU::JSONItem> CU::JSONArray::data() const
{
	return data_;
}

std::string CU::JSONArray::toString() const
{
	if (data_.begin() == data_.end()) {
		std::string JSONText("[]");
		return JSONText;
	} else if ((data_.begin() + 1) == data_.end()) {
		auto JSONText = std::string("[") + data_.front().toRaw() + "]";
		return JSONText;
	}
	std::string JSONText("[");
	for (auto iter = data_.begin(); iter < (data_.end() - 1); iter++) {
		JSONText += iter->toRaw() + ",";
	}
	JSONText += data_.back().toRaw() + "]";
	return JSONText;
}

CU::JSONItem &CU::JSONArray::front()
{
	return data_.front();
}

CU::JSONItem &CU::JSONArray::back()
{
	return data_.back();
}

CU::JSONArray::Iterator CU::JSONArray::begin()
{
	return data_.begin();
}

CU::JSONArray::Iterator CU::JSONArray::end()
{
	return data_.end();
}

CU::JSONArray::ConstIterator CU::JSONArray::begin() const
{
	return data_.begin();
}

CU::JSONArray::ConstIterator CU::JSONArray::end() const
{
	return data_.end();
}

CU::JSONObject::JSONObject() : data_(), order_() { }

CU::JSONObject::JSONObject(const std::string &JSONString) : data_(), order_() 
{
	enum class ObjectIdx : uint8_t 
	{NONE, KEY_FRONT, KEY_CONTENT, KEY_BACK, VALUE_FRONT, VALUE_COMMON, VALUE_STRING, VALUE_ARRAY, VALUE_OBJECT, VALUE_BACK};
	size_t pos = 0;
	auto idx = ObjectIdx::NONE;
	uint32_t count = 0;
	std::string key{}, value{};
	while (pos < JSONString.size()) {
		const auto &ch = JSONString.at(pos);
		switch (ch) {
			case '{':
				if (idx == ObjectIdx::NONE) {
					idx = ObjectIdx::KEY_FRONT;
				} else if (idx == ObjectIdx::VALUE_FRONT) {
					idx = ObjectIdx::VALUE_OBJECT;
					count = 1;
					value += ch;
				} else if (idx == ObjectIdx::VALUE_OBJECT) {
					count++;
					value += ch;
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_ARRAY) {
					value += ch;
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key += ch;
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case '}':
				if (idx == ObjectIdx::KEY_FRONT && data_.empty()) {
					idx = ObjectIdx::NONE;
				} else if (idx == ObjectIdx::VALUE_OBJECT) {
					if (count > 0) {
						count--;
						value += ch;
						if (count == 0) {
							idx = ObjectIdx::VALUE_BACK;
						}
					} else {
						throw JSONExcept("Invalid JSONObject Structure");
					}
				} else if (idx == ObjectIdx::VALUE_BACK || idx == ObjectIdx::VALUE_COMMON) {
					idx = ObjectIdx::NONE;
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_ARRAY) {
					value += ch;
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key += ch;
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case '\"':
				if (idx == ObjectIdx::KEY_FRONT) {
					idx = ObjectIdx::KEY_CONTENT;
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					idx = ObjectIdx::KEY_BACK;
				} else if (idx == ObjectIdx::VALUE_FRONT) {
					idx = ObjectIdx::VALUE_STRING;
					value += ch;
				} else if (idx == ObjectIdx::VALUE_STRING) {
					idx = ObjectIdx::VALUE_BACK;
					value += ch;
				} else if (idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT) {
					value += ch;
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case ':':
				if (idx == ObjectIdx::KEY_BACK) {
					idx = ObjectIdx::VALUE_FRONT;
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT) {
					value += ch;
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key += ch;
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case ',':
				if (idx == ObjectIdx::VALUE_COMMON || idx == ObjectIdx::VALUE_BACK) {
					idx = ObjectIdx::KEY_FRONT;
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT) {
					value += ch;
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key += ch;
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case '[':
				if (idx == ObjectIdx::VALUE_FRONT) {
					idx = ObjectIdx::VALUE_ARRAY;
					count = 1;
					value += ch;
				} else if (idx == ObjectIdx::VALUE_ARRAY) {
					count++;
					value += ch;
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_OBJECT) {
					value += ch;
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key += ch;
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case ']':
				if (idx == ObjectIdx::VALUE_ARRAY) {
					if (count > 0) {
						count--;
						value += ch;
						if (count == 0) {
							idx = ObjectIdx::VALUE_BACK;
						}
					} else {
						throw JSONExcept("Invalid JSONObject Structure");
					}
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_OBJECT) {
					value += ch;
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key += ch;
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case ' ':
				if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT) {
					value += ch;
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key += ch;
				}
				break;
			case '\n':
			case '\t':
			case '\r':
			case '\f':
			case '\a':
			case '\b':
			case '\v':
				break;
			case '\\':
				if (idx == ObjectIdx::KEY_CONTENT) {
					pos++;
					key += _GetEscapeChar(JSONString.at(pos));
				} else if (idx == ObjectIdx::VALUE_STRING) {
					pos++;
					value += _GetEscapeChar(JSONString.at(pos));
				} else if (idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT) {
					value += ch;
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			default:
				if (idx == ObjectIdx::VALUE_FRONT) {
					idx = ObjectIdx::VALUE_COMMON;
					value += ch;
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key += ch;
				} else if (idx == ObjectIdx::VALUE_COMMON || idx == ObjectIdx::VALUE_STRING ||
					idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT
				) {
					value += ch;
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
		}
		if ((idx == ObjectIdx::KEY_FRONT || idx == ObjectIdx::NONE) && !key.empty() && !value.empty()) {
			order_.emplace_back(key);
			data_.emplace(key, JSONItem::_To_Init_Val(value));
			key.clear();
			value.clear();
		}
		pos++;
	}
	if (idx != ObjectIdx::NONE) {
		throw JSONExcept("Invalid JSONObject Structure");
	}
}

CU::JSONObject::JSONObject(const std::unordered_map<std::string, JSONItem> &data, const std::vector<std::string> &order) : 
	data_(data),
	order_(order)
{ }

CU::JSONObject::JSONObject(const JSONObject &other) : data_(), order_()
{
	if (std::addressof(other) != this) {
		data_ = other.data();
		order_ = other.order();
	}
}

CU::JSONObject::JSONObject(JSONObject &&other) noexcept : data_(), order_()
{
	if (std::addressof(other) != this) {
		data_ = other.data();
		order_ = other.order();
	}
}

CU::JSONObject::~JSONObject() { }

CU::JSONObject &CU::JSONObject::operator()(const JSONObject &other)
{
	if (std::addressof(other) != this) {
		data_ = other.data();
		order_ = other.order();
	}
	return *this;
}

CU::JSONObject &CU::JSONObject::operator=(const JSONObject &other)
{
	if (std::addressof(other) != this) {
		data_ = other.data();
		order_ = other.order();
	}
	return *this;
}

CU::JSONObject &CU::JSONObject::operator+=(const JSONObject &other)
{
	if (std::addressof(other) != this) {
		auto other_data = other.data();
		auto other_order = other.order();
		for (const auto &key : other_order) {
			if (data_.count(key) == 0) {
				order_.emplace_back(key);
				data_.emplace(key, other_data.at(key));
			} else {
				data_[key] = other_data.at(key);
			}
		}
	}
	return *this;
}

CU::JSONItem &CU::JSONObject::operator[](const std::string &key)
{
	if (data_.count(key) == 0) {
		order_.emplace_back(key);
	}
	return data_[key];
}

CU::JSONObject CU::JSONObject::operator+(const JSONObject &other) const
{
	auto merged_data = data_;
	auto merged_order = order_;
	const auto &other_data = other.data();
	const auto &other_order = other.order();
	for (const auto &key : other_order) {
		if (merged_data.count(key) == 1) {
			merged_data[key] = other_data.at(key);
		} else {
			merged_order.emplace_back(key);
			merged_data.emplace(key, other_data.at(key));
		}
	}
	return JSONObject(merged_data, merged_order);
}

bool CU::JSONObject::operator==(const JSONObject &other) const
{
	return (data_ == other.data() && order_ == other.order());
}

bool CU::JSONObject::operator!=(const JSONObject &other) const
{
	return (data_ != other.data() || order_ != other.order());
}

bool CU::JSONObject::contains(const std::string &key) const
{
	return (data_.count(key) == 1);
}

CU::JSONItem CU::JSONObject::at(const std::string &key) const
{
	auto iter = data_.find(key);
	if (iter == data_.end()) {
		throw JSONExcept("Key not found");
	}
	return iter->second;
}

void CU::JSONObject::add(const std::string &key, const JSONItem &value)
{
	if (data_.count(key) == 0) {
		order_.emplace_back(key);
		data_.emplace(key, value);
	} else {
		data_[key] = value;
	}
}

void CU::JSONObject::remove(const std::string &key)
{
	auto iter = std::find(order_.begin(), order_.end(), key);
	if (iter == order_.end()) {
		throw JSONExcept("Key not found");
	}
	data_.erase(key);
	order_.erase(iter);
}

void CU::JSONObject::clear()
{
	data_.clear();
	order_.clear();
}

size_t CU::JSONObject::size() const
{
	return data_.size();
}

bool CU::JSONObject::empty() const
{
	return (data_.begin() == data_.end());
}

std::unordered_map<std::string, CU::JSONItem> CU::JSONObject::data() const
{
	return data_;
}

std::vector<std::string> CU::JSONObject::order() const
{
	return order_;
}

std::string CU::JSONObject::toString() const
{
	if (order_.begin() == order_.end()) {
		std::string JSONString("{}");
		return JSONString;
	} else if ((order_.begin() + 1) == order_.end()) {
		std::string JSONString("{");
		JSONString += _StringToJSONRaw(order_.front()) + ":" + data_.at(order_.front()).toRaw() + "}";
		return JSONString;
	}
	std::string JSONString("{");
	for (auto iter = order_.begin(); iter < (order_.end() - 1); iter++) {
		const auto &key = *iter;
		JSONString += _StringToJSONRaw(key) + ":" + data_.at(key).toRaw() + ",";
	}
	JSONString += _StringToJSONRaw(order_.back()) + ":" + data_.at(order_.back()).toRaw() + "}";
	return JSONString;
}

std::string CU::JSONObject::toFormatedString() const
{
	if (order_.begin() == order_.end()) {
		std::string JSONString("{ }");
		return JSONString;
	} else if ((order_.begin() + 1) == order_.end()) {
		std::string JSONString("{\n");
		JSONString += std::string("  ") + _StringToJSONRaw(order_.front()) + ": " + data_.at(order_.front()).toRaw() + "\n}";
		return JSONString;
	}
	std::string JSONString("{\n");
	for (auto iter = order_.begin(); iter < (order_.end() - 1); iter++) {
		const auto &key = *iter;
		JSONString += std::string("  ") + _StringToJSONRaw(key) + ": " + data_.at(key).toRaw() + ",\n";
	}
	JSONString += std::string("  ") + _StringToJSONRaw(order_.back()) + ": " + data_.at(order_.back()).toRaw() + "\n}";
	return JSONString;
}

std::vector<CU::JSONObject::JSONPair> CU::JSONObject::toPairs() const
{
	std::vector<CU::JSONObject::JSONPair> pairs{};
	for (const auto &key : order_) {
		JSONPair pair{};
		pair.key = key;
		pair.value = data_.at(key);
		pairs.emplace_back(pair);
	}
	return pairs;
}

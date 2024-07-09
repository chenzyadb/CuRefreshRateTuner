#include "CuJSONObject.h"

CU::JSONItem::JSONItem() : 
	type_(ItemType::ITEM_NULL), 
	value_(nullptr) 
{ }

CU::JSONItem::JSONItem(const _JSON_String &raw) : type_(), value_()
{
	auto len = raw.length;
	auto raw_data = raw.data();
	switch (*raw_data) {
		case '{':
			if (*(raw_data + len - 1) == '}') {
				type_ = ItemType::OBJECT;
				value_ = new JSONObject(raw_data);
				return;
			}
			break;
		case '[':
			if (*(raw_data + len - 1) == ']') {
				type_ = ItemType::ARRAY;
				value_ = new JSONArray(raw_data);
				return;
			}
			break;
		case '\"':
			if (*(raw_data + len - 1) == '\"') {
				type_ = ItemType::STRING;
				value_ = std::string(raw_data).substr(1, (len - 2));
				return;
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
				auto num = std::strtod(raw_data, nullptr);
				if (num != 0) {
					if (num == static_cast<int64_t>(num)) {
						if (num > static_cast<double>(INT_MAX) || num < static_cast<double>(INT_MIN)) {
							type_ = ItemType::LONG;
							value_ = static_cast<int64_t>(num);
							return;
						} else {
							type_ = ItemType::INTEGER;
							value_ = static_cast<int>(num);
							return;
						}
					} else {
						type_ = ItemType::DOUBLE;
						value_ = num;
						return;
					}
				} else if (raw.equals("0")) {
					type_ = ItemType::INTEGER;
					value_ = 0;
					return;
				} else if (raw.equals("0.0")) {
					type_ = ItemType::DOUBLE;
					value_ = 0.0;
					return;
				}
			}
			break;
		case 't':
			if (raw.equals("true")) {
				type_ = ItemType::BOOLEAN;
				value_ = true;
				return;
			}
			break;
		case 'f':
			if (raw.equals("false")) {
				type_ = ItemType::BOOLEAN;
				value_ = false;
				return;
			}
			break;
		case 'n':
			if (raw.equals("null")) {
				type_ = ItemType::ITEM_NULL;
				value_ = nullptr;
				return;
			}
			break;
		default:
			break;
	}
	throw JSONExcept("Invalid JSONItem");
}

CU::JSONItem::JSONItem(ItemNull _null) :
	type_(ItemType::ITEM_NULL),
	value_(nullptr)
{ 
	(void)_null;
}

CU::JSONItem::JSONItem(bool value) :
	type_(ItemType::BOOLEAN),
	value_(value) 
{ }

CU::JSONItem::JSONItem(int value) :
	type_(ItemType::INTEGER),
	value_(value) 
{ }

CU::JSONItem::JSONItem(int64_t value) :
	type_(ItemType::LONG),
	value_(value)
{ }

CU::JSONItem::JSONItem(double value) : 
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

CU::JSONItem::JSONItem(const JSONItem &other) : type_(other.type()), value_()
{
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

CU::JSONItem::JSONItem(JSONItem &&other) noexcept : type_(other.type()), value_(other.value_rv()) { }

CU::JSONItem::~JSONItem()
{
	if (type_ == ItemType::ARRAY) {
		delete std::get<JSONArray*>(value_);
	} else if (type_ == ItemType::OBJECT) {
		delete std::get<JSONObject*>(value_);
	}
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

CU::JSONItem &CU::JSONItem::operator=(JSONItem &&other) noexcept
{
	if (std::addressof(other) != this) {
		clear();
		type_ = other.type();
		value_ = other.value_rv();
	}
	return *this;
}

bool CU::JSONItem::operator==(const JSONItem &other) const
{
	return (type_ == other.type() && value_ == other.value());
}

bool CU::JSONItem::operator!=(const JSONItem &other) const
{
	return (type_ != other.type() || value_ != other.value());
}

CU::JSONItem::ItemType CU::JSONItem::type() const
{
	return type_;
}

CU::JSONItem::ItemValue CU::JSONItem::value() const
{
	return value_;
}

CU::JSONItem::ItemValue &&CU::JSONItem::value_rv()
{
	type_ = ItemType::ITEM_NULL;
	return std::move(value_);
}

void CU::JSONItem::clear()
{
	if (type_ == ItemType::ARRAY) {
		delete std::get<JSONArray*>(value_);
	} else if (type_ == ItemType::OBJECT) {
		delete std::get<JSONObject*>(value_);
	}
	type_ = ItemType::ITEM_NULL;
	value_ = nullptr;
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

size_t CU::JSONItem::hash() const
{
	switch (type_) {
		case ItemType::ITEM_NULL:
			break;
		case ItemType::BOOLEAN:
			{
				std::hash<bool> hashVal{};
				return hashVal(std::get<bool>(value_));
			}
		case ItemType::INTEGER:
			{
				std::hash<int> hashVal{};
				return hashVal(std::get<int>(value_));
			}
		case ItemType::LONG:
			{
				std::hash<int64_t> hashVal{};
				return hashVal(std::get<int64_t>(value_));
			}
		case ItemType::DOUBLE:
			{
				std::hash<double> hashVal{};
				return hashVal(std::get<double>(value_));
			}
		case ItemType::STRING:
			{
				std::hash<std::string> hashVal{};
				return hashVal(std::get<std::string>(value_));
			}
		case ItemType::ARRAY:
			return std::get<CU::JSONArray*>(value_)->hash();
		case ItemType::OBJECT:
			return std::get<CU::JSONObject*>(value_)->hash();
	}
	return 0;
}

bool CU::JSONItem::isNull() const
{
	return (type_ == ItemType::ITEM_NULL);
}

bool CU::JSONItem::isBoolean() const
{
	return (type_ == ItemType::BOOLEAN);
}

bool CU::JSONItem::isInt() const
{
	return (type_ == ItemType::INTEGER);
}

bool CU::JSONItem::isLong() const
{
	return (type_ == ItemType::LONG);
}

bool CU::JSONItem::isDouble() const
{
	return (type_ == ItemType::DOUBLE);
}

bool CU::JSONItem::isString() const
{
	return (type_ == ItemType::STRING);
}

bool CU::JSONItem::isArray() const
{
	return (type_ == ItemType::ARRAY);
}

bool CU::JSONItem::isObject() const
{
	return (type_ == ItemType::OBJECT);
}

bool CU::JSONItem::toBoolean() const
{
	if (type_ != ItemType::BOOLEAN) {
		throw CU::JSONExcept("Item is not of boolean type");
	}
	return std::get<bool>(value_);
}

int CU::JSONItem::toInt() const
{
	if (type_ != ItemType::INTEGER) {
		throw CU::JSONExcept("Item is not of int type");
	}
	return std::get<int>(value_);
}

int64_t CU::JSONItem::toLong() const
{
	if (type_ != ItemType::LONG) {
		throw CU::JSONExcept("Item is not of long type");
	}
	return std::get<int64_t>(value_);
}

double CU::JSONItem::toDouble() const
{
	if (type_ != ItemType::DOUBLE) {
		throw CU::JSONExcept("Item is not of double type");
	}
	return std::get<double>(value_);
}

std::string CU::JSONItem::toString() const
{
	if (type_ != ItemType::STRING) {
		throw CU::JSONExcept("Item is not of string type");
	}
	return std::get<std::string>(value_);
}

CU::JSONArray CU::JSONItem::toArray() const
{
	if (type_ != ItemType::ARRAY) {
		throw CU::JSONExcept("Item is not of array type");
	}
	return *(std::get<JSONArray*>(value_));
}

CU::JSONObject CU::JSONItem::toObject() const
{
	if (type_ != ItemType::OBJECT) {
		throw CU::JSONExcept("Item is not of object type");
	}
	return *(std::get<JSONObject*>(value_));
}

std::string CU::JSONItem::toRaw() const
{
	switch (type_) {
		case ItemType::ITEM_NULL:
			return "null";
		case ItemType::BOOLEAN:
			if (std::get<bool>(value_)) {
				return "true";
			}
			return "false";
		case ItemType::INTEGER:
			return std::to_string(std::get<int>(value_));
		case ItemType::LONG:
			return std::to_string(std::get<int64_t>(value_));
		case ItemType::DOUBLE:
			return std::to_string(std::get<double>(value_));
		case ItemType::STRING:
			return _JSON_Misc::StringToJSONRaw(std::get<std::string>(value_)).data();
		case ItemType::ARRAY:
			return std::get<JSONArray*>(value_)->toString();
		case ItemType::OBJECT:
			return std::get<JSONObject*>(value_)->toString();
	}
	return {};
}

CU::JSONArray::JSONArray() : data_() { }

CU::JSONArray::JSONArray(size_t init_size) : data_(init_size) { }

CU::JSONArray::JSONArray(size_t init_size, const JSONItem &init_value) : data_(init_size, init_value) { }

CU::JSONArray::JSONArray(iterator begin_iter, iterator end_iter) : data_(begin_iter, end_iter) { }

CU::JSONArray::JSONArray(const std::string &jsonText) : data_()
{
	Parse_Impl_(jsonText.data());
}

CU::JSONArray::JSONArray(const char* jsonText) : data_()
{
	Parse_Impl_(jsonText);
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

CU::JSONArray::JSONArray(const JSONArray &other) : data_(other.data()) { }

CU::JSONArray::JSONArray(JSONArray &&other) noexcept : data_(other.data_rv()) { }

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

CU::JSONItem &CU::JSONArray::operator[](size_t pos)
{
	if (pos >= data_.size()) {
		throw JSONExcept("Position out of bounds");
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

CU::JSONItem CU::JSONArray::at(size_t pos) const
{
	if (pos >= data_.size()) {
		throw JSONExcept("Position out of bounds");
	}
	return data_.at(pos);
}

CU::JSONArray::iterator CU::JSONArray::find(const JSONItem &item)
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

void CU::JSONArray::resize(size_t new_size)
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

size_t CU::JSONArray::hash() const
{
	size_t hashVal = data_.size();
	for (const auto &item : data_) {
		hashVal ^= item.hash() + 2654435769 + (hashVal << 6) + (hashVal >> 2);
	}
	return hashVal;
}

bool CU::JSONArray::empty() const
{
	return (data_.begin() == data_.end());
}

const std::vector<CU::JSONItem> &CU::JSONArray::data() const
{
	return data_;
}

std::vector<CU::JSONItem> &&CU::JSONArray::data_rv()
{
	return std::move(data_);
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

CU::JSONArray::iterator CU::JSONArray::begin()
{
	return data_.begin();
}

CU::JSONArray::iterator CU::JSONArray::end()
{
	return data_.end();
}

CU::JSONArray::const_iterator CU::JSONArray::begin() const
{
	return data_.begin();
}

CU::JSONArray::const_iterator CU::JSONArray::end() const
{
	return data_.end();
}

void CU::JSONArray::Parse_Impl_(const char* json_text)
{
	enum class ArrayIdx : uint8_t { NONE, ITEM_FRONT, ITEM_COMMON, ITEM_STRING, ITEM_ARRAY, ITEM_OBJECT, ITEM_BACK };
	size_t pos = 0;
	auto idx = ArrayIdx::NONE;
	uint32_t count = 0;
	_JSON_String content{};
	while (*(json_text + pos) != '\0') {
		char ch = *(json_text + pos);
		switch (ch) {
			case '[':
				if (idx == ArrayIdx::NONE) {
					idx = ArrayIdx::ITEM_FRONT;
				} else if (idx == ArrayIdx::ITEM_FRONT) {
					idx = ArrayIdx::ITEM_ARRAY;
					count = 1;
					content.append(ch);
				} else if (idx == ArrayIdx::ITEM_ARRAY) {
					count++;
					content.append(ch);
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_OBJECT) {
					content.append(ch);
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
						content.append(ch);
						if (count == 0) {
							idx = ArrayIdx::ITEM_BACK;
						}
					} else {
						throw JSONExcept("Invalid JSONArray Structure");
					}
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_OBJECT) {
					content.append(ch);
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case ',':
				if (idx == ArrayIdx::ITEM_BACK || idx == ArrayIdx::ITEM_COMMON) {
					idx = ArrayIdx::ITEM_FRONT;
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_ARRAY || idx == ArrayIdx::ITEM_OBJECT) {
					content.append(ch);
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case '{':
				if (idx == ArrayIdx::ITEM_FRONT) {
					idx = ArrayIdx::ITEM_OBJECT;
					count = 1;
					content.append(ch);
				} else if (idx == ArrayIdx::ITEM_OBJECT) {
					count++;
					content.append(ch);
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_ARRAY) {
					content.append(ch);
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case '}':
				if (idx == ArrayIdx::ITEM_OBJECT) {
					if (count > 0) {
						count--;
						content.append(ch);
						if (count == 0) {
							idx = ArrayIdx::ITEM_BACK;
						}
					} else {
						throw JSONExcept("Invalid JSONArray Structure");
					}
				} else if (idx == ArrayIdx::ITEM_STRING || idx == ArrayIdx::ITEM_ARRAY) {
					content.append(ch);
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case '\"':
				if (idx == ArrayIdx::ITEM_FRONT) {
					idx = ArrayIdx::ITEM_STRING;
					content.append(ch);
				} else if (idx == ArrayIdx::ITEM_STRING) {
					idx = ArrayIdx::ITEM_BACK;
					content.append(ch);
				} else if (idx == ArrayIdx::ITEM_ARRAY || idx == ArrayIdx::ITEM_OBJECT) {
					content.append(ch);
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			case ' ':
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
					if (*(json_text + pos + 1) == '\0') {
						throw JSONExcept("Invalid JSONArray Structure");
					}
					pos++;
					content.append(_JSON_Misc::GetEscapeChar(*(json_text + pos)));
				} else if (idx == ArrayIdx::ITEM_ARRAY || idx == ArrayIdx::ITEM_OBJECT) {
					content.append(ch);
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
			default:
				if (idx == ArrayIdx::ITEM_FRONT) {
					idx = ArrayIdx::ITEM_COMMON;
					content.append(ch);
				} else if (idx != ArrayIdx::NONE && idx != ArrayIdx::ITEM_BACK) {
					content.append(ch);
				} else {
					throw JSONExcept("Invalid JSONArray Structure");
				}
				break;
		}
		if ((idx == ArrayIdx::ITEM_FRONT || idx == ArrayIdx::NONE) && content.length > 0) {
			data_.emplace_back(JSONItem(content));
			content.shrink(0);
		}
		pos++;
	}
	if (idx != ArrayIdx::NONE) {
		throw JSONExcept("Invalid JSONArray Structure");
	}
}

CU::JSONObject::JSONObject() : data_(), order_() { }

CU::JSONObject::JSONObject(const std::string &jsonText, bool enableComments) : data_(), order_() 
{
	Parse_Impl_(jsonText.data(), enableComments);
}

CU::JSONObject::JSONObject(const char* jsonText, bool enableComments) : data_(), order_()
{
	Parse_Impl_(jsonText, enableComments);
}

CU::JSONObject::JSONObject(const JSONObject &other) : 
	data_(other.data()), 
	order_(other.order())
{ }

CU::JSONObject::JSONObject(JSONObject &&other) noexcept : 
	data_(other.data_rv()), 
	order_(other.order_rv())
{ }

CU::JSONObject::JSONObject(std::unordered_map<std::string, JSONItem> &&data, std::vector<std::string> &&order) noexcept :
	data_(data),
	order_(order)
{ }

CU::JSONObject::~JSONObject() { }

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
	std::unordered_map<std::string, JSONItem> merged_data(other.data());
	std::vector<std::string> merged_order(other.order());
	for (const auto &key : order_) {
		if (merged_data.count(key) == 1) {
			merged_data[key] = data_.at(key);
		} else {
			merged_order.emplace_back(key);
			merged_data.emplace(key, data_.at(key));
		}
	}
	return JSONObject(std::move(merged_data), std::move(merged_order));
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

size_t CU::JSONObject::hash() const
{
	size_t hashVal = data_.size();
	for (auto iter = data_.begin(); iter != data_.end(); iter++) {
		hashVal ^= (iter->second).hash() + 2654435769 + (hashVal << 6) + (hashVal >> 2);
	}
	return hashVal;
}

bool CU::JSONObject::empty() const
{
	return (data_.begin() == data_.end());
}

const std::unordered_map<std::string, CU::JSONItem> &CU::JSONObject::data() const
{
	return data_;
}

std::unordered_map<std::string, CU::JSONItem> &&CU::JSONObject::data_rv()
{
	return std::move(data_);
}

const std::vector<std::string> &CU::JSONObject::order() const
{
	return order_;
}

std::vector<std::string> &&CU::JSONObject::order_rv()
{
	return std::move(order_);
}

std::string CU::JSONObject::toString() const
{
	if (order_.size() == 1) {
		_JSON_String jsonText("{");
		jsonText.append(_JSON_Misc::StringToJSONRaw(order_.front()));
		jsonText.append(':');
		jsonText.append(data_.at(order_.front()).toRaw().data());
		jsonText.append('}');
		return jsonText.data();
	} else if (order_.size() > 1) {
		_JSON_String jsonText("{");
		for (auto iter = order_.begin(); iter < (order_.end() - 1); iter++) {
			const auto &key = *iter;
			jsonText.append(_JSON_Misc::StringToJSONRaw(key));
			jsonText.append(':');
			jsonText.append(data_.at(key).toRaw().data());
			jsonText.append(',');
		}
		jsonText.append(_JSON_Misc::StringToJSONRaw(order_.back()));
		jsonText.append(':');
		jsonText.append(data_.at(order_.back()).toRaw().data());
		jsonText.append('}');
		return jsonText.data();
	}
	return "{}";
}

std::string CU::JSONObject::toFormatedString() const
{
	if (order_.size() == 1) {
		_JSON_String jsonText("{\n");
		jsonText.append(_JSON_Misc::StringToJSONRaw(order_.front()));
		jsonText.append(": ");
		jsonText.append(data_.at(order_.front()).toRaw().data());
		jsonText.append("\n }");
		return jsonText.data();
	} else if (order_.size() > 1) {
		_JSON_String jsonText("{\n");
		for (auto iter = order_.begin(); iter < (order_.end() - 1); iter++) {
			const auto &key = *iter;
			jsonText.append("  ");
			jsonText.append(_JSON_Misc::StringToJSONRaw(key));
			jsonText.append(": ");
			jsonText.append(data_.at(key).toRaw().data());
			jsonText.append(",\n");
		}
		jsonText.append(_JSON_Misc::StringToJSONRaw(order_.back()));
		jsonText.append(": ");
		jsonText.append(data_.at(order_.back()).toRaw().data());
		jsonText.append("\n}");
		return jsonText.data();
	}
	return "{ }";
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

void CU::JSONObject::Parse_Impl_(const char* json_text, bool enable_comments)
{
	enum class ObjectIdx : uint8_t
	{NONE, KEY_FRONT, KEY_CONTENT, KEY_BACK, VALUE_FRONT, VALUE_COMMON, VALUE_STRING, VALUE_ARRAY, VALUE_OBJECT, VALUE_BACK};
	size_t pos = 0;
	auto idx = ObjectIdx::NONE;
	uint32_t count = 0;
	_JSON_String key{}, value{};
	while (*(json_text + pos) != '\0') {
		char ch = *(json_text + pos);
		switch (ch) {
			case '{':
				if (idx == ObjectIdx::NONE) {
					idx = ObjectIdx::KEY_FRONT;
				} else if (idx == ObjectIdx::VALUE_FRONT) {
					idx = ObjectIdx::VALUE_OBJECT;
					count = 1;
					value.append(ch);
				} else if (idx == ObjectIdx::VALUE_OBJECT) {
					count++;
					value.append(ch);
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_ARRAY) {
					value.append(ch);
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key.append(ch);
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
						value.append(ch);
						if (count == 0) {
							idx = ObjectIdx::VALUE_BACK;
						}
					} else {
						throw JSONExcept("Invalid JSONObject Structure");
					}
				} else if (idx == ObjectIdx::VALUE_BACK || idx == ObjectIdx::VALUE_COMMON) {
					idx = ObjectIdx::NONE;
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_ARRAY) {
					value.append(ch);
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key.append(ch);
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
					value.append(ch);
				} else if (idx == ObjectIdx::VALUE_STRING) {
					idx = ObjectIdx::VALUE_BACK;
					value.append(ch);
				} else if (idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT) {
					value.append(ch);
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case ':':
				if (idx == ObjectIdx::KEY_BACK) {
					idx = ObjectIdx::VALUE_FRONT;
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT) {
					value.append(ch);
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key.append(ch);
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case ',':
				if (idx == ObjectIdx::VALUE_COMMON || idx == ObjectIdx::VALUE_BACK) {
					idx = ObjectIdx::KEY_FRONT;
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT) {
					value.append(ch);
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key.append(ch);
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case '[':
				if (idx == ObjectIdx::VALUE_FRONT) {
					idx = ObjectIdx::VALUE_ARRAY;
					count = 1;
					value.append(ch);
				} else if (idx == ObjectIdx::VALUE_ARRAY) {
					count++;
					value.append(ch);
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_OBJECT) {
					value.append(ch);
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key.append(ch);
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case ']':
				if (idx == ObjectIdx::VALUE_ARRAY) {
					if (count > 0) {
						count--;
						value.append(ch);
						if (count == 0) {
							idx = ObjectIdx::VALUE_BACK;
						}
					} else {
						throw JSONExcept("Invalid JSONObject Structure");
					}
				} else if (idx == ObjectIdx::VALUE_STRING || idx == ObjectIdx::VALUE_OBJECT) {
					value.append(ch);
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key.append(ch);
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case ' ':
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
					if (*(json_text + pos + 1) == '\0') {
						throw JSONExcept("Invalid JSONObject Structure");
					}
					pos++;
					key.append(_JSON_Misc::GetEscapeChar(*(json_text + pos)));
				} else if (idx == ObjectIdx::VALUE_STRING) {
					if (*(json_text + pos + 1) == '\0') {
						throw JSONExcept("Invalid JSONObject Structure");
					}
					pos++;
					value.append(_JSON_Misc::GetEscapeChar(*(json_text + pos)));
				} else if (idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT) {
					value.append(ch);
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
			case '#':
				if (enable_comments) {
					auto next_pos = _JSON_Misc::FindChar(json_text, '\n', (pos + 1));
					if (next_pos == _JSON_Misc::npos) {
						next_pos = std::strlen(json_text) - 1;
					}
					pos = next_pos;
					break;
				}
			default:
				if (idx == ObjectIdx::VALUE_FRONT) {
					idx = ObjectIdx::VALUE_COMMON;
					value.append(ch);
				} else if (idx == ObjectIdx::KEY_CONTENT) {
					key.append(ch);
				} else if (idx == ObjectIdx::VALUE_COMMON || idx == ObjectIdx::VALUE_STRING ||
						   idx == ObjectIdx::VALUE_ARRAY || idx == ObjectIdx::VALUE_OBJECT
						   ) {
					value.append(ch);
				} else {
					throw JSONExcept("Invalid JSONObject Structure");
				}
				break;
		}
		if ((idx == ObjectIdx::KEY_FRONT || idx == ObjectIdx::NONE) && key.length > 0 && value.length > 0) {
			order_.emplace_back(key.data());
			data_.emplace(key.data(), JSONItem(value));
			key.shrink(0);
			value.shrink(0);
		}
		pos++;
	}
	if (idx != ObjectIdx::NONE) {
		throw JSONExcept("Invalid JSONObject Structure");
	}
}

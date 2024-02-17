// CuStringMatcher by chenzyadb@github.com.
// Based on C++14 STL (MSVC).

#ifndef _CU_STRING_MATCHER_
#define _CU_STRING_MATCHER_

#include <exception>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <cctype>

namespace CU
{
	class MatchExcept : public std::exception
	{
		public:
			MatchExcept(const std::string &message) : message_(message) { }

			const char* what() const noexcept override
			{
				return message_.c_str();
			}

		private:
			const std::string message_;
	};

	class StringMatcher
	{
		public:
			StringMatcher() : rule_(), front_(), middle_(), back_(), entire_() { }

			StringMatcher(const std::string &rule) : rule_(rule), front_(), middle_(), back_(), entire_()
			{
				UpdateRule_();
			}

			StringMatcher(const StringMatcher &other) : rule_(), front_(), middle_(), back_(), entire_()
			{
				if (std::addressof(other) != this) {
					rule_ = other.rule();
					UpdateRule_();
				}
			}

			StringMatcher(StringMatcher &&other) noexcept : rule_(), front_(), middle_(), back_(), entire_()
			{
				if (std::addressof(other) != this) {
					rule_ = other.rule();
					UpdateRule_();
				}
			}

			~StringMatcher() { }

			StringMatcher &operator()(const StringMatcher &other)
			{
				if (std::addressof(other) != this) {
					if (rule_ != other.rule()) {
						rule_ = other.rule();
						UpdateRule_();
					}
				}
				return *this;
			}

			StringMatcher &operator=(const StringMatcher &other)
			{
				if (std::addressof(other) != this) {
					if (rule_ != other.rule()) {
						rule_ = other.rule();
						UpdateRule_();
					}
				}
				return *this;
			}

			bool operator==(const StringMatcher &other) const
			{
				return (rule_ == other.rule());
			}

			bool operator!=(const StringMatcher &other) const
			{
				return (rule_ != other.rule());
			}

			bool operator>(const StringMatcher &other) const
			{
				return (rule_ > other.rule());
			}

			bool operator<(const StringMatcher &other) const
			{
				return (rule_ < other.rule());
			}

			bool match(const std::string &str) const
			{
				static const auto matchFront = [](const std::string &s, const std::string &p) -> bool {
					size_t len = p.size();
					if (s.size() < len) {
						return false;
					}
					return (memcmp(s.data(), p.data(), len) == 0);
				};
				static const auto matchBack = [](const std::string &s, const std::string &p) -> bool {
					size_t s_len = s.size(), p_len = p.size();
					if (s_len < p_len) {
						return false;
					}
					return (memcmp(s.data() + (s_len - p_len), p.data(), p_len) == 0);
				};
				static const auto matchEntire = [](const std::string &s, const std::string &p) -> bool {
					size_t len = p.size();
					if (s.size() != len) {
						return false;
					}
					return (memcmp(s.data(), p.data(), len) == 0);
				};

				if (str.empty()) {
					return false;
				}
				for (const auto &key : middle_) {
					if (str.find(key) != std::string::npos) {
						return true;
					}
				}
				for (const auto &key : front_) {
					if (matchFront(str, key)) {
						return true;
					}
				}
				for (const auto &key : back_) {
					if (matchBack(str, key)) {
						return true;
					}
				}
				for (const auto &key : entire_) {
					if (matchEntire(str, key)) {
						return true;
					}
				}
				return false;
			}

			std::string rule() const
			{
				return rule_;
			}

			void setRule(const std::string &rule)
			{
				if (rule_ != rule) {
					rule_ = rule;
					UpdateRule_();
				}
			}

			void clear()
			{
				rule_.clear();
				front_.clear();
				middle_.clear();
				back_.clear();
				entire_.clear();
			}

		private:
			std::string rule_;
			std::vector<std::string> front_;
			std::vector<std::string> middle_;
			std::vector<std::string> back_;
			std::vector<std::string> entire_;

			void UpdateRule_()
			{
				front_.clear();
				middle_.clear();
				back_.clear();
				entire_.clear();
				if (rule_.empty()) {
					return;
				}

				enum class RuleIdx : uint8_t {RULE_FRONT, RULE_CONTENT, RULE_SET, RULE_BACK};
				RuleIdx idx = RuleIdx::RULE_FRONT;
				std::string ruleContent{};
				bool matchFront = true, matchBack = true;
				size_t pos = 0;
				while (pos < rule_.size()) {
					switch (rule_[pos]) {
						case '*':
							if (idx == RuleIdx::RULE_FRONT && matchFront) {
								matchFront = false;
							} else if (idx == RuleIdx::RULE_CONTENT && matchBack) {
								matchBack = false;
							} else {
								throw MatchExcept("Invalid Matching Rule");
							}
							break;
						case '|':
							if (idx == RuleIdx::RULE_CONTENT) {
								idx = RuleIdx::RULE_BACK;
							} else if (idx == RuleIdx::RULE_SET) {
								ruleContent += rule_[pos];
							} else {
								throw MatchExcept("Invalid Matching Rule");
							}
							break;
						case '(':
							if (idx == RuleIdx::RULE_FRONT) {
								idx = RuleIdx::RULE_SET;
							} else {
								throw MatchExcept("Invalid Matching Rule");
							}
							break;
						case ')':
							if (idx == RuleIdx::RULE_SET) {
								idx = RuleIdx::RULE_CONTENT;
							} else {
								throw MatchExcept("Invalid Matching Rule");
							}
							break;
						default:
							if (idx == RuleIdx::RULE_FRONT) {
								idx = RuleIdx::RULE_CONTENT;
							}
							ruleContent += rule_[pos];
							break;
					}
					if (idx == RuleIdx::RULE_BACK || pos == (rule_.size() - 1)) {
						auto rules = ParseRuleContent_(ruleContent);
						if (matchFront && matchBack) {
							entire_.insert(entire_.end(), rules.begin(), rules.end());
						} else if (matchFront && !matchBack) {
							front_.insert(front_.end(), rules.begin(), rules.end());
						} else if (!matchFront && matchBack) {
							back_.insert(back_.end(), rules.begin(), rules.end());
						} else {
							middle_.insert(middle_.end(), rules.begin(), rules.end());
						}
						matchFront = true, matchBack = true;
						ruleContent.clear();
						idx = RuleIdx::RULE_FRONT;
					}
					pos++;
				}
			}

			std::vector<std::string> ParseRuleContent_(const std::string &content)
			{
				std::vector<std::string> rules{};
				size_t pos = 0, len = content.size();
				while (pos < len) {
					auto next_pos = content.find('|', pos);
					if (next_pos == std::string::npos) {
						next_pos = len;
					}
					auto rule = content.substr(pos, next_pos - pos);
					if (rule.find('[') != std::string::npos) {
						auto parsedRules = ParseCharSet_(rule);
						rules.insert(rules.end(), parsedRules.begin(), parsedRules.end());
					} else {
						rules.emplace_back(rule);
					}
					pos = next_pos + 1;
				}
				return rules;
			}

			std::vector<std::string> ParseCharSet_(const std::string &str)
			{
				static const auto getCharSet = [](const std::string &str) -> std::string {
					if (str.size() == 3 && str[1] == '-' && isalnum(str[0]) != 0 && isalnum(str[2]) != 0) {
						std::string charSet{};
						for (auto ch = str[0]; ch <= str[2]; ch++) {
							charSet += ch;
						}
						return charSet;
					} else if (str.size() == 2 && isalnum(str[0]) != 0 && isalnum(str[1]) != 0) {
						return str;
					}
					return {};
				};

				std::vector<std::string> parsedRules(1, str);
				size_t pos = 0;
				while (pos < parsedRules[0].size()) {
					auto set_begin = parsedRules[0].find('[');
					auto set_end = parsedRules[0].find(']');
					if (set_end != std::string::npos && set_begin < (set_end - 1)) {
						auto charSet = getCharSet(parsedRules[0].substr(set_begin + 1, set_end - set_begin - 1));
						std::vector<std::string> newParsedRules{};
						for (const auto &rule : parsedRules) {
							auto frontStr = rule.substr(0, set_begin);
							auto backStr = rule.substr(set_end + 1);
							for (const auto &ch : charSet) {
								newParsedRules.emplace_back(frontStr + ch + backStr);
							}
						}
						parsedRules = newParsedRules;
					} else {
						break;
					}
					pos = set_begin;
				}
				return parsedRules;
			}
	};
}

namespace std
{
	template<>
	struct hash<CU::StringMatcher>
	{
		size_t operator()(const CU::StringMatcher &val) const
		{
			return reinterpret_cast<size_t>(std::addressof(val));
		}
	};
}

#endif

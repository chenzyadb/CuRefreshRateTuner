// CuStringMatcher by chenzyadb@github.com.
// Based on C++17 STL (MSVC).

#ifndef _CU_STRING_MATCHER_
#define _CU_STRING_MATCHER_

#include <exception>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cstring>

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
			enum class MatchIndex : uint8_t {FRONT, MIDDLE, BACK, ENTIRE};

			StringMatcher() : matchRule_(), matchAll_(false) { }

			StringMatcher(const std::string &ruleText) : matchRule_(), matchAll_(false)
			{
				if (ruleText.size() == 0) {
					return;
				}
				if (ruleText == "*") {
					matchAll_ = true;
					return;
				}
				enum class RuleIdx : uint8_t {RULE_FRONT, RULE_CONTENT, RULE_SET, RULE_BACK};
				auto idx = RuleIdx::RULE_FRONT;
				std::string ruleContent{};
				bool matchFront = true, matchBack = true;
				size_t pos = 0;
				while (pos < ruleText.size()) {
					switch (ruleText[pos]) {
						case '\\':
							if ((pos + 1) == ruleText.size()) {
								throw MatchExcept("Invalid matching rule");
							}
							if (idx == RuleIdx::RULE_FRONT) {
								idx = RuleIdx::RULE_CONTENT;
							}
							pos++;
							ruleContent += ruleText[pos];
							break;
						case '*':
							if (idx == RuleIdx::RULE_FRONT && matchFront) {
								matchFront = false;
							} else if (idx == RuleIdx::RULE_CONTENT && matchBack) {
								matchBack = false;
							} else {
								throw MatchExcept("Invalid matching rule");
							}
							break;
						case '|':
							if (idx == RuleIdx::RULE_CONTENT) {
								idx = RuleIdx::RULE_BACK;
							} else if (idx == RuleIdx::RULE_SET) {
								ruleContent += ruleText[pos];
							} else {
								throw MatchExcept("Invalid matching rule");
							}
							break;
						case '(':
							if (idx == RuleIdx::RULE_FRONT) {
								idx = RuleIdx::RULE_SET;
							} else {
								throw MatchExcept("Invalid matching rule");
							}
							break;
						case ')':
							if (idx == RuleIdx::RULE_SET) {
								idx = RuleIdx::RULE_CONTENT;
							} else {
								throw MatchExcept("Invalid matching rule");
							}
							break;
						default:
							if (idx == RuleIdx::RULE_FRONT) {
								idx = RuleIdx::RULE_CONTENT;
							}
							ruleContent += ruleText[pos];
							break;
					}
					if (idx == RuleIdx::RULE_BACK || pos == (ruleText.size() - 1)) {
						auto rules = ParseRuleContent_(ruleContent);
						if (matchFront && matchBack) {
							auto &entire = matchRule_[MatchIndex::ENTIRE];
							entire.insert(entire.end(), rules.begin(), rules.end());
						} else if (matchFront && !matchBack) {
							auto &front = matchRule_[MatchIndex::FRONT];
							front.insert(front.end(), rules.begin(), rules.end());
						} else if (!matchFront && matchBack) {
							auto &back = matchRule_[MatchIndex::BACK];
							back.insert(back.end(), rules.begin(), rules.end());
						} else {
							auto &middle = matchRule_[MatchIndex::MIDDLE];
							middle.insert(middle.end(), rules.begin(), rules.end());
						}
						matchFront = true, matchBack = true;
						ruleContent.clear();
						idx = RuleIdx::RULE_FRONT;
					}
					pos++;
				}
			}

			StringMatcher(const StringMatcher &other) : 
				matchRule_(other._Get_MatchRule()),
				matchAll_(other._Is_MatchAll())
			{ }

			StringMatcher(StringMatcher &&other) noexcept : 
				matchRule_(other._Get_MatchRule()),
				matchAll_(other._Is_MatchAll())
			{ }

			~StringMatcher() { }

			StringMatcher &operator=(const StringMatcher &other)
			{
				if (std::addressof(other) != this) {
					matchRule_ = other._Get_MatchRule();
					matchAll_ = other._Is_MatchAll();
				}
				return *this;
			}

			bool operator==(const StringMatcher &other) const
			{
				return (matchAll_ == other._Is_MatchAll() && matchRule_ == other._Get_MatchRule());
			}

			bool operator!=(const StringMatcher &other) const
			{
				return (matchAll_ != other._Is_MatchAll() || matchRule_ != other._Get_MatchRule());
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
					return (memcmp((s.data() + (s_len - p_len)), p.data(), p_len) == 0);
				};

				if (matchAll_) {
					return true;
				}
				if (str.empty()) {
					return false;
				}
				if (matchRule_.count(MatchIndex::FRONT) == 1) {
					const auto &front = matchRule_.at(MatchIndex::FRONT);
					for (const auto &key : front) {
						if (matchFront(str, key)) {
							return true;
						}
					}
				}
				if (matchRule_.count(MatchIndex::BACK) == 1) {
					const auto &back = matchRule_.at(MatchIndex::BACK);
					for (const auto &key : back) {
						if (matchBack(str, key)) {
							return true;
						}
					}
				}
				if (matchRule_.count(MatchIndex::ENTIRE) == 1) {
					const auto &entire = matchRule_.at(MatchIndex::ENTIRE);
					for (const auto &key : entire) {
						if (str == key) {
							return true;
						}
					}
				}
				if (matchRule_.count(MatchIndex::MIDDLE) == 1) {
					const auto &middle = matchRule_.at(MatchIndex::MIDDLE);
					for (const auto &key : middle) {
						if (str.find(key) != std::string::npos) {
							return true;
						}
					}
				}
				return false;
			}

			std::unordered_map<MatchIndex, std::vector<std::string>> _Get_MatchRule() const
			{
				return matchRule_;
			}

			bool _Is_MatchAll() const
			{
				return matchAll_;
			}

		private:
			std::unordered_map<MatchIndex, std::vector<std::string>> matchRule_;
			bool matchAll_;

			static std::vector<std::string> ParseRuleContent_(const std::string &content)
			{
				std::vector<std::string> rules{};
				size_t pos = 0, len = content.size();
				while (pos < len) {
					auto next_pos = content.find('|', pos);
					if (next_pos == std::string::npos) {
						next_pos = len;
					}
					auto rule = content.substr(pos, next_pos - pos);
					if (rule.find('[') != std::string::npos && rule.find(']') != std::string::npos) {
						auto parsedRules = ParseCharsets_(rule);
						rules.insert(rules.end(), parsedRules.begin(), parsedRules.end());
					} else {
						rules.emplace_back(rule);
					}
					pos = next_pos + 1;
				}
				return rules;
			}

			static std::vector<std::string> ParseCharsets_(const std::string &str)
			{
				std::vector<std::string> parsedRules{};
				parsedRules.emplace_back(str);
				size_t pos = 0;
				while (pos < parsedRules[0].size()) {
					auto set_begin = parsedRules[0].find('[', pos);
					if (set_begin == std::string::npos) {
						break;
					}
					auto set_end = parsedRules[0].find(']', set_begin + 1);
					if (set_end != std::string::npos) {
						auto charSet = GetCharset_(parsedRules[0].substr(set_begin + 1, set_end - set_begin - 1));
						std::vector<std::string> expandedRules{};
						for (const auto &rule : parsedRules) {
							auto frontStr = rule.substr(0, set_begin);
							auto backStr = rule.substr(set_end + 1);
							for (const auto &ch : charSet) {
								expandedRules.emplace_back(frontStr + ch + backStr);
							}
						}
						parsedRules = expandedRules;
					} else {
						break;
					}
					pos = set_begin + 1;
				}
				return parsedRules;
			}

			static std::string GetCharset_(const std::string &content)
			{
				static const auto isCharRange = [](char start_ch, char end_ch) -> bool {
					if (start_ch >= '0' && start_ch <= '9' && end_ch >= '0' && end_ch <= '9') {
						return true;
					}
					if (start_ch >= 'A' && start_ch <= 'Z' && end_ch >= 'A' && end_ch <= 'Z') {
						return true;
					}
					if (start_ch >= 'a' && start_ch <= 'z' && end_ch >= 'a' && end_ch <= 'z') {
						return true;
					}
					return false;
				};

				std::string charSet{};
				for (size_t pos = 0; pos < content.size(); pos++) {
					if (content[pos] == '-' && pos > 0 && (pos + 1) < content.size()) {
						if (isCharRange(content[pos - 1], content[pos + 1])) {
							for (char ch = (content[pos - 1] + 1); ch < content[pos + 1]; ch++) {
								charSet += ch;
							}
						} else {
							charSet += content[pos];
						}
					} else {
						charSet += content[pos];
					}
				}
				return charSet;
			}
	};
}

#endif

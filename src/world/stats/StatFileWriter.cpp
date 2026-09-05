#include "world/stats/StatFileWriter.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "client/User.h"
#include "java/File.h"
#include "world/stats/Achievement.h"
#include "world/stats/StatBase.h"
#include "world/stats/StatList.h"
#include "world/stats/StatsSyncher.h"

namespace
{
	int_t javaIntAdd(int_t left, int_t right)
	{
		uint_t resultBits = static_cast<uint_t>(left) + static_cast<uint_t>(right);
		int_t result;
		std::memcpy(&result, &resultBits, sizeof(result));
		return result;
	}

	class JsonSyntaxError : public std::runtime_error
	{
	public:
		explicit JsonSyntaxError(const char *message) : std::runtime_error(message) {}
	};

	enum class JsonType
	{
		object,
		array,
		string,
		number,
		boolean,
		nullValue
	};

	struct JsonValue
	{
		JsonType type = JsonType::nullValue;
		jstring stringValue;
		std::string numberValue;
		std::vector<std::pair<jstring, JsonValue>> fields;
		std::vector<JsonValue> elements;
	};

	class JsonParser
	{
	private:
		const std::string &input;
		size_t position = 0;

		[[noreturn]] void syntaxError() const
		{
			throw JsonSyntaxError("Invalid JSON syntax");
		}

		bool atEnd() const
		{
			return position >= input.size();
		}

		char peek() const
		{
			if (atEnd())
				syntaxError();
			return input[position];
		}

		char take()
		{
			char value = peek();
			position++;
			return value;
		}

		void skipWhitespace()
		{
			while (!atEnd())
			{
				char value = input[position];
				if (value != '\t' && value != '\n' && value != '\r' && value != ' ')
					break;
				position++;
			}
		}

		void expect(char expected)
		{
			if (take() != expected)
				syntaxError();
		}

		void expect(const char *expected)
		{
			while (*expected != '\0')
			{
				if (take() != *expected)
					syntaxError();
				expected++;
			}
		}

		static int_t hexDigit(char value)
		{
			if (value >= '0' && value <= '9')
				return value - '0';
			if (value >= 'a' && value <= 'f')
				return value - 'a' + 10;
			if (value >= 'A' && value <= 'F')
				return value - 'A' + 10;
			throw JsonSyntaxError("Invalid JSON unicode escape");
		}

		jstring parseString()
		{
			expect('"');
			jstring result;
			while (true)
			{
				char value = take();
				if (value == '"')
					return result;
				if (value != '\\')
				{
					result.push_back(static_cast<unsigned char>(value));
					continue;
				}

				switch (take())
				{
				case '"': result.push_back(u'"'); break;
				case '/': result.push_back(u'/'); break;
				case '\\': result.push_back(u'\\'); break;
				case 'b': result.push_back(u'\b'); break;
				case 'f': result.push_back(u'\f'); break;
				case 'n': result.push_back(u'\n'); break;
				case 'r': result.push_back(u'\r'); break;
				case 't': result.push_back(u'\t'); break;
				case 'u':
				{
					int_t codePoint = 0;
					for (int_t i = 0; i < 4; i++)
						codePoint = codePoint * 16 + hexDigit(take());
					result.push_back(static_cast<char_t>(codePoint));
					break;
				}
				default:
					syntaxError();
				}
			}
		}

		std::string parseNumber()
		{
			std::string result;
			if (peek() == '-')
				result.push_back(take());

			char first = take();
			if (first == '0')
				result.push_back(first);
			else if (first >= '1' && first <= '9')
			{
				result.push_back(first);
				while (!atEnd() && input[position] >= '0' && input[position] <= '9')
					result.push_back(input[position++]);
			}
			else
				syntaxError();

			if (!atEnd() && input[position] == '.')
			{
				result.push_back(input[position++]);
				if (atEnd() || input[position] < '0' || input[position] > '9')
					syntaxError();
				while (!atEnd() && input[position] >= '0' && input[position] <= '9')
					result.push_back(input[position++]);
			}

			
			if (!atEnd() && (input[position] == 'E' || input[position] == '.'))
			{
				position++;
				result.push_back('E');
				if (!atEnd() && (input[position] == '+' || input[position] == '-'))
					result.push_back(input[position++]);
				if (atEnd() || input[position] < '0' || input[position] > '9')
					syntaxError();
				while (!atEnd() && input[position] >= '0' && input[position] <= '9')
					result.push_back(input[position++]);
			}
			return result;
		}

		JsonValue parseArray()
		{
			expect('[');
			JsonValue result;
			result.type = JsonType::array;
			skipWhitespace();
			if (!atEnd() && peek() == ']')
			{
				position++;
				return result;
			}

			while (true)
			{
				result.elements.push_back(parseValue());
				skipWhitespace();
				char separator = take();
				if (separator == ']')
					return result;
				if (separator != ',')
					syntaxError();
				skipWhitespace();
			}
		}

		JsonValue parseObject()
		{
			expect('{');
			JsonValue result;
			result.type = JsonType::object;
			skipWhitespace();
			if (!atEnd() && peek() == '}')
			{
				position++;
				return result;
			}

			while (true)
			{
				if (peek() != '"')
					syntaxError();
				jstring key = parseString();
				skipWhitespace();
				expect(':');
				result.fields.emplace_back(std::move(key), parseValue());
				skipWhitespace();
				char separator = take();
				if (separator == '}')
					return result;
				if (separator != ',')
					syntaxError();
				skipWhitespace();
			}
		}

		JsonValue parseValue()
		{
			skipWhitespace();
			char value = peek();
			if (value == '{')
				return parseObject();
			if (value == '[')
				return parseArray();

			JsonValue result;
			if (value == '"')
			{
				result.type = JsonType::string;
				result.stringValue = parseString();
			}
			else if (value == '-' || (value >= '0' && value <= '9'))
			{
				result.type = JsonType::number;
				result.numberValue = parseNumber();
			}
			else if (value == 't')
			{
				expect("true");
				result.type = JsonType::boolean;
			}
			else if (value == 'f')
			{
				expect("false");
				result.type = JsonType::boolean;
			}
			else if (value == 'n')
			{
				expect("null");
				result.type = JsonType::nullValue;
			}
			else
				syntaxError();
			return result;
		}

	public:
		explicit JsonParser(const std::string &input) : input(input) {}

		JsonValue parse()
		{
			if (atEnd() || (peek() != '{' && peek() != '['))
				syntaxError();
			JsonValue result = parseValue();
			skipWhitespace();
			if (!atEnd())
				syntaxError();
			return result;
		}
	};

	const JsonValue &getField(const JsonValue &object, const jstring &name)
	{
		if (object.type != JsonType::object)
			throw std::runtime_error("Expected JSON object");
		for (auto it = object.fields.rbegin(); it != object.fields.rend(); ++it)
		{
			if (it->first == name)
				return it->second;
		}
		throw std::runtime_error("Missing JSON field");
	}

	std::string getText(const JsonValue &value)
	{
		if (value.type == JsonType::string)
			return String::toUTF8(value.stringValue);
		if (value.type == JsonType::number)
			return value.numberValue;
		throw std::runtime_error("Expected textual JSON value");
	}

	int_t parseJavaInt(const std::string &text)
	{
		if (text.empty())
			throw std::invalid_argument("Invalid Java integer");
		size_t position = 0;
		bool negative = false;
		if (text[position] == '-' || text[position] == '+')
		{
			negative = text[position] == '-';
			if (++position == text.size())
				throw std::invalid_argument("Invalid Java integer");
		}

		uint64_t limit = static_cast<uint64_t>(std::numeric_limits<int_t>::max()) + (negative ? 1ULL : 0ULL);
		uint64_t value = 0;
		for (; position < text.size(); position++)
		{
			char digit = text[position];
			if (digit < '0' || digit > '9')
				throw std::invalid_argument("Invalid Java integer");
			uint64_t numericDigit = static_cast<uint64_t>(digit - '0');
			if (value > (limit - numericDigit) / 10)
				throw std::out_of_range("Java integer overflow");
			value = value * 10 + numericDigit;
		}

		if (negative && value == limit)
			return std::numeric_limits<int_t>::min();
		int_t result = static_cast<int_t>(value);
		return negative ? -result : result;
	}

	uint32_t rotateLeft(uint32_t value, uint32_t amount)
	{
		return (value << amount) | (value >> (32 - amount));
	}

	std::string md5(const std::string &input)
	{
		static const uint32_t shifts[64] = {
			7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
			5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
			4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
			6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
		};
		static const uint32_t constants[64] = {
			0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
			0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
			0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
			0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
			0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
			0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
			0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
			0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
		};

		std::vector<uint8_t> message(input.begin(), input.end());
		uint64_t bitLength = static_cast<uint64_t>(message.size()) * 8;
		message.push_back(0x80);
		while (message.size() % 64 != 56)
			message.push_back(0);
		for (int i = 0; i < 8; ++i)
			message.push_back(static_cast<uint8_t>(bitLength >> (8 * i)));

		uint32_t a0 = 0x67452301;
		uint32_t b0 = 0xefcdab89;
		uint32_t c0 = 0x98badcfe;
		uint32_t d0 = 0x10325476;
		for (size_t offset = 0; offset < message.size(); offset += 64)
		{
			uint32_t words[16];
			for (int i = 0; i < 16; ++i)
			{
				words[i] = static_cast<uint32_t>(message[offset + i * 4]) |
					(static_cast<uint32_t>(message[offset + i * 4 + 1]) << 8) |
					(static_cast<uint32_t>(message[offset + i * 4 + 2]) << 16) |
					(static_cast<uint32_t>(message[offset + i * 4 + 3]) << 24);
			}

			uint32_t a = a0;
			uint32_t b = b0;
			uint32_t c = c0;
			uint32_t d = d0;
			for (uint32_t i = 0; i < 64; ++i)
			{
				uint32_t f;
				uint32_t g;
				if (i < 16)
				{
					f = (b & c) | (~b & d);
					g = i;
				}
				else if (i < 32)
				{
					f = (d & b) | (~d & c);
					g = (5 * i + 1) % 16;
				}
				else if (i < 48)
				{
					f = b ^ c ^ d;
					g = (3 * i + 5) % 16;
				}
				else
				{
					f = c ^ (b | ~d);
					g = (7 * i) % 16;
				}
				uint32_t next = d;
				d = c;
				c = b;
				b += rotateLeft(a + f + constants[i] + words[g], shifts[i]);
				a = next;
			}
			a0 += a;
			b0 += b;
			c0 += c;
			d0 += d;
		}

		std::array<uint8_t, 16> digest{};
		uint32_t values[4] = {a0, b0, c0, d0};
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				digest[i * 4 + j] = static_cast<uint8_t>(values[i] >> (8 * j));

		static const char *digits = "0123456789abcdef";
		std::string result;
		result.reserve(32);
		for (uint8_t byte : digest)
		{
			result.push_back(digits[byte >> 4]);
			result.push_back(digits[byte & 15]);
		}
		size_t first = result.find_first_not_of('0');
		return first == std::string::npos ? "0" : result.substr(first);
	}
}

StatFileWriter::StatFileWriter(const User &user, const File &directory)
{
	std::unique_ptr<File> statsDirectory(File::open(directory, u"stats"));
	if (!statsDirectory->exists())
		statsDirectory->mkdir();

	for (auto &file : directory.listFiles())
	{
		jstring name = file->getName();
		if (name.rfind(u"stats_", 0) == 0 && name.size() >= 4 && name.substr(name.size() - 4) == u".dat")
		{
			std::unique_ptr<File> destination(File::open(*statsDirectory, name));
			if (!destination->exists())
			{
				std::cout << "Relocating " << String::toUTF8(name) << std::endl;
				file->renameTo(*destination);
			}
		}
	}

	statsSyncher = std::make_unique<StatsSyncher>(user, *this, *statsDirectory);
}

StatFileWriter::~StatFileWriter() = default;

void StatFileWriter::addToMap(StatMap &map, const StatBase &stat, int_t amount)
{
	auto current = map.find(&stat);
	int_t value = current == map.end() ? 0 : current->second;
	map[&stat] = javaIntAdd(value, amount);
}

void StatFileWriter::readStat(const StatBase &stat, int_t amount)
{
	addToMap(sessionStats, stat, amount);
	addToMap(totalStats, stat, amount);
	dirty = true;
}

StatMap StatFileWriter::copySessionStats() const
{
	return sessionStats;
}

void StatFileWriter::mergeUnsent(const StatMap &stats)
{
	dirty = true;
	for (const auto &entry : stats)
	{
		addToMap(sessionStats, *entry.first, entry.second);
		addToMap(totalStats, *entry.first, entry.second);
	}
}

void StatFileWriter::mergeLoadedTotal(const StatMap &stats)
{
	for (const auto &entry : stats)
	{
		auto current = sessionStats.find(entry.first);
		totalStats[entry.first] = javaIntAdd(entry.second, current == sessionStats.end() ? 0 : current->second);
	}
}

void StatFileWriter::mergeSessionOnly(const StatMap &stats)
{
	dirty = true;
	for (const auto &entry : stats)
		addToMap(sessionStats, *entry.first, entry.second);
}

std::unique_ptr<StatMap> StatFileWriter::parse(const std::string &json)
{
	std::unique_ptr<StatMap> result = std::make_unique<StatMap>();
	JsonValue root;
	try
	{
		root = JsonParser(json).parse();
	}
	catch (const JsonSyntaxError &e)
	{
		std::cerr << e.what() << std::endl;
		return result;
	}

	const JsonValue &entries = getField(root, u"stats-change");
	if (entries.type != JsonType::array)
		throw std::runtime_error("Expected stats-change array");

	std::string checksumMaterial;
	for (const JsonValue &entry : entries.elements)
	{
		if (entry.type != JsonType::object)
			throw std::runtime_error("Expected stat object");
		if (entry.fields.empty())
			throw std::runtime_error("Expected stat entry");

		const auto &field = entry.fields.front();
		int_t id = parseJavaInt(String::toUTF8(field.first));
		int_t value = parseJavaInt(getText(field.second));
		StatBase *stat = StatList::getStat(id);
		if (stat == nullptr)
		{
			std::cout << id << " is not a valid stat" << std::endl;
			continue;
		}
		checksumMaterial += String::toUTF8(stat->statGuid) + "," + std::to_string(value) + ",";
		(*result)[stat] = value;
	}

	const JsonValue &checksum = getField(root, u"checksum");
	if (checksum.type != JsonType::string)
		throw std::runtime_error("Expected checksum string");
	if (md5("local" + checksumMaterial) != String::toUTF8(checksum.stringValue))
	{
		std::cout << "CHECKSUM MISMATCH" << std::endl;
		return nullptr;
	}
	return result;
}

std::string StatFileWriter::serialize(const jstring *username, const jstring *sessionId, const StatMap &stats)
{
	std::string output = "{\r\n";
	if (username != nullptr && sessionId != nullptr)
	{
		output += "  \"user\":{\r\n";
		output += "    \"name\":\"" + String::toUTF8(*username) + "\",\r\n";
		output += "    \"sessionid\":\"" + String::toUTF8(*sessionId) + "\"\r\n";
		output += "  },\r\n";
	}

	output += "  \"stats-change\":[";
	std::string checksumMaterial;
	bool first = true;
	for (const auto &entry : stats)
	{
		const StatBase *stat = entry.first;
		int_t value = entry.second;
		if (!first)
			output += "},";
		first = false;
		output += "\r\n    {\"" + std::to_string(stat->statId) + "\":" + std::to_string(value);
		checksumMaterial += String::toUTF8(stat->statGuid) + "," + std::to_string(value) + ",";
	}
	if (!first)
		output += "}";

	const std::string salt = sessionId == nullptr ? "" : String::toUTF8(*sessionId);
	output += "\r\n  ],\r\n";
	output += "  \"checksum\":\"" + md5(salt + checksumMaterial) + "\"\r\n";
	output += "}";
	return output;
}

bool StatFileWriter::hasAchievementUnlocked(const Achievement &achievement) const
{
	return totalStats.find(&achievement) != totalStats.end();
}

bool StatFileWriter::canUnlockAchievement(const Achievement &achievement) const
{
	return achievement.parentAchievement == nullptr || hasAchievementUnlocked(*achievement.parentAchievement);
}

int_t StatFileWriter::getStat(const StatBase &stat) const
{
	auto it = totalStats.find(&stat);
	return it == totalStats.end() ? 0 : it->second;
}

void StatFileWriter::syncStats()
{
	statsSyncher->syncStatsFileWithMap(copySessionStats());
}

void StatFileWriter::tick()
{
	if (dirty && statsSyncher->canSave())
		statsSyncher->startSave(copySessionStats());
	statsSyncher->tick();
}

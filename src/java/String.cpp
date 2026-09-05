#include "java/String.h"

#include <cmath>
#include <codecvt>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <type_traits>

namespace String
{

jstring fromUTF8(const std::string &str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.from_bytes(str);
}
std::string toUTF8(const jstring &str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.to_bytes(str);
}

template <typename T>
static jstring intToStringImpl(T v, int_t base)
{
	if (base < 2 || base > 36)
		base = 10;
	jstring out;
	bool negative = v < 0;
	typedef typename std::make_unsigned<T>::type Unsigned;
	Unsigned magnitude = static_cast<Unsigned>(v);
	if (negative)
		magnitude = Unsigned(0) - magnitude;

	while (magnitude)
	{
		int_t digit = static_cast<int_t>(magnitude % static_cast<Unsigned>(base));
		magnitude /= static_cast<Unsigned>(base);
		out.insert(out.begin(), static_cast<char16_t>(digit + (digit < 10 ? '0' : 'a' - 10)));
	}

	if (out.empty())
		out.push_back('0');

	if (negative)
		out.insert(out.begin(), '-');

	return out;
}

jstring toString(int_t v, int_t base)
{
	return intToStringImpl<int_t>(v, base);
}

jstring toString(long_t v, int_t base)
{
	return intToStringImpl<long_t>(v, base);
}

jstring toString(uint_t v, int_t base)
{
	return intToStringImpl<uint_t>(v, base);
}

jstring toString(ulong_t v, int_t base)
{
	return intToStringImpl<ulong_t>(v, base);
}





static bool javaIntegralDigits(ulong_t fractBits, int binExp, int significantBits, std::string &mantissa, int &decExponent)
{
	const int expShift = 52;
	if (binExp > 62 || binExp < -21)
		return false;
	int tailZeros = 0;
	while (((fractBits >> tailZeros) & 1u) == 0)
		tailZeros++;
	int fractionBits = expShift + 1 - tailZeros;
	if (fractionBits - binExp - 1 > 0)
		return false;

	int insignificant = 0;
	if (binExp > significantBits)
	{
		int p2 = binExp - significantBits - 1;
		if (p2 > 1 && p2 < 64)
			insignificant = static_cast<int>(std::floor(p2 * 0.30102999566398120)); 
	}
	ulong_t lvalue = binExp >= expShift ? fractBits << (binExp - expShift) : fractBits >> (expShift - binExp);

	decExponent = 0;
	if (insignificant != 0)
	{
		ulong_t pow10 = 1;
		for (int i = 0; i < insignificant; ++i)
			pow10 *= 10u;
		ulong_t residue = lvalue % pow10;
		lvalue /= pow10;
		decExponent += insignificant;
		if (residue >= (pow10 >> 1))
			lvalue++;
	}
	
	int c = static_cast<int>(lvalue % 10u);
	lvalue /= 10u;
	while (c == 0)
	{
		decExponent++;
		c = static_cast<int>(lvalue % 10u);
		lvalue /= 10u;
	}
	std::string reversed;
	while (lvalue != 0)
	{
		reversed.push_back(static_cast<char>('0' + c));
		decExponent++;
		c = static_cast<int>(lvalue % 10u);
		lvalue /= 10u;
	}
	reversed.push_back(static_cast<char>('0' + c));
	mantissa.assign(reversed.rbegin(), reversed.rend());
	decExponent += 1;
	return true;
}

static bool javaIntegralDigits(double value, std::string &mantissa, int &decExponent)
{
	ulong_t bits;
	std::memcpy(&bits, &value, sizeof bits);
	int expField = static_cast<int>((bits >> 52) & 0x7FFu);
	if (expField == 0)
		return false;
	ulong_t fractBits = (bits & ((1ull << 52) - 1)) | (1ull << 52);
	return javaIntegralDigits(fractBits, expField - 1023, 53, mantissa, decExponent);
}

static bool javaIntegralDigits(float value, std::string &mantissa, int &decExponent)
{
	uint_t bits;
	std::memcpy(&bits, &value, sizeof bits);
	int expField = static_cast<int>((bits >> 23) & 0xFFu);
	if (expField == 0)
		return false;
	
	ulong_t fractBits = static_cast<ulong_t>((bits & 0x7FFFFFu) | 0x800000u) << 29;
	return javaIntegralDigits(fractBits, expField - 127, 24, mantissa, decExponent);
}




template <typename T>
static std::string javaFloatingToString(T value, int maxDigits)
{
	if (std::isnan(value))
		return "NaN";
	if (std::isinf(value))
		return value < 0 ? "-Infinity" : "Infinity";
	if (value == 0)
		return std::signbit(value) ? "-0.0" : "0.0";

	std::string result = value < 0 ? "-" : "";
	T magnitude = value < 0 ? -value : value;

	std::string mantissa;
	int exponent = 0;
	if (javaIntegralDigits(magnitude, mantissa, exponent))
	{
		exponent -= 1; 
	}
	else
	{
		
		char buffer[64];
		int digits = 1;
		for (; digits <= maxDigits; ++digits)
		{
			std::snprintf(buffer, sizeof(buffer), "%.*e", digits - 1, static_cast<double>(magnitude));
			if (static_cast<T>(std::strtod(buffer, nullptr)) == magnitude)
				break;
		}
		if (digits > maxDigits)
			std::snprintf(buffer, sizeof(buffer), "%.*e", maxDigits - 1, static_cast<double>(magnitude));

		
		const char *p = buffer;
		for (; *p != 'e'; ++p)
			if (*p != '.')
				mantissa.push_back(*p);
		exponent = std::atoi(p + 1);
	}
	while (mantissa.size() > 1 && mantissa.back() == '0')
		mantissa.pop_back();

	if (magnitude >= static_cast<T>(1e-3) && magnitude < static_cast<T>(1e7))
	{
		int pointPosition = exponent + 1;
		if (pointPosition <= 0)
		{
			result += "0.";
			result.append(static_cast<size_t>(-pointPosition), '0');
			result += mantissa;
		}
		else if (static_cast<size_t>(pointPosition) >= mantissa.size())
		{
			result += mantissa;
			result.append(static_cast<size_t>(pointPosition) - mantissa.size(), '0');
			result += ".0";
		}
		else
		{
			result += mantissa.substr(0, static_cast<size_t>(pointPosition));
			result += '.';
			result += mantissa.substr(static_cast<size_t>(pointPosition));
		}
		return result;
	}

	result += mantissa[0];
	result += '.';
	result += mantissa.size() > 1 ? mantissa.substr(1) : "0";
	result += 'E';
	result += std::to_string(exponent);
	return result;
}

jstring toString(float v)
{
	return fromUTF8(javaFloatingToString<float>(v, 9));
}

jstring toString(double v)
{
	return fromUTF8(javaFloatingToString<double>(v, 17));
}

}

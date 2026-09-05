#include "java/IOUtil.h"
#include "java/Number.h"

#include <cstring>
#include <stdexcept>

namespace IOUtil
{

static void writeBytes(std::ostream &os, const char *bytes, std::streamsize size)
{
	os.write(bytes, size);
	if (!os)
		throw std::runtime_error("java.io.IOException: write failed");
}

static void readBytes(std::istream &is, char *bytes, std::streamsize size)
{
	if (size == 0)
		return;
	is.read(bytes, size);
	if (is.bad() || (is.fail() && !is.eof()))
		throw std::runtime_error("java.io.IOException: read failed");
	if (is.gcount() != size)
		throw std::runtime_error("java.io.EOFException");
}

static void writeBits(std::ostream &os, ulong_t bits, int count)
{
	unsigned char bytes[8];
	for (int i = count; i != 0; bits >>= 8)
		bytes[--i] = static_cast<unsigned char>(bits);
	writeBytes(os, reinterpret_cast<const char *>(bytes), count);
}

static ulong_t readBits(std::istream &is, int count)
{
	unsigned char bytes[8];
	readBytes(is, reinterpret_cast<char *>(bytes), count);
	ulong_t bits = 0;
	for (int i = 0; i < count; ++i)
		bits = (bits << 8) | bytes[i];
	return bits;
}

void writeBoolean(std::ostream &os, bool b)
{
	writeBits(os, b ? 1 : 0, 1);
}
void writeByte(std::ostream &os, int_t b)
{
	writeBits(os, static_cast<uint_t>(b), 1);
}
void writeChar(std::ostream &os, int_t c)
{
	writeBits(os, static_cast<uint_t>(c), 2);
}
void writeShort(std::ostream &os, int_t s)
{
	writeBits(os, static_cast<uint_t>(s), 2);
}
void writeInt(std::ostream &os, int_t i)
{
	writeBits(os, static_cast<uint_t>(i), 4);
}
void writeLong(std::ostream &os, long_t l)
{
	writeBits(os, static_cast<ulong_t>(l), 8);
}
void writeFloat(std::ostream &os, float f)
{
	uint_t bits;
	std::memcpy(&bits, &f, sizeof(bits));
	if ((bits & 0x7fffffffU) > 0x7f800000U)
		bits = 0x7fc00000U;
	writeBits(os, bits, 4);
}
void writeDouble(std::ostream &os, double d)
{
	ulong_t bits;
	std::memcpy(&bits, &d, sizeof(bits));
	if ((bits & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL)
		bits = 0x7ff8000000000000ULL;
	writeBits(os, bits, 8);
}
void writeUTF(std::ostream &os, const jstring &str)
{
	size_t length = 0;
	for (char16_t c : str)
	{
		length += c >= 1 && c <= 0x7f ? 1 : c <= 0x7ff ? 2 : 3;
		if (length > 65535)
			throw std::runtime_error("java.io.UTFDataFormatException: encoded string too long");
	}

	std::vector<unsigned char> bytes;
	bytes.reserve(length + 2);
	bytes.push_back(static_cast<unsigned char>(length >> 8));
	bytes.push_back(static_cast<unsigned char>(length));
	for (char16_t c : str)
	{
		if (c >= 1 && c <= 0x7f)
			bytes.push_back(static_cast<unsigned char>(c));
		else
		{
			if (c > 0x7ff)
			{
				bytes.push_back(static_cast<unsigned char>(0xe0 | (c >> 12)));
				bytes.push_back(static_cast<unsigned char>(0x80 | ((c >> 6) & 0x3f)));
			}
			else
				bytes.push_back(static_cast<unsigned char>(0xc0 | (c >> 6)));
			bytes.push_back(static_cast<unsigned char>(0x80 | (c & 0x3f)));
		}
	}
	writeBytes(os, reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

bool readBoolean(std::istream &is)
{
	return readBits(is, 1) != 0;
}

byte_t readByte(std::istream &is)
{
	int value = static_cast<int>(readBits(is, 1));
	return static_cast<byte_t>(value < 0x80 ? value : value - 0x100);
}

char_t readChar(std::istream &is)
{
	return static_cast<char_t>(readBits(is, 2));
}

short_t readShort(std::istream &is)
{
	int value = static_cast<int>(readBits(is, 2));
	return static_cast<short_t>(value < 0x8000 ? value : value - 0x10000);
}

int_t readInt(std::istream &is)
{
	return Java::intFromBits(static_cast<uint_t>(readBits(is, 4)));
}

long_t readLong(std::istream &is)
{
	return Java::longFromBits(readBits(is, 8));
}

float readFloat(std::istream &is)
{
	uint_t bits = static_cast<uint_t>(readBits(is, 4));
	float result;
	std::memcpy(&result, &bits, sizeof(result));
	return result;
}

double readDouble(std::istream &is)
{
	ulong_t bits = readBits(is, 8);
	double result;
	std::memcpy(&result, &bits, sizeof(result));
	return result;
}

jstring readUTF(std::istream &is)
{
	size_t length = readChar(is);
	std::vector<unsigned char> bytes(length);
	readBytes(is, reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(length));
	jstring result;
	result.reserve(length);
	for (size_t i = 0; i < length;)
	{
		unsigned int c = bytes[i++];
		if (c <= 0x7f)
			result.push_back(static_cast<char16_t>(c));
		else
		{
			int continuation = c >= 0xc0 && c <= 0xdf ? 1 : c >= 0xe0 && c <= 0xef ? 2 : -1;
			if (continuation < 0 || length - i < static_cast<size_t>(continuation))
				throw std::runtime_error("java.io.UTFDataFormatException: malformed input");
			c &= continuation == 1 ? 0x1f : 0x0f;
			while (continuation-- != 0)
			{
				unsigned int next = bytes[i++];
				if ((next & 0xc0) != 0x80)
					throw std::runtime_error("java.io.UTFDataFormatException: malformed input");
				c = (c << 6) | (next & 0x3f);
			}
			result.push_back(static_cast<char16_t>(c));
		}
	}
	return result;
}

std::vector<char> readAllBytes(std::istream &is)
{
	std::vector<char> result;
	char bytes[8192];
	for (;;)
	{
		is.read(bytes, sizeof(bytes));
		result.insert(result.end(), bytes, bytes + is.gcount());
		if (is.bad() || (is.fail() && !is.eof()))
			throw std::runtime_error("java.io.IOException: read failed");
		if (is.eof())
			return result;
	}
}

}

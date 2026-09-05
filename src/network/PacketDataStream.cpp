#include "network/PacketDataStream.h"

#include <cmath>
#include <cstring>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

PacketEndOfStream::PacketEndOfStream() : std::runtime_error("Reached end of stream")
{
}

PacketDataInput::PacketDataInput(std::istream &input) : input(input)
{
}

int_t PacketDataInput::read()
{
	int value = input.get();
	return value == std::char_traits<char>::eof() ? -1 : value;
}

void PacketDataInput::readFully(void *data, std::size_t size)
{
	if (size == 0)
		return;
	input.read(static_cast<char *>(data), static_cast<std::streamsize>(size));
	if (input.gcount() != static_cast<std::streamsize>(size))
		throw PacketEndOfStream();
}

bool PacketDataInput::readBoolean()
{
	return readUnsignedByte() != 0;
}

byte_t PacketDataInput::readByte()
{
	ubyte_t value = readUnsignedByte();
	byte_t result;
	std::memcpy(&result, &value, sizeof(result));
	return result;
}

ubyte_t PacketDataInput::readUnsignedByte()
{
	int_t value = read();
	if (value < 0)
		throw PacketEndOfStream();
	return static_cast<ubyte_t>(value);
}

short_t PacketDataInput::readShort()
{
	ushort_t value = readUnsignedShort();
	short_t result;
	std::memcpy(&result, &value, sizeof(result));
	return result;
}

ushort_t PacketDataInput::readUnsignedShort()
{
	uint_t value = static_cast<uint_t>(readUnsignedByte()) << 8;
	value |= readUnsignedByte();
	return static_cast<ushort_t>(value);
}

uchar_t PacketDataInput::readChar()
{
	return readUnsignedShort();
}

int_t PacketDataInput::readInt()
{
	uint_t value = static_cast<uint_t>(readUnsignedByte()) << 24;
	value |= static_cast<uint_t>(readUnsignedByte()) << 16;
	value |= static_cast<uint_t>(readUnsignedByte()) << 8;
	value |= readUnsignedByte();
	int_t result;
	std::memcpy(&result, &value, sizeof(result));
	return result;
}

long_t PacketDataInput::readLong()
{
	ulong_t value = static_cast<ulong_t>(readUnsignedByte()) << 56;
	value |= static_cast<ulong_t>(readUnsignedByte()) << 48;
	value |= static_cast<ulong_t>(readUnsignedByte()) << 40;
	value |= static_cast<ulong_t>(readUnsignedByte()) << 32;
	value |= static_cast<ulong_t>(readUnsignedByte()) << 24;
	value |= static_cast<ulong_t>(readUnsignedByte()) << 16;
	value |= static_cast<ulong_t>(readUnsignedByte()) << 8;
	value |= readUnsignedByte();
	long_t result;
	std::memcpy(&result, &value, sizeof(result));
	return result;
}

float PacketDataInput::readFloat()
{
	int_t bits = readInt();
	float value;
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

double PacketDataInput::readDouble()
{
	long_t bits = readLong();
	double value;
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

jstring PacketDataInput::readUTF()
{
	std::size_t utfLength = readUnsignedShort();
	std::vector<ubyte_t> bytes(utfLength);
	readFully(bytes.data(), bytes.size());

	jstring result;
	result.reserve(utfLength);
	std::size_t count = 0;
	while (count < utfLength)
	{
		int_t value = bytes[count] & 0xff;
		if (value > 127)
			break;
		++count;
		result.push_back(static_cast<uchar_t>(value));
	}

	while (count < utfLength)
	{
		int_t value = bytes[count] & 0xff;
		switch (value >> 4)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			++count;
			result.push_back(static_cast<uchar_t>(value));
			break;
		case 12:
		case 13:
		{
			count += 2;
			if (count > utfLength)
				throw std::runtime_error("malformed input: partial character at end");
			int_t second = bytes[count - 1];
			if ((second & 0xc0) != 0x80)
				throw std::runtime_error("malformed input around byte " + std::to_string(count));
			result.push_back(static_cast<uchar_t>(((value & 0x1f) << 6) | (second & 0x3f)));
			break;
		}
		case 14:
		{
			count += 3;
			if (count > utfLength)
				throw std::runtime_error("malformed input: partial character at end");
			int_t second = bytes[count - 2];
			int_t third = bytes[count - 1];
			if ((second & 0xc0) != 0x80 || (third & 0xc0) != 0x80)
				throw std::runtime_error("malformed input around byte " + std::to_string(count - 1));
			result.push_back(static_cast<uchar_t>(((value & 0x0f) << 12) |
				((second & 0x3f) << 6) | (third & 0x3f)));
			break;
		}
		default:
			throw std::runtime_error("malformed input around byte " + std::to_string(count));
		}
	}

	return result;
}

PacketDataOutput::PacketDataOutput(std::ostream &output) : output(output)
{
}

void PacketDataOutput::write(int_t value)
{
	output.put(static_cast<char>(static_cast<ubyte_t>(value)));
	if (!output)
		throw std::runtime_error("Failed to write packet data");
}

void PacketDataOutput::write(const void *data, std::size_t size)
{
	if (size == 0)
		return;
	output.write(static_cast<const char *>(data), static_cast<std::streamsize>(size));
	if (!output)
		throw std::runtime_error("Failed to write packet data");
}

void PacketDataOutput::writeBoolean(bool value)
{
	write(value ? 1 : 0);
}

void PacketDataOutput::writeByte(int_t value)
{
	write(value);
}

void PacketDataOutput::writeShort(int_t value)
{
	uint_t bits = static_cast<uint_t>(value);
	write(static_cast<int_t>(bits >> 8));
	write(static_cast<int_t>(bits));
}

void PacketDataOutput::writeChar(int_t value)
{
	writeShort(value);
}

void PacketDataOutput::writeInt(int_t value)
{
	uint_t bits = static_cast<uint_t>(value);
	write(static_cast<int_t>(bits >> 24));
	write(static_cast<int_t>(bits >> 16));
	write(static_cast<int_t>(bits >> 8));
	write(static_cast<int_t>(bits));
}

void PacketDataOutput::writeLong(long_t value)
{
	ulong_t bits = static_cast<ulong_t>(value);
	write(static_cast<int_t>(bits >> 56));
	write(static_cast<int_t>(bits >> 48));
	write(static_cast<int_t>(bits >> 40));
	write(static_cast<int_t>(bits >> 32));
	write(static_cast<int_t>(bits >> 24));
	write(static_cast<int_t>(bits >> 16));
	write(static_cast<int_t>(bits >> 8));
	write(static_cast<int_t>(bits));
}

void PacketDataOutput::writeFloat(float value)
{
	if (std::isnan(value))
	{
		writeInt(0x7fc00000);
		return;
	}

	int_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	writeInt(bits);
}

void PacketDataOutput::writeDouble(double value)
{
	if (std::isnan(value))
	{
		writeLong(0x7ff8000000000000LL);
		return;
	}

	long_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	writeLong(bits);
}

void PacketDataOutput::writeUTF(const jstring &value)
{
	std::size_t utfLength = 0;
	for (uchar_t character : value)
	{
		if (character >= 0x0001 && character <= 0x007f)
			++utfLength;
		else if (character > 0x07ff)
			utfLength += 3;
		else
			utfLength += 2;
	}

	if (utfLength > 65535)
		throw std::runtime_error("encoded string too long: " + std::to_string(utfLength) + " bytes");

	writeShort(static_cast<int_t>(utfLength));
	for (uchar_t character : value)
	{
		if (character >= 0x0001 && character <= 0x007f)
		{
			write(character);
		}
		else if (character > 0x07ff)
		{
			write(0xe0 | ((character >> 12) & 0x0f));
			write(0x80 | ((character >> 6) & 0x3f));
			write(0x80 | (character & 0x3f));
		}
		else
		{
			write(0xc0 | ((character >> 6) & 0x1f));
			write(0x80 | (character & 0x3f));
		}
	}
}

void PacketDataOutput::flush()
{
	output.flush();
	if (!output)
		throw std::runtime_error("Failed to flush packet data");
}

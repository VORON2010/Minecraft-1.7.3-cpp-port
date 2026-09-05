#pragma once

#include <cstddef>
#include <iosfwd>
#include <stdexcept>

#include "java/String.h"
#include "java/Type.h"

class PacketEndOfStream : public std::runtime_error
{
public:
	PacketEndOfStream();
};

class PacketDataInput
{
private:
	std::istream &input;

public:
	explicit PacketDataInput(std::istream &input);

	int_t read();
	void readFully(void *data, std::size_t size);
	bool readBoolean();
	byte_t readByte();
	ubyte_t readUnsignedByte();
	short_t readShort();
	ushort_t readUnsignedShort();
	uchar_t readChar();
	int_t readInt();
	long_t readLong();
	float readFloat();
	double readDouble();
	jstring readUTF();
};

class PacketDataOutput
{
private:
	std::ostream &output;

public:
	explicit PacketDataOutput(std::ostream &output);

	void write(int_t value);
	void write(const void *data, std::size_t size);
	void writeBoolean(bool value);
	void writeByte(int_t value);
	void writeShort(int_t value);
	void writeChar(int_t value);
	void writeInt(int_t value);
	void writeLong(long_t value);
	void writeFloat(float value);
	void writeDouble(double value);
	void writeUTF(const jstring &value);
	void flush();
};

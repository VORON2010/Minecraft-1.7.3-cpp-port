#include "nbt/ByteArrayTag.h"

#include "java/IOUtil.h"

#include <stdexcept>

ByteArrayTag::ByteArrayTag()
{

}

ByteArrayTag::ByteArrayTag(std::vector<byte_t> &&data)
{
	this->data = data;
}

void ByteArrayTag::write(std::ostream &os)
{
	IOUtil::writeInt(os, data.size());
	os.write(reinterpret_cast<const char*>(data.data()), data.size());
	if (!os)
		throw std::runtime_error("java.io.IOException: write failed");
}

void ByteArrayTag::load(std::istream &is)
{
	int_t size = IOUtil::readInt(is);
	if (size < 0)
		throw std::runtime_error("java.lang.NegativeArraySizeException");
	data.resize(static_cast<size_t>(size));
	if (size != 0)
		is.read(reinterpret_cast<char*>(data.data()), size);
	if (is.bad() || (is.fail() && !is.eof()))
		throw std::runtime_error("java.io.IOException: read failed");
	if (size != 0 && is.gcount() != size)
		throw std::runtime_error("java.io.EOFException");
}

byte_t ByteArrayTag::getId() const
{
	return TAG_Byte_Array;
}

jstring ByteArrayTag::toString() const
{
	return u"[" + String::fromUTF8(std::to_string(data.size())) + u" bytes]";
}

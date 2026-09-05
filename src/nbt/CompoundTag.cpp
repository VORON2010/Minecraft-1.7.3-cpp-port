#include "nbt/CompoundTag.h"

#include "nbt/ByteTag.h"
#include "nbt/ShortTag.h"
#include "nbt/IntTag.h"
#include "nbt/LongTag.h"
#include "nbt/FloatTag.h"
#include "nbt/DoubleTag.h"
#include "nbt/StringTag.h"
#include "nbt/ByteArrayTag.h"
#include "nbt/ListTag.h"

#include "java/IOUtil.h"

void CompoundTag::write(std::ostream &os)
{
	for (auto &tag : tags)
		Tag::writeNamedTag(*tag.second, os);
	IOUtil::writeByte(os, TAG_End);
}

void CompoundTag::load(std::istream &is)
{
	tags.clear();
	while (1)
	{
		std::unique_ptr<Tag> tag(Tag::readNamedTag(is));
		if (tag->getId() == TAG_End)
			break;
		tags.emplace(tag->getName(), std::move(tag));
	}
}

byte_t CompoundTag::getId() const
{
	return TAG_Compound;
}

void CompoundTag::put(const jstring &name, std::shared_ptr<Tag> tag)
{
	tag->setName(name);
	tags[name] = std::move(tag);
}

void CompoundTag::putByte(const jstring &name, byte_t value)
{
	put(name, Util::make_shared<ByteTag>(value));
}

void CompoundTag::putShort(const jstring &name, short_t value)
{
	put(name, Util::make_shared<ShortTag>(value));
}

void CompoundTag::putInt(const jstring &name, int_t value)
{
	put(name, Util::make_shared<IntTag>(value));
}

void CompoundTag::putLong(const jstring &name, long_t value)
{
	put(name, Util::make_shared<LongTag>(value));
}

void CompoundTag::putFloat(const jstring &name, float value)
{
	put(name, Util::make_shared<FloatTag>(value));
}

void CompoundTag::putDouble(const jstring &name, double value)
{
	put(name, Util::make_shared<DoubleTag>(value));
}

void CompoundTag::putString(const jstring &name, const jstring &value)
{
	put(name, Util::make_shared<StringTag>(value));
}

void CompoundTag::putByteArray(const jstring &name, std::vector<byte_t> &&value)
{
	put(name, Util::make_shared<ByteArrayTag>(std::move(value)));
}

void CompoundTag::putCompound(const jstring &name, std::unique_ptr<CompoundTag> &&value)
{
	value->setName(name);
	tags[name] = std::shared_ptr<Tag>(std::move(value));
}

void CompoundTag::putBoolean(const jstring &name, bool value)
{
	putByte(name, value ? 1 : 0);
}


std::shared_ptr<Tag> CompoundTag::get(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end())
		return nullptr;
	return it->second;
}

bool CompoundTag::contains(const jstring &name) const
{
	return tags.find(name) != tags.end();
}

byte_t CompoundTag::getByte(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end() || it->second->getId() != TAG_Byte) return 0;
	return static_cast<ByteTag *>(it->second.get())->data;
}

short_t CompoundTag::getShort(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end() || it->second->getId() != TAG_Short) return 0;
	return static_cast<ShortTag *>(it->second.get())->data;
}

int_t CompoundTag::getInt(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end() || it->second->getId() != TAG_Int)
		return 0;
	return static_cast<IntTag *>(it->second.get())->data;
}

long_t CompoundTag::getLong(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end() || it->second->getId() != TAG_Long) return 0;
	return static_cast<LongTag *>(it->second.get())->data;
}

float CompoundTag::getFloat(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end() || it->second->getId() != TAG_Float) return 0;
	return static_cast<FloatTag *>(it->second.get())->data;
}

double CompoundTag::getDouble(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end() || it->second->getId() != TAG_Double) return 0;
	return static_cast<DoubleTag *>(it->second.get())->data;
}

jstring CompoundTag::getString(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end() || it->second->getId() != TAG_String) return u"";
	return static_cast<StringTag *>(it->second.get())->data;
}

const std::vector<byte_t> &CompoundTag::getByteArray(const jstring &name) const
{
	static std::vector<byte_t> empty;
	auto it = tags.find(name);
	if (it == tags.end()) return empty;
	if (it->second->getId() != TAG_Byte_Array) return empty;
	return static_cast<ByteArrayTag *>(it->second.get())->data;
}

std::shared_ptr<CompoundTag> CompoundTag::getCompound(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end() || it->second->getId() != TAG_Compound) return Util::make_shared<CompoundTag>();
	return std::static_pointer_cast<CompoundTag>(it->second);
}

std::shared_ptr<ListTag> CompoundTag::getList(const jstring &name) const
{
	auto it = tags.find(name);
	if (it == tags.end() || it->second->getId() != TAG_List) return Util::make_shared<ListTag>();
	return std::static_pointer_cast<ListTag>(it->second);
}

bool CompoundTag::getBoolean(const jstring &name) const
{
	return getByte(name) != 0;
}

jstring CompoundTag::toString() const
{
	return String::fromUTF8(std::to_string(tags.size())) + u" entries";
}

void CompoundTag::print(const std::string &indent, std::ostream &os) const
{
	Tag::print(indent, os);
	
	os << indent << "{\n";

	std::string new_indent = indent + "  ";
	for (const auto &tag : tags)
	{
		tag.second->print(new_indent, os);
	}

	os << indent << "}\n";
}

bool CompoundTag::isEmpty()
{
	return tags.empty();
}

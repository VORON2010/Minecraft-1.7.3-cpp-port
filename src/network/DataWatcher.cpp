#include "network/DataWatcher.h"

#include <cmath>
#include <cstring>
#include <stdexcept>
#include <utility>

#include "network/Packet.h"
#include "network/PacketDataStream.h"
#include "world/item/Item.h"
#include "world/level/tile/Tile.h"

WatchableObject::WatchableObject(int_t objectType, int_t dataValueId, byte_t value)
	: objectType(objectType), dataValueId(dataValueId), runtimeType(0), byteValue(value)
{
}

WatchableObject::WatchableObject(int_t objectType, int_t dataValueId, short_t value)
	: objectType(objectType), dataValueId(dataValueId), runtimeType(1), shortValue(value)
{
}

WatchableObject::WatchableObject(int_t objectType, int_t dataValueId, int_t value)
	: objectType(objectType), dataValueId(dataValueId), runtimeType(2), intValue(value)
{
}

WatchableObject::WatchableObject(int_t objectType, int_t dataValueId, float value)
	: objectType(objectType), dataValueId(dataValueId), runtimeType(3), floatValue(value)
{
}

WatchableObject::WatchableObject(int_t objectType, int_t dataValueId, const jstring &value)
	: objectType(objectType), dataValueId(dataValueId), runtimeType(4), stringValue(value)
{
}

WatchableObject::WatchableObject(int_t objectType, int_t dataValueId, const ItemInstance &value)
	: WatchableObject(objectType, dataValueId, std::make_shared<ItemInstance>(value))
{
}

WatchableObject::WatchableObject(int_t objectType, int_t dataValueId,
	std::shared_ptr<ItemInstance> value)
	: objectType(objectType), dataValueId(dataValueId), runtimeType(5),
	  itemValue(std::move(value))
{
	if (!itemValue)
		throw std::runtime_error("java.lang.NullPointerException");
}

WatchableObject::WatchableObject(int_t objectType, int_t dataValueId, const TilePos &value)
	: WatchableObject(objectType, dataValueId, std::make_shared<TilePos>(value))
{
}

WatchableObject::WatchableObject(int_t objectType, int_t dataValueId,
	std::shared_ptr<TilePos> value)
	: objectType(objectType), dataValueId(dataValueId), runtimeType(6),
	  coordinateValue(std::move(value))
{
	if (!coordinateValue)
		throw std::runtime_error("java.lang.NullPointerException");
}

int_t WatchableObject::getDataValueId() const
{
	return dataValueId;
}

int_t WatchableObject::getObjectType() const
{
	return objectType;
}

int_t WatchableObject::getRuntimeType() const
{
	return runtimeType;
}

void WatchableObject::setWatching(bool value)
{
	watching = value;
}

byte_t WatchableObject::getByte() const
{
	return byteValue;
}

short_t WatchableObject::getShort() const
{
	return shortValue;
}

int_t WatchableObject::getInt() const
{
	return intValue;
}

float WatchableObject::getFloat() const
{
	return floatValue;
}

const jstring &WatchableObject::getString() const
{
	return stringValue;
}

const ItemInstance &WatchableObject::getItem() const
{
	return *itemValue;
}

TilePos WatchableObject::getCoordinates() const
{
	return *coordinateValue;
}

bool WatchableObject::hasSameObject(const WatchableObject &other) const
{
	if (runtimeType != other.runtimeType)
		return false;
	switch (runtimeType)
	{
		case 0:
			return byteValue == other.byteValue;
		case 1:
			return shortValue == other.shortValue;
		case 2:
			return intValue == other.intValue;
		case 3:
		{
			if (std::isnan(floatValue) && std::isnan(other.floatValue))
				return true;
			uint_t left;
			uint_t right;
			std::memcpy(&left, &floatValue, sizeof(left));
			std::memcpy(&right, &other.floatValue, sizeof(right));
			return left == right;
		}
		case 4:
			return stringValue == other.stringValue;
		case 5:
			return itemValue == other.itemValue;
		case 6:
			return *coordinateValue == *other.coordinateValue;
	}
	return false;
}

void WatchableObject::copyObjectFrom(const WatchableObject &other)
{
	byteValue = other.byteValue;
	shortValue = other.shortValue;
	intValue = other.intValue;
	floatValue = other.floatValue;
	stringValue = other.stringValue;
	itemValue = other.itemValue;
	coordinateValue = other.coordinateValue;
	runtimeType = other.runtimeType;
}

uint_t DataWatcher::javaHash(int_t key)
{
	uint_t hash = static_cast<uint_t>(key);
	hash ^= (hash >> 20) ^ (hash >> 12);
	return hash ^ (hash >> 7) ^ (hash >> 4);
}

int_t DataWatcher::findEntry(int_t dataValueId) const
{
	uint_t hash = javaHash(dataValueId);
	int_t index = bucketHeads[hash & (bucketHeads.size() - 1)];
	while (index >= 0)
	{
		const WatchedEntry &entry = watchedObjects[static_cast<size_t>(index)];
		if (entry.hash == hash && entry.key == dataValueId)
			return index;
		index = entry.next;
	}
	return -1;
}

void DataWatcher::resize(int_t capacity)
{
	std::vector<int_t> newBucketHeads(static_cast<size_t>(capacity), -1);
	for (int_t bucketHead : bucketHeads)
	{
		int_t entryIndex = bucketHead;
		while (entryIndex >= 0)
		{
			WatchedEntry &entry = watchedObjects[static_cast<size_t>(entryIndex)];
			int_t next = entry.next;
			int_t bucket = static_cast<int_t>(entry.hash & (newBucketHeads.size() - 1));
			entry.next = newBucketHeads[static_cast<size_t>(bucket)];
			newBucketHeads[static_cast<size_t>(bucket)] = entryIndex;
			entryIndex = next;
		}
	}
	bucketHeads = std::move(newBucketHeads);
	resizeThreshold = capacity * 3 / 4;
}

void DataWatcher::addObject(int_t dataValueId, const std::shared_ptr<WatchableObject> &object)
{
	if (dataValueId > 31)
	{
		throw std::invalid_argument("Data value id is too big with " +
			std::to_string(dataValueId) + "! (Max is 31)");
	}
	if (findEntry(dataValueId) >= 0)
		throw std::invalid_argument("Duplicate id value for " + std::to_string(dataValueId) + "!");
	uint_t hash = javaHash(dataValueId);
	int_t bucket = static_cast<int_t>(hash & (bucketHeads.size() - 1));
	int_t sizeBefore = static_cast<int_t>(watchedObjects.size());
	watchedObjects.push_back({hash, dataValueId, object, bucketHeads[static_cast<size_t>(bucket)]});
	bucketHeads[static_cast<size_t>(bucket)] = sizeBefore;
	if (sizeBefore >= resizeThreshold)
		resize(static_cast<int_t>(bucketHeads.size() * 2));
}

void DataWatcher::updateObject(int_t dataValueId, const WatchableObject &object)
{
	int_t entry = findEntry(dataValueId);
	if (entry < 0)
		throw std::out_of_range("Data value id is not present");
	std::shared_ptr<WatchableObject> watchedObject = watchedObjects[static_cast<size_t>(entry)].value;
	if (!object.hasSameObject(*watchedObject))
	{
		watchedObject->copyObjectFrom(object);
		watchedObject->setWatching(true);
		objectChanged = true;
	}
}

void DataWatcher::addObject(int_t dataValueId, byte_t value)
{
	addObject(dataValueId, std::make_shared<WatchableObject>(0, dataValueId, value));
}

void DataWatcher::addObject(int_t dataValueId, short_t value)
{
	addObject(dataValueId, std::make_shared<WatchableObject>(1, dataValueId, value));
}

void DataWatcher::addObject(int_t dataValueId, int_t value)
{
	addObject(dataValueId, std::make_shared<WatchableObject>(2, dataValueId, value));
}

void DataWatcher::addObject(int_t dataValueId, float value)
{
	addObject(dataValueId, std::make_shared<WatchableObject>(3, dataValueId, value));
}

void DataWatcher::addObject(int_t dataValueId, const jstring &value)
{
	addObject(dataValueId, std::make_shared<WatchableObject>(4, dataValueId, value));
}

void DataWatcher::addObject(int_t dataValueId, const ItemInstance &value)
{
	addObject(dataValueId, std::make_shared<WatchableObject>(5, dataValueId, value));
}

void DataWatcher::addObject(int_t dataValueId, std::shared_ptr<ItemInstance> value)
{
	addObject(dataValueId, std::make_shared<WatchableObject>(5, dataValueId, std::move(value)));
}

void DataWatcher::addObject(int_t dataValueId, const TilePos &value)
{
	addObject(dataValueId, std::make_shared<WatchableObject>(6, dataValueId, value));
}

void DataWatcher::addObject(int_t dataValueId, std::shared_ptr<TilePos> value)
{
	addObject(dataValueId, std::make_shared<WatchableObject>(6, dataValueId, std::move(value)));
}

void DataWatcher::updateObject(int_t dataValueId, byte_t value)
{
	updateObject(dataValueId, WatchableObject(0, dataValueId, value));
}

void DataWatcher::updateObject(int_t dataValueId, short_t value)
{
	updateObject(dataValueId, WatchableObject(1, dataValueId, value));
}

void DataWatcher::updateObject(int_t dataValueId, int_t value)
{
	updateObject(dataValueId, WatchableObject(2, dataValueId, value));
}

void DataWatcher::updateObject(int_t dataValueId, float value)
{
	updateObject(dataValueId, WatchableObject(3, dataValueId, value));
}

void DataWatcher::updateObject(int_t dataValueId, const jstring &value)
{
	updateObject(dataValueId, WatchableObject(4, dataValueId, value));
}

void DataWatcher::updateObject(int_t dataValueId, const ItemInstance &value)
{
	updateObject(dataValueId, WatchableObject(5, dataValueId, value));
}

void DataWatcher::updateObject(int_t dataValueId, std::shared_ptr<ItemInstance> value)
{
	updateObject(dataValueId, WatchableObject(5, dataValueId, std::move(value)));
}

void DataWatcher::updateObject(int_t dataValueId, const TilePos &value)
{
	updateObject(dataValueId, WatchableObject(6, dataValueId, value));
}

void DataWatcher::updateObject(int_t dataValueId, std::shared_ptr<TilePos> value)
{
	updateObject(dataValueId, WatchableObject(6, dataValueId, std::move(value)));
}

byte_t DataWatcher::getWatchableObjectByte(int_t dataValueId) const
{
	int_t entry = findEntry(dataValueId);
	if (entry < 0)
		throw std::out_of_range("Data value id is not present");
	return watchedObjects[static_cast<size_t>(entry)].value->getByte();
}

int_t DataWatcher::getWatchableObjectInt(int_t dataValueId) const
{
	int_t entry = findEntry(dataValueId);
	if (entry < 0)
		throw std::out_of_range("Data value id is not present");
	return watchedObjects[static_cast<size_t>(entry)].value->getInt();
}

const jstring &DataWatcher::getWatchableObjectString(int_t dataValueId) const
{
	int_t entry = findEntry(dataValueId);
	if (entry < 0)
		throw std::out_of_range("Data value id is not present");
	return watchedObjects[static_cast<size_t>(entry)].value->getString();
}

void DataWatcher::writeWatchableObject(PacketDataOutput &output, const WatchableObject &object)
{
	int_t type = object.getObjectType();
	int_t header = static_cast<int_t>((static_cast<uint_t>(type) << 5
		| (static_cast<uint_t>(object.getDataValueId()) & 31U)) & 255U);
	output.writeByte(header);
	if (type >= 0 && type <= 6 && object.getRuntimeType() != type)
		throw std::runtime_error("java.lang.ClassCastException");
	switch (object.getObjectType())
	{
		case 0:
			output.writeByte(object.getByte());
			break;
		case 1:
			output.writeShort(object.getShort());
			break;
		case 2:
			output.writeInt(object.getInt());
			break;
		case 3:
			output.writeFloat(object.getFloat());
			break;
		case 4:
			Packet::writeString(object.getString(), output);
			break;
		case 5:
		{
			const ItemInstance &item = object.getItem();
			int_t id = item.itemID;
			Item *itemType = item.getItem();
			bool tileItem = id > 0 && id < static_cast<int_t>(Tile::tiles.size())
				&& Tile::tiles[id] != nullptr;
			if (itemType == nullptr && !tileItem)
				throw std::runtime_error("java.lang.NullPointerException");
			output.writeShort(itemType == nullptr ? id : itemType->getShiftedIndex());
			output.writeByte(item.stackSize);
			output.writeShort(item.getAuxValue());
			break;
		}
		case 6:
		{
			TilePos coordinates = object.getCoordinates();
			output.writeInt(coordinates.x);
			output.writeInt(coordinates.y);
			output.writeInt(coordinates.z);
			break;
		}
	}
}

void DataWatcher::writeObjectsInListToStream(const WatchableObjectList *objects,
	PacketDataOutput &output)
{
	if (objects != nullptr)
	{
		for (const auto &object : *objects)
			writeWatchableObject(output, *object);
	}
	output.writeByte(127);
}

void DataWatcher::writeWatchableObjects(PacketDataOutput &output) const
{
	for (int_t bucketHead : bucketHeads)
	{
		for (int_t entryIndex = bucketHead; entryIndex >= 0;
			entryIndex = watchedObjects[static_cast<size_t>(entryIndex)].next)
		{
			writeWatchableObject(output,
				*watchedObjects[static_cast<size_t>(entryIndex)].value);
		}
	}
	output.writeByte(127);
}

std::unique_ptr<WatchableObjectList> DataWatcher::readWatchableObjects(PacketDataInput &input)
{
	std::unique_ptr<WatchableObjectList> objects;
	for (byte_t header = input.readByte(); header != 127; header = input.readByte())
	{
		if (!objects)
			objects = std::make_unique<WatchableObjectList>();

		int_t objectType = (header & 224) >> 5;
		int_t dataValueId = header & 31;
		std::shared_ptr<WatchableObject> object;
		switch (objectType)
		{
			case 0:
				object = std::make_shared<WatchableObject>(objectType, dataValueId, input.readByte());
				break;
			case 1:
				object = std::make_shared<WatchableObject>(objectType, dataValueId, input.readShort());
				break;
			case 2:
				object = std::make_shared<WatchableObject>(objectType, dataValueId, input.readInt());
				break;
			case 3:
				object = std::make_shared<WatchableObject>(objectType, dataValueId, input.readFloat());
				break;
			case 4:
				object = std::make_shared<WatchableObject>(objectType, dataValueId,
					Packet::readString(input, 64));
				break;
			case 5:
			{
				short_t itemId = input.readShort();
				byte_t stackSize = input.readByte();
				short_t itemDamage = input.readShort();
				object = std::make_shared<WatchableObject>(objectType, dataValueId,
					ItemInstance(itemId, stackSize, itemDamage));
				break;
			}
			case 6:
			{
				int_t x = input.readInt();
				int_t y = input.readInt();
				int_t z = input.readInt();
				object = std::make_shared<WatchableObject>(objectType, dataValueId, TilePos(x, y, z));
				break;
			}
		}
		objects->push_back(object);
	}
	return objects;
}

void DataWatcher::updateWatchedObjectsFromList(const WatchableObjectList &objects)
{
	for (const auto &object : objects)
	{
		int_t entry = findEntry(object->getDataValueId());
		if (entry >= 0)
			watchedObjects[static_cast<size_t>(entry)].value->copyObjectFrom(*object);
	}
}

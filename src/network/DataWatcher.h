#pragma once

#include <memory>
#include <vector>

#include "java/String.h"
#include "java/Type.h"
#include "world/item/ItemInstance.h"
#include "world/level/TilePos.h"

class PacketDataInput;
class PacketDataOutput;

class WatchableObject
{
private:
	const int_t objectType;
	const int_t dataValueId;
	int_t runtimeType;
	bool watching = true;

	byte_t byteValue = 0;
	short_t shortValue = 0;
	int_t intValue = 0;
	float floatValue = 0.0f;
	jstring stringValue;
	std::shared_ptr<ItemInstance> itemValue;
	std::shared_ptr<TilePos> coordinateValue;

public:
	WatchableObject(int_t objectType, int_t dataValueId, byte_t value);
	WatchableObject(int_t objectType, int_t dataValueId, short_t value);
	WatchableObject(int_t objectType, int_t dataValueId, int_t value);
	WatchableObject(int_t objectType, int_t dataValueId, float value);
	WatchableObject(int_t objectType, int_t dataValueId, const jstring &value);
	WatchableObject(int_t objectType, int_t dataValueId, const ItemInstance &value);
	WatchableObject(int_t objectType, int_t dataValueId, std::shared_ptr<ItemInstance> value);
	WatchableObject(int_t objectType, int_t dataValueId, const TilePos &value);
	WatchableObject(int_t objectType, int_t dataValueId, std::shared_ptr<TilePos> value);

	int_t getDataValueId() const;
	int_t getObjectType() const;
	int_t getRuntimeType() const;
	void setWatching(bool value);

	byte_t getByte() const;
	short_t getShort() const;
	int_t getInt() const;
	float getFloat() const;
	const jstring &getString() const;
	const ItemInstance &getItem() const;
	TilePos getCoordinates() const;

	bool hasSameObject(const WatchableObject &other) const;
	void copyObjectFrom(const WatchableObject &other);
};

using WatchableObjectList = std::vector<std::shared_ptr<WatchableObject>>;

class DataWatcher
{
private:
	struct WatchedEntry
	{
		uint_t hash;
		int_t key;
		std::shared_ptr<WatchableObject> value;
		int_t next;
	};

	std::vector<WatchedEntry> watchedObjects;
	std::vector<int_t> bucketHeads = std::vector<int_t>(16, -1);
	int_t resizeThreshold = 12;
	bool objectChanged = false;

	static uint_t javaHash(int_t key);
	int_t findEntry(int_t dataValueId) const;
	void resize(int_t capacity);
	void addObject(int_t dataValueId, const std::shared_ptr<WatchableObject> &object);
	void updateObject(int_t dataValueId, const WatchableObject &object);
	static void writeWatchableObject(PacketDataOutput &output, const WatchableObject &object);

public:
	void addObject(int_t dataValueId, byte_t value);
	void addObject(int_t dataValueId, short_t value);
	void addObject(int_t dataValueId, int_t value);
	void addObject(int_t dataValueId, float value);
	void addObject(int_t dataValueId, const jstring &value);
	void addObject(int_t dataValueId, const ItemInstance &value);
	void addObject(int_t dataValueId, std::shared_ptr<ItemInstance> value);
	void addObject(int_t dataValueId, const TilePos &value);
	void addObject(int_t dataValueId, std::shared_ptr<TilePos> value);
	void updateObject(int_t dataValueId, byte_t value);
	void updateObject(int_t dataValueId, short_t value);
	void updateObject(int_t dataValueId, int_t value);
	void updateObject(int_t dataValueId, float value);
	void updateObject(int_t dataValueId, const jstring &value);
	void updateObject(int_t dataValueId, const ItemInstance &value);
	void updateObject(int_t dataValueId, std::shared_ptr<ItemInstance> value);
	void updateObject(int_t dataValueId, const TilePos &value);
	void updateObject(int_t dataValueId, std::shared_ptr<TilePos> value);

	byte_t getWatchableObjectByte(int_t dataValueId) const;
	int_t getWatchableObjectInt(int_t dataValueId) const;
	const jstring &getWatchableObjectString(int_t dataValueId) const;

	static void writeObjectsInListToStream(const WatchableObjectList *objects,
		PacketDataOutput &output);
	void writeWatchableObjects(PacketDataOutput &output) const;
	static std::unique_ptr<WatchableObjectList> readWatchableObjects(PacketDataInput &input);
	void updateWatchedObjectsFromList(const WatchableObjectList &objects);
};

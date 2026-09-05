#pragma once

#include <memory>

#include "network/DataWatcher.h"
#include "network/Packet.h"

class Entity;
class EntityItem;
class EntityPainting;
class Mob;
class Player;

class Packet20NamedEntitySpawn : public Packet
{
public:
	int_t entityId = 0;
	jstring name;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	byte_t rotation = 0;
	byte_t pitch = 0;
	int_t currentItem = 0;

	Packet20NamedEntitySpawn();
	explicit Packet20NamedEntitySpawn(Player &player);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet21PickupSpawn : public Packet
{
public:
	int_t entityId = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	byte_t rotation = 0;
	byte_t pitch = 0;
	byte_t roll = 0;
	int_t itemID = 0;
	int_t count = 0;
	int_t itemDamage = 0;

	Packet21PickupSpawn();
	explicit Packet21PickupSpawn(EntityItem &item);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet22Collect : public Packet
{
public:
	int_t collectedEntityId = 0;
	int_t collectorEntityId = 0;

	Packet22Collect();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet23VehicleSpawn : public Packet
{
public:
	int_t entityId = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t field_28047_e = 0;
	int_t field_28046_f = 0;
	int_t field_28045_g = 0;
	int_t type = 0;
	int_t field_28044_i = 0;

	Packet23VehicleSpawn();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet24MobSpawn : public Packet
{
public:
	int_t entityId = 0;
	byte_t type = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	byte_t yaw = 0;
	byte_t pitch = 0;

private:
	friend int runNetworkSmoke();
	std::shared_ptr<const DataWatcher> metaData;
	std::unique_ptr<WatchableObjectList> receivedMetadata;

public:
	Packet24MobSpawn();
	explicit Packet24MobSpawn(const std::shared_ptr<Mob> &mob);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
	WatchableObjectList *getMetadata();
};

class Packet25EntityPainting : public Packet
{
public:
	int_t entityId = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t direction = 0;
	jstring title;

	Packet25EntityPainting();
	explicit Packet25EntityPainting(EntityPainting &painting);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet27Position : public Packet
{
private:
	float field_22039_a = 0.0f;
	float field_22038_b = 0.0f;
	bool field_22043_c = false;
	bool field_22042_d = false;
	float field_22041_e = 0.0f;
	float field_22040_f = 0.0f;

public:
	Packet27Position();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet28EntityVelocity : public Packet
{
public:
	int_t entityId = 0;
	int_t motionX = 0;
	int_t motionY = 0;
	int_t motionZ = 0;

	Packet28EntityVelocity();
	explicit Packet28EntityVelocity(Entity &entity);
	Packet28EntityVelocity(int_t entityId, double motionX, double motionY, double motionZ);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet29DestroyEntity : public Packet
{
public:
	int_t entityId = 0;

	Packet29DestroyEntity();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet30Entity : public Packet
{
public:
	int_t entityId = 0;
	byte_t xPosition = 0;
	byte_t yPosition = 0;
	byte_t zPosition = 0;
	byte_t yaw = 0;
	byte_t pitch = 0;
	bool rotating = false;

protected:
	explicit Packet30Entity(int_t packetId);

public:
	Packet30Entity();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet31RelEntityMove : public Packet30Entity
{
public:
	Packet31RelEntityMove();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	int_t getPacketSize() const override;
};

class Packet32EntityLook : public Packet30Entity
{
public:
	Packet32EntityLook();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	int_t getPacketSize() const override;
};

class Packet33RelEntityMoveLook : public Packet30Entity
{
public:
	Packet33RelEntityMoveLook();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	int_t getPacketSize() const override;
};

class Packet34EntityTeleport : public Packet
{
public:
	int_t entityId = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	byte_t yaw = 0;
	byte_t pitch = 0;

	Packet34EntityTeleport();
	explicit Packet34EntityTeleport(Entity &entity);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet38EntityStatus : public Packet
{
public:
	int_t entityId = 0;
	byte_t entityStatus = 0;

	Packet38EntityStatus();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet39AttachEntity : public Packet
{
public:
	int_t entityId = 0;
	int_t vehicleEntityId = 0;

	Packet39AttachEntity();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet40EntityMetadata : public Packet
{
public:
	int_t entityId = 0;

private:
	std::unique_ptr<WatchableObjectList> field_21048_b;

public:
	Packet40EntityMetadata();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
	WatchableObjectList *func_21047_b();
};

void registerEntityPackets();

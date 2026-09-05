#pragma once

#include <memory>

#include "network/Packet.h"
#include "world/item/ItemInstance.h"

class Entity;

class Packet0KeepAlive : public Packet
{
public:
	Packet0KeepAlive();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet1Login : public Packet
{
public:
	int_t protocolVersion = 0;
	jstring username;
	long_t mapSeed = 0;
	byte_t dimension = 0;

	Packet1Login();
	Packet1Login(const jstring &username, int_t protocolVersion);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet2Handshake : public Packet
{
public:
	jstring username;

	Packet2Handshake();
	explicit Packet2Handshake(const jstring &username);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet3Chat : public Packet
{
public:
	jstring message;

	Packet3Chat();
	explicit Packet3Chat(const jstring &message);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet4UpdateTime : public Packet
{
public:
	long_t time = 0;

	Packet4UpdateTime();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet5PlayerInventory : public Packet
{
public:
	int_t entityID = 0;
	int_t slot = 0;
	int_t itemID = 0;
	int_t itemDamage = 0;

	Packet5PlayerInventory();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet6SpawnPosition : public Packet
{
public:
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;

	Packet6SpawnPosition();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet7UseEntity : public Packet
{
public:
	int_t playerEntityId = 0;
	int_t targetEntity = 0;
	int_t isLeftClick = 0;

	Packet7UseEntity();
	Packet7UseEntity(int_t playerEntityId, int_t targetEntity, int_t isLeftClick);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet8UpdateHealth : public Packet
{
public:
	int_t healthMP = 0;

	Packet8UpdateHealth();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet9Respawn : public Packet
{
public:
	byte_t field_28048_a = 0;

	Packet9Respawn();
	explicit Packet9Respawn(byte_t dimension);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet10Flying : public Packet
{
protected:
	explicit Packet10Flying(int_t packetId);

public:
	double xPosition = 0.0;
	double yPosition = 0.0;
	double zPosition = 0.0;
	double stance = 0.0;
	float yaw = 0.0f;
	float pitch = 0.0f;
	bool onGround = false;
	bool moving = false;
	bool rotating = false;

	Packet10Flying();
	explicit Packet10Flying(bool onGround);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
	virtual std::unique_ptr<Packet10Flying> copyForSend() const;
};

class Packet11PlayerPosition : public Packet10Flying
{
public:
	Packet11PlayerPosition();
	Packet11PlayerPosition(double x, double y, double stance, double z, bool onGround);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	int_t getPacketSize() const override;
	std::unique_ptr<Packet10Flying> copyForSend() const override;
};

class Packet12PlayerLook : public Packet10Flying
{
public:
	Packet12PlayerLook();
	Packet12PlayerLook(float yaw, float pitch, bool onGround);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	int_t getPacketSize() const override;
	std::unique_ptr<Packet10Flying> copyForSend() const override;
};

class Packet13PlayerLookMove : public Packet10Flying
{
public:
	Packet13PlayerLookMove();
	Packet13PlayerLookMove(double x, double y, double stance, double z, float yaw, float pitch, bool onGround);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	int_t getPacketSize() const override;
	std::unique_ptr<Packet10Flying> copyForSend() const override;
};

class Packet14BlockDig : public Packet
{
public:
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t face = 0;
	int_t status = 0;

	Packet14BlockDig();
	Packet14BlockDig(int_t status, int_t x, int_t y, int_t z, int_t face);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet15Place : public Packet
{
private:
	std::shared_ptr<const ItemInstanceReference> referencedItemStack;

public:
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t direction = 0;
	std::unique_ptr<ItemInstance> itemStack;

	Packet15Place();
	Packet15Place(int_t x, int_t y, int_t z, int_t direction, const ItemInstance *itemStack);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet16BlockItemSwitch : public Packet
{
public:
	int_t id = 0;

	Packet16BlockItemSwitch();
	explicit Packet16BlockItemSwitch(int_t id);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet17Sleep : public Packet
{
public:
	int_t field_22045_a = 0;
	int_t field_22044_b = 0;
	int_t field_22048_c = 0;
	int_t field_22047_d = 0;
	int_t field_22046_e = 0;

	Packet17Sleep();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet18Animation : public Packet
{
public:
	int_t entityId = 0;
	int_t animate = 0;

	Packet18Animation();
	Packet18Animation(Entity &entity, int_t animate);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet19EntityAction : public Packet
{
public:
	int_t entityId = 0;
	int_t state = 0;

	Packet19EntityAction();
	Packet19EntityAction(Entity &entity, int_t state);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet255KickDisconnect : public Packet
{
public:
	jstring reason;

	Packet255KickDisconnect();
	explicit Packet255KickDisconnect(const jstring &reason);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

void registerCorePackets();

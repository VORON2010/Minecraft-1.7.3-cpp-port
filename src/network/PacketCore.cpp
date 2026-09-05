#include "network/PacketCore.h"

#include "network/NetHandler.h"
#include "network/PacketDataStream.h"
#include "world/entity/Entity.h"

namespace
{

template<typename T>
std::unique_ptr<Packet> makePacket()
{
	return std::make_unique<T>();
}

}

Packet0KeepAlive::Packet0KeepAlive() : Packet(0)
{
}

void Packet0KeepAlive::readPacketData(PacketDataInput &)
{
}

void Packet0KeepAlive::writePacketData(PacketDataOutput &)
{
}

void Packet0KeepAlive::processPacket(NetHandler &)
{
}

int_t Packet0KeepAlive::getPacketSize() const
{
	return 0;
}

Packet1Login::Packet1Login() : Packet(1)
{
}

Packet1Login::Packet1Login(const jstring &username, int_t protocolVersion)
	: Packet(1), protocolVersion(protocolVersion), username(username)
{
}

void Packet1Login::readPacketData(PacketDataInput &input)
{
	protocolVersion = input.readInt();
	username = readString(input, 16);
	mapSeed = input.readLong();
	dimension = input.readByte();
}

void Packet1Login::writePacketData(PacketDataOutput &output)
{
	output.writeInt(protocolVersion);
	writeString(username, output);
	output.writeLong(mapSeed);
	output.writeByte(dimension);
}

void Packet1Login::processPacket(NetHandler &handler)
{
	handler.handleLogin(*this);
}

int_t Packet1Login::getPacketSize() const
{
	return 4 + static_cast<int_t>(username.size()) + 4 + 5;
}

Packet2Handshake::Packet2Handshake() : Packet(2)
{
}

Packet2Handshake::Packet2Handshake(const jstring &username) : Packet(2), username(username)
{
}

void Packet2Handshake::readPacketData(PacketDataInput &input)
{
	username = readString(input, 32);
}

void Packet2Handshake::writePacketData(PacketDataOutput &output)
{
	writeString(username, output);
}

void Packet2Handshake::processPacket(NetHandler &handler)
{
	handler.handleHandshake(*this);
}

int_t Packet2Handshake::getPacketSize() const
{
	return 4 + static_cast<int_t>(username.size()) + 4;
}

Packet3Chat::Packet3Chat() : Packet(3)
{
}

Packet3Chat::Packet3Chat(const jstring &message) : Packet(3)
{
	this->message = message.size() > 119 ? message.substr(0, 119) : message;
}

void Packet3Chat::readPacketData(PacketDataInput &input)
{
	message = readString(input, 119);
}

void Packet3Chat::writePacketData(PacketDataOutput &output)
{
	writeString(message, output);
}

void Packet3Chat::processPacket(NetHandler &handler)
{
	handler.handleChat(*this);
}

int_t Packet3Chat::getPacketSize() const
{
	return static_cast<int_t>(message.size());
}

Packet4UpdateTime::Packet4UpdateTime() : Packet(4)
{
}

void Packet4UpdateTime::readPacketData(PacketDataInput &input)
{
	time = input.readLong();
}

void Packet4UpdateTime::writePacketData(PacketDataOutput &output)
{
	output.writeLong(time);
}

void Packet4UpdateTime::processPacket(NetHandler &handler)
{
	handler.handleUpdateTime(*this);
}

int_t Packet4UpdateTime::getPacketSize() const
{
	return 8;
}

Packet5PlayerInventory::Packet5PlayerInventory() : Packet(5)
{
}

void Packet5PlayerInventory::readPacketData(PacketDataInput &input)
{
	entityID = input.readInt();
	slot = input.readShort();
	itemID = input.readShort();
	itemDamage = input.readShort();
}

void Packet5PlayerInventory::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityID);
	output.writeShort(slot);
	output.writeShort(itemID);
	output.writeShort(itemDamage);
}

void Packet5PlayerInventory::processPacket(NetHandler &handler)
{
	handler.handlePlayerInventory(*this);
}

int_t Packet5PlayerInventory::getPacketSize() const
{
	return 8;
}

Packet6SpawnPosition::Packet6SpawnPosition() : Packet(6)
{
}

void Packet6SpawnPosition::readPacketData(PacketDataInput &input)
{
	xPosition = input.readInt();
	yPosition = input.readInt();
	zPosition = input.readInt();
}

void Packet6SpawnPosition::writePacketData(PacketDataOutput &output)
{
	output.writeInt(xPosition);
	output.writeInt(yPosition);
	output.writeInt(zPosition);
}

void Packet6SpawnPosition::processPacket(NetHandler &handler)
{
	handler.handleSpawnPosition(*this);
}

int_t Packet6SpawnPosition::getPacketSize() const
{
	return 12;
}

Packet7UseEntity::Packet7UseEntity() : Packet(7)
{
}

Packet7UseEntity::Packet7UseEntity(int_t playerEntityId, int_t targetEntity, int_t isLeftClick)
	: Packet(7), playerEntityId(playerEntityId), targetEntity(targetEntity), isLeftClick(isLeftClick)
{
}

void Packet7UseEntity::readPacketData(PacketDataInput &input)
{
	playerEntityId = input.readInt();
	targetEntity = input.readInt();
	isLeftClick = input.readByte();
}

void Packet7UseEntity::writePacketData(PacketDataOutput &output)
{
	output.writeInt(playerEntityId);
	output.writeInt(targetEntity);
	output.writeByte(isLeftClick);
}

void Packet7UseEntity::processPacket(NetHandler &handler)
{
	handler.handleUseEntity(*this);
}

int_t Packet7UseEntity::getPacketSize() const
{
	return 9;
}

Packet8UpdateHealth::Packet8UpdateHealth() : Packet(8)
{
}

void Packet8UpdateHealth::readPacketData(PacketDataInput &input)
{
	healthMP = input.readShort();
}

void Packet8UpdateHealth::writePacketData(PacketDataOutput &output)
{
	output.writeShort(healthMP);
}

void Packet8UpdateHealth::processPacket(NetHandler &handler)
{
	handler.handleHealth(*this);
}

int_t Packet8UpdateHealth::getPacketSize() const
{
	return 2;
}

Packet9Respawn::Packet9Respawn() : Packet(9)
{
}

Packet9Respawn::Packet9Respawn(byte_t dimension) : Packet(9), field_28048_a(dimension)
{
}

void Packet9Respawn::readPacketData(PacketDataInput &input)
{
	field_28048_a = input.readByte();
}

void Packet9Respawn::writePacketData(PacketDataOutput &output)
{
	output.writeByte(field_28048_a);
}

void Packet9Respawn::processPacket(NetHandler &handler)
{
	handler.func_9448_a(*this);
}

int_t Packet9Respawn::getPacketSize() const
{
	return 1;
}

Packet10Flying::Packet10Flying(int_t packetId) : Packet(packetId)
{
}

Packet10Flying::Packet10Flying() : Packet10Flying(10)
{
}

Packet10Flying::Packet10Flying(bool onGround) : Packet10Flying(10)
{
	this->onGround = onGround;
}

void Packet10Flying::readPacketData(PacketDataInput &input)
{
	onGround = input.read() != 0;
}

void Packet10Flying::writePacketData(PacketDataOutput &output)
{
	output.write(onGround ? 1 : 0);
}

void Packet10Flying::processPacket(NetHandler &handler)
{
	handler.handleFlying(*this);
}

int_t Packet10Flying::getPacketSize() const
{
	return 1;
}

std::unique_ptr<Packet10Flying> Packet10Flying::copyForSend() const
{
	return std::make_unique<Packet10Flying>(*this);
}

Packet11PlayerPosition::Packet11PlayerPosition() : Packet10Flying(11)
{
	moving = true;
}

Packet11PlayerPosition::Packet11PlayerPosition(double x, double y, double stance, double z, bool onGround)
	: Packet10Flying(11)
{
	xPosition = x;
	yPosition = y;
	this->stance = stance;
	zPosition = z;
	this->onGround = onGround;
	moving = true;
}

void Packet11PlayerPosition::readPacketData(PacketDataInput &input)
{
	xPosition = input.readDouble();
	yPosition = input.readDouble();
	stance = input.readDouble();
	zPosition = input.readDouble();
	Packet10Flying::readPacketData(input);
}

void Packet11PlayerPosition::writePacketData(PacketDataOutput &output)
{
	output.writeDouble(xPosition);
	output.writeDouble(yPosition);
	output.writeDouble(stance);
	output.writeDouble(zPosition);
	Packet10Flying::writePacketData(output);
}

int_t Packet11PlayerPosition::getPacketSize() const
{
	return 33;
}

std::unique_ptr<Packet10Flying> Packet11PlayerPosition::copyForSend() const
{
	return std::make_unique<Packet11PlayerPosition>(*this);
}

Packet12PlayerLook::Packet12PlayerLook() : Packet10Flying(12)
{
	rotating = true;
}

Packet12PlayerLook::Packet12PlayerLook(float yaw, float pitch, bool onGround) : Packet10Flying(12)
{
	this->yaw = yaw;
	this->pitch = pitch;
	this->onGround = onGround;
	rotating = true;
}

void Packet12PlayerLook::readPacketData(PacketDataInput &input)
{
	yaw = input.readFloat();
	pitch = input.readFloat();
	Packet10Flying::readPacketData(input);
}

void Packet12PlayerLook::writePacketData(PacketDataOutput &output)
{
	output.writeFloat(yaw);
	output.writeFloat(pitch);
	Packet10Flying::writePacketData(output);
}

int_t Packet12PlayerLook::getPacketSize() const
{
	return 9;
}

std::unique_ptr<Packet10Flying> Packet12PlayerLook::copyForSend() const
{
	return std::make_unique<Packet12PlayerLook>(*this);
}

Packet13PlayerLookMove::Packet13PlayerLookMove() : Packet10Flying(13)
{
	rotating = true;
	moving = true;
}

Packet13PlayerLookMove::Packet13PlayerLookMove(double x, double y, double stance, double z,
	float yaw, float pitch, bool onGround) : Packet10Flying(13)
{
	xPosition = x;
	yPosition = y;
	this->stance = stance;
	zPosition = z;
	this->yaw = yaw;
	this->pitch = pitch;
	this->onGround = onGround;
	rotating = true;
	moving = true;
}

void Packet13PlayerLookMove::readPacketData(PacketDataInput &input)
{
	xPosition = input.readDouble();
	yPosition = input.readDouble();
	stance = input.readDouble();
	zPosition = input.readDouble();
	yaw = input.readFloat();
	pitch = input.readFloat();
	Packet10Flying::readPacketData(input);
}

void Packet13PlayerLookMove::writePacketData(PacketDataOutput &output)
{
	output.writeDouble(xPosition);
	output.writeDouble(yPosition);
	output.writeDouble(stance);
	output.writeDouble(zPosition);
	output.writeFloat(yaw);
	output.writeFloat(pitch);
	Packet10Flying::writePacketData(output);
}

int_t Packet13PlayerLookMove::getPacketSize() const
{
	return 41;
}

std::unique_ptr<Packet10Flying> Packet13PlayerLookMove::copyForSend() const
{
	return std::make_unique<Packet13PlayerLookMove>(*this);
}

Packet14BlockDig::Packet14BlockDig() : Packet(14)
{
}

Packet14BlockDig::Packet14BlockDig(int_t status, int_t x, int_t y, int_t z, int_t face)
	: Packet(14), xPosition(x), yPosition(y), zPosition(z), face(face), status(status)
{
}

void Packet14BlockDig::readPacketData(PacketDataInput &input)
{
	status = input.read();
	xPosition = input.readInt();
	yPosition = input.read();
	zPosition = input.readInt();
	face = input.read();
}

void Packet14BlockDig::writePacketData(PacketDataOutput &output)
{
	output.write(status);
	output.writeInt(xPosition);
	output.write(yPosition);
	output.writeInt(zPosition);
	output.write(face);
}

void Packet14BlockDig::processPacket(NetHandler &handler)
{
	handler.handleBlockDig(*this);
}

int_t Packet14BlockDig::getPacketSize() const
{
	return 11;
}

Packet15Place::Packet15Place() : Packet(15)
{
}

Packet15Place::Packet15Place(int_t x, int_t y, int_t z, int_t direction, const ItemInstance *itemStack)
	: Packet(15), xPosition(x), yPosition(y), zPosition(z), direction(direction)
{
	if (itemStack)
		referencedItemStack = itemStack->retainReference();
}

void Packet15Place::readPacketData(PacketDataInput &input)
{
	xPosition = input.readInt();
	yPosition = input.read();
	zPosition = input.readInt();
	direction = input.read();
	referencedItemStack.reset();
	short_t itemId = input.readShort();
	if (itemId >= 0)
	{
		byte_t stackSize = input.readByte();
		short_t itemDamage = input.readShort();
		itemStack = std::make_unique<ItemInstance>(itemId, stackSize, itemDamage);
	}
	else
	{
		itemStack.reset();
	}
}

void Packet15Place::writePacketData(PacketDataOutput &output)
{
	output.writeInt(xPosition);
	output.write(yPosition);
	output.writeInt(zPosition);
	output.write(direction);
	if (referencedItemStack)
	{
		ItemInstanceReference::Snapshot item = referencedItemStack->get();
		output.writeShort(item.itemID);
		output.writeByte(item.stackSize);
		output.writeShort(item.itemDamage);
	}
	else if (!itemStack)
	{
		output.writeShort(-1);
	}
	else
	{
		output.writeShort(itemStack->itemID);
		output.writeByte(itemStack->stackSize);
		output.writeShort(itemStack->getAuxValue());
	}
}

void Packet15Place::processPacket(NetHandler &handler)
{
	handler.handlePlace(*this);
}

int_t Packet15Place::getPacketSize() const
{
	return 15;
}

Packet16BlockItemSwitch::Packet16BlockItemSwitch() : Packet(16)
{
}

Packet16BlockItemSwitch::Packet16BlockItemSwitch(int_t id) : Packet(16), id(id)
{
}

void Packet16BlockItemSwitch::readPacketData(PacketDataInput &input)
{
	id = input.readShort();
}

void Packet16BlockItemSwitch::writePacketData(PacketDataOutput &output)
{
	output.writeShort(id);
}

void Packet16BlockItemSwitch::processPacket(NetHandler &handler)
{
	handler.handleBlockItemSwitch(*this);
}

int_t Packet16BlockItemSwitch::getPacketSize() const
{
	return 2;
}

Packet17Sleep::Packet17Sleep() : Packet(17)
{
}

void Packet17Sleep::readPacketData(PacketDataInput &input)
{
	field_22045_a = input.readInt();
	field_22046_e = input.readByte();
	field_22044_b = input.readInt();
	field_22048_c = input.readByte();
	field_22047_d = input.readInt();
}

void Packet17Sleep::writePacketData(PacketDataOutput &output)
{
	output.writeInt(field_22045_a);
	output.writeByte(field_22046_e);
	output.writeInt(field_22044_b);
	output.writeByte(field_22048_c);
	output.writeInt(field_22047_d);
}

void Packet17Sleep::processPacket(NetHandler &handler)
{
	handler.func_22186_a(*this);
}

int_t Packet17Sleep::getPacketSize() const
{
	return 14;
}

Packet18Animation::Packet18Animation() : Packet(18)
{
}

Packet18Animation::Packet18Animation(Entity &entity, int_t animate)
	: Packet(18), entityId(entity.entityId), animate(animate)
{
}

void Packet18Animation::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	animate = input.readByte();
}

void Packet18Animation::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	output.writeByte(animate);
}

void Packet18Animation::processPacket(NetHandler &handler)
{
	handler.handleArmAnimation(*this);
}

int_t Packet18Animation::getPacketSize() const
{
	return 5;
}

Packet19EntityAction::Packet19EntityAction() : Packet(19)
{
}

Packet19EntityAction::Packet19EntityAction(Entity &entity, int_t state)
	: Packet(19), entityId(entity.entityId), state(state)
{
}

void Packet19EntityAction::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	state = input.readByte();
}

void Packet19EntityAction::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	output.writeByte(state);
}

void Packet19EntityAction::processPacket(NetHandler &handler)
{
	handler.func_21147_a(*this);
}

int_t Packet19EntityAction::getPacketSize() const
{
	return 5;
}

Packet255KickDisconnect::Packet255KickDisconnect() : Packet(255)
{
}

Packet255KickDisconnect::Packet255KickDisconnect(const jstring &reason) : Packet(255), reason(reason)
{
}

void Packet255KickDisconnect::readPacketData(PacketDataInput &input)
{
	reason = readString(input, 100);
}

void Packet255KickDisconnect::writePacketData(PacketDataOutput &output)
{
	writeString(reason, output);
}

void Packet255KickDisconnect::processPacket(NetHandler &handler)
{
	handler.handleKickDisconnect(*this);
}

int_t Packet255KickDisconnect::getPacketSize() const
{
	return static_cast<int_t>(reason.size());
}

void registerCorePackets()
{
	Packet::addIdClassMapping(0, true, true, &makePacket<Packet0KeepAlive>, "Packet0KeepAlive");
	Packet::addIdClassMapping(1, true, true, &makePacket<Packet1Login>, "Packet1Login");
	Packet::addIdClassMapping(2, true, true, &makePacket<Packet2Handshake>, "Packet2Handshake");
	Packet::addIdClassMapping(3, true, true, &makePacket<Packet3Chat>, "Packet3Chat");
	Packet::addIdClassMapping(4, true, false, &makePacket<Packet4UpdateTime>, "Packet4UpdateTime");
	Packet::addIdClassMapping(5, true, false, &makePacket<Packet5PlayerInventory>, "Packet5PlayerInventory");
	Packet::addIdClassMapping(6, true, false, &makePacket<Packet6SpawnPosition>, "Packet6SpawnPosition");
	Packet::addIdClassMapping(7, false, true, &makePacket<Packet7UseEntity>, "Packet7UseEntity");
	Packet::addIdClassMapping(8, true, false, &makePacket<Packet8UpdateHealth>, "Packet8UpdateHealth");
	Packet::addIdClassMapping(9, true, true, &makePacket<Packet9Respawn>, "Packet9Respawn");
	Packet::addIdClassMapping(10, true, true, &makePacket<Packet10Flying>, "Packet10Flying");
	Packet::addIdClassMapping(11, true, true, &makePacket<Packet11PlayerPosition>, "Packet11PlayerPosition");
	Packet::addIdClassMapping(12, true, true, &makePacket<Packet12PlayerLook>, "Packet12PlayerLook");
	Packet::addIdClassMapping(13, true, true, &makePacket<Packet13PlayerLookMove>, "Packet13PlayerLookMove");
	Packet::addIdClassMapping(14, false, true, &makePacket<Packet14BlockDig>, "Packet14BlockDig");
	Packet::addIdClassMapping(15, false, true, &makePacket<Packet15Place>, "Packet15Place");
	Packet::addIdClassMapping(16, false, true, &makePacket<Packet16BlockItemSwitch>, "Packet16BlockItemSwitch");
	Packet::addIdClassMapping(17, true, false, &makePacket<Packet17Sleep>, "Packet17Sleep");
	Packet::addIdClassMapping(18, true, true, &makePacket<Packet18Animation>, "Packet18Animation");
	Packet::addIdClassMapping(19, false, true, &makePacket<Packet19EntityAction>, "Packet19EntityAction");
	Packet::addIdClassMapping(255, true, true, &makePacket<Packet255KickDisconnect>, "Packet255KickDisconnect");
}

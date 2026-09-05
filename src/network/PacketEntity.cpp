#include "network/PacketEntity.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

#include "network/NetHandler.h"
#include "network/PacketDataStream.h"
#include "util/Mth.h"
#include "world/entity/Entity.h"
#include "world/entity/Mob.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/item/EntityPainting.h"
#include "world/entity/player/Player.h"

namespace
{

template<typename T>
std::unique_ptr<Packet> makePacket()
{
	return std::make_unique<T>();
}

int_t javaDoubleToInt(double value)
{
	if (std::isnan(value))
		return 0;
	if (value <= std::numeric_limits<int_t>::min())
		return std::numeric_limits<int_t>::min();
	if (value >= std::numeric_limits<int_t>::max())
		return std::numeric_limits<int_t>::max();
	return static_cast<int_t>(value);
}

byte_t javaNarrowByte(int_t value)
{
	int_t narrowed = static_cast<int_t>(static_cast<uint_t>(value) & 255U);
	return static_cast<byte_t>(narrowed >= 128 ? narrowed - 256 : narrowed);
}

byte_t javaNarrowByte(double value)
{
	return javaNarrowByte(javaDoubleToInt(value));
}

int_t getMobTypeId(Mob &mob)
{
	const jstring id = mob.getEncodeId();
	if (id == u"Mob") return 48;
	if (id == u"Monster") return 49;
	if (id == u"Creeper") return 50;
	if (id == u"Skeleton") return 51;
	if (id == u"Spider") return 52;
	if (id == u"Giant") return 53;
	if (id == u"Zombie") return 54;
	if (id == u"Slime") return 55;
	if (id == u"Ghast") return 56;
	if (id == u"PigZombie") return 57;
	if (id == u"Pig") return 90;
	if (id == u"Sheep") return 91;
	if (id == u"Cow") return 92;
	if (id == u"Chicken") return 93;
	if (id == u"Squid") return 94;
	if (id == u"Wolf") return 95;
	throw std::invalid_argument("Unknown mob entity type");
}

const jstring &getPaintingTitle(EntityPainting &painting)
{
	if (painting.art == nullptr)
		throw std::runtime_error("java.lang.NullPointerException");
	return painting.art->title;
}

}

Packet20NamedEntitySpawn::Packet20NamedEntitySpawn() : Packet(20)
{
}

Packet20NamedEntitySpawn::Packet20NamedEntitySpawn(Player &player)
	: Packet(20), entityId(player.entityId), name(player.name),
	  xPosition(Mth::floor(player.x * 32.0)),
	  yPosition(Mth::floor(player.y * 32.0)),
	  zPosition(Mth::floor(player.z * 32.0)),
	  rotation(javaNarrowByte(player.yRot * 256.0f / 360.0f)),
	  pitch(javaNarrowByte(player.xRot * 256.0f / 360.0f))
{
	ItemInstance *item = player.inventory.getCurrentItem();
	currentItem = item == nullptr ? 0 : item->itemID.load(std::memory_order_relaxed);
}

void Packet20NamedEntitySpawn::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	name = readString(input, 16);
	xPosition = input.readInt();
	yPosition = input.readInt();
	zPosition = input.readInt();
	rotation = input.readByte();
	pitch = input.readByte();
	currentItem = input.readShort();
}

void Packet20NamedEntitySpawn::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	writeString(name, output);
	output.writeInt(xPosition);
	output.writeInt(yPosition);
	output.writeInt(zPosition);
	output.writeByte(rotation);
	output.writeByte(pitch);
	output.writeShort(currentItem);
}

void Packet20NamedEntitySpawn::processPacket(NetHandler &handler)
{
	handler.handleNamedEntitySpawn(*this);
}

int_t Packet20NamedEntitySpawn::getPacketSize() const
{
	return 28;
}

Packet21PickupSpawn::Packet21PickupSpawn() : Packet(21)
{
}

Packet21PickupSpawn::Packet21PickupSpawn(EntityItem &item)
	: Packet(21), entityId(item.entityId),
	  xPosition(Mth::floor(item.x * 32.0)),
	  yPosition(Mth::floor(item.y * 32.0)),
	  zPosition(Mth::floor(item.z * 32.0)),
	  rotation(javaNarrowByte(item.xd * 128.0)),
	  pitch(javaNarrowByte(item.yd * 128.0)),
	  roll(javaNarrowByte(item.zd * 128.0)),
	  itemID(item.item.itemID), count(item.item.stackSize), itemDamage(item.item.getAuxValue())
{
}

void Packet21PickupSpawn::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	itemID = input.readShort();
	count = input.readByte();
	itemDamage = input.readShort();
	xPosition = input.readInt();
	yPosition = input.readInt();
	zPosition = input.readInt();
	rotation = input.readByte();
	pitch = input.readByte();
	roll = input.readByte();
}

void Packet21PickupSpawn::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	output.writeShort(itemID);
	output.writeByte(count);
	output.writeShort(itemDamage);
	output.writeInt(xPosition);
	output.writeInt(yPosition);
	output.writeInt(zPosition);
	output.writeByte(rotation);
	output.writeByte(pitch);
	output.writeByte(roll);
}

void Packet21PickupSpawn::processPacket(NetHandler &handler)
{
	handler.handlePickupSpawn(*this);
}

int_t Packet21PickupSpawn::getPacketSize() const
{
	return 24;
}

Packet22Collect::Packet22Collect() : Packet(22)
{
}

void Packet22Collect::readPacketData(PacketDataInput &input)
{
	collectedEntityId = input.readInt();
	collectorEntityId = input.readInt();
}

void Packet22Collect::writePacketData(PacketDataOutput &output)
{
	output.writeInt(collectedEntityId);
	output.writeInt(collectorEntityId);
}

void Packet22Collect::processPacket(NetHandler &handler)
{
	handler.handleCollect(*this);
}

int_t Packet22Collect::getPacketSize() const
{
	return 8;
}

Packet23VehicleSpawn::Packet23VehicleSpawn() : Packet(23)
{
}

void Packet23VehicleSpawn::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	type = input.readByte();
	xPosition = input.readInt();
	yPosition = input.readInt();
	zPosition = input.readInt();
	field_28044_i = input.readInt();
	if (field_28044_i > 0)
	{
		field_28047_e = input.readShort();
		field_28046_f = input.readShort();
		field_28045_g = input.readShort();
	}
}

void Packet23VehicleSpawn::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	output.writeByte(type);
	output.writeInt(xPosition);
	output.writeInt(yPosition);
	output.writeInt(zPosition);
	output.writeInt(field_28044_i);
	if (field_28044_i > 0)
	{
		output.writeShort(field_28047_e);
		output.writeShort(field_28046_f);
		output.writeShort(field_28045_g);
	}
}

void Packet23VehicleSpawn::processPacket(NetHandler &handler)
{
	handler.handleVehicleSpawn(*this);
}

int_t Packet23VehicleSpawn::getPacketSize() const
{
	uint_t sumBits = static_cast<uint_t>(field_28044_i) + 21U;
	int_t wrappedSum;
	std::memcpy(&wrappedSum, &sumBits, sizeof(wrappedSum));
	return wrappedSum > 0 ? 6 : 0;
}

Packet24MobSpawn::Packet24MobSpawn() : Packet(24)
{
}

Packet24MobSpawn::Packet24MobSpawn(const std::shared_ptr<Mob> &mob)
	: Packet(24)
{
	if (!mob)
		throw std::runtime_error("java.lang.NullPointerException");
	entityId = mob->entityId;
	type = javaNarrowByte(getMobTypeId(*mob));
	xPosition = Mth::floor(mob->x * 32.0);
	yPosition = Mth::floor(mob->y * 32.0);
	zPosition = Mth::floor(mob->z * 32.0);
	yaw = javaNarrowByte(mob->yRot * 256.0f / 360.0f);
	pitch = javaNarrowByte(mob->xRot * 256.0f / 360.0f);
	metaData = std::shared_ptr<const DataWatcher>(mob, &mob->getDataWatcher());
}

void Packet24MobSpawn::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	type = input.readByte();
	xPosition = input.readInt();
	yPosition = input.readInt();
	zPosition = input.readInt();
	yaw = input.readByte();
	pitch = input.readByte();
	receivedMetadata = DataWatcher::readWatchableObjects(input);
}

void Packet24MobSpawn::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	output.writeByte(type);
	output.writeInt(xPosition);
	output.writeInt(yPosition);
	output.writeInt(zPosition);
	output.writeByte(yaw);
	output.writeByte(pitch);
	if (!metaData)
		throw std::runtime_error("Packet24MobSpawn metadata is null");
	metaData->writeWatchableObjects(output);
}

void Packet24MobSpawn::processPacket(NetHandler &handler)
{
	handler.handleMobSpawn(*this);
}

int_t Packet24MobSpawn::getPacketSize() const
{
	return 20;
}

WatchableObjectList *Packet24MobSpawn::getMetadata()
{
	return receivedMetadata.get();
}

Packet25EntityPainting::Packet25EntityPainting() : Packet(25)
{
}

Packet25EntityPainting::Packet25EntityPainting(EntityPainting &painting)
	: Packet(25), entityId(painting.entityId), xPosition(painting.xPosition),
	  yPosition(painting.yPosition), zPosition(painting.zPosition),
	  direction(painting.direction), title(getPaintingTitle(painting))
{
}

void Packet25EntityPainting::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	title = readString(input, 13);
	xPosition = input.readInt();
	yPosition = input.readInt();
	zPosition = input.readInt();
	direction = input.readInt();
}

void Packet25EntityPainting::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	writeString(title, output);
	output.writeInt(xPosition);
	output.writeInt(yPosition);
	output.writeInt(zPosition);
	output.writeInt(direction);
}

void Packet25EntityPainting::processPacket(NetHandler &handler)
{
	handler.func_21146_a(*this);
}

int_t Packet25EntityPainting::getPacketSize() const
{
	return 24;
}

Packet27Position::Packet27Position() : Packet(27)
{
}

void Packet27Position::readPacketData(PacketDataInput &input)
{
	field_22039_a = input.readFloat();
	field_22038_b = input.readFloat();
	field_22041_e = input.readFloat();
	field_22040_f = input.readFloat();
	field_22043_c = input.readBoolean();
	field_22042_d = input.readBoolean();
}

void Packet27Position::writePacketData(PacketDataOutput &output)
{
	output.writeFloat(field_22039_a);
	output.writeFloat(field_22038_b);
	output.writeFloat(field_22041_e);
	output.writeFloat(field_22040_f);
	output.writeBoolean(field_22043_c);
	output.writeBoolean(field_22042_d);
}

void Packet27Position::processPacket(NetHandler &handler)
{
	handler.func_22185_a(*this);
}

int_t Packet27Position::getPacketSize() const
{
	return 18;
}

Packet28EntityVelocity::Packet28EntityVelocity() : Packet(28)
{
}

Packet28EntityVelocity::Packet28EntityVelocity(Entity &entity)
	: Packet28EntityVelocity(entity.entityId, entity.xd, entity.yd, entity.zd)
{
}

Packet28EntityVelocity::Packet28EntityVelocity(int_t entityId, double motionX, double motionY,
	double motionZ) : Packet(28), entityId(entityId)
{
	double limit = 3.9;
	if (motionX < -limit) motionX = -limit;
	if (motionY < -limit) motionY = -limit;
	if (motionZ < -limit) motionZ = -limit;
	if (motionX > limit) motionX = limit;
	if (motionY > limit) motionY = limit;
	if (motionZ > limit) motionZ = limit;
	this->motionX = javaDoubleToInt(motionX * 8000.0);
	this->motionY = javaDoubleToInt(motionY * 8000.0);
	this->motionZ = javaDoubleToInt(motionZ * 8000.0);
}

void Packet28EntityVelocity::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	motionX = input.readShort();
	motionY = input.readShort();
	motionZ = input.readShort();
}

void Packet28EntityVelocity::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	output.writeShort(motionX);
	output.writeShort(motionY);
	output.writeShort(motionZ);
}

void Packet28EntityVelocity::processPacket(NetHandler &handler)
{
	handler.func_6498_a(*this);
}

int_t Packet28EntityVelocity::getPacketSize() const
{
	return 10;
}

Packet29DestroyEntity::Packet29DestroyEntity() : Packet(29)
{
}

void Packet29DestroyEntity::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
}

void Packet29DestroyEntity::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
}

void Packet29DestroyEntity::processPacket(NetHandler &handler)
{
	handler.handleDestroyEntity(*this);
}

int_t Packet29DestroyEntity::getPacketSize() const
{
	return 4;
}

Packet30Entity::Packet30Entity(int_t packetId) : Packet(packetId)
{
}

Packet30Entity::Packet30Entity() : Packet30Entity(30)
{
}

void Packet30Entity::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
}

void Packet30Entity::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
}

void Packet30Entity::processPacket(NetHandler &handler)
{
	handler.handleEntity(*this);
}

int_t Packet30Entity::getPacketSize() const
{
	return 4;
}

Packet31RelEntityMove::Packet31RelEntityMove() : Packet30Entity(31)
{
}

void Packet31RelEntityMove::readPacketData(PacketDataInput &input)
{
	Packet30Entity::readPacketData(input);
	xPosition = input.readByte();
	yPosition = input.readByte();
	zPosition = input.readByte();
}

void Packet31RelEntityMove::writePacketData(PacketDataOutput &output)
{
	Packet30Entity::writePacketData(output);
	output.writeByte(xPosition);
	output.writeByte(yPosition);
	output.writeByte(zPosition);
}

int_t Packet31RelEntityMove::getPacketSize() const
{
	return 7;
}

Packet32EntityLook::Packet32EntityLook() : Packet30Entity(32)
{
	rotating = true;
}

void Packet32EntityLook::readPacketData(PacketDataInput &input)
{
	Packet30Entity::readPacketData(input);
	yaw = input.readByte();
	pitch = input.readByte();
}

void Packet32EntityLook::writePacketData(PacketDataOutput &output)
{
	Packet30Entity::writePacketData(output);
	output.writeByte(yaw);
	output.writeByte(pitch);
}

int_t Packet32EntityLook::getPacketSize() const
{
	return 6;
}

Packet33RelEntityMoveLook::Packet33RelEntityMoveLook() : Packet30Entity(33)
{
	rotating = true;
}

void Packet33RelEntityMoveLook::readPacketData(PacketDataInput &input)
{
	Packet30Entity::readPacketData(input);
	xPosition = input.readByte();
	yPosition = input.readByte();
	zPosition = input.readByte();
	yaw = input.readByte();
	pitch = input.readByte();
}

void Packet33RelEntityMoveLook::writePacketData(PacketDataOutput &output)
{
	Packet30Entity::writePacketData(output);
	output.writeByte(xPosition);
	output.writeByte(yPosition);
	output.writeByte(zPosition);
	output.writeByte(yaw);
	output.writeByte(pitch);
}

int_t Packet33RelEntityMoveLook::getPacketSize() const
{
	return 9;
}

Packet34EntityTeleport::Packet34EntityTeleport() : Packet(34)
{
}

Packet34EntityTeleport::Packet34EntityTeleport(Entity &entity)
	: Packet(34), entityId(entity.entityId),
	  xPosition(Mth::floor(entity.x * 32.0)),
	  yPosition(Mth::floor(entity.y * 32.0)),
	  zPosition(Mth::floor(entity.z * 32.0)),
	  yaw(javaNarrowByte(entity.yRot * 256.0f / 360.0f)),
	  pitch(javaNarrowByte(entity.xRot * 256.0f / 360.0f))
{
}

void Packet34EntityTeleport::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	xPosition = input.readInt();
	yPosition = input.readInt();
	zPosition = input.readInt();
	yaw = javaNarrowByte(input.read());
	pitch = javaNarrowByte(input.read());
}

void Packet34EntityTeleport::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	output.writeInt(xPosition);
	output.writeInt(yPosition);
	output.writeInt(zPosition);
	output.write(yaw);
	output.write(pitch);
}

void Packet34EntityTeleport::processPacket(NetHandler &handler)
{
	handler.handleEntityTeleport(*this);
}

int_t Packet34EntityTeleport::getPacketSize() const
{
	return 34;
}

Packet38EntityStatus::Packet38EntityStatus() : Packet(38)
{
}

void Packet38EntityStatus::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	entityStatus = input.readByte();
}

void Packet38EntityStatus::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	output.writeByte(entityStatus);
}

void Packet38EntityStatus::processPacket(NetHandler &handler)
{
	handler.func_9447_a(*this);
}

int_t Packet38EntityStatus::getPacketSize() const
{
	return 5;
}

Packet39AttachEntity::Packet39AttachEntity() : Packet(39)
{
}

void Packet39AttachEntity::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	vehicleEntityId = input.readInt();
}

void Packet39AttachEntity::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	output.writeInt(vehicleEntityId);
}

void Packet39AttachEntity::processPacket(NetHandler &handler)
{
	handler.func_6497_a(*this);
}

int_t Packet39AttachEntity::getPacketSize() const
{
	return 8;
}

Packet40EntityMetadata::Packet40EntityMetadata() : Packet(40)
{
}

void Packet40EntityMetadata::readPacketData(PacketDataInput &input)
{
	entityId = input.readInt();
	field_21048_b = DataWatcher::readWatchableObjects(input);
}

void Packet40EntityMetadata::writePacketData(PacketDataOutput &output)
{
	output.writeInt(entityId);
	DataWatcher::writeObjectsInListToStream(field_21048_b.get(), output);
}

void Packet40EntityMetadata::processPacket(NetHandler &handler)
{
	handler.func_21148_a(*this);
}

int_t Packet40EntityMetadata::getPacketSize() const
{
	return 5;
}

WatchableObjectList *Packet40EntityMetadata::func_21047_b()
{
	return field_21048_b.get();
}

void registerEntityPackets()
{
	Packet::addIdClassMapping(20, true, false, &makePacket<Packet20NamedEntitySpawn>,
		"Packet20NamedEntitySpawn");
	Packet::addIdClassMapping(21, true, false, &makePacket<Packet21PickupSpawn>,
		"Packet21PickupSpawn");
	Packet::addIdClassMapping(22, true, false, &makePacket<Packet22Collect>, "Packet22Collect");
	Packet::addIdClassMapping(23, true, false, &makePacket<Packet23VehicleSpawn>,
		"Packet23VehicleSpawn");
	Packet::addIdClassMapping(24, true, false, &makePacket<Packet24MobSpawn>, "Packet24MobSpawn");
	Packet::addIdClassMapping(25, true, false, &makePacket<Packet25EntityPainting>,
		"Packet25EntityPainting");
	Packet::addIdClassMapping(27, false, true, &makePacket<Packet27Position>, "Packet27Position");
	Packet::addIdClassMapping(28, true, false, &makePacket<Packet28EntityVelocity>,
		"Packet28EntityVelocity");
	Packet::addIdClassMapping(29, true, false, &makePacket<Packet29DestroyEntity>,
		"Packet29DestroyEntity");
	Packet::addIdClassMapping(30, true, false, &makePacket<Packet30Entity>, "Packet30Entity");
	Packet::addIdClassMapping(31, true, false, &makePacket<Packet31RelEntityMove>,
		"Packet31RelEntityMove");
	Packet::addIdClassMapping(32, true, false, &makePacket<Packet32EntityLook>,
		"Packet32EntityLook");
	Packet::addIdClassMapping(33, true, false, &makePacket<Packet33RelEntityMoveLook>,
		"Packet33RelEntityMoveLook");
	Packet::addIdClassMapping(34, true, false, &makePacket<Packet34EntityTeleport>,
		"Packet34EntityTeleport");
	Packet::addIdClassMapping(38, true, false, &makePacket<Packet38EntityStatus>,
		"Packet38EntityStatus");
	Packet::addIdClassMapping(39, true, false, &makePacket<Packet39AttachEntity>,
		"Packet39AttachEntity");
	Packet::addIdClassMapping(40, true, false, &makePacket<Packet40EntityMetadata>,
		"Packet40EntityMetadata");
}

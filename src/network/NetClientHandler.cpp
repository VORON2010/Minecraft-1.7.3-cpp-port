#include "network/NetClientHandler.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "client/Minecraft.h"
#include "client/gamemode/MultiplayerGameMode.h"
#include "client/gui/ConnectFailedScreen.h"
#include "client/gui/DownloadTerrainScreen.h"
#include "client/level/MultiplayerLevel.h"
#include "client/particle/TakeAnimationParticle.h"
#include "client/player/MultiplayerLocalPlayer.h"
#include "client/player/RemotePlayer.h"
#include "client/spc/SPCCommand.h"
#include "network/NetworkManager.h"
#include "network/PacketCore.h"
#include "network/PacketEntity.h"
#include "network/PacketWindow.h"
#include "network/PacketWorld.h"
#include "util/Memory.h"
#include "util/Mth.h"
#include "world/entity/EntityIO.h"
#include "world/entity/EntityLightningBolt.h"
#include "world/entity/Mob.h"
#include "world/entity/PrimedTNT.h"
#include "world/entity/item/EntityBoat.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/item/EntityMinecart.h"
#include "world/entity/item/EntityPainting.h"
#include "world/entity/item/FallingTile.h"
#include "world/entity/projectile/EntityArrow.h"
#include "world/entity/projectile/EntityFireball.h"
#include "world/entity/projectile/EntityFish.h"
#include "world/entity/projectile/EntitySnowball.h"
#include "world/entity/projectile/EntityThrownEgg.h"
#include "world/item/ItemMap.h"
#include "world/item/Items.h"
#include "world/inventory/BasicInventory.h"
#include "world/inventory/Container.h"
#include "world/level/Explosion.h"
#include "world/level/MapData.h"
#include "world/level/chunk/LevelChunk.h"
#include "world/level/tile/GravelTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/DispenserTileEntity.h"
#include "world/level/tile/entity/FurnaceTileEntity.h"
#include "world/level/tile/entity/SignTileEntity.h"
#include "world/stats/StatFileWriter.h"
#include "world/stats/StatList.h"

namespace
{

int_t javaIntAdd(int_t left, int_t right)
{
	uint_t resultBits = static_cast<uint_t>(left) + static_cast<uint_t>(right);
	int_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

int_t javaIntMultiply(int_t left, int_t right)
{
	uint_t resultBits = static_cast<uint_t>(left) * static_cast<uint_t>(right);
	int_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

float packetAngle(byte_t angle)
{
	return static_cast<float>(angle) * 360.0f / 256.0f;
}


}

NetClientHandler::NetClientHandler(Minecraft &minecraft) : minecraft(minecraft)
{
}

void NetClientHandler::start(const std::string &host, std::uint16_t port)
{
	networkManager = std::make_unique<NetworkManager>(host, port, *this, "Client");
}

std::shared_ptr<NetClientHandler> NetClientHandler::connect(Minecraft &minecraft,
	const std::string &host, std::uint16_t port)
{
	std::shared_ptr<NetClientHandler> handler(new NetClientHandler(minecraft));
	handler->start(host, port);
	return handler;
}

NetClientHandler::~NetClientHandler() = default;

bool NetClientHandler::isServerHandler() const
{
	return false;
}

bool NetClientHandler::isDisconnected() const
{
	return disconnected;
}

std::shared_ptr<MultiplayerLevel> NetClientHandler::getLevel() const
{
	return multiplayerLevel;
}

void NetClientHandler::processReadPackets()
{
	if (networkManager == nullptr)
		return;
	if (!disconnected)
		networkManager->processReadPackets();
	networkManager->wakeThreads();
}

void NetClientHandler::addToSendQueue(std::unique_ptr<Packet> packet)
{
	if (!disconnected && networkManager != nullptr)
		networkManager->addToSendQueue(std::move(packet));
}

void NetClientHandler::sendAndQuit(std::unique_ptr<Packet> packet)
{
	if (!disconnected && networkManager != nullptr)
	{
		networkManager->addToSendQueue(std::move(packet));
		networkManager->func_28142_c();
	}
}

void NetClientHandler::disconnect()
{
	disconnected = true;
	if (networkManager != nullptr)
	{
		networkManager->wakeThreads();
		networkManager->networkShutdown(u"disconnect.closed");
	}
}

std::shared_ptr<Entity> NetClientHandler::getEntityById(int_t entityId) const
{
	if (minecraft.player != nullptr && entityId == minecraft.player->entityId)
		return std::static_pointer_cast<Entity>(minecraft.player);
	return multiplayerLevel == nullptr ? nullptr : multiplayerLevel->getEntityById(entityId);
}

void NetClientHandler::handleLogin(Packet1Login &packet)
{
	minecraft.gameMode = Util::make_shared<MultiplayerGameMode>(minecraft, *this);
	if (minecraft.statFileWriter != nullptr && StatList::joinMultiplayerStat != nullptr)
		minecraft.statFileWriter->readStat(*StatList::joinMultiplayerStat, 1);

	multiplayerLevel = Util::make_shared<MultiplayerLevel>(*this, packet.mapSeed, packet.dimension);
	multiplayerLevel->isOnline = true;
	minecraft.setLevel(multiplayerLevel);
	if (minecraft.player != nullptr)
		minecraft.player->dimension = packet.dimension;
	minecraft.setScreen(Util::make_shared<DownloadTerrainScreen>(minecraft, shared_from_this()));
	if (minecraft.player != nullptr)
		minecraft.player->entityId = packet.protocolVersion;
}

void NetClientHandler::handleHandshake(Packet2Handshake &packet)
{
	if (packet.username == u"-")
	{
		addToSendQueue(std::make_unique<Packet1Login>(minecraft.user->name, 14));
		return;
	}

	
	networkManager->networkShutdown(u"disconnect.loginFailedInfo",
		{u"Legacy session authentication is unavailable; use an offline-mode server."});
}

void NetClientHandler::handleChat(Packet3Chat &packet)
{
	SPCCommand::addChatMessage(packet.message);
}

void NetClientHandler::handleUpdateTime(Packet4UpdateTime &packet)
{
	if (multiplayerLevel != nullptr)
		multiplayerLevel->setTime(packet.time);
}

void NetClientHandler::handleSpawnPosition(Packet6SpawnPosition &packet)
{
	if (minecraft.player != nullptr)
	{
		TilePos position(packet.xPosition, packet.yPosition, packet.zPosition);
		minecraft.player->setPlayerSpawnPosition(&position);
	}
	if (multiplayerLevel != nullptr)
	{
		multiplayerLevel->xSpawn = packet.xPosition;
		multiplayerLevel->ySpawn = packet.yPosition;
		multiplayerLevel->zSpawn = packet.zPosition;
	}
}

void NetClientHandler::handleFlying(Packet10Flying &packet)
{
	if (minecraft.player == nullptr)
		return;

	Player &player = *minecraft.player;
	double x = player.x;
	double y = player.y;
	double z = player.z;
	float yRot = player.yRot;
	float xRot = player.xRot;
	if (packet.moving)
	{
		x = packet.xPosition;
		y = packet.yPosition;
		z = packet.zPosition;
	}
	if (packet.rotating)
	{
		yRot = packet.yaw;
		xRot = packet.pitch;
	}

	player.ySlideOffset = 0.0f;
	player.xd = player.yd = player.zd = 0.0;
	player.absMoveTo(x, y, z, yRot, xRot);
	packet.xPosition = player.x;
	packet.yPosition = player.bb.y0;
	packet.zPosition = player.z;
	packet.stance = player.y;
	addToSendQueue(packet.copyForSend());

	if (!receivedFirstPosition)
	{
		player.xo = player.x;
		player.yo = player.y;
		player.zo = player.z;
		receivedFirstPosition = true;
		minecraft.setScreen(nullptr);
	}
}

void NetClientHandler::handlePreChunk(Packet50PreChunk &packet)
{
	if (multiplayerLevel != nullptr)
		multiplayerLevel->doPreChunk(packet.xPosition, packet.yPosition, packet.mode);
}

void NetClientHandler::handleMultiBlockChange(Packet52MultiBlockChange &packet)
{
	if (multiplayerLevel == nullptr)
		return;
	std::shared_ptr<LevelChunk> chunk = multiplayerLevel->getChunk(packet.xPosition, packet.zPosition);
	int_t xBase = javaIntMultiply(packet.xPosition, 16);
	int_t zBase = javaIntMultiply(packet.zPosition, 16);
	for (int_t i = 0; i < packet.size; ++i)
	{
		short_t coordinate = packet.coordinateArray.at(static_cast<size_t>(i));
		int_t tile = static_cast<uint8_t>(packet.typeArray.at(static_cast<size_t>(i)));
		byte_t data = packet.metadataArray.at(static_cast<size_t>(i));
		int_t x = coordinate >> 12 & 15;
		int_t z = coordinate >> 8 & 15;
		int_t y = coordinate & 255;
		chunk->setTileAndData(x, y, z, tile, data);
		int_t worldX = javaIntAdd(xBase, x);
		int_t worldZ = javaIntAdd(zBase, z);
		multiplayerLevel->clearPendingBlockChanges(worldX, y, worldZ, worldX, y, worldZ);
		multiplayerLevel->setTilesDirty(worldX, y, worldZ, worldX, y, worldZ);
	}
}

void NetClientHandler::handleMapChunk(Packet51MapChunk &packet)
{
	if (multiplayerLevel == nullptr)
		return;
	int_t x1 = javaIntAdd(packet.xPosition, packet.xSize - 1);
	int_t y1 = javaIntAdd(packet.yPosition, packet.ySize - 1);
	int_t z1 = javaIntAdd(packet.zPosition, packet.zSize - 1);
	multiplayerLevel->clearPendingBlockChanges(packet.xPosition, packet.yPosition, packet.zPosition,
		x1, y1, z1);
	multiplayerLevel->setChunkData(packet.xPosition, packet.yPosition, packet.zPosition,
		packet.xSize, packet.ySize, packet.zSize, packet.chunk);
}

void NetClientHandler::handleBlockChange(Packet53BlockChange &packet)
{
	if (multiplayerLevel != nullptr)
		multiplayerLevel->setTileAndDataRemote(packet.xPosition, packet.yPosition, packet.zPosition,
			packet.type, packet.metadata);
}

void NetClientHandler::handleNotePlay(Packet54PlayNoteBlock &packet)
{
	if (multiplayerLevel != nullptr)
		multiplayerLevel->playNoteAt(packet.xLocation, packet.yLocation, packet.zLocation,
			packet.instrumentType, packet.pitch);
}

void NetClientHandler::handleKickDisconnect(Packet255KickDisconnect &packet)
{
	if (networkManager != nullptr)
		networkManager->networkShutdown(u"disconnect.kicked");
	disconnected = true;
	minecraft.setLevel(nullptr);
	minecraft.setScreen(Util::make_shared<ConnectFailedScreen>(minecraft,
		u"disconnect.disconnected", u"disconnect.genericReason", packet.reason));
}

void NetClientHandler::handleErrorMessage(const jstring &reason,
	const std::vector<jstring> &arguments)
{
	if (disconnected)
		return;
	disconnected = true;
	minecraft.setLevel(nullptr);
	minecraft.setScreen(Util::make_shared<ConnectFailedScreen>(minecraft,
		u"disconnect.lost", reason, arguments));
}

void NetClientHandler::handlePickupSpawn(Packet21PickupSpawn &packet)
{
	if (multiplayerLevel == nullptr)
		return;
	double x = packet.xPosition / 32.0;
	double y = packet.yPosition / 32.0;
	double z = packet.zPosition / 32.0;
	std::shared_ptr<EntityItem> item = Util::make_shared<EntityItem>(*multiplayerLevel,
		x, y, z, ItemInstance(packet.itemID, packet.count, packet.itemDamage));
	item->xd = packet.rotation / 128.0;
	item->yd = packet.pitch / 128.0;
	item->zd = packet.roll / 128.0;
	item->serverPosX = packet.xPosition;
	item->serverPosY = packet.yPosition;
	item->serverPosZ = packet.zPosition;
	multiplayerLevel->addEntityById(packet.entityId, item);
}

void NetClientHandler::handleVehicleSpawn(Packet23VehicleSpawn &packet)
{
	if (multiplayerLevel == nullptr)
		return;
	double x = packet.xPosition / 32.0;
	double y = packet.yPosition / 32.0;
	double z = packet.zPosition / 32.0;
	std::shared_ptr<Entity> entity;
	if (packet.type == 10)
		entity = Util::make_shared<EntityMinecart>(*multiplayerLevel, x, y, z, 0);
	if (packet.type == 11)
		entity = Util::make_shared<EntityMinecart>(*multiplayerLevel, x, y, z, 1);
	if (packet.type == 12)
		entity = Util::make_shared<EntityMinecart>(*multiplayerLevel, x, y, z, 2);
	if (packet.type == 90)
		entity = Util::make_shared<EntityFish>(*multiplayerLevel, x, y, z);
	if (packet.type == 60)
		entity = Util::make_shared<EntityArrow>(*multiplayerLevel, x, y, z);
	if (packet.type == 61)
		entity = Util::make_shared<EntitySnowball>(*multiplayerLevel, x, y, z);
	if (packet.type == 63)
	{
		entity = Util::make_shared<EntityFireball>(*multiplayerLevel, x, y, z,
			packet.field_28047_e / 8000.0, packet.field_28046_f / 8000.0,
			packet.field_28045_g / 8000.0);
		packet.field_28044_i = 0;
	}
	if (packet.type == 62)
		entity = Util::make_shared<EntityThrownEgg>(*multiplayerLevel, x, y, z);
	if (packet.type == 1)
		entity = Util::make_shared<EntityBoat>(*multiplayerLevel, x, y, z);
	if (packet.type == 50)
		entity = Util::make_shared<PrimedTNT>(*multiplayerLevel, x, y, z);
	if (packet.type == 70)
		entity = Util::make_shared<FallingTile>(*multiplayerLevel, x, y, z, Tile::sand.id);
	if (packet.type == 71)
		entity = Util::make_shared<FallingTile>(*multiplayerLevel, x, y, z, Tile::gravel.id);

	if (entity == nullptr)
		return;
	entity->serverPosX = packet.xPosition;
	entity->serverPosY = packet.yPosition;
	entity->serverPosZ = packet.zPosition;
	entity->yRot = 0.0f;
	entity->xRot = 0.0f;
	entity->entityId = packet.entityId;
	multiplayerLevel->addEntityById(packet.entityId, entity);
	if (packet.field_28044_i > 0)
	{
		if (packet.type == 60)
		{
			std::shared_ptr<Mob> owner = std::dynamic_pointer_cast<Mob>(
				getEntityById(packet.field_28044_i));
			if (owner != nullptr)
				std::static_pointer_cast<EntityArrow>(entity)->owner = owner;
		}
		entity->lerpMotion(packet.field_28047_e / 8000.0,
			packet.field_28046_f / 8000.0, packet.field_28045_g / 8000.0);
	}
}

void NetClientHandler::handleWeather(Packet71Weather &packet)
{
	if (multiplayerLevel == nullptr || packet.field_27055_e != 1)
		return;
	double x = packet.field_27053_b / 32.0;
	double y = packet.field_27057_c / 32.0;
	double z = packet.field_27056_d / 32.0;
	std::shared_ptr<EntityLightningBolt> lightning =
		Util::make_shared<EntityLightningBolt>(*multiplayerLevel, x, y, z);
	lightning->serverPosX = packet.field_27053_b;
	lightning->serverPosY = packet.field_27057_c;
	lightning->serverPosZ = packet.field_27056_d;
	lightning->yRot = 0.0f;
	lightning->xRot = 0.0f;
	lightning->entityId = packet.field_27054_a;
	multiplayerLevel->addWeatherEffect(lightning);
}

void NetClientHandler::func_21146_a(Packet25EntityPainting &packet)
{
	if (multiplayerLevel == nullptr)
		return;
	std::shared_ptr<EntityPainting> painting = Util::make_shared<EntityPainting>(
		*multiplayerLevel, packet.xPosition, packet.yPosition, packet.zPosition,
		packet.direction, packet.title);
	multiplayerLevel->addEntityById(packet.entityId, painting);
}

void NetClientHandler::func_6498_a(Packet28EntityVelocity &packet)
{
	std::shared_ptr<Entity> entity = getEntityById(packet.entityId);
	if (entity != nullptr)
		entity->lerpMotion(packet.motionX / 8000.0, packet.motionY / 8000.0,
			packet.motionZ / 8000.0);
}

void NetClientHandler::func_21148_a(Packet40EntityMetadata &packet)
{
	std::shared_ptr<Entity> entity = getEntityById(packet.entityId);
	WatchableObjectList *metadata = packet.func_21047_b();
	if (entity != nullptr && metadata != nullptr)
		entity->getDataWatcher().updateWatchedObjectsFromList(*metadata);
}

void NetClientHandler::handleNamedEntitySpawn(Packet20NamedEntitySpawn &packet)
{
	if (multiplayerLevel == nullptr)
		return;
	double x = packet.xPosition / 32.0;
	double y = packet.yPosition / 32.0;
	double z = packet.zPosition / 32.0;
	std::shared_ptr<RemotePlayer> player =
		Util::make_shared<RemotePlayer>(*multiplayerLevel, packet.name);
	player->xo = player->xOld = player->serverPosX = packet.xPosition;
	player->yo = player->yOld = player->serverPosY = packet.yPosition;
	player->zo = player->zOld = player->serverPosZ = packet.zPosition;
	player->setEquippedSlot(0, packet.currentItem == 0 ? -1 : packet.currentItem, 0);
	player->absMoveTo(x, y, z, packetAngle(packet.rotation), packetAngle(packet.pitch));
	multiplayerLevel->addEntityById(packet.entityId, player);
}

void NetClientHandler::handleEntityTeleport(Packet34EntityTeleport &packet)
{
	std::shared_ptr<Entity> entity = getEntityById(packet.entityId);
	if (entity == nullptr)
		return;
	entity->serverPosX = packet.xPosition;
	entity->serverPosY = packet.yPosition;
	entity->serverPosZ = packet.zPosition;
	entity->lerpTo(entity->serverPosX / 32.0, entity->serverPosY / 32.0 + 1.0 / 64.0,
		entity->serverPosZ / 32.0, packetAngle(packet.yaw), packetAngle(packet.pitch), 3);
}

void NetClientHandler::handleEntity(Packet30Entity &packet)
{
	std::shared_ptr<Entity> entity = getEntityById(packet.entityId);
	if (entity == nullptr)
		return;
	entity->serverPosX = javaIntAdd(entity->serverPosX, packet.xPosition);
	entity->serverPosY = javaIntAdd(entity->serverPosY, packet.yPosition);
	entity->serverPosZ = javaIntAdd(entity->serverPosZ, packet.zPosition);
	float yRot = packet.rotating ? packetAngle(packet.yaw) : entity->yRot;
	float xRot = packet.rotating ? packetAngle(packet.pitch) : entity->xRot;
	entity->lerpTo(entity->serverPosX / 32.0, entity->serverPosY / 32.0,
		entity->serverPosZ / 32.0, yRot, xRot, 3);
}

void NetClientHandler::handleDestroyEntity(Packet29DestroyEntity &packet)
{
	if (multiplayerLevel != nullptr)
		multiplayerLevel->removeEntityById(packet.entityId);
}

void NetClientHandler::handleCollect(Packet22Collect &packet)
{
	std::shared_ptr<Entity> collected = getEntityById(packet.collectedEntityId);
	std::shared_ptr<Entity> collector = getEntityById(packet.collectorEntityId);
	if (std::dynamic_pointer_cast<Mob>(collector) == nullptr)
		collector = std::static_pointer_cast<Entity>(minecraft.player);
	if (collected == nullptr || collector == nullptr || multiplayerLevel == nullptr)
		return;
	multiplayerLevel->playSoundAtEntity(*collected, u"random.pop", 0.2f,
		((random.nextFloat() - random.nextFloat()) * 0.7f + 1.0f) * 2.0f);
	minecraft.particleEngine.add(std::make_unique<TakeAnimationParticle>(
		*multiplayerLevel, collected, collector, -0.5f));
	multiplayerLevel->removeEntityById(packet.collectedEntityId);
}

void NetClientHandler::handleArmAnimation(Packet18Animation &packet)
{
	std::shared_ptr<Entity> entity = getEntityById(packet.entityId);
	if (entity == nullptr)
		return;
	if (packet.animate == 1)
	{
		std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(entity);
		if (player != nullptr)
			player->swing();
	}
	else if (packet.animate == 2)
	{
		entity->animateHurt();
	}
	else if (packet.animate == 3)
	{
		std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(entity);
		if (player != nullptr)
			player->wakeUpPlayer(false, false, false);
	}
	else if (packet.animate == 4)
	{
		std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(entity);
		if (player != nullptr)
			player->animateRespawn();
	}
}

void NetClientHandler::func_22186_a(Packet17Sleep &packet)
{
	std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(
		getEntityById(packet.field_22045_a));
	if (player != nullptr && packet.field_22046_e == 0)
		player->sleepInBedAt(packet.field_22044_b, packet.field_22048_c,
			packet.field_22047_d);
}

void NetClientHandler::handleMobSpawn(Packet24MobSpawn &packet)
{
	if (multiplayerLevel == nullptr)
		return;
	std::shared_ptr<Entity> created = EntityIO::newEntity(packet.type, *multiplayerLevel);
	std::shared_ptr<Mob> mob = std::dynamic_pointer_cast<Mob>(created);
	if (mob == nullptr)
		throw std::runtime_error("Mob packet contained a non-mob entity type");
	mob->serverPosX = packet.xPosition;
	mob->serverPosY = packet.yPosition;
	mob->serverPosZ = packet.zPosition;
	mob->entityId = packet.entityId;
	mob->absMoveTo(packet.xPosition / 32.0, packet.yPosition / 32.0,
		packet.zPosition / 32.0, packetAngle(packet.yaw), packetAngle(packet.pitch));
	mob->interpolateOnly = true;
	multiplayerLevel->addEntityById(packet.entityId, mob);
	WatchableObjectList *metadata = packet.getMetadata();
	if (metadata != nullptr)
		mob->getDataWatcher().updateWatchedObjectsFromList(*metadata);
}

void NetClientHandler::func_6497_a(Packet39AttachEntity &packet)
{
	std::shared_ptr<Entity> entity = getEntityById(packet.entityId);
	std::shared_ptr<Entity> vehicle = getEntityById(packet.vehicleEntityId);
	if (minecraft.player != nullptr && packet.entityId == minecraft.player->entityId)
		entity = std::static_pointer_cast<Entity>(minecraft.player);
	if (entity != nullptr)
		entity->ride(vehicle);
}

void NetClientHandler::func_9447_a(Packet38EntityStatus &packet)
{
	std::shared_ptr<Entity> entity = getEntityById(packet.entityId);
	if (entity != nullptr)
		entity->handleEntityEvent(packet.entityStatus);
}

void NetClientHandler::handleHealth(Packet8UpdateHealth &packet)
{
	std::shared_ptr<MultiplayerLocalPlayer> player =
		std::dynamic_pointer_cast<MultiplayerLocalPlayer>(minecraft.player);
	if (player != nullptr)
		player->setHealth(packet.healthMP);
}

void NetClientHandler::func_9448_a(Packet9Respawn &packet)
{
	if (minecraft.player == nullptr || multiplayerLevel == nullptr)
		return;
	if (packet.field_28048_a != minecraft.player->dimension)
	{
		receivedFirstPosition = false;
		long_t seed = multiplayerLevel->seed;
		std::shared_ptr<MultiplayerLevel> previousLevel = multiplayerLevel;
		multiplayerLevel = Util::make_shared<MultiplayerLevel>(*this, seed, packet.field_28048_a);
		multiplayerLevel->shareItemDataWith(*previousLevel);
		multiplayerLevel->isOnline = true;
		minecraft.setLevel(multiplayerLevel);
		minecraft.player->dimension = packet.field_28048_a;
		minecraft.setScreen(Util::make_shared<DownloadTerrainScreen>(minecraft, shared_from_this()));
	}
	minecraft.respawnPlayer(packet.field_28048_a);
}

void NetClientHandler::func_12245_a(Packet60Explosion &packet)
{
	if (multiplayerLevel == nullptr)
		return;
	Explosion explosion(*multiplayerLevel, nullptr, packet.explosionX, packet.explosionY,
		packet.explosionZ, packet.explosionSize);
	explosion.destroyedBlockPositions = packet.destroyedBlockPositions;
	explosion.doExplosionB(true);
}

void NetClientHandler::func_20087_a(Packet100OpenWindow &packet)
{
	if (minecraft.player == nullptr)
		return;
	if (packet.inventoryType == 0)
	{
		std::shared_ptr<BasicInventory> inventory =
			Util::make_shared<BasicInventory>(packet.windowTitle, packet.slotsCount);
		minecraft.player->startChest(inventory);
		minecraft.player->craftingInventory->windowId = packet.windowId;
	}
	else if (packet.inventoryType == 2)
	{
		std::shared_ptr<FurnaceTileEntity> furnace = Util::make_shared<FurnaceTileEntity>();
		minecraft.player->startFurnace(furnace);
		minecraft.player->craftingInventory->windowId = packet.windowId;
	}
	else if (packet.inventoryType == 3)
	{
		std::shared_ptr<DispenserTileEntity> dispenser =
			Util::make_shared<DispenserTileEntity>();
		minecraft.player->startDispenser(dispenser);
		minecraft.player->craftingInventory->windowId = packet.windowId;
	}
	else if (packet.inventoryType == 1)
	{
		minecraft.player->startCrafting(Mth::floor(minecraft.player->x),
			Mth::floor(minecraft.player->y), Mth::floor(minecraft.player->z));
		minecraft.player->craftingInventory->windowId = packet.windowId;
	}
}

void NetClientHandler::func_20088_a(Packet103SetSlot &packet)
{
	if (minecraft.player == nullptr)
		return;
	if (packet.windowId == -1)
	{
		if (packet.myItemStack != nullptr)
			minecraft.player->inventory.setCarried(*packet.myItemStack);
		else
			minecraft.player->inventory.setCarriedNull();
	}
	else if (packet.windowId == 0 && packet.itemSlot >= 36 && packet.itemSlot < 45)
	{
		ItemInstance *current = minecraft.player->inventorySlots->getSlot(packet.itemSlot).getItem();
		if (packet.myItemStack != nullptr &&
			(current == nullptr || current->stackSize < packet.myItemStack->stackSize))
		{
			packet.myItemStack->popTime = 5;
		}
		minecraft.player->inventorySlots->putStackInSlot(packet.itemSlot,
			packet.myItemStack.get());
	}
	else if (minecraft.player->craftingInventory != nullptr &&
		packet.windowId == minecraft.player->craftingInventory->windowId)
	{
		minecraft.player->craftingInventory->putStackInSlot(packet.itemSlot,
			packet.myItemStack.get());
	}
}

void NetClientHandler::func_20089_a(Packet106Transaction &packet)
{
	if (minecraft.player == nullptr)
		return;
	std::shared_ptr<Container> container;
	if (packet.windowId == 0)
		container = minecraft.player->inventorySlots;
	else if (minecraft.player->craftingInventory != nullptr &&
		packet.windowId == minecraft.player->craftingInventory->windowId)
		container = minecraft.player->craftingInventory;

	if (container != nullptr)
	{
		if (packet.field_20030_c)
		{
			container->transactionAccepted(packet.field_20028_b);
		}
		else
		{
			container->transactionRejected(packet.field_20028_b);
			addToSendQueue(std::make_unique<Packet106Transaction>(
				packet.windowId, packet.field_20028_b, true));
		}
	}
}

void NetClientHandler::func_20094_a(Packet104WindowItems &packet)
{
	if (minecraft.player == nullptr)
		return;
	if (packet.windowId == 0)
		minecraft.player->inventorySlots->putStacksInSlots(packet.itemStack);
	else if (minecraft.player->craftingInventory != nullptr &&
		packet.windowId == minecraft.player->craftingInventory->windowId)
		minecraft.player->craftingInventory->putStacksInSlots(packet.itemStack);
}

void NetClientHandler::func_20090_a(Packet105UpdateProgressbar &packet)
{
	registerPacket(packet);
	if (minecraft.player != nullptr && minecraft.player->craftingInventory != nullptr &&
		minecraft.player->craftingInventory->windowId == packet.windowId)
	{
		minecraft.player->craftingInventory->updateProgressBar(
			packet.progressBar, packet.progressBarValue);
	}
}

void NetClientHandler::func_20092_a(Packet101CloseWindow &)
{
	if (minecraft.player != nullptr)
		minecraft.player->closeContainer();
}

void NetClientHandler::handlePlayerInventory(Packet5PlayerInventory &packet)
{
	std::shared_ptr<Entity> entity = getEntityById(packet.entityID);
	if (entity != nullptr)
		entity->setEquippedSlot(packet.slot, packet.itemID, packet.itemDamage);
}

void NetClientHandler::handleSignUpdate(Packet130UpdateSign &packet)
{
	if (multiplayerLevel == nullptr ||
		!multiplayerLevel->hasChunkAt(packet.xPosition, packet.yPosition, packet.zPosition))
		return;
	std::shared_ptr<SignTileEntity> sign = std::dynamic_pointer_cast<SignTileEntity>(
		multiplayerLevel->getTileEntity(packet.xPosition, packet.yPosition, packet.zPosition));
	if (sign == nullptr)
		return;
	for (int_t i = 0; i < 4; ++i)
		sign->signText.at(static_cast<size_t>(i)) = packet.signLines.at(static_cast<size_t>(i));
	sign->setChanged();
}

void NetClientHandler::func_25118_a(Packet70Bed &packet)
{
	int_t event = packet.field_25019_b;
	if (minecraft.player != nullptr && event >= 0 &&
		event < static_cast<int_t>(Packet70Bed::field_25020_a.size()) &&
		Packet70Bed::field_25020_a.at(static_cast<size_t>(event)) != nullptr)
	{
		minecraft.player->displayClientMessage(
			*Packet70Bed::field_25020_a.at(static_cast<size_t>(event)));
	}
	if (multiplayerLevel == nullptr)
		return;
	if (event == 1)
	{
		multiplayerLevel->setRaining(true);
		multiplayerLevel->setRainingStrength(1.0f);
	}
	else if (event == 2)
	{
		multiplayerLevel->setRaining(false);
		multiplayerLevel->setRainingStrength(0.0f);
	}
}

void NetClientHandler::func_28116_a(Packet131MapData &packet)
{
	if (multiplayerLevel == nullptr)
		return;
	if (Items::map != nullptr && packet.field_28055_a == Items::map->getShiftedIndex())
	{
		MapData *data = ItemMap::getMapData(packet.field_28054_b, *multiplayerLevel);
		if (data != nullptr)
			data->updateData(packet.field_28056_c);
	}
	else
	{
		std::cout << "Unknown itemid: " << packet.field_28054_b << '\n';
	}
}

void NetClientHandler::func_27245_a(Packet200Statistic &packet)
{
	std::shared_ptr<MultiplayerLocalPlayer> player =
		std::dynamic_pointer_cast<MultiplayerLocalPlayer>(minecraft.player);
	StatBase *stat = StatList::getStat(packet.field_27052_a);
	if (player != nullptr && stat != nullptr)
		player->awardServerStat(*stat, packet.field_27051_b);
}

void NetClientHandler::func_28115_a(Packet61DoorChange &packet)
{
	if (multiplayerLevel != nullptr)
		multiplayerLevel->levelEvent(packet.field_28050_a, packet.field_28053_c,
			packet.field_28052_d, packet.field_28051_e, packet.field_28049_b);
}

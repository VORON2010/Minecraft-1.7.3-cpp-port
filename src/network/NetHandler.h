#pragma once

#include <vector>

#include "java/String.h"

class Packet;
class Packet1Login;
class Packet2Handshake;
class Packet3Chat;
class Packet4UpdateTime;
class Packet5PlayerInventory;
class Packet6SpawnPosition;
class Packet7UseEntity;
class Packet8UpdateHealth;
class Packet9Respawn;
class Packet10Flying;
class Packet14BlockDig;
class Packet15Place;
class Packet16BlockItemSwitch;
class Packet17Sleep;
class Packet18Animation;
class Packet19EntityAction;
class Packet20NamedEntitySpawn;
class Packet21PickupSpawn;
class Packet22Collect;
class Packet23VehicleSpawn;
class Packet24MobSpawn;
class Packet25EntityPainting;
class Packet27Position;
class Packet28EntityVelocity;
class Packet29DestroyEntity;
class Packet30Entity;
class Packet34EntityTeleport;
class Packet38EntityStatus;
class Packet39AttachEntity;
class Packet40EntityMetadata;
class Packet50PreChunk;
class Packet51MapChunk;
class Packet52MultiBlockChange;
class Packet53BlockChange;
class Packet54PlayNoteBlock;
class Packet60Explosion;
class Packet61DoorChange;
class Packet70Bed;
class Packet71Weather;
class Packet100OpenWindow;
class Packet101CloseWindow;
class Packet102WindowClick;
class Packet103SetSlot;
class Packet104WindowItems;
class Packet105UpdateProgressbar;
class Packet106Transaction;
class Packet130UpdateSign;
class Packet131MapData;
class Packet200Statistic;
class Packet255KickDisconnect;

class NetHandler
{
public:
	virtual ~NetHandler() = default;

	virtual bool isServerHandler() const = 0;
	virtual void registerPacket(Packet &packet);
	virtual void handleErrorMessage(const jstring &reason, const std::vector<jstring> &arguments = {});
	virtual void handleLogin(Packet1Login &packet);
	virtual void handleHandshake(Packet2Handshake &packet);
	virtual void handleChat(Packet3Chat &packet);
	virtual void handleUpdateTime(Packet4UpdateTime &packet);
	virtual void handlePlayerInventory(Packet5PlayerInventory &packet);
	virtual void handleSpawnPosition(Packet6SpawnPosition &packet);
	virtual void handleUseEntity(Packet7UseEntity &packet);
	virtual void handleHealth(Packet8UpdateHealth &packet);
	virtual void func_9448_a(Packet9Respawn &packet);
	virtual void handleFlying(Packet10Flying &packet);
	virtual void handleBlockDig(Packet14BlockDig &packet);
	virtual void handlePlace(Packet15Place &packet);
	virtual void handleBlockItemSwitch(Packet16BlockItemSwitch &packet);
	virtual void func_22186_a(Packet17Sleep &packet);
	virtual void handleArmAnimation(Packet18Animation &packet);
	virtual void func_21147_a(Packet19EntityAction &packet);
	virtual void handleNamedEntitySpawn(Packet20NamedEntitySpawn &packet);
	virtual void handlePickupSpawn(Packet21PickupSpawn &packet);
	virtual void handleCollect(Packet22Collect &packet);
	virtual void handleVehicleSpawn(Packet23VehicleSpawn &packet);
	virtual void handleMobSpawn(Packet24MobSpawn &packet);
	virtual void func_21146_a(Packet25EntityPainting &packet);
	virtual void func_22185_a(Packet27Position &packet);
	virtual void func_6498_a(Packet28EntityVelocity &packet);
	virtual void handleDestroyEntity(Packet29DestroyEntity &packet);
	virtual void handleEntity(Packet30Entity &packet);
	virtual void handleEntityTeleport(Packet34EntityTeleport &packet);
	virtual void func_9447_a(Packet38EntityStatus &packet);
	virtual void func_6497_a(Packet39AttachEntity &packet);
	virtual void func_21148_a(Packet40EntityMetadata &packet);
	virtual void handlePreChunk(Packet50PreChunk &packet);
	virtual void handleMapChunk(Packet51MapChunk &packet);
	virtual void handleMultiBlockChange(Packet52MultiBlockChange &packet);
	virtual void handleBlockChange(Packet53BlockChange &packet);
	virtual void handleNotePlay(Packet54PlayNoteBlock &packet);
	virtual void func_12245_a(Packet60Explosion &packet);
	virtual void func_28115_a(Packet61DoorChange &packet);
	virtual void func_25118_a(Packet70Bed &packet);
	virtual void handleWeather(Packet71Weather &packet);
	virtual void func_20087_a(Packet100OpenWindow &packet);
	virtual void func_20092_a(Packet101CloseWindow &packet);
	virtual void func_20091_a(Packet102WindowClick &packet);
	virtual void func_20088_a(Packet103SetSlot &packet);
	virtual void func_20094_a(Packet104WindowItems &packet);
	virtual void func_20090_a(Packet105UpdateProgressbar &packet);
	virtual void func_20089_a(Packet106Transaction &packet);
	virtual void handleSignUpdate(Packet130UpdateSign &packet);
	virtual void func_28116_a(Packet131MapData &packet);
	virtual void func_27245_a(Packet200Statistic &packet);
	virtual void handleKickDisconnect(Packet255KickDisconnect &packet);
};

#include "network/NetHandler.h"

#include "network/Packet.h"
#include "network/PacketCore.h"
#include "network/PacketEntity.h"
#include "network/PacketWindow.h"
#include "network/PacketWorld.h"

void NetHandler::registerPacket(Packet &)
{
}

void NetHandler::handleErrorMessage(const jstring &, const std::vector<jstring> &)
{
}

void NetHandler::handleLogin(Packet1Login &packet)
{
	registerPacket(packet);
}

void NetHandler::handleHandshake(Packet2Handshake &packet)
{
	registerPacket(packet);
}

void NetHandler::handleChat(Packet3Chat &packet)
{
	registerPacket(packet);
}

void NetHandler::handleUpdateTime(Packet4UpdateTime &packet)
{
	registerPacket(packet);
}

void NetHandler::handlePlayerInventory(Packet5PlayerInventory &packet)
{
	registerPacket(packet);
}

void NetHandler::handleSpawnPosition(Packet6SpawnPosition &packet)
{
	registerPacket(packet);
}

void NetHandler::handleUseEntity(Packet7UseEntity &packet)
{
	registerPacket(packet);
}

void NetHandler::handleHealth(Packet8UpdateHealth &packet)
{
	registerPacket(packet);
}

void NetHandler::func_9448_a(Packet9Respawn &packet)
{
	registerPacket(packet);
}

void NetHandler::handleFlying(Packet10Flying &packet)
{
	registerPacket(packet);
}

void NetHandler::handleBlockDig(Packet14BlockDig &packet)
{
	registerPacket(packet);
}

void NetHandler::handlePlace(Packet15Place &packet)
{
	registerPacket(packet);
}

void NetHandler::handleBlockItemSwitch(Packet16BlockItemSwitch &packet)
{
	registerPacket(packet);
}

void NetHandler::func_22186_a(Packet17Sleep &packet)
{
	registerPacket(packet);
}

void NetHandler::handleArmAnimation(Packet18Animation &packet)
{
	registerPacket(packet);
}

void NetHandler::func_21147_a(Packet19EntityAction &packet)
{
	registerPacket(packet);
}

void NetHandler::handleNamedEntitySpawn(Packet20NamedEntitySpawn &packet)
{
	registerPacket(packet);
}

void NetHandler::handlePickupSpawn(Packet21PickupSpawn &packet)
{
	registerPacket(packet);
}

void NetHandler::handleCollect(Packet22Collect &packet)
{
	registerPacket(packet);
}

void NetHandler::handleVehicleSpawn(Packet23VehicleSpawn &packet)
{
	registerPacket(packet);
}

void NetHandler::handleMobSpawn(Packet24MobSpawn &packet)
{
	registerPacket(packet);
}

void NetHandler::func_21146_a(Packet25EntityPainting &packet)
{
	registerPacket(packet);
}

void NetHandler::func_22185_a(Packet27Position &packet)
{
	registerPacket(packet);
}

void NetHandler::func_6498_a(Packet28EntityVelocity &packet)
{
	registerPacket(packet);
}

void NetHandler::handleDestroyEntity(Packet29DestroyEntity &packet)
{
	registerPacket(packet);
}

void NetHandler::handleEntity(Packet30Entity &packet)
{
	registerPacket(packet);
}

void NetHandler::handleEntityTeleport(Packet34EntityTeleport &packet)
{
	registerPacket(packet);
}

void NetHandler::func_9447_a(Packet38EntityStatus &packet)
{
	registerPacket(packet);
}

void NetHandler::func_6497_a(Packet39AttachEntity &packet)
{
	registerPacket(packet);
}

void NetHandler::func_21148_a(Packet40EntityMetadata &packet)
{
	registerPacket(packet);
}

void NetHandler::handlePreChunk(Packet50PreChunk &packet)
{
	registerPacket(packet);
}

void NetHandler::handleMapChunk(Packet51MapChunk &)
{
}

void NetHandler::handleMultiBlockChange(Packet52MultiBlockChange &packet)
{
	registerPacket(packet);
}

void NetHandler::handleBlockChange(Packet53BlockChange &packet)
{
	registerPacket(packet);
}

void NetHandler::handleNotePlay(Packet54PlayNoteBlock &packet)
{
	registerPacket(packet);
}

void NetHandler::func_12245_a(Packet60Explosion &packet)
{
	registerPacket(packet);
}

void NetHandler::func_28115_a(Packet61DoorChange &packet)
{
	registerPacket(packet);
}

void NetHandler::func_25118_a(Packet70Bed &packet)
{
	registerPacket(packet);
}

void NetHandler::handleWeather(Packet71Weather &packet)
{
	registerPacket(packet);
}

void NetHandler::handleSignUpdate(Packet130UpdateSign &packet)
{
	registerPacket(packet);
}

void NetHandler::func_28116_a(Packet131MapData &packet)
{
	registerPacket(packet);
}

void NetHandler::func_20087_a(Packet100OpenWindow &packet)
{
	registerPacket(packet);
}

void NetHandler::func_20092_a(Packet101CloseWindow &packet)
{
	registerPacket(packet);
}

void NetHandler::func_20091_a(Packet102WindowClick &packet)
{
	registerPacket(packet);
}

void NetHandler::func_20088_a(Packet103SetSlot &packet)
{
	registerPacket(packet);
}

void NetHandler::func_20094_a(Packet104WindowItems &packet)
{
	registerPacket(packet);
}

void NetHandler::func_20090_a(Packet105UpdateProgressbar &packet)
{
	registerPacket(packet);
}

void NetHandler::func_20089_a(Packet106Transaction &packet)
{
	registerPacket(packet);
}

void NetHandler::func_27245_a(Packet200Statistic &packet)
{
	registerPacket(packet);
}

void NetHandler::handleKickDisconnect(Packet255KickDisconnect &packet)
{
	registerPacket(packet);
}

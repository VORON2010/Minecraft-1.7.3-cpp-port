#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "network/NetHandler.h"
#include "java/Random.h"

class Entity;
class Minecraft;
class MultiplayerLevel;
class NetworkManager;

class NetClientHandler final : public NetHandler, public std::enable_shared_from_this<NetClientHandler>
{
private:
	Minecraft &minecraft;
	std::unique_ptr<NetworkManager> networkManager;
	std::shared_ptr<MultiplayerLevel> multiplayerLevel;
	bool disconnected = false;
	bool receivedFirstPosition = false;
	Random random;

	explicit NetClientHandler(Minecraft &minecraft);
	void start(const std::string &host, std::uint16_t port);
	std::shared_ptr<Entity> getEntityById(int_t entityId) const;

public:
	jstring field_1209_a;

	static std::shared_ptr<NetClientHandler> connect(Minecraft &minecraft,
		const std::string &host, std::uint16_t port);
	~NetClientHandler() override;

	bool isServerHandler() const override;
	bool isDisconnected() const;
	std::shared_ptr<MultiplayerLevel> getLevel() const;

	void processReadPackets();
	void addToSendQueue(std::unique_ptr<Packet> packet);
	void sendAndQuit(std::unique_ptr<Packet> packet);
	void disconnect();

	void handleLogin(Packet1Login &packet) override;
	void handleHandshake(Packet2Handshake &packet) override;
	void handleChat(Packet3Chat &packet) override;
	void handleUpdateTime(Packet4UpdateTime &packet) override;
	void handlePlayerInventory(Packet5PlayerInventory &packet) override;
	void handleSpawnPosition(Packet6SpawnPosition &packet) override;
	void handleHealth(Packet8UpdateHealth &packet) override;
	void func_9448_a(Packet9Respawn &packet) override;
	void handleFlying(Packet10Flying &packet) override;
	void func_22186_a(Packet17Sleep &packet) override;
	void handleArmAnimation(Packet18Animation &packet) override;
	void handleNamedEntitySpawn(Packet20NamedEntitySpawn &packet) override;
	void handlePickupSpawn(Packet21PickupSpawn &packet) override;
	void handleCollect(Packet22Collect &packet) override;
	void handleVehicleSpawn(Packet23VehicleSpawn &packet) override;
	void handleMobSpawn(Packet24MobSpawn &packet) override;
	void func_21146_a(Packet25EntityPainting &packet) override;
	void func_6498_a(Packet28EntityVelocity &packet) override;
	void handleDestroyEntity(Packet29DestroyEntity &packet) override;
	void handleEntity(Packet30Entity &packet) override;
	void handleEntityTeleport(Packet34EntityTeleport &packet) override;
	void func_9447_a(Packet38EntityStatus &packet) override;
	void func_6497_a(Packet39AttachEntity &packet) override;
	void func_21148_a(Packet40EntityMetadata &packet) override;
	void handlePreChunk(Packet50PreChunk &packet) override;
	void handleMapChunk(Packet51MapChunk &packet) override;
	void handleMultiBlockChange(Packet52MultiBlockChange &packet) override;
	void handleBlockChange(Packet53BlockChange &packet) override;
	void handleNotePlay(Packet54PlayNoteBlock &packet) override;
	void func_12245_a(Packet60Explosion &packet) override;
	void func_28115_a(Packet61DoorChange &packet) override;
	void func_25118_a(Packet70Bed &packet) override;
	void handleWeather(Packet71Weather &packet) override;
	void func_20087_a(Packet100OpenWindow &packet) override;
	void func_20092_a(Packet101CloseWindow &packet) override;
	void func_20088_a(Packet103SetSlot &packet) override;
	void func_20094_a(Packet104WindowItems &packet) override;
	void func_20090_a(Packet105UpdateProgressbar &packet) override;
	void func_20089_a(Packet106Transaction &packet) override;
	void handleSignUpdate(Packet130UpdateSign &packet) override;
	void func_28116_a(Packet131MapData &packet) override;
	void func_27245_a(Packet200Statistic &packet) override;
	void handleKickDisconnect(Packet255KickDisconnect &packet) override;
	void handleErrorMessage(const jstring &reason, const std::vector<jstring> &arguments) override;
};

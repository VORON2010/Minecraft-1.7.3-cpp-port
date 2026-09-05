#include "client/player/MultiplayerLocalPlayer.h"

#include <memory>

#include "network/NetClientHandler.h"
#include "network/PacketCore.h"
#include "network/PacketWindow.h"
#include "util/Mth.h"
#include "world/inventory/Container.h"
#include "world/level/Level.h"
#include "world/stats/StatBase.h"

MultiplayerLocalPlayer::MultiplayerLocalPlayer(Minecraft &minecraft, Level &level, User *user,
	NetClientHandler &sendQueue) : LocalPlayer(minecraft, level, user, 0), sendQueue(sendQueue)
{
}

bool MultiplayerLocalPlayer::hurt(Entity *source, int_t dmg)
{
	(void)source;
	(void)dmg;
	return false;
}

void MultiplayerLocalPlayer::heal(int_t amount)
{
	(void)amount;
}

void MultiplayerLocalPlayer::tick()
{
	if (level.hasChunkAt(Mth::floor(x), 64, Mth::floor(z)))
	{
		LocalPlayer::tick();
		sendMotionUpdates();
	}
}

void MultiplayerLocalPlayer::sendMotionUpdates()
{
	if (inventoryUpdateTicks++ == 20)
	{
		sendInventoryChanged();
		inventoryUpdateTicks = 0;
	}

	bool sneaking = isSneaking();
	if (sneaking != wasSneaking)
	{
		sendQueue.addToSendQueue(std::make_unique<Packet19EntityAction>(*this, sneaking ? 1 : 2));
		wasSneaking = sneaking;
	}

	double xDelta = x - lastSentX;
	double stanceDelta = bb.y0 - lastSentStance;
	double yDelta = y - lastSentY;
	double zDelta = z - lastSentZ;
	double yRotDelta = yRot - lastSentYRot;
	double xRotDelta = xRot - lastSentXRot;
	bool moved = stanceDelta != 0.0 || yDelta != 0.0 || xDelta != 0.0 || zDelta != 0.0;
	bool rotated = yRotDelta != 0.0 || xRotDelta != 0.0;

	if (riding != nullptr)
	{
		if (rotated)
			sendQueue.addToSendQueue(std::make_unique<Packet11PlayerPosition>(xd, -999.0, -999.0, zd, onGround));
		else
			sendQueue.addToSendQueue(std::make_unique<Packet13PlayerLookMove>(xd, -999.0, -999.0, zd,
				yRot, xRot, onGround));

		moved = false;
	}
	else if (moved && rotated)
	{
		sendQueue.addToSendQueue(std::make_unique<Packet13PlayerLookMove>(x, bb.y0, y, z,
			yRot, xRot, onGround));
		ticksSinceLastMovePacket = 0;
	}
	else if (moved)
	{
		sendQueue.addToSendQueue(std::make_unique<Packet11PlayerPosition>(x, bb.y0, y, z, onGround));
		ticksSinceLastMovePacket = 0;
	}
	else if (rotated)
	{
		sendQueue.addToSendQueue(std::make_unique<Packet12PlayerLook>(yRot, xRot, onGround));
		ticksSinceLastMovePacket = 0;
	}
	else
	{
		sendQueue.addToSendQueue(std::make_unique<Packet10Flying>(onGround));
		if (lastSentOnGround == onGround && ticksSinceLastMovePacket <= 200)
			++ticksSinceLastMovePacket;
		else
			ticksSinceLastMovePacket = 0;
	}

	lastSentOnGround = onGround;
	if (moved)
	{
		lastSentX = x;
		lastSentStance = bb.y0;
		lastSentY = y;
		lastSentZ = z;
	}

	if (rotated)
	{
		lastSentYRot = yRot;
		lastSentXRot = xRot;
	}
}

void MultiplayerLocalPlayer::drop()
{
	sendQueue.addToSendQueue(std::make_unique<Packet14BlockDig>(4, 0, 0, 0, 0));
}

void MultiplayerLocalPlayer::reallyDrop(std::shared_ptr<EntityItem> itemEntity)
{
	(void)itemEntity;
}

void MultiplayerLocalPlayer::sendInventoryChanged()
{
}

void MultiplayerLocalPlayer::sendChatMessage(const jstring &message)
{
	sendQueue.addToSendQueue(std::make_unique<Packet3Chat>(message));
}

void MultiplayerLocalPlayer::swing()
{
	LocalPlayer::swing();
	sendQueue.addToSendQueue(std::make_unique<Packet18Animation>(*this, 1));
}

void MultiplayerLocalPlayer::respawn()
{
	sendInventoryChanged();
	sendQueue.addToSendQueue(std::make_unique<Packet9Respawn>(static_cast<byte_t>(dimension)));
}

void MultiplayerLocalPlayer::actuallyHurt(int_t dmg)
{
	health -= dmg;
}

void MultiplayerLocalPlayer::closeContainer()
{
	sendQueue.addToSendQueue(std::make_unique<Packet101CloseWindow>(craftingInventory->windowId));
	inventory.setCarriedNull();
	LocalPlayer::closeContainer();
}

void MultiplayerLocalPlayer::setHealth(int_t newHealth)
{
	if (!receivedInitialHealth)
	{
		health = newHealth;
		receivedInitialHealth = true;
		return;
	}

	int_t damage = health - newHealth;
	if (damage <= 0)
	{
		health = newHealth;
		if (damage < 0)
			invulnerableTime = invulnerableDuration / 2;
	}
	else
	{
		lastHurt = damage;
		lastHealth = health;
		invulnerableTime = invulnerableDuration;
		actuallyHurt(damage);
		hurtTime = hurtDuration = 10;
	}
}

void MultiplayerLocalPlayer::addStat(const StatBase &stat, int_t amount)
{
	if (stat.independent)
		LocalPlayer::addStat(stat, amount);
}

void MultiplayerLocalPlayer::awardServerStat(const StatBase &stat, int_t amount)
{
	if (!stat.independent)
		LocalPlayer::addStat(stat, amount);
}

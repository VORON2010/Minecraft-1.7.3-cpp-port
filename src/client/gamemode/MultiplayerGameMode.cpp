#include "client/gamemode/MultiplayerGameMode.h"

#include <cmath>

#include "client/Minecraft.h"
#include "client/player/MultiplayerLocalPlayer.h"
#include "network/NetClientHandler.h"
#include "network/PacketCore.h"
#include "network/PacketWindow.h"
#include "util/Memory.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/inventory/Container.h"
#include "world/level/Level.h"
#include "world/level/tile/StepSound.h"
#include "world/level/tile/Tile.h"

MultiplayerGameMode::MultiplayerGameMode(Minecraft &minecraft, NetClientHandler &netClientHandler)
	: GameMode(minecraft), netClientHandler(netClientHandler)
{
}

void MultiplayerGameMode::initPlayer(std::shared_ptr<Player> player)
{
	player->yRot = -180.0f;
}

bool MultiplayerGameMode::destroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	int_t tileId = minecraft.level->getTile(x, y, z);
	bool changed = GameMode::destroyBlock(x, y, z, face);
	ItemInstance *selected = minecraft.player->getSelectedItem();
	if (selected != nullptr)
	{
		selected->mineBlock(tileId, x, y, z, *minecraft.player);
		if (selected->stackSize == 0)
			minecraft.player->removeSelectedItem();
	}

	return changed;
}

void MultiplayerGameMode::startDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	if (!isDestroying || x != xDestroyBlock || y != yDestroyBlock || z != zDestroyBlock)
	{
		netClientHandler.addToSendQueue(std::make_unique<Packet14BlockDig>(
			0, x, y, z, static_cast<int_t>(face)));
		int_t tileId = minecraft.level->getTile(x, y, z);
		if (tileId > 0 && destroyProgress == 0.0f)
			Tile::tiles[tileId]->attack(*minecraft.level, x, y, z, *minecraft.player);

		if (tileId > 0 && Tile::tiles[tileId]->getDestroyProgress(*minecraft.player) >= 1.0f)
		{
			destroyBlock(x, y, z, face);
		}
		else
		{
			isDestroying = true;
			xDestroyBlock = x;
			yDestroyBlock = y;
			zDestroyBlock = z;
			destroyProgress = 0.0f;
			oDestroyProgress = 0.0f;
			destroyTicks = 0.0f;
		}
	}
}

void MultiplayerGameMode::stopDestroyBlock()
{
	destroyProgress = 0.0f;
	isDestroying = false;
}

void MultiplayerGameMode::continueDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	if (isDestroying)
	{
		syncCurrentPlayItem();
		if (destroyDelay > 0)
		{
			destroyDelay--;
		}
		else
		{
			if (x == xDestroyBlock && y == yDestroyBlock && z == zDestroyBlock)
			{
				int_t tileId = minecraft.level->getTile(x, y, z);
				if (tileId == 0)
				{
					isDestroying = false;
					return;
				}

				Tile *tile = Tile::tiles[tileId];
				destroyProgress += tile->getDestroyProgress(*minecraft.player);
				if (std::fmod(destroyTicks, 4.0f) == 0.0f && tile != nullptr)
				{
					StepSound *sound = tile->soundType;
					minecraft.soundEngine.play(sound->getStepResourcePath(),
						static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f,
						static_cast<float>(z) + 0.5f, (sound->getVolume() + 1.0f) / 8.0f,
						sound->getPitch() * 0.5f);
				}

				destroyTicks++;
				if (destroyProgress >= 1.0f)
				{
					isDestroying = false;
					netClientHandler.addToSendQueue(std::make_unique<Packet14BlockDig>(
						2, x, y, z, static_cast<int_t>(face)));
					destroyBlock(x, y, z, face);
					destroyProgress = 0.0f;
					oDestroyProgress = 0.0f;
					destroyTicks = 0.0f;
					destroyDelay = 5;
				}
			}
			else
			{
				startDestroyBlock(x, y, z, face);
			}
		}
	}
}

void MultiplayerGameMode::render(float a)
{
	if (destroyProgress <= 0.0f)
	{
		minecraft.gui.progress = 0.0f;
		minecraft.levelRenderer.destroyProgress = 0.0f;
	}
	else
	{
		float progress = oDestroyProgress + (destroyProgress - oDestroyProgress) * a;
		minecraft.gui.progress = progress;
		minecraft.levelRenderer.destroyProgress = progress;
	}
}

float MultiplayerGameMode::getPickRange()
{
	return 4.0f;
}

void MultiplayerGameMode::initLevel(std::shared_ptr<Level> level)
{
	GameMode::initLevel(level);
}

void MultiplayerGameMode::tick()
{
	syncCurrentPlayItem();
	oDestroyProgress = destroyProgress;
	minecraft.soundEngine.playMusicTick();
}

void MultiplayerGameMode::syncCurrentPlayItem()
{
	int_t selected = minecraft.player->inventory.currentItem;
	if (selected != currentPlayerItem)
	{
		currentPlayerItem = selected;
		netClientHandler.addToSendQueue(std::make_unique<Packet16BlockItemSwitch>(currentPlayerItem));
	}
}

bool MultiplayerGameMode::useItemOn(std::shared_ptr<Player> &player, Level &level, ItemInstance *item,
	int_t x, int_t y, int_t z, Facing face)
{
	syncCurrentPlayItem();
	netClientHandler.addToSendQueue(std::make_unique<Packet15Place>(
		x, y, z, static_cast<int_t>(face), player->inventory.getSelected()));
	return GameMode::useItemOn(player, level, item, x, y, z, face);
}

bool MultiplayerGameMode::useItem(std::shared_ptr<Player> &player, Level &level, ItemInstance *item)
{
	syncCurrentPlayItem();
	netClientHandler.addToSendQueue(std::make_unique<Packet15Place>(
		-1, -1, -1, 255, player->inventory.getSelected()));
	return GameMode::useItem(player, level, item);
}

std::shared_ptr<Player> MultiplayerGameMode::createPlayer(Level &level)
{
	return Util::make_shared<MultiplayerLocalPlayer>(
		minecraft, level, minecraft.user.get(), netClientHandler);
}

void MultiplayerGameMode::attack(std::shared_ptr<Player> &player, std::shared_ptr<Entity> &entity)
{
	syncCurrentPlayItem();
	netClientHandler.addToSendQueue(std::make_unique<Packet7UseEntity>(
		player->entityId, entity->entityId, 1));
	player->attack(entity);
}

void MultiplayerGameMode::interact(std::shared_ptr<Player> &player, std::shared_ptr<Entity> &entity)
{
	syncCurrentPlayItem();
	netClientHandler.addToSendQueue(std::make_unique<Packet7UseEntity>(
		player->entityId, entity->entityId, 0));
	player->interact(entity);
}

std::unique_ptr<ItemInstance> MultiplayerGameMode::clickContainer(int_t windowId, int_t slot,
	int_t button, bool shiftClick, Player &player)
{
	short_t action = player.craftingInventory->getNextTransactionId(player.inventory);
	std::unique_ptr<ItemInstance> result = GameMode::clickContainer(
		windowId, slot, button, shiftClick, player);
	netClientHandler.addToSendQueue(std::make_unique<Packet102WindowClick>(
		windowId, slot, button, shiftClick, result.get(), action));
	return result;
}

void MultiplayerGameMode::closeContainer(int_t windowId, Player &player)
{
	(void)windowId;
	(void)player;
}

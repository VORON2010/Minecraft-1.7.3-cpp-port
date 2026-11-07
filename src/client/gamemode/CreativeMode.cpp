#include "client/gamemode/CreativeMode.h"

#include "client/Minecraft.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/WoodTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/LeafTile.h"
#include "world/level/tile/TorchTile.h"
#include "world/level/tile/SlabTile.h"

namespace
{
	// 1.0 Session.CREATIVE_INVENTORY: the blocks that fill an empty creative hotbar
	int_t creativeHotbarTile(int_t slot)
	{
		const int_t ids[] = {
			Tile::rock.id, Tile::cobblestone.id, Tile::brick.id, Tile::dirt.id, Tile::wood.id, Tile::treeTrunk.id, Tile::leaves.id, Tile::torch.id, Tile::slabSingle.id
		};
		return ids[slot];
	}
}

CreativeMode::CreativeMode(Minecraft &minecraft) : GameMode(minecraft)
{
	instaBuild = true;
}

void CreativeMode::setCreative(Player &player)
{
	player.abilities.mayfly = true;
	player.abilities.instabuild = true;
	player.abilities.invulnerable = true;
}

void CreativeMode::setSurvival(Player &player)
{
	player.abilities.mayfly = false;
	player.abilities.flying = false;
	player.abilities.instabuild = false;
	player.abilities.invulnerable = false;
	player.isFlying = false;
}

void CreativeMode::initPlayer(std::shared_ptr<Player> player)
{
	GameMode::initPlayer(player);
	if (player != nullptr)
		setCreative(*player);
}

void CreativeMode::adjustPlayer(std::shared_ptr<Player> player)
{
	if (player == nullptr)
		return;
	setCreative(*player);
	for (int_t i = 0; i < 9; i++)
	{
		if (player->inventory.mainInventory[i].isEmpty())
			player->inventory.mainInventory[i] = ItemInstance(creativeHotbarTile(i), 1, 0);
	}
}

void CreativeMode::destroyInCreative(int_t x, int_t y, int_t z, Facing face)
{
	minecraft.level->extinguishFire(x, y, z, face);
	destroyBlock(x, y, z, face);
}

void CreativeMode::startDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	destroyInCreative(x, y, z, face);
	destroyDelay = 5;
}

void CreativeMode::continueDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	destroyDelay--;
	if (destroyDelay <= 0)
	{
		destroyDelay = 5;
		destroyInCreative(x, y, z, face);
	}
}

void CreativeMode::stopDestroyBlock()
{
}

bool CreativeMode::useItemOn(std::shared_ptr<Player> &player, Level &level, ItemInstance *item, int_t x, int_t y, int_t z, Facing face)
{
	// 1.0 CreativeInteractionManager: place or use as in survival, then put the stack back untouched
	if (item == nullptr || item->isEmpty())
		return GameMode::useItemOn(player, level, item, x, y, z, face);
	ItemInstance saved = *item;
	bool used = GameMode::useItemOn(player, level, item, x, y, z, face);
	*item = saved;
	return used;
}

float CreativeMode::getPickRange()
{
	return 5.0f;
}

bool CreativeMode::canHurtPlayer()
{
	return false;
}

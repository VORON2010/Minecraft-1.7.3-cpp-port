#include "client/gamemode/GameMode.h"

#include "client/Minecraft.h"
#include "world/item/ItemInstance.h"
#include "world/entity/player/Player.h"
#include "world/inventory/Container.h"
#include "world/level/Level.h"
#include "world/level/tile/StepSound.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/SnowTile.h"
#include "world/phys/AABB.h"
#include "world/stats/StatList.h"

namespace
{
	bool isPlacementReplaceable(Level &level, int_t x, int_t y, int_t z)
	{
		int_t existingId = level.getTile(x, y, z);
		if (existingId == 0 || existingId == Tile::snow.id)
			return true;
		Tile *existingTile = Tile::tiles[existingId];
		return existingTile != nullptr && existingTile->material.isLiquid();
	}

	void moveToPlacementTarget(int_t &x, int_t &y, int_t &z, Facing face)
	{
		switch (face)
		{
		case Facing::DOWN: y--; break;
		case Facing::UP: y++; break;
		case Facing::NORTH: z--; break;
		case Facing::SOUTH: z++; break;
		case Facing::WEST: x--; break;
		case Facing::EAST: x++; break;
		default: break;
		}
	}

	bool canPlaceSelectedTile(Level &level, Tile &tile, int_t x, int_t y, int_t z)
	{
		if (!isPlacementReplaceable(level, x, y, z))
			return false;
		if (!tile.mayPlace(level, x, y, z))
			return false;
		AABB *placementBox = tile.getAABB(level, x, y, z);
		return placementBox == nullptr || level.isUnobstructed(*placementBox);
	}
}

GameMode::GameMode(Minecraft &minecraft) : minecraft(minecraft)
{

}

void GameMode::initLevel(std::shared_ptr<Level> level)
{

}

void GameMode::startDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	destroyBlock(x, y, z, face);
}

bool GameMode::destroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	
	minecraft.particleEngine.destroy(x, y, z);

	Level &level = *minecraft.level;
	Tile *oldTile = Tile::tiles[level.getTile(x, y, z)];
	int_t data = level.getData(x, y, z);
	bool changed = level.setTile(x, y, z, 0);
	if (oldTile != nullptr && changed)
	{
		oldTile->destroy(level, x, y, z, data);

		
		if (oldTile->soundType != nullptr)
		{
			StepSound *ss = oldTile->soundType;
			level.playSoundEffect((double)x + 0.5, (double)y + 0.5, (double)z + 0.5,
				ss->stepSoundDir(), (ss->getVolume() + 1.0f) / 2.0f, ss->getPitch() * 0.8f);
		}
	}
	return changed;
}

void GameMode::continueDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{

}

void GameMode::stopDestroyBlock()
{

}

void GameMode::render(float a)
{

}

float GameMode::getPickRange()
{
	return 5.0f;
}

bool GameMode::useItem(std::shared_ptr<Player> &player, Level &level, ItemInstance *item)
{
	if (item == nullptr || item->isEmpty())
		return false;

	ItemInstance before = *item;
	item->use(level, *player);
	if (item->sameItem(before) && item->stackSize == before.stackSize)
		return false;
	if (item->isEmpty())
		player->removeSelectedItem();
	return true;
}

void GameMode::initPlayer(std::shared_ptr<Player> player)
{

}

void GameMode::tick()
{

}

bool GameMode::canHurtPlayer()
{
	return true;
}

void GameMode::adjustPlayer(std::shared_ptr<Player> player)
{

}

bool GameMode::useItemOn(std::shared_ptr<Player> &player, Level &level, ItemInstance *item, int_t x, int_t y, int_t z, Facing face)
{
	int_t targetTileId = level.getTile(x, y, z);
	if (targetTileId > 0)
	{
		Tile *targetTile = Tile::tiles[targetTileId];
		if (targetTile != nullptr && targetTile->use(level, x, y, z, *player))
			return true;
	}

	if (item == nullptr || item->isEmpty())
		return false;

	bool used = item->useOn(*player, level, x, y, z, face);
	if (used)
	{
		if (item->isEmpty())
			player->removeSelectedItem();
		return true;
	}

	Tile *placedTile = (item->itemID >= 0 && item->itemID < static_cast<int_t>(Tile::tiles.size())) ? Tile::tiles[item->itemID] : nullptr;
	if (placedTile == nullptr)
		return false;
	int_t placeX = x;
	int_t placeY = y;
	int_t placeZ = z;
	Facing placementFace = face;
	if (level.getTile(x, y, z) != Tile::snow.id)
		moveToPlacementTarget(placeX, placeY, placeZ, face);
	else
		placementFace = Facing::DOWN;

	if (!canPlaceSelectedTile(level, *placedTile, placeX, placeY, placeZ))
		return false;

	if (!level.setTileAndData(placeX, placeY, placeZ, placedTile->id, item->itemDamage))
		return false;
	if (level.getTile(placeX, placeY, placeZ) != placedTile->id)
		return false;

	placedTile->setPlacedOnFace(level, placeX, placeY, placeZ, placementFace);
	placedTile->setPlacedBy(level, placeX, placeY, placeZ, *player);
	if (placedTile->soundType != nullptr)
	{
		StepSound *ss = placedTile->soundType;
		level.playSoundEffect((double)placeX + 0.5, (double)placeY + 0.5, (double)placeZ + 0.5,
			ss->getStepResourcePath(), (ss->getVolume() + 1.0f) / 2.0f, ss->getPitch() * 0.8f);
	}

	item->stackSize--;
	if (StatBase *stat = StatList::useItemStats[item->itemID])
		player->addStat(*stat, 1);
	if (item->isEmpty())
		player->removeSelectedItem();
	return true;
}

std::shared_ptr<Player> GameMode::createPlayer(Level &level)
{
	return Util::make_shared<LocalPlayer>(minecraft, level, minecraft.user.get(), level.dimension->id);
}

void GameMode::interact(std::shared_ptr<Player> &player, std::shared_ptr<Entity> &entity)
{
	player->interact(entity);
}

void GameMode::attack(std::shared_ptr<Player> &player, std::shared_ptr<Entity> &entity)
{
	player->attack(entity);
}

std::unique_ptr<ItemInstance> GameMode::clickContainer(int_t windowId, int_t slot, int_t button,
	bool shiftClick, Player &player)
{
	(void)windowId;
	return player.craftingInventory->click(slot, button, shiftClick, player);
}

void GameMode::closeContainer(int_t windowId, Player &player)
{
	(void)windowId;
	player.craftingInventory->onClosed(player);
	player.craftingInventory = player.inventorySlots;
}

#include "client/gamemode/CreativeMode.h"
#include "client/Minecraft.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"

CreativeMode::CreativeMode(Minecraft &minecraft) : GameMode(minecraft)
{
	instaBuild = true;
}

void CreativeMode::initPlayer(std::shared_ptr<Player> player)
{
	GameMode::initPlayer(player);
}

void CreativeMode::initLevel(std::shared_ptr<Level> level)
{
	GameMode::initLevel(level);
}

void CreativeMode::startDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	destroyBlock(x, y, z, face);
}

void CreativeMode::continueDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{
}

bool CreativeMode::destroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	int_t t = minecraft.level->getTile(x, y, z);
	int_t data = minecraft.level->getData(x, y, z);
	if (t > 0)
		minecraft.particleEngine.destroy(x, y, z, t, data);
	
	return GameMode::destroyBlock(x, y, z, face);
}

bool CreativeMode::useItemOn(std::shared_ptr<Player> &player, Level &level, ItemInstance *item, int_t x, int_t y, int_t z, Facing face)
{
	if (item == nullptr) return false;
	int_t initialStackSize = item->stackSize.load(std::memory_order_relaxed);
	int_t initialDamage = item->itemDamage.load(std::memory_order_relaxed);
	
	item->stackSize.store(initialStackSize + 1, std::memory_order_relaxed); // Prevent removeSelectedItem
	
	bool result = GameMode::useItemOn(player, level, item, x, y, z, face);
	item->stackSize.store(initialStackSize, std::memory_order_relaxed);
	item->itemDamage.store(initialDamage, std::memory_order_relaxed);
	return result;
}

bool CreativeMode::useItem(std::shared_ptr<Player> &player, Level &level, ItemInstance *item)
{
	if (item == nullptr) return false;
	int_t initialStackSize = item->stackSize.load(std::memory_order_relaxed);
	int_t initialDamage = item->itemDamage.load(std::memory_order_relaxed);

	item->stackSize.store(initialStackSize + 1, std::memory_order_relaxed); // Prevent removeSelectedItem
	
	bool result = GameMode::useItem(player, level, item);
	item->stackSize.store(initialStackSize, std::memory_order_relaxed);
	item->itemDamage.store(initialDamage, std::memory_order_relaxed);
	return result;
}

float CreativeMode::getPickRange()
{
	return 5.0f; // Creative reach is longer
}

bool CreativeMode::canHurtPlayer()
{
	return false;
}

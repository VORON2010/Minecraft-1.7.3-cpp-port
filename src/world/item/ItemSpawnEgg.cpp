#include "world/item/ItemSpawnEgg.h"
#include "world/entity/EntityIO.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"

ItemSpawnEgg::ItemSpawnEgg(int_t baseId, int_t entityId)
	: Item(baseId), spawnedEntityId(entityId)
{
	setMaxStackSize(64);
}

bool ItemSpawnEgg::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	if (level.isOnline)
		return true;

	if (face == Facing::DOWN) y--;
	if (face == Facing::UP) y++;
	if (face == Facing::NORTH) z--;
	if (face == Facing::SOUTH) z++;
	if (face == Facing::WEST) x--;
	if (face == Facing::EAST) x++;

	auto entity = EntityIO::newEntity(spawnedEntityId, level);
	if (entity != nullptr)
	{
		entity->moveTo(x + 0.5, y, z + 0.5, 0.0f, 0.0f);
		level.addEntity(entity);
		stack.stackSize--;
		return true;
	}

	return false;
}

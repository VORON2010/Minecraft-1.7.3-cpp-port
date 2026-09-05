#include "world/item/ItemPainting.h"

#include <memory>

#include "world/entity/item/EntityPainting.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

bool ItemPainting::useOn(ItemInstance &stack, Player &player, Level &level,
	int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	if (face == Facing::DOWN || face == Facing::UP)
		return false;
	int_t direction = 0;
	if (face == Facing::WEST)
		direction = 1;
	if (face == Facing::SOUTH)
		direction = 2;
	if (face == Facing::EAST)
		direction = 3;
	auto painting = std::make_shared<EntityPainting>(level, x, y, z, direction);
	if (painting->survives())
	{
		if (!level.isOnline)
			level.addEntity(painting);
		stack.stackSize--;
	}
	return true;
}

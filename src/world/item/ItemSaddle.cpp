#include "world/item/ItemSaddle.h"

#include "world/entity/Mob.h"
#include "world/entity/animal/Pig.h"
#include "world/item/ItemInstance.h"

ItemSaddle::ItemSaddle(int_t id) : Item(id)
{
	setMaxStackSize(1);
}

void ItemSaddle::saddleEntity(ItemInstance &stack, Mob &target) const
{
	Pig *pig = dynamic_cast<Pig *>(&target);
	if (pig != nullptr && !pig->isSaddled())
	{
		pig->setSaddled(true);
		stack.stackSize--;
	}
}

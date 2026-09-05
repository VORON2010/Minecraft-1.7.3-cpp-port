#include "world/entity/animal/Cow.h"

#include "world/entity/player/Player.h"
#include "world/item/Items.h"
#include "world/item/Item.h"

Cow::Cow(Level &level) : Animal(level)
{
	textureName = u"/mob/cow.png";
	setSize(0.9f, 1.3f);
}

bool Cow::interact(Player &player)
{
	ItemInstance *selected = player.getSelectedItem();
	if (selected != nullptr && selected->itemID == Items::bucketEmpty->getShiftedIndex())
	{
		player.inventory.setItem(player.inventory.currentItem, ItemInstance(Items::bucketMilk->getShiftedIndex(), 1, 0));
		return true;
	}
	return false;
}

jstring Cow::getAmbientSound()
{
	return u"mob.cow";
}

jstring Cow::getHurtSound()
{
	return u"mob.cowhurt";
}

jstring Cow::getDeathSound()
{
	return u"mob.cowhurt";
}

float Cow::getSoundVolume()
{
	return 0.4f;
}

int_t Cow::getDeathLoot()
{
	return Items::leather->getShiftedIndex();
}

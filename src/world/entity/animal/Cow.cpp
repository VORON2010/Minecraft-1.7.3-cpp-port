#include "world/level/Level.h"
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
	return Animal::interact(player);
}

jstring Cow::getAmbientSound()
{
	return u"mob.cow.say";
}

jstring Cow::getHurtSound()
{
	return u"mob.cow.hurt";
}

jstring Cow::getDeathSound()
{
	return u"mob.cow.hurt";
}

float Cow::getSoundVolume()
{
	return 0.4f;
}

int_t Cow::getDeathLoot()
{
	return Items::leather->getShiftedIndex();
}

void Cow::dropDeathLoot()
{
	Animal::dropDeathLoot();
	int_t count = (random.nextInt(100) < 5) ? 2 : 1;
	if (lootBonus > 0)
		count += random.nextInt(1 + lootBonus);
	for (int_t i = 0; i < count; ++i)
		spawnAtLocation(ItemInstance(onFire > 0 ? Items::beefCooked->getShiftedIndex() : Items::beefRaw->getShiftedIndex(), 1, 0), 0.0f);
}

std::shared_ptr<Animal> Cow::getBreedOffspring(Animal &partner)
{
	(void)partner;
	return std::make_shared<Cow>(level);
}

// 1.5.2 playStepSound
void Cow::playStepSound(int_t x, int_t y, int_t z, int_t tile)
{
	(void)x; (void)y; (void)z; (void)tile;
	level.playSoundAtEntity(*this, u"mob.cow.step", 0.15f, 1.0f);
}

#include "world/entity/animal/Animal.h"

#include <cmath>

#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/level/Level.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/Tile.h"

Animal::Animal(Level &level) : PathfinderMob(level)
{
}

int_t Animal::getAmbientSoundInterval()
{
	return 120;
}

float Animal::getWalkTargetValue(int_t x, int_t y, int_t z)
{
	return level.getTile(x, y - 1, z) == Tile::grass.id ? 10.0f : level.getBrightness(x, y, z) - 0.5f;
}

bool Animal::canSpawn()
{
	int_t x = Mth::floor(this->x);
	int_t y = Mth::floor(bb.y0);
	int_t z = Mth::floor(this->z);
	return level.getTile(x, y - 1, z) == Tile::grass.id && level.getFullBrightness(x, y, z) > 8 && PathfinderMob::canSpawn();
}

bool Animal::isFood(const ItemInstance &item) const
{
	return item.itemID == Items::wheat->getShiftedIndex();
}

void Animal::hearts(int_t count, bool moving)
{
	for (int_t i = 0; i < count; ++i)
	{
		double xa = moving ? random.nextGaussian() * 0.02 : 0.0;
		double ya = moving ? random.nextGaussian() * 0.02 : 0.0;
		double za = moving ? random.nextGaussian() * 0.02 : 0.0;
		level.addParticle(u"heart", x + random.nextFloat() * bbWidth * 2.0f - bbWidth, y + 0.5 + random.nextFloat() * bbHeight,
			z + random.nextFloat() * bbWidth * 2.0f - bbWidth, xa, ya, za);
	}
}

void Animal::updateAi()
{
	// 1.0 AnimalEntity.mobTick
	if (age < 0)
		++age;
	else if (age > 0)
		--age;
	if (inLove > 0)
	{
		inLove--;
		if (inLove % 10 == 0)
			hearts(1, true);
	}
	else
		loveTime = 0;
	PathfinderMob::updateAi();
}

void Animal::checkHurtTarget(Entity &target, float distance)
{
	// 1.0 targetInSight
	if (Player *player = dynamic_cast<Player *>(&target))
	{
		if (distance < 3.0f)
		{
			double dx = target.x - x;
			double dz = target.z - z;
			yRot = static_cast<float>(std::atan2(dz, dx) * 180.0 / Mth::PI) - 90.0f;
			holdGround = true;
		}
		ItemInstance *held = player->getSelectedItem();
		if (held == nullptr || !isFood(*held))
			attackTarget = nullptr;
		return;
	}
	Animal *other = dynamic_cast<Animal *>(&target);
	if (other == nullptr)
		return;
	if (age > 0 && other->age < 0)
	{
		if (distance < 2.5)
			holdGround = true;
	}
	else if (inLove > 0 && other->inLove > 0)
	{
		if (other->attackTarget == nullptr)
			other->attackTarget = level.getEntityRef(*this);
		if (other->attackTarget.get() == this && distance < 3.5)
		{
			other->inLove++;
			inLove++;
			loveTime++;
			if (loveTime % 4 == 0)
				hearts(1, false);
			if (loveTime == 60)
				breed(*other);
		}
		else
			loveTime = 0;
	}
	else
		loveTime = 0;
}

void Animal::breed(Animal &partner)
{
	std::shared_ptr<Animal> child = getBreedOffspring(partner);
	if (child == nullptr)
		return;
	setAge(6000);
	partner.setAge(6000);
	inLove = 0;
	loveTime = 0;
	attackTarget = nullptr;
	partner.attackTarget = nullptr;
	partner.loveTime = 0;
	partner.inLove = 0;
	child->setAge(-24000);
	child->moveTo(x, y, z, yRot, xRot);
	hearts(7, true);
	level.addEntity(child);
}

bool Animal::hurt(Entity *source, int_t dmg)
{
	fleeTime = 60;
	attackTarget = nullptr;
	inLove = 0;
	return PathfinderMob::hurt(source, dmg);
}

std::shared_ptr<Entity> Animal::findAttackTarget()
{
	// 1.0 findTarget: a partner in love, a player holding food, or a baby to watch over
	if (fleeTime > 0)
		return nullptr;
	float r = 8.0f;
	AABB *area = bb.grow(r, r, r);
	const std::vector<std::shared_ptr<Entity>> &near = level.getEntities(this, *area);
	if (inLove > 0)
	{
		for (const auto &e : near)
		{
			Animal *other = dynamic_cast<Animal *>(e.get());
			if (other != nullptr && other != this && typeid(*other) == typeid(*this) && other->inLove > 0)
				return e;
		}
	}
	else if (age == 0)
	{
		for (const auto &e : near)
		{
			Player *player = dynamic_cast<Player *>(e.get());
			if (player == nullptr)
				continue;
			ItemInstance *held = player->getSelectedItem();
			if (held != nullptr && isFood(*held))
				return e;
		}
	}
	else if (age > 0)
	{
		for (const auto &e : near)
		{
			Animal *other = dynamic_cast<Animal *>(e.get());
			if (other != nullptr && other != this && typeid(*other) == typeid(*this) && other->age < 0)
				return e;
		}
	}
	return nullptr;
}

bool Animal::interact(Player &player)
{
	ItemInstance *held = player.inventory.getCurrentItem();
	if (held != nullptr && isFood(*held) && age == 0)
	{
		held->stackSize--;
		if (held->stackSize <= 0)
			player.inventory.setItem(player.inventory.currentItem, ItemInstance());
		inLove = 600;
		attackTarget = nullptr;
		hearts(7, true);
		return true;
	}
	return PathfinderMob::interact(player);
}

void Animal::addAdditionalSaveData(CompoundTag &tag)
{
	PathfinderMob::addAdditionalSaveData(tag);
	tag.putInt(u"Age", age);
	tag.putInt(u"InLove", inLove);
}

void Animal::readAdditionalSaveData(CompoundTag &tag)
{
	PathfinderMob::readAdditionalSaveData(tag);
	age = tag.getInt(u"Age");
	inLove = tag.getInt(u"InLove");
}

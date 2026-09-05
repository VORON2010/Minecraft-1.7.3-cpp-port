#include "world/entity/item/EntityItem.h"

#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/TreeTile.h"
#include "world/stats/AchievementList.h"
#include "world/stats/Achievement.h"

EntityItem::EntityItem(Level &level) : Entity(level)
{
	setSize(0.25f, 0.25f);
	heightOffset = bbHeight / 2.0f;
	bobOffs = random.nextFloat() * Mth::PI * 2.0f;
	makeStepSound = false;
}

EntityItem::EntityItem(Level &level, double x, double y, double z, const ItemInstance &stack) : EntityItem(level)
{
	item = stack;
	setPos(x, y, z);
	yRot = random.nextFloat() * 360.0f;
	xd = (random.nextFloat() * 0.2f - 0.1f);
	yd = 0.2;
	zd = (random.nextFloat() * 0.2f - 0.1f);
	xOld = xo = x;
	yOld = yo = y;
	zOld = zo = z;
}

void EntityItem::tick()
{
	Entity::tick();
	if (throwTime > 0)
		throwTime--;

	xo = x;
	yo = y;
	zo = z;
	yd -= 0.04;
	move(xd, yd, zd);

	float friction = 0.98f;
	if (onGround)
		friction = 0.588f;

	xd *= friction;
	yd *= 0.98;
	zd *= friction;
	if (onGround)
		yd *= -0.5;

	age++;
	if (age >= 6000)
		remove();
}

void EntityItem::playerTouch(Player &player)
{
	if (level.isOnline)
		return;

	int_t originalCount = item.stackSize;
	int_t originalId = item.itemID;
	if (throwTime == 0 && player.inventory.add(item))
	{
		if (originalId == Tile::treeTrunk.id)
			player.triggerAchievement(*AchievementList::mineWood);
		if (originalId == Items::leather->getShiftedIndex())
			player.triggerAchievement(*AchievementList::killCow);
		level.playSoundAtEntity(*this, u"random.pop", 0.2f, ((random.nextFloat() - random.nextFloat()) * 0.7f + 1.0f) * 2.0f);
		player.take(*this, originalCount);
		if (item.isEmpty())
			remove();
	}
}

bool EntityItem::hurt(Entity *source, int_t dmg)
{
	health -= dmg;
	if (health <= 0)
		remove();
	return false;
}

bool EntityItem::shouldRenderAtSqrDistance(double distance)
{
	return distance < 64.0 * 64.0;
}

bool EntityItem::handleWaterMovement()
{
	return level.handleMaterialAcceleration(bb, Material::water, *this);
}

void EntityItem::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putShort(u"Health", static_cast<short_t>(health));
	tag.putShort(u"Age", static_cast<short_t>(age));
	auto itemTag = std::make_shared<CompoundTag>();
	item.save(*itemTag);
	tag.put(u"Item", itemTag);
}

void EntityItem::readAdditionalSaveData(CompoundTag &tag)
{
	health = tag.getShort(u"Health");
	age = tag.getShort(u"Age");
	auto itemTag = tag.getCompound(u"Item");
	if (itemTag)
		item.load(*itemTag);
}

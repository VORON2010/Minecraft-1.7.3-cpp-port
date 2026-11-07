#include "world/entity/animal/MushroomCow.h"

#include <cmath>

#include "util/Mth.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/monster/Monster.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/entity/projectile/EntitySnowball.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/level/Level.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/tile/MushroomTile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/Tile.h"

MushroomCow::MushroomCow(Level &level) : Cow(level)
{
	textureName = u"/mob/redcow.png";
	setSize(0.9f, 1.3f);
}

bool MushroomCow::interact(Player &player)
{
	ItemInstance *held = player.inventory.getCurrentItem();
	if (held != nullptr && held->itemID == Items::bowlEmpty->getShiftedIndex() && getAge() >= 0)
	{
		player.inventory.setItem(player.inventory.currentItem, ItemInstance(Items::bowlSoup->getShiftedIndex(), 1, 0));
		return true;
	}
	if (held != nullptr && held->itemID == Items::shears->getShiftedIndex() && getAge() >= 0)
	{
		remove();
		auto cow = std::make_shared<Cow>(level);
		cow->moveTo(x, y, z, yRot, xRot);
		cow->health = health;
		cow->yBodyRot = yBodyRot;
		level.addEntity(cow);
		level.addParticle(u"largeexplode", x, y + bbHeight / 2.0f, z, 0.0, 0.0, 0.0);
		for (int_t i = 0; i < 5; ++i)
			level.addEntity(std::make_shared<EntityItem>(level, x, y + bbHeight, z, ItemInstance(Tile::redMushroom.id, 1, 0)));
		return true;
	}
	return Cow::interact(player);
}

std::shared_ptr<Animal> MushroomCow::getBreedOffspring(Animal &partner)
{
	(void)partner;
	return std::make_shared<MushroomCow>(level);
}

SnowMan::SnowMan(Level &level) : PathfinderMob(level)
{
	textureName = u"/mob/snowman.png";
	setSize(0.4f, 1.8f);
	health = 4;
}

void SnowMan::updateAi()
{
	PathfinderMob::updateAi();
	// 1.0 SnowGolemEntity.mobTick
	if (attackTarget == nullptr && !hasPath() && level.random.nextInt(100) == 0)
	{
		AABB *area = AABB::newTemp(x, y, z, x + 1.0, y + 1.0, z + 1.0)->grow(16.0, 4.0, 16.0);
		std::vector<std::shared_ptr<Entity>> monsters;
		for (const auto &e : level.getEntities(this, *area))
			if (dynamic_cast<Monster *>(e.get()) != nullptr)
				monsters.push_back(e);
		if (!monsters.empty())
			attackTarget = monsters[level.random.nextInt(static_cast<int_t>(monsters.size()))];
	}
	int_t bx = Mth::floor(x);
	int_t by = Mth::floor(y);
	int_t bz = Mth::floor(z);
	if (level.getBiomeSource().getTemperature(bx, by, bz) > 1.0f)
		hurtByFire(1);
	for (int_t i = 0; i < 4; ++i)
	{
		bx = Mth::floor(x + (i % 2 * 2 - 1) * 0.25f);
		by = Mth::floor(y);
		bz = Mth::floor(z + (i / 2 % 2 * 2 - 1) * 0.25f);
		if (level.getTile(bx, by, bz) == 0 && level.getBiomeSource().getTemperature(bx, by, bz) < 0.8f && Tile::snow.mayPlace(level, bx, by, bz))
			level.setTile(bx, by, bz, Tile::snow.id);
	}
}

void SnowMan::checkHurtTarget(Entity &target, float distance)
{
	// 1.0 targetInSight: a snowball every 10 ticks within 10 blocks
	if (distance >= 10.0f)
		return;
	double dx = target.x - x;
	double dz = target.z - z;
	if (attackTime == 0)
	{
		auto ball = std::make_shared<EntitySnowball>(level, *this);
		double dy = target.y + target.getHeadHeight() - 1.1f - ball->y;
		float arc = Mth::sqrt(dx * dx + dz * dz) * 0.2f;
		level.playSoundAtEntity(*this, u"random.bow", 1.0f, 1.0f / (random.nextFloat() * 0.4f + 0.8f));
		level.addEntity(ball);
		ball->shoot(dx, dy + arc, dz, 1.6f, 12.0f);
		attackTime = 10;
	}
	yRot = static_cast<float>(std::atan2(dz, dx) * 180.0 / Mth::PI) - 90.0f;
	holdGround = true;
}

int_t SnowMan::getDeathLoot()
{
	return Items::snowball->getShiftedIndex();
}

void SnowMan::dropDeathLoot()
{
	int_t count = random.nextInt(16);
	for (int_t i = 0; i < count; ++i)
		spawnAtLocation(ItemInstance(Items::snowball->getShiftedIndex(), 1, 0), 0.0f);
}

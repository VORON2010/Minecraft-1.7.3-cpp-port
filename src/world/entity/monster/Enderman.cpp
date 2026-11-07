#include "world/entity/monster/Enderman.h"

#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/entity/projectile/EntityArrow.h"
#include "world/item/ItemInstance.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/phys/Vec3.h"

std::array<bool, 256> Enderman::mayTake = {};
bool Enderman::mayTakeInitialized = false;

void Enderman::initMayTake()
{
	if (mayTakeInitialized)
		return;
	mayTakeInitialized = true;
	// stone, grass, dirt, cobble, planks, sand, gravel, ores, log, leaves, sponge, glass, lapis, sandstone, wool,
	// flowers, mushrooms, gold/iron blocks, bricks, tnt, bookshelf, mossy cobble, diamond, workbench, redstone ore,
	// ice, cactus, clay, pumpkin, netherrack, soul sand, glowstone, jack o'lantern, stone bricks, huge mushrooms, melon
	for (int_t id : {1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 35, 37, 38, 39, 40, 41, 42, 45, 46, 47,
		48, 56, 57, 58, 73, 74, 79, 81, 82, 86, 87, 88, 89, 91, 98, 99, 100, 103})
		mayTake[id] = true;
}

Enderman::Enderman(Level &level) : Monster(level)
{
	initMayTake();
	textureName = u"/mob/enderman.png";
	runSpeed = 0.2f;
	attackDamage = 7;
	setSize(0.6f, 2.9f);
	footSize = 1.0f;
	health = 40;
}

void Enderman::addAdditionalSaveData(CompoundTag &tag)
{
	Monster::addAdditionalSaveData(tag);
	tag.putShort(u"carried", static_cast<short_t>(carriedTile));
	tag.putShort(u"carriedData", static_cast<short_t>(carriedData));
}

void Enderman::readAdditionalSaveData(CompoundTag &tag)
{
	Monster::readAdditionalSaveData(tag);
	setCarriedTile(tag.getShort(u"carried"));
	setCarriedData(tag.getShort(u"carriedData"));
}

std::shared_ptr<Entity> Enderman::findAttackTarget()
{
	std::shared_ptr<Player> player = level.getNearestPlayer(*this, 64.0);
	if (player != nullptr)
	{
		if (isLookingAtMe(*player))
		{
			if (aggroTime++ == 5)
			{
				aggroTime = 0;
				return std::static_pointer_cast<Entity>(player);
			}
		}
		else
		{
			aggroTime = 0;
		}
	}
	return nullptr;
}

bool Enderman::isLookingAtMe(Player &player)
{
	const ItemInstance &helmet = player.inventory.armorInventory[3];
	if (!helmet.isEmpty() && helmet.itemID == 86)
		return false;

	Vec3 *look = player.getViewVector(1.0f)->normalize();
	Vec3 *toMe = Vec3::newTemp(x - player.x, bb.y0 + static_cast<double>(bbHeight / 2.0f) - (player.y + static_cast<double>(player.getHeadHeight())), z - player.z);
	double distance = toMe->length();
	toMe = toMe->normalize();
	double dot = look->dot(*toMe);
	return dot > 1.0 - 0.025 / distance ? player.canSee(*this) : false;
}

void Enderman::aiStep()
{
	if (isWet())
		hurt(nullptr, 1);

	creepy = attackTarget != nullptr;
	runSpeed = attackTarget != nullptr ? 6.5f : 0.3f;

	if (!level.isOnline)
	{
		if (carriedTile == 0)
		{
			if (random.nextInt(20) == 0)
			{
				int_t tx = Mth::floor(x - 2.0 + random.nextDouble() * 4.0);
				int_t ty = Mth::floor(y + random.nextDouble() * 3.0);
				int_t tz = Mth::floor(z - 2.0 + random.nextDouble() * 4.0);
				int_t tile = level.getTile(tx, ty, tz);
				if (mayTake[tile])
				{
					setCarriedTile(tile);
					setCarriedData(level.getData(tx, ty, tz));
					level.setTile(tx, ty, tz, 0);
				}
			}
		}
		else if (random.nextInt(2000) == 0)
		{
			int_t tx = Mth::floor(x - 1.0 + random.nextDouble() * 2.0);
			int_t ty = Mth::floor(y + random.nextDouble() * 2.0);
			int_t tz = Mth::floor(z - 1.0 + random.nextDouble() * 2.0);
			int_t tile = level.getTile(tx, ty, tz);
			int_t below = level.getTile(tx, ty - 1, tz);
			if (tile == 0 && below > 0 && Tile::tiles[below] != nullptr && Tile::tiles[below]->isCubeShaped())
			{
				level.setTileAndData(tx, ty, tz, carriedTile, carriedData);
				setCarriedTile(0);
			}
		}
	}

	for (int_t i = 0; i < 2; ++i)
	{
		level.addParticle(u"portal",
			x + (random.nextDouble() - 0.5) * bbWidth,
			y + random.nextDouble() * bbHeight - 0.25,
			z + (random.nextDouble() - 0.5) * bbWidth,
			(random.nextDouble() - 0.5) * 2.0, -random.nextDouble(), (random.nextDouble() - 0.5) * 2.0);
	}

	if (level.isDay() && !level.isOnline)
	{
		float br = getBrightness(1.0f);
		if (br > 0.5f && level.canSeeSky(Mth::floor(x), Mth::floor(y), Mth::floor(z)) && random.nextFloat() * 30.0f < (br - 0.4f) * 2.0f)
		{
			attackTarget = nullptr;
			teleport();
		}
	}

	if (isWet())
	{
		attackTarget = nullptr;
		teleport();
	}

	jumping = false;
	if (attackTarget != nullptr)
		lookAt(*attackTarget, 100.0f, 100.0f);

	if (!level.isOnline && isAlive())
	{
		if (attackTarget != nullptr)
		{
			Player *player = dynamic_cast<Player *>(attackTarget.get());
			if (player != nullptr && isLookingAtMe(*player))
			{
				xxa = 0.0f;
				yya = 0.0f;
				runSpeed = 0.0f;
				if (attackTarget->distanceToSqr(*this) < 16.0)
					teleport();
				teleportTime = 0;
			}
			else if (attackTarget->distanceToSqr(*this) > 256.0 && teleportTime++ >= 30 && teleportTowards(*attackTarget))
			{
				teleportTime = 0;
			}
		}
		else
		{
			teleportTime = 0;
		}
	}

	Monster::aiStep();
}

bool Enderman::teleport()
{
	double tx = x + (random.nextDouble() - 0.5) * 64.0;
	double ty = y + static_cast<double>(random.nextInt(64) - 32);
	double tz = z + (random.nextDouble() - 0.5) * 64.0;
	return teleport(tx, ty, tz);
}

bool Enderman::teleportTowards(Entity &target)
{
	Vec3 *dir = Vec3::newTemp(x - target.x, bb.y0 + static_cast<double>(bbHeight / 2.0f) - target.y + static_cast<double>(target.getHeadHeight()), z - target.z);
	dir = dir->normalize();
	double distance = 16.0;
	double tx = x + (random.nextDouble() - 0.5) * 8.0 - dir->x * distance;
	double ty = y + static_cast<double>(random.nextInt(16) - 8) - dir->y * distance;
	double tz = z + (random.nextDouble() - 0.5) * 8.0 - dir->z * distance;
	return teleport(tx, ty, tz);
}

bool Enderman::teleport(double tx, double ty, double tz)
{
	double ox = x;
	double oy = y;
	double oz = z;
	x = tx;
	y = ty;
	z = tz;
	bool ok = false;
	int_t bx = Mth::floor(x);
	int_t by = Mth::floor(y);
	int_t bz = Mth::floor(z);
	if (level.hasChunkAt(bx, by, bz))
	{
		bool landed = false;
		while (!landed && by > 0)
		{
			int_t below = level.getTile(bx, by - 1, bz);
			if (below != 0 && Tile::tiles[below] != nullptr && Tile::tiles[below]->material.blocksMotion())
			{
				landed = true;
			}
			else
			{
				y--;
				by--;
			}
		}

		if (landed)
		{
			setPos(x, y, z);
			if (level.getCubes(*this, bb).empty() && !level.containsAnyLiquid(bb))
				ok = true;
		}
	}

	if (!ok)
	{
		setPos(ox, oy, oz);
		return false;
	}

	int_t count = 128;
	for (int_t i = 0; i < count; ++i)
	{
		double t = static_cast<double>(i) / (static_cast<double>(count) - 1.0);
		float xa = (random.nextFloat() - 0.5f) * 0.2f;
		float ya = (random.nextFloat() - 0.5f) * 0.2f;
		float za = (random.nextFloat() - 0.5f) * 0.2f;
		double px = ox + (x - ox) * t + (random.nextDouble() - 0.5) * bbWidth * 2.0;
		double py = oy + (y - oy) * t + random.nextDouble() * bbHeight;
		double pz = oz + (z - oz) * t + (random.nextDouble() - 0.5) * bbWidth * 2.0;
		level.addParticle(u"portal", px, py, pz, xa, ya, za);
	}

	level.playSoundEffect(ox, oy, oz, u"mob.endermen.portal", 1.0f, 1.0f);
	level.playSoundAtEntity(*this, u"mob.endermen.portal", 1.0f, 1.0f);
	return true;
}

bool Enderman::hurt(Entity *source, int_t damage)
{
	if (Entity::damageProjectile != nullptr || dynamic_cast<EntityArrow *>(source) != nullptr)
	{
		for (int_t i = 0; i < 64; ++i)
		{
			if (teleport())
				return true;
		}
		return false;
	}
	return Monster::hurt(source, damage);
}

jstring Enderman::getAmbientSound()
{
	return u"mob.endermen.idle";
}

jstring Enderman::getHurtSound()
{
	return u"mob.endermen.hit";
}

jstring Enderman::getDeathSound()
{
	return u"mob.endermen.death";
}

int_t Enderman::getDeathLoot()
{
	return Items::enderPearl->getShiftedIndex();
}

void Enderman::dropDeathLoot()
{
	int_t loot = getDeathLoot();
	if (loot <= 0)
		return;
	int_t count = random.nextInt(2);
	for (int_t i = 0; i < count; ++i)
		spawnAtLocation(ItemInstance(loot, 1, 0), 0.0f);
}

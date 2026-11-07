#include "world/entity/monster/Silverfish.h"

#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/InfestedTile.h"
#include "world/level/tile/StoneTile.h"

namespace
{
	// face offsets: down, up, north, south, west, east
	constexpr int_t FACE_X[6] = {0, 0, 0, 0, -1, 1};
	constexpr int_t FACE_Y[6] = {-1, 1, 0, 0, 0, 0};
	constexpr int_t FACE_Z[6] = {0, 0, -1, 1, 0, 0};
}

Silverfish::Silverfish(Level &level) : Monster(level)
{
	textureName = u"/mob/silverfish.png";
	setSize(0.3f, 0.7f);
	runSpeed = 0.6f;
	attackDamage = 1;
	health = 8;
	makeStepSound = false;
}

void Silverfish::tick()
{
	yBodyRot = yRot;
	Monster::tick();
}

std::shared_ptr<Entity> Silverfish::findAttackTarget()
{
	return std::static_pointer_cast<Entity>(level.getNearestAttackablePlayer(*this, 8.0));
}

jstring Silverfish::getAmbientSound()
{
	return u"mob.silverfish.say";
}

jstring Silverfish::getHurtSound()
{
	return u"mob.silverfish.hit";
}

jstring Silverfish::getDeathSound()
{
	return u"mob.silverfish.kill";
}

bool Silverfish::hurt(Entity *source, int_t damage)
{
	if (lookForFriends <= 0 && source != nullptr)
		lookForFriends = 20;
	return Monster::hurt(source, damage);
}

void Silverfish::checkHurtTarget(Entity &entity, float distance)
{
	if (attackTime <= 0 && distance < 1.2f && entity.bb.y1 > bb.y0 && entity.bb.y0 < bb.y1)
	{
		attackTime = 20;
		entity.hurt(this, attackDamage);
	}
}

int_t Silverfish::getDeathLoot()
{
	return 0;
}

void Silverfish::updateAi()
{
	Monster::updateAi();
	if (level.isOnline)
		return;

	if (lookForFriends > 0)
	{
		lookForFriends--;
		if (lookForFriends == 0)
		{
			int_t bx = Mth::floor(x);
			int_t by = Mth::floor(y);
			int_t bz = Mth::floor(z);
			bool done = false;
			// search outwards: 0, 1, -1, 2, -2, ...
			for (int_t dy = 0; !done && dy <= 5 && dy >= -5; dy = dy <= 0 ? 1 - dy : -dy)
			{
				for (int_t dx = 0; !done && dx <= 10 && dx >= -10; dx = dx <= 0 ? 1 - dx : -dx)
				{
					for (int_t dz = 0; !done && dz <= 10 && dz >= -10; dz = dz <= 0 ? 1 - dz : -dz)
					{
						int_t tx = bx + dx, ty = by + dy, tz = bz + dz;
						if (level.getTile(tx, ty, tz) != Tile::monsterEgg.id)
							continue;
						level.levelEvent(nullptr, 2001, tx, ty, tz, Tile::monsterEgg.id + level.getData(tx, ty, tz) * 256);
						level.setTile(tx, ty, tz, 0);
						Tile::monsterEgg.destroy(level, tx, ty, tz, 0);
						if (random.nextBoolean())
							done = true;
					}
				}
			}
		}
	}

	if (attackTarget == nullptr && !hasPath())
	{
		int_t bx = Mth::floor(x);
		int_t by = Mth::floor(y + 0.5);
		int_t bz = Mth::floor(z);
		int_t face = random.nextInt(6);
		int_t tx = bx + FACE_X[face], ty = by + FACE_Y[face], tz = bz + FACE_Z[face];
		int_t tile = level.getTile(tx, ty, tz);
		if (InfestedTile::canBeInfested(tile))
		{
			level.setTileAndData(tx, ty, tz, Tile::monsterEgg.id, InfestedTile::getVariant(tile));
			spawnAnim();
			remove();
		}
		else
		{
			findRandomStrollLocation();
		}
	}
	else if (attackTarget != nullptr && !hasPath())
	{
		attackTarget = nullptr;
	}
}

float Silverfish::getWalkTargetValue(int_t x, int_t y, int_t z)
{
	return level.getTile(x, y - 1, z) == Tile::rock.id ? 10.0f : Monster::getWalkTargetValue(x, y, z);
}

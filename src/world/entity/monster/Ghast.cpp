#include "world/entity/monster/Ghast.h"

#include "world/entity/projectile/EntityFireball.h"
#include "world/entity/player/Player.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"

Ghast::Ghast(Level &level) : Mob(level)
{
	dataWatcher.addObject(16, static_cast<byte_t>(0));
	textureName = u"/mob/ghast.png";
	setSize(4.0f, 4.0f);
	fireImmune = true;
}

jstring Ghast::getTexture()
{
	return textureName;
}

void Ghast::tick()
{
	Mob::tick();
	textureName = dataWatcher.getWatchableObjectByte(16) == 1
		? u"/mob/ghast_fire.png" : u"/mob/ghast.png";
}

void Ghast::updateAi()
{
	if (!level.isOnline && level.difficulty == 0)
		remove();
	prevAttackCounter = attackCounter;
	double dx = waypointX - x;
	double dy = waypointY - y;
	double dz = waypointZ - z;
	double distance = Mth::sqrt(dx * dx + dy * dy + dz * dz);
	if (distance < 1.0 || distance > 60.0)
	{
		waypointX = x + (random.nextFloat() * 2.0f - 1.0f) * 16.0f;
		waypointY = y + (random.nextFloat() * 2.0f - 1.0f) * 16.0f;
		waypointZ = z + (random.nextFloat() * 2.0f - 1.0f) * 16.0f;
	}
	if (courseChangeCooldown-- <= 0)
	{
		courseChangeCooldown += random.nextInt(5) + 2;
		if (isCourseTraversable(waypointX, waypointY, waypointZ, distance))
		{
			xd += dx / distance * 0.1;
			yd += dy / distance * 0.1;
			zd += dz / distance * 0.1;
		}
		else
		{
			waypointX = x;
			waypointY = y;
			waypointZ = z;
		}
	}

	auto targetRef = target.lock();
	if (targetRef != nullptr && targetRef->removed)
		target.reset();
	if (target.lock() == nullptr || aggroCooldown-- <= 0)
	{
		target = std::static_pointer_cast<Entity>(level.getNearestPlayer(*this, 100.0));
		if (target.lock() != nullptr)
			aggroCooldown = 20;
		targetRef = target.lock();
	}

	constexpr double targetRange = 64.0;
	if (targetRef != nullptr && distanceToSqr(*targetRef) < targetRange * targetRange)
	{
		double tx = targetRef->x - x;
		double ty = targetRef->bb.y0 + targetRef->bbHeight / 2.0f - (y + bbHeight / 2.0f);
		double tz = targetRef->z - z;
		yBodyRot = yRot = -static_cast<float>(std::atan2(tx, tz) * 180.0 / Mth::PI);
		if (canSee(*targetRef))
		{
			if (attackCounter == 10)
				level.playSoundAtEntity(*this, u"mob.ghast.charge", getSoundVolume(), (random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
			attackCounter++;
			if (attackCounter == 20)
			{
				level.playSoundAtEntity(*this, u"mob.ghast.fireball", getSoundVolume(), (random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
				auto fireball = std::make_shared<EntityFireball>(level, *this, tx, ty, tz);
				double lookX = -Mth::sin(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD);
				double lookZ = Mth::cos(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD);
				double lookY = -Mth::sin(xRot * Mth::DEGRAD);
				fireball->moveTo(x + lookX * 4.0, y + bbHeight / 2.0f + 0.5, z + lookZ * 4.0, yRot, xRot);
				level.addEntity(fireball);
				attackCounter = -40;
			}
		}
		else if (attackCounter > 0)
		{
			attackCounter--;
		}
	}
	else
	{
		yBodyRot = yRot = -static_cast<float>(std::atan2(xd, zd) * 180.0 / Mth::PI);
		if (attackCounter > 0)
			attackCounter--;
	}

	if (!level.isOnline)
	{
		byte_t current = dataWatcher.getWatchableObjectByte(16);
		byte_t attacking = static_cast<byte_t>(attackCounter > 10 ? 1 : 0);
		if (current != attacking)
			dataWatcher.updateObject(16, attacking);
	}
}

void Ghast::travel(float x, float z)
{
	if (isInWater())
	{
		moveRelative(x, z, 0.02f);
		move(xd, yd, zd);
		xd *= 0.8f;
		yd *= 0.8f;
		zd *= 0.8f;
	}
	else if (isInLava())
	{
		moveRelative(x, z, 0.02f);
		move(xd, yd, zd);
		xd *= 0.5;
		yd *= 0.5;
		zd *= 0.5;
	}
	else
	{
		float friction = 0.91f;
		if (onGround)
		{
			friction = 0.546f;
			int_t tile = level.getTile(Mth::floor(this->x), Mth::floor(bb.y0) - 1, Mth::floor(this->z));
			if (tile > 0)
				friction = Tile::tiles[tile]->friction * 0.91f;
		}

		float acceleration = 0.16277136f / (friction * friction * friction);
		moveRelative(x, z, onGround ? (0.1f * acceleration) : 0.02f);

		friction = 0.91f;
		if (onGround)
		{
			friction = 0.546f;
			int_t tile = level.getTile(Mth::floor(this->x), Mth::floor(bb.y0) - 1, Mth::floor(this->z));
			if (tile > 0)
				friction = Tile::tiles[tile]->friction * 0.91f;
		}

		move(xd, yd, zd);
		xd *= friction;
		yd *= friction;
		zd *= friction;
	}

	walkAnimSpeedO = walkAnimSpeed;
	double fdx = this->x - this->xo;
	double fdz = this->z - this->zo;
	float targetWalkAnimSpeed = Mth::sqrt(fdx * fdx + fdz * fdz) * 4.0f;
	if (targetWalkAnimSpeed > 1.0f) targetWalkAnimSpeed = 1.0f;
	walkAnimSpeed += (targetWalkAnimSpeed - walkAnimSpeed) * 0.4f;
	walkAnimPos += walkAnimSpeed;
}

void Ghast::causeFallDamage(float distance)
{
}

bool Ghast::canSpawn()
{
	return random.nextInt(20) == 0 && level.difficulty > 0 && Mob::canSpawn();
}

int_t Ghast::getMaxSpawnClusterSize()
{
	return 1;
}

jstring Ghast::getAmbientSound() { return u"mob.ghast.moan"; }
jstring Ghast::getHurtSound() { return u"mob.ghast.scream"; }
jstring Ghast::getDeathSound() { return u"mob.ghast.death"; }
int_t Ghast::getDeathLoot() { return Items::gunpowder->getShiftedIndex(); }
float Ghast::getSoundVolume() { return 10.0f; }

bool Ghast::isCourseTraversable(double x, double y, double z, double distance)
{
	double stepX = (x - this->x) / distance;
	double stepY = (y - this->y) / distance;
	double stepZ = (z - this->z) / distance;
	AABB *course = bb.copy();
	for (int_t i = 1; i < distance; ++i)
	{
		course->move(stepX, stepY, stepZ);
		if (!level.getCubes(*this, *course).empty())
			return false;
	}
	return true;
}

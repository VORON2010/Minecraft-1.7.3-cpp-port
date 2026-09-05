#include "world/entity/item/EntityBoat.h"

#include <cmath>

#include "world/entity/item/EntityItem.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/material/Material.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/WoodTile.h"
#include "world/phys/AABB.h"
#include "util/Mth.h"

namespace
{
	std::shared_ptr<Entity> findSharedEntity(Level &level, Entity *self)
	{
		for (const auto &entity : level.getAllEntities())
		{
			if (entity.get() == self)
				return entity;
		}
		return nullptr;
	}
}

EntityBoat::EntityBoat(Level &level) : Entity(level)
{
	blocksBuilding = true;
	setSize(1.5f, 0.6f);
	heightOffset = bbHeight / 2.0f;
	makeStepSound = false;
}

EntityBoat::EntityBoat(Level &level, double x, double y, double z) : EntityBoat(level)
{
	setPos(x, y + heightOffset, z);
	xOld = xo = x;
	yOld = yo = y;
	zOld = zo = z;
}

AABB *EntityBoat::getCollideAgainstBox(Entity &entity)
{
	return entity.getCollideBox();
}

AABB *EntityBoat::getCollideBox()
{
	return &bb;
}

double EntityBoat::getRideHeight()
{
	return bbHeight * 0.0 - 0.3;
}

bool EntityBoat::hurt(Entity *source, int_t dmg)
{
	(void)source;
	if (level.isOnline || removed)
		return true;

	boatRockDirection = -boatRockDirection;
	boatTimeSinceHit = 10;
	markHurt();
	boatCurrentDamage += dmg * 10;

	if (boatCurrentDamage > 40)
	{
		if (rider != nullptr)
			rider->ride(nullptr);

		for (int_t i = 0; i < 3; i++)
			spawnAtLocation(ItemInstance(Tile::wood.id, 1, 0), 0.0f);
		for (int_t i = 0; i < 2; i++)
			spawnAtLocation(ItemInstance(Items::stick->getShiftedIndex(), 1, 0), 0.0f);

		remove();
	}

	return true;
}

void EntityBoat::animateHurt()
{
	boatRockDirection = -boatRockDirection;
	boatTimeSinceHit = 10;
	boatCurrentDamage = boatCurrentDamage + boatCurrentDamage * 10;
}

bool EntityBoat::interact(Player &player)
{
	if (rider != nullptr && rider->isPlayer() && rider.get() != &player)
		return true;

	if (!level.isOnline)
	{
		auto self = findSharedEntity(level, this);
		if (self != nullptr)
			player.ride(self);
	}

	return true;
}

void EntityBoat::tick()
{
	Entity::tick();

	if (boatTimeSinceHit > 0)
		boatTimeSinceHit--;
	if (boatCurrentDamage > 0)
		boatCurrentDamage--;

	xo = x;
	yo = y;
	zo = z;

	if (level.isOnline)
	{
		if (lerpSteps > 0)
		{
			double nx = x + (lerpX - x) / lerpSteps;
			double ny = y + (lerpY - y) / lerpSteps;
			double nz = z + (lerpZ - z) / lerpSteps;
			double dyaw = lerpYaw - yRot;
			while (dyaw < -180.0)
				dyaw += 360.0;
			while (dyaw >= 180.0)
				dyaw -= 360.0;
			yRot = static_cast<float>(yRot + dyaw / lerpSteps);
			xRot = static_cast<float>(xRot + (lerpPitch - xRot) / lerpSteps);
			lerpSteps--;
			setPos(nx, ny, nz);
			setRot(yRot, xRot);
		}
		else
		{
			setPos(x + xd, y + yd, z + zd);
			if (onGround)
			{
				xd *= 0.5;
				yd *= 0.5;
				zd *= 0.5;
			}
			xd *= 0.99;
			yd *= 0.95;
			zd *= 0.99;
		}
		return;
	}

	
	byte_t slices = 5;
	double waterCoverage = 0.0;
	for (int_t i = 0; i < slices; i++)
	{
		double y0 = bb.y0 + (bb.y1 - bb.y0) * i / slices - 0.125;
		double y1 = bb.y0 + (bb.y1 - bb.y0) * (i + 1) / slices - 0.125;
		AABB *sample = AABB::newTemp(bb.x0, y0, bb.z0, bb.x1, y1, bb.z1);
		if (level.isMaterialInBB(*sample, Material::water))
			waterCoverage += 1.0 / slices;
	}

	if (waterCoverage < 1.0)
	{
		double buoyancy = waterCoverage * 2.0 - 1.0;
		yd += 0.04 * buoyancy;
	}
	else
	{
		if (yd < 0.0)
			yd /= 2.0;
		yd += 0.007;
	}

	if (rider != nullptr)
	{
		xd += rider->xd * 0.2;
		zd += rider->zd * 0.2;
	}

	double maxSpeed = 0.4;
	if (xd < -maxSpeed) xd = -maxSpeed;
	if (xd > maxSpeed) xd = maxSpeed;
	if (zd < -maxSpeed) zd = -maxSpeed;
	if (zd > maxSpeed) zd = maxSpeed;

	if (onGround)
	{
		xd *= 0.5;
		yd *= 0.5;
		zd *= 0.5;
	}

	move(xd, yd, zd);

	double speed = std::sqrt(xd * xd + zd * zd);
	if (speed > 0.15)
	{
		double cy = std::cos(yRot * Mth::PI / 180.0);
		double sy = std::sin(yRot * Mth::PI / 180.0);

		int_t particles = static_cast<int_t>(1.0 + speed * 60.0);
		for (int_t i = 0; i < particles; i++)
		{
			double r1 = random.nextFloat() * 2.0f - 1.0f;
			double r2 = (random.nextInt(2) * 2 - 1) * 0.7;
			if (random.nextBoolean())
			{
				double px = x - cy * r1 * 0.8 + sy * r2;
				double pz = z - sy * r1 * 0.8 - cy * r2;
				level.addParticle(u"splash", px, y - 0.125, pz, xd, yd, zd);
			}
			else
			{
				double px = x + cy + sy * r1 * 0.7;
				double pz = z + sy - cy * r1 * 0.7;
				level.addParticle(u"splash", px, y - 0.125, pz, xd, yd, zd);
			}
		}
	}

	if (!horizontalCollision || speed <= 0.15)
	{
		xd *= 0.99;
		yd *= 0.95;
		zd *= 0.99;
	}
	else if (!level.isOnline)
	{
		remove();
		for (int_t i = 0; i < 3; i++)
			spawnAtLocation(ItemInstance(Tile::wood.id, 1, 0), 0.0f);
		for (int_t i = 0; i < 2; i++)
			spawnAtLocation(ItemInstance(Items::stick->getShiftedIndex(), 1, 0), 0.0f);
	}

	xRot = 0.0f;
	double targetYaw = yRot;
	double dx = xo - x;
	double dz = zo - z;
	if (dx * dx + dz * dz > 0.001)
		targetYaw = static_cast<float>(std::atan2(dz, dx) * 180.0 / Mth::PI);

	double yawDelta = targetYaw - yRot;
	while (yawDelta >= 180.0)
		yawDelta -= 360.0;
	while (yawDelta < -180.0)
		yawDelta += 360.0;
	if (yawDelta > 20.0)
		yawDelta = 20.0;
	if (yawDelta < -20.0)
		yawDelta = -20.0;

	yRot = static_cast<float>(yRot + yawDelta);
	setRot(yRot, xRot);

	auto nearby = level.getEntities(this, *bb.grow(0.2, 0.0, 0.2));
	for (const auto &other : nearby)
	{
		if (other.get() != rider.get() && other->isPushable())
		{
			EntityBoat *otherBoat = dynamic_cast<EntityBoat *>(other.get());
			if (otherBoat != nullptr)
				other->push(*this);
		}
	}

	
	for (int_t i = 0; i < 4; i++)
	{
		int_t sx = Mth::floor(x + (i % 2 - 0.5) * 0.8);
		int_t sy = Mth::floor(y);
		int_t sz = Mth::floor(z + (i / 2 - 0.5) * 0.8);
		if (level.getTile(sx, sy, sz) == Tile::snow.id)
			level.setTile(sx, sy, sz, 0);
	}

	if (rider != nullptr && rider->removed)
		rider = nullptr;
}

void EntityBoat::positionRider()
{
	if (rider != nullptr)
	{
		double ox = std::cos(yRot * Mth::PI / 180.0) * 0.4;
		double oz = std::sin(yRot * Mth::PI / 180.0) * 0.4;
		rider->setPos(x + ox, y + getRideHeight() + rider->getRidingHeight(), z + oz);
	}
}

void EntityBoat::lerpTo(double x, double y, double z, float yRot, float xRot, int_t steps)
{
	lerpX = x;
	lerpY = y;
	lerpZ = z;
	lerpYaw = yRot;
	lerpPitch = xRot;
	lerpSteps = steps + 4;
	xd = lerpXd;
	yd = lerpYd;
	zd = lerpZd;
}

void EntityBoat::lerpMotion(double x, double y, double z)
{
	lerpXd = this->xd = x;
	lerpYd = this->yd = y;
	lerpZd = this->zd = z;
}

float EntityBoat::getShadowHeightOffs()
{
	return 0.0f;
}

void EntityBoat::addAdditionalSaveData(CompoundTag &tag)
{
	(void)tag;
}

void EntityBoat::readAdditionalSaveData(CompoundTag &tag)
{
	(void)tag;
}

#include "world/entity/projectile/EntityFish.h"

#include <cmath>
#include <memory>

#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/phys/AABB.h"
#include "world/phys/HitResult.h"
#include "world/phys/Vec3.h"
#include "world/stats/StatList.h"

EntityFish::EntityFish(Level &level) : Entity(level)
{
	setSize(0.25f, 0.25f);
	noCulling = true;
}

EntityFish::EntityFish(Level &level, double x, double y, double z) : EntityFish(level)
{
	setPos(x, y, z);
	noCulling = true;
}

EntityFish::EntityFish(Level &level, Player &angler) : Entity(level)
{
	noCulling = true;
	this->angler = &angler;
	setSize(0.25f, 0.25f);
	absMoveTo(angler.x, angler.y + 1.62 - angler.heightOffset, angler.z, angler.yRot, angler.xRot);
	x -= Mth::cos(yRot / 180.0f * Mth::PI) * 0.16f;
	y -= 0.1f;
	z -= Mth::sin(yRot / 180.0f * Mth::PI) * 0.16f;
	setPos(x, y, z);
	heightOffset = 0.0f;
	float speed = 0.4f;
	xd = -Mth::sin(yRot / 180.0f * Mth::PI) * Mth::cos(xRot / 180.0f * Mth::PI) * speed;
	zd = Mth::cos(yRot / 180.0f * Mth::PI) * Mth::cos(xRot / 180.0f * Mth::PI) * speed;
	yd = -Mth::sin(xRot / 180.0f * Mth::PI) * speed;
	shoot(xd, yd, zd, 1.5f, 1.0f);
}

bool EntityFish::shouldRenderAtSqrDistance(double distance)
{
	double size = bb.getSize() * 4.0;
	size *= 64.0;
	return distance < size * size;
}

void EntityFish::shoot(double xd, double yd, double zd, float speed, float spread)
{
	float length = Mth::sqrt(xd * xd + yd * yd + zd * zd);
	xd /= length;
	yd /= length;
	zd /= length;
	xd += random.nextGaussian() * 0.0075f * spread;
	yd += random.nextGaussian() * 0.0075f * spread;
	zd += random.nextGaussian() * 0.0075f * spread;
	xd *= speed;
	yd *= speed;
	zd *= speed;
	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
	float horizontal = Mth::sqrt(xd * xd + zd * zd);
	yRotO = yRot = static_cast<float>(std::atan2(xd, zd) * 180.0 / Mth::PI);
	xRotO = xRot = static_cast<float>(std::atan2(yd, horizontal) * 180.0 / Mth::PI);
	ticksInGround = 0;
}

void EntityFish::lerpTo(double x, double y, double z, float yRot, float xRot, int_t steps)
{
	lerpX = x;
	lerpY = y;
	lerpZ = z;
	lerpYRot = yRot;
	lerpXRot = xRot;
	lerpSteps = steps;
	xd = velocityX;
	yd = velocityY;
	zd = velocityZ;
}

void EntityFish::lerpMotion(double xd, double yd, double zd)
{
	velocityX = this->xd = xd;
	velocityY = this->yd = yd;
	velocityZ = this->zd = zd;
}

void EntityFish::tick()
{
	Entity::tick();
	if (lerpSteps > 0)
	{
		double nx = x + (lerpX - x) / lerpSteps;
		double ny = y + (lerpY - y) / lerpSteps;
		double nz = z + (lerpZ - z) / lerpSteps;
		double yawDelta = lerpYRot - yRot;
		while (yawDelta < -180.0)
			yawDelta += 360.0;
		while (yawDelta >= 180.0)
			yawDelta -= 360.0;
		yRot = static_cast<float>(yRot + yawDelta / lerpSteps);
		xRot = static_cast<float>(xRot + (lerpXRot - xRot) / lerpSteps);
		lerpSteps--;
		setPos(nx, ny, nz);
		setRot(yRot, xRot);
		return;
	}

	if (!level.isOnline)
	{
		ItemInstance *held = angler->getSelectedItem();
		if (angler->removed || !angler->isAlive() || held == nullptr || held->getItem() != Items::fishingRod || distanceToSqr(*angler) > 1024.0)
		{
			remove();
			angler->fishEntity.reset();
			return;
		}

		if (bobber != nullptr)
		{
			if (!bobber->removed)
			{
				x = bobber->x;
				y = bobber->bb.y0 + bobber->bbHeight * 0.8;
				z = bobber->z;
				return;
			}
			bobber.reset();
		}
	}

	if (shake > 0)
		shake--;

	if (inGround)
	{
		int_t tile = level.getTile(xTile, yTile, zTile);
		if (tile == inTile)
		{
			ticksInGround++;
			if (ticksInGround == 1200)
				remove();
			return;
		}
		inGround = false;
		xd *= random.nextFloat() * 0.2f;
		yd *= random.nextFloat() * 0.2f;
		zd *= random.nextFloat() * 0.2f;
		ticksInGround = 0;
		ticksInAir = 0;
	}
	else
	{
		ticksInAir++;
	}

	Vec3 *from = Vec3::newTemp(x, y, z);
	Vec3 *to = Vec3::newTemp(x + xd, y + yd, z + zd);
	HitResult hit = level.clip(*from, *to);
	from = Vec3::newTemp(x, y, z);
	to = Vec3::newTemp(x + xd, y + yd, z + zd);
	if (hit.type != HitResult::Type::NONE && hit.pos != nullptr)
		to = Vec3::newTemp(hit.pos->x, hit.pos->y, hit.pos->z);

	std::shared_ptr<Entity> nearest;
	double nearestDistance = 0.0;
	AABB *search = bb.expand(xd, yd, zd)->grow(1.0, 1.0, 1.0);
	const auto &entities = level.getEntities(this, *search);
	for (const auto &candidate : entities)
	{
		if (candidate->isPickable() && (candidate.get() != angler || ticksInAir >= 5))
		{
			float padding = 0.3f;
			HitResult candidateHit = candidate->bb.grow(padding, padding, padding)->clip(*from, *to);
			if (candidateHit.type != HitResult::Type::NONE && candidateHit.pos != nullptr)
			{
				double distance = from->distanceTo(*candidateHit.pos);
				if (distance < nearestDistance || nearestDistance == 0.0)
				{
					nearest = candidate;
					nearestDistance = distance;
				}
			}
		}
	}

	if (nearest != nullptr)
		hit = HitResult(nearest);
	if (hit.type != HitResult::Type::NONE)
	{
		if (hit.entity != nullptr)
		{
			if (hit.entity->hurt(angler, 0))
				bobber = hit.entity;
		}
		else
		{
			inGround = true;
		}
	}

	if (!inGround)
	{
		move(xd, yd, zd);
		float horizontal = Mth::sqrt(xd * xd + zd * zd);
		yRot = static_cast<float>(std::atan2(xd, zd) * 180.0 / Mth::PI);
		xRot = static_cast<float>(std::atan2(yd, horizontal) * 180.0 / Mth::PI);
		while (xRot - xRotO < -180.0f)
			xRotO -= 360.0f;
		while (xRot - xRotO >= 180.0f)
			xRotO += 360.0f;
		while (yRot - yRotO < -180.0f)
			yRotO -= 360.0f;
		while (yRot - yRotO >= 180.0f)
			yRotO += 360.0f;
		xRot = xRotO + (xRot - xRotO) * 0.2f;
		yRot = yRotO + (yRot - yRotO) * 0.2f;
		float drag = 0.92f;
		if (onGround || horizontalCollision)
			drag = 0.5f;

		const int_t slices = 5;
		double waterFraction = 0.0;
		for (int_t i = 0; i < slices; i++)
		{
			double y0 = bb.y0 + (bb.y1 - bb.y0) * (i + 0) / slices - 0.125 + 0.125;
			double y1 = bb.y0 + (bb.y1 - bb.y0) * (i + 1) / slices - 0.125 + 0.125;
			AABB *slice = AABB::newTemp(bb.x0, y0, bb.z0, bb.x1, y1, bb.z1);
			if (level.isMaterialInBB(*slice, static_cast<const Material &>(Material::water)))
				waterFraction += 1.0 / slices;
		}

		if (waterFraction > 0.0)
		{
			if (ticksCatchable > 0)
			{
				ticksCatchable--;
			}
			else
			{
				int_t interval = 500;
				if (level.canBlockBeRainedOn(Mth::floor(x), Mth::floor(y) + 1, Mth::floor(z)))
					interval = 300;
				if (random.nextInt(interval) == 0)
				{
					ticksCatchable = random.nextInt(30) + 10;
					yd -= 0.2f;
					level.playSoundAtEntity(*this, u"random.splash", 0.25f,
						1.0f + (random.nextFloat() - random.nextFloat()) * 0.4f);
					float waterY = static_cast<float>(Mth::floor(bb.y0));
					for (int_t i = 0; i < 1.0f + bbWidth * 20.0f; i++)
					{
						float ox = (random.nextFloat() * 2.0f - 1.0f) * bbWidth;
						float oz = (random.nextFloat() * 2.0f - 1.0f) * bbWidth;
						level.addParticle(u"bubble", x + ox, waterY + 1.0f, z + oz,
							xd, yd - random.nextFloat() * 0.2f, zd);
					}
					for (int_t i = 0; i < 1.0f + bbWidth * 20.0f; i++)
					{
						float ox = (random.nextFloat() * 2.0f - 1.0f) * bbWidth;
						float oz = (random.nextFloat() * 2.0f - 1.0f) * bbWidth;
						level.addParticle(u"splash", x + ox, waterY + 1.0f, z + oz, xd, yd, zd);
					}
				}
			}
		}

		if (ticksCatchable > 0)
			yd -= random.nextFloat() * random.nextFloat() * random.nextFloat() * 0.2;
		double buoyancy = waterFraction * 2.0 - 1.0;
		yd += 0.04f * buoyancy;
		if (waterFraction > 0.0)
		{
			drag = static_cast<float>(drag * 0.9);
			yd *= 0.8;
		}
		xd *= drag;
		yd *= drag;
		zd *= drag;
		setPos(x, y, z);
	}
}

void EntityFish::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putShort(u"xTile", static_cast<short_t>(xTile));
	tag.putShort(u"yTile", static_cast<short_t>(yTile));
	tag.putShort(u"zTile", static_cast<short_t>(zTile));
	tag.putByte(u"inTile", static_cast<byte_t>(inTile));
	tag.putByte(u"shake", static_cast<byte_t>(shake));
	tag.putByte(u"inGround", static_cast<byte_t>(inGround ? 1 : 0));
}

void EntityFish::readAdditionalSaveData(CompoundTag &tag)
{
	xTile = tag.getShort(u"xTile");
	yTile = tag.getShort(u"yTile");
	zTile = tag.getShort(u"zTile");
	inTile = static_cast<ubyte_t>(tag.getByte(u"inTile"));
	shake = static_cast<ubyte_t>(tag.getByte(u"shake"));
	inGround = tag.getByte(u"inGround") == 1;
}

float EntityFish::getShadowHeightOffs()
{
	return 0.0f;
}

int_t EntityFish::catchFish()
{
	int_t result = 0;
	if (bobber != nullptr)
	{
		double dx = angler->x - x;
		double dy = angler->y - y;
		double dz = angler->z - z;
		double distance = Mth::sqrt(dx * dx + dy * dy + dz * dz);
		double pull = 0.1;
		bobber->xd += dx * pull;
		bobber->yd += dy * pull + Mth::sqrt(distance) * 0.08;
		bobber->zd += dz * pull;
		result = 3;
	}
	else if (ticksCatchable > 0)
	{
		auto item = std::make_shared<EntityItem>(level, x, y, z,
			ItemInstance(Items::fishRaw->getShiftedIndex(), 1, 0));
		double dx = angler->x - x;
		double dy = angler->y - y;
		double dz = angler->z - z;
		double distance = Mth::sqrt(dx * dx + dy * dy + dz * dz);
		double pull = 0.1;
		item->xd = dx * pull;
		item->yd = dy * pull + Mth::sqrt(distance) * 0.08;
		item->zd = dz * pull;
		level.addEntity(item);
		angler->addStat(*StatList::fishCaughtStat, 1);
		result = 1;
	}
	if (inGround)
		result = 2;
	remove();
	angler->fishEntity.reset();
	return result;
}

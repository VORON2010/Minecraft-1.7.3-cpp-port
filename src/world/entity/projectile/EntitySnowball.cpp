#include "world/entity/projectile/EntitySnowball.h"

#include <cmath>

#include "world/entity/Mob.h"
#include "world/item/Item.h"
#include "world/level/Level.h"
#include "world/phys/AABB.h"
#include "world/phys/HitResult.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"

EntitySnowball::EntitySnowball(Level &level) : Entity(level)
{
	setSize(0.25f, 0.25f);
}

EntitySnowball::EntitySnowball(Level &level, double x, double y, double z) : EntitySnowball(level)
{
	setPos(x, y, z);
	heightOffset = 0.0f;
}

EntitySnowball::EntitySnowball(Level &level, Mob &owner) : EntitySnowball(level)
{
	this->owner = level.getEntityRef(owner);
	absMoveTo(owner.x, owner.y + owner.getHeadHeight(), owner.z, owner.yRot, owner.xRot);
	x -= Mth::cos(yRot * Mth::DEGRAD) * 0.16f;
	y -= 0.1f;
	z -= Mth::sin(yRot * Mth::DEGRAD) * 0.16f;
	setPos(x, y, z);
	heightOffset = 0.0f;
	float speed = 0.4f;
	xd = -Mth::sin(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD) * speed;
	zd = Mth::cos(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD) * speed;
	yd = -Mth::sin(xRot * Mth::DEGRAD) * speed;
	shoot(xd, yd, zd, 1.5f, 1.0f);
}

bool EntitySnowball::shouldRenderAtSqrDistance(double distance)
{
	double size = bb.getSize() * 4.0;
	size *= 64.0;
	return distance < size * size;
}

void EntitySnowball::shoot(double xd, double yd, double zd, float speed, float spread)
{
	float distance = Mth::sqrt(xd * xd + yd * yd + zd * zd);
	xd /= distance;
	yd /= distance;
	zd /= distance;
	xd += random.nextGaussian() * 0.0075f * spread;
	yd += random.nextGaussian() * 0.0075f * spread;
	zd += random.nextGaussian() * 0.0075f * spread;
	this->xd = xd * speed;
	this->yd = yd * speed;
	this->zd = zd * speed;
	float horizontal = Mth::sqrt(this->xd * this->xd + this->zd * this->zd);
	yRotO = yRot = static_cast<float>(std::atan2(this->xd, this->zd) * 180.0 / Mth::PI);
	xRotO = xRot = static_cast<float>(std::atan2(this->yd, horizontal) * 180.0 / Mth::PI);
	ticksInGround = 0;
}

void EntitySnowball::setVelocity(double xd, double yd, double zd)
{
	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
	if (xRotO == 0.0f && yRotO == 0.0f)
	{
		float horizontal = Mth::sqrt(xd * xd + zd * zd);
		yRotO = yRot = static_cast<float>(std::atan2(xd, zd) * 180.0 / Mth::PI);
		xRotO = xRot = static_cast<float>(std::atan2(yd, horizontal) * 180.0 / Mth::PI);
	}
}

void EntitySnowball::tick()
{
	xOld = x;
	yOld = y;
	zOld = z;
	Entity::tick();
	if (shakeTime > 0)
		shakeTime--;

	if (inGround)
	{
		if (level.getTile(xTile, yTile, zTile) == inTile)
		{
			if (++ticksInGround == 1200)
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
	HitResult hit = level.clip(*from, *to, false);
	Vec3 *collisionEnd = to;
	if (hit.type != HitResult::Type::NONE && hit.pos != nullptr)
		collisionEnd = Vec3::newTemp(hit.pos->x, hit.pos->y, hit.pos->z);

	std::shared_ptr<Entity> hitEntity;
	double bestDistance = 0.0;
	std::shared_ptr<Entity> ownerRef = owner.lock();
	AABB *searchBox = bb.expand(xd, yd, zd)->grow(1.0, 1.0, 1.0);
	const auto &entities = level.getEntities(this, *searchBox);
	for (const auto &entity : entities)
	{
		if (!entity->isPickable())
			continue;
		if (ownerRef != nullptr && entity.get() == ownerRef.get() && ticksInAir < 5)
			continue;
		AABB *entityBox = entity->bb.grow(0.3f, 0.3f, 0.3f);
		HitResult entityHit = entityBox->clip(*from, *collisionEnd);
		if (entityHit.type == HitResult::Type::NONE || entityHit.pos == nullptr)
			continue;
		double distance = from->distanceTo(*entityHit.pos);
		if (distance < bestDistance || bestDistance == 0.0)
		{
			hitEntity = entity;
			bestDistance = distance;
		}
	}
	if (hitEntity != nullptr)
		hit = HitResult(hitEntity);

	if (hit.type != HitResult::Type::NONE)
	{
		if (hit.type == HitResult::Type::ENTITY && hit.entity != nullptr)
			hit.entity->hurt(ownerRef.get(), 0);
		for (int_t i = 0; i < 8; ++i)
			level.addParticle(u"snowballpoof", x, y, z, 0.0, 0.0, 0.0);
		remove();
		return;
	}

	x += xd;
	y += yd;
	z += zd;
	float horizontal = Mth::sqrt(xd * xd + zd * zd);
	yRot = static_cast<float>(std::atan2(xd, zd) * 180.0 / Mth::PI);
	xRot = static_cast<float>(std::atan2(yd, horizontal) * 180.0 / Mth::PI);
	while (xRot - xRotO < -180.0f) xRotO -= 360.0f;
	while (xRot - xRotO >= 180.0f) xRotO += 360.0f;
	while (yRot - yRotO < -180.0f) yRotO -= 360.0f;
	while (yRot - yRotO >= 180.0f) yRotO += 360.0f;
	xRot = xRotO + (xRot - xRotO) * 0.2f;
	yRot = yRotO + (yRot - yRotO) * 0.2f;
	float drag = 0.99f;
	float gravity = 0.03f;
	if (isInWater())
	{
		for (int_t i = 0; i < 4; ++i)
			level.addParticle(u"bubble", x - xd * 0.25f, y - yd * 0.25f, z - zd * 0.25f, xd, yd, zd);
		drag = 0.8f;
	}
	xd *= drag;
	yd *= drag;
	zd *= drag;
	yd -= gravity;
	setPos(x, y, z);
}

void EntitySnowball::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putShort(u"xTile", static_cast<short_t>(xTile));
	tag.putShort(u"yTile", static_cast<short_t>(yTile));
	tag.putShort(u"zTile", static_cast<short_t>(zTile));
	tag.putByte(u"inTile", static_cast<byte_t>(inTile));
	tag.putByte(u"shake", static_cast<byte_t>(shakeTime));
	tag.putByte(u"inGround", static_cast<byte_t>(inGround ? 1 : 0));
}

void EntitySnowball::readAdditionalSaveData(CompoundTag &tag)
{
	xTile = tag.getShort(u"xTile");
	yTile = tag.getShort(u"yTile");
	zTile = tag.getShort(u"zTile");
	inTile = tag.getByte(u"inTile") & 255;
	shakeTime = tag.getByte(u"shake") & 255;
	inGround = tag.getByte(u"inGround") == 1;
}

#include "world/entity/projectile/EntityArrow.h"

#include <cmath>

#include "world/entity/Mob.h"
#include "world/entity/player/Player.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/phys/AABB.h"
#include "world/phys/HitResult.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"

EntityArrow::EntityArrow(Level &level) : Entity(level)
{
	setSize(0.5f, 0.5f);
}

EntityArrow::EntityArrow(Level &level, double x, double y, double z) : EntityArrow(level)
{
	setPos(x, y, z);
	heightOffset = 0.0f;
}

EntityArrow::EntityArrow(Level &level, Mob &owner) : EntityArrow(level)
{
	this->owner = level.getEntityRef(owner);
	doesArrowBelongToPlayer = owner.isPlayer();
	absMoveTo(owner.x, owner.y + owner.getHeadHeight(), owner.z, owner.yRot, owner.xRot);
	x -= Mth::cos(yRot * Mth::DEGRAD) * 0.16f;
	y -= 0.1f;
	z -= Mth::sin(yRot * Mth::DEGRAD) * 0.16f;
	setPos(x, y, z);
	heightOffset = 0.0f;
	xd = -Mth::sin(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD);
	zd = Mth::cos(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD);
	yd = -Mth::sin(xRot * Mth::DEGRAD);
	setArrowHeading(xd, yd, zd, 1.5f, 1.0f);
}

void EntityArrow::setArrowHeading(double xd, double yd, double zd, float speed, float spread)
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

void EntityArrow::setVelocity(double xd, double yd, double zd)
{
	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
	if (xRotO == 0.0f && yRotO == 0.0f)
	{
		float horizontal = Mth::sqrt(xd * xd + zd * zd);
		yRotO = yRot = static_cast<float>(std::atan2(xd, zd) * 180.0 / Mth::PI);
		xRotO = xRot = static_cast<float>(std::atan2(yd, horizontal) * 180.0 / Mth::PI);
		absMoveTo(x, y, z, yRot, xRot);
		ticksInGround = 0;
	}
}

void EntityArrow::tick()
{
	Entity::tick();
	if (xRotO == 0.0f && yRotO == 0.0f)
	{
		float horizontal = Mth::sqrt(xd * xd + zd * zd);
		yRotO = yRot = static_cast<float>(std::atan2(xd, zd) * 180.0 / Mth::PI);
		xRotO = xRot = static_cast<float>(std::atan2(yd, horizontal) * 180.0 / Mth::PI);
	}

	int_t tileId = level.getTile(xTile, yTile, zTile);
	if (tileId > 0)
	{
		Tile *tile = Tile::tiles[tileId];
		if (tile != nullptr)
		{
			AABB *aabb = tile->getAABB(level, xTile, yTile, zTile);
			if (aabb != nullptr && aabb->contains(*Vec3::newTemp(x, y, z)))
				inGround = true;
		}
	}

	if (arrowShake > 0)
		arrowShake--;

	if (inGround)
	{
		tileId = level.getTile(xTile, yTile, zTile);
		int_t data = level.getData(xTile, yTile, zTile);
		if (tileId == inTile && data == inData)
		{
			ticksInGround++;
			if (ticksInGround == 1200)
				remove();
		}
		else
		{
			inGround = false;
			xd *= random.nextFloat() * 0.2f;
			yd *= random.nextFloat() * 0.2f;
			zd *= random.nextFloat() * 0.2f;
			ticksInGround = 0;
			ticksInAir = 0;
		}
		return;
	}

	ticksInAir++;
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
		{
			Entity *damageSource = ownerRef.get();
			if (hit.entity->hurt(damageSource, 4))
			{
				level.playSoundAtEntity(*this, u"random.drr", 1.0f, 1.2f / (random.nextFloat() * 0.2f + 0.9f));
				remove();
			}
			else
			{
				xd *= -0.1f;
				yd *= -0.1f;
				zd *= -0.1f;
				yRot += 180.0f;
				yRotO += 180.0f;
				ticksInAir = 0;
			}
		}
		else if (hit.pos != nullptr)
		{
			xTile = hit.x;
			yTile = hit.y;
			zTile = hit.z;
			inTile = level.getTile(xTile, yTile, zTile);
			inData = level.getData(xTile, yTile, zTile);
			xd = hit.pos->x - x;
			yd = hit.pos->y - y;
			zd = hit.pos->z - z;
			float impactDistance = Mth::sqrt(xd * xd + yd * yd + zd * zd);
			x -= xd / impactDistance * 0.05f;
			y -= yd / impactDistance * 0.05f;
			z -= zd / impactDistance * 0.05f;
			level.playSoundAtEntity(*this, u"random.drr", 1.0f, 1.2f / (random.nextFloat() * 0.2f + 0.9f));
			inGround = true;
			arrowShake = 7;
		}
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
		{
			float bubbleOffset = 0.25f;
			level.addParticle(u"bubble", x - xd * bubbleOffset, y - yd * bubbleOffset, z - zd * bubbleOffset, xd, yd, zd);
		}
		drag = 0.8f;
	}
	xd *= drag;
	yd *= drag;
	zd *= drag;
	yd -= gravity;
	setPos(x, y, z);
}

void EntityArrow::playerTouch(Player &player)
{
	if (!level.isOnline && inGround && doesArrowBelongToPlayer && arrowShake <= 0)
	{
		ItemInstance stack(Items::arrow->getShiftedIndex(), 1, 0);
		if (player.inventory.add(stack))
		{
			level.playSoundAtEntity(*this, u"random.pop", 0.2f, ((random.nextFloat() - random.nextFloat()) * 0.7f + 1.0f) * 2.0f);
			player.take(*this, 1);
			remove();
		}
	}
}

void EntityArrow::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putShort(u"xTile", static_cast<short_t>(xTile));
	tag.putShort(u"yTile", static_cast<short_t>(yTile));
	tag.putShort(u"zTile", static_cast<short_t>(zTile));
	tag.putByte(u"inTile", static_cast<byte_t>(inTile));
	tag.putByte(u"inData", static_cast<byte_t>(inData));
	tag.putByte(u"shake", static_cast<byte_t>(arrowShake));
	tag.putByte(u"inGround", static_cast<byte_t>(inGround ? 1 : 0));
	tag.putBoolean(u"player", doesArrowBelongToPlayer);
}

void EntityArrow::readAdditionalSaveData(CompoundTag &tag)
{
	xTile = tag.getShort(u"xTile");
	yTile = tag.getShort(u"yTile");
	zTile = tag.getShort(u"zTile");
	inTile = tag.getByte(u"inTile") & 255;
	inData = tag.getByte(u"inData") & 255;
	arrowShake = tag.getByte(u"shake") & 255;
	inGround = tag.getByte(u"inGround") == 1;
	doesArrowBelongToPlayer = tag.getBoolean(u"player");
}

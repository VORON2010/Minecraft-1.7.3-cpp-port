#include "world/entity/projectile/EntityFireball.h"

#include "world/entity/Mob.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Explosion.h"
#include "world/level/Level.h"
#include "world/phys/HitResult.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"

EntityFireball::EntityFireball(Level &level) : Entity(level)
{
	setSize(1.0f, 1.0f);
}

EntityFireball::EntityFireball(Level &level, double x, double y, double z,
	double xPower, double yPower, double zPower) : Entity(level)
{
	setSize(1.0f, 1.0f);
	moveTo(x, y, z, yRot, xRot);
	setPos(x, y, z);
	double power = Mth::sqrt(xPower * xPower + yPower * yPower + zPower * zPower);
	accelX = xPower / power * 0.1;
	accelY = yPower / power * 0.1;
	accelZ = zPower / power * 0.1;
}

EntityFireball::EntityFireball(Level &level, Mob &owner, double xPower, double yPower, double zPower)
	: Entity(level)
{
	this->owner = level.getEntityRef(owner);
	setSize(1.0f, 1.0f);
	moveTo(owner.x, owner.y, owner.z, owner.yRot, owner.xRot);
	heightOffset = 0.0f;
	xd = yd = zd = 0.0;
	xPower += random.nextGaussian() * 0.4;
	yPower += random.nextGaussian() * 0.4;
	zPower += random.nextGaussian() * 0.4;
	double power = Mth::sqrt(xPower * xPower + yPower * yPower + zPower * zPower);
	accelX = xPower / power * 0.1;
	accelY = yPower / power * 0.1;
	accelZ = zPower / power * 0.1;
}

bool EntityFireball::shouldRenderAtSqrDistance(double distance)
{
	double range = bb.getSize() * 4.0;
	range *= 64.0;
	return distance < range * range;
}

void EntityFireball::tick()
{
	xOld = x;
	yOld = y;
	zOld = z;
	Entity::tick();
	onFire = 10;
	if (shakeTime > 0)
		shakeTime--;
	if (inGround)
	{
		if (level.getTile(xTile, yTile, zTile) == inTile)
		{
			if (++ticksAlive == 1200)
				remove();
			return;
		}
		inGround = false;
		xd *= random.nextFloat() * 0.2f;
		yd *= random.nextFloat() * 0.2f;
		zd *= random.nextFloat() * 0.2f;
		ticksAlive = 0;
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
	auto ownerRef = owner.lock();
	AABB *searchBox = bb.expand(xd, yd, zd)->grow(1.0, 1.0, 1.0);
	const auto &entities = level.getEntities(this, *searchBox);
	for (const auto &entity : entities)
	{
		if (!entity->isPickable())
			continue;
		if (ownerRef != nullptr && entity.get() == ownerRef.get() && ticksInAir < 25)
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
		if (!level.isOnline)
		{
			if (hit.type == HitResult::Type::ENTITY && hit.entity != nullptr)
				hit.entity->hurt(ownerRef.get(), 0);
			level.createExplosion(nullptr, x, y, z, 1.0f, true);
		}
		remove();
		return;
	}

	x += xd;
	y += yd;
	z += zd;
	float horizontal = Mth::sqrt((float)(xd * xd + zd * zd));
	yRot = static_cast<float>(std::atan2(xd, zd) * 180.0 / Mth::PI);
	xRot = static_cast<float>(std::atan2(yd, horizontal) * 180.0 / Mth::PI);
	while (xRot - xRotO < -180.0f) xRotO -= 360.0f;
	while (xRot - xRotO >= 180.0f) xRotO += 360.0f;
	while (yRot - yRotO < -180.0f) yRotO -= 360.0f;
	while (yRot - yRotO >= 180.0f) yRotO += 360.0f;
	xRot = xRotO + (xRot - xRotO) * 0.2f;
	yRot = yRotO + (yRot - yRotO) * 0.2f;
	float drag = 0.95f;
	if (isInWater())
	{
		for (int_t i = 0; i < 4; ++i)
			level.addParticle(u"bubble", x - xd * 0.25f, y - yd * 0.25f, z - zd * 0.25f, xd, yd, zd);
		drag = 0.8f;
	}
	xd = (xd + accelX) * drag;
	yd = (yd + accelY) * drag;
	zd = (zd + accelZ) * drag;
	level.addParticle(u"smoke", x, y + 0.5, z, 0.0, 0.0, 0.0);
	setPos(x, y, z);
}

bool EntityFireball::isPickable()
{
	return true;
}

float EntityFireball::getPickRadius()
{
	return 1.0f;
}

bool EntityFireball::hurt(Entity *source, int_t dmg)
{
	(void)dmg;
	markHurt();
	if (source == nullptr)
		return false;
	double lookX = -Mth::sin(source->yRot * Mth::DEGRAD) * Mth::cos(source->xRot * Mth::DEGRAD);
	double lookZ = Mth::cos(source->yRot * Mth::DEGRAD) * Mth::cos(source->xRot * Mth::DEGRAD);
	double lookY = -Mth::sin(source->xRot * Mth::DEGRAD);
	xd = lookX;
	yd = lookY;
	zd = lookZ;
	accelX = xd * 0.1;
	accelY = yd * 0.1;
	accelZ = zd * 0.1;
	if (auto mob = dynamic_cast<Mob *>(source))
		owner = level.getEntityRef(*mob);
	return true;
}

void EntityFireball::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putShort(u"xTile", static_cast<short_t>(xTile));
	tag.putShort(u"yTile", static_cast<short_t>(yTile));
	tag.putShort(u"zTile", static_cast<short_t>(zTile));
	tag.putByte(u"inTile", static_cast<byte_t>(inTile));
	tag.putByte(u"shake", static_cast<byte_t>(shakeTime));
	tag.putByte(u"inGround", static_cast<byte_t>(inGround ? 1 : 0));
}

void EntityFireball::readAdditionalSaveData(CompoundTag &tag)
{
	xTile = tag.getShort(u"xTile");
	yTile = tag.getShort(u"yTile");
	zTile = tag.getShort(u"zTile");
	inTile = tag.getByte(u"inTile") & 255;
	shakeTime = tag.getByte(u"shake") & 255;
	inGround = tag.getByte(u"inGround") == 1;
}

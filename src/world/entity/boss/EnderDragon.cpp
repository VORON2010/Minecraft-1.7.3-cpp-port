#include "world/entity/boss/EnderDragon.h"

#include <cfloat>
#include <cmath>

#include "util/Mth.h"
#include "world/entity/item/EnderCrystal.h"
#include "world/entity/item/ExperienceOrb.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/EndPortalTile.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/TorchTile.h"
#include "world/level/tile/DragonEggTile.h"
#include "world/phys/AABB.h"
#include "world/phys/Vec3.h"

EnderDragonPart::EnderDragonPart(EnderDragon &dragon, const jstring &id, float w, float h) : Entity(dragon.level), dragon(dragon), id(id)
{
	setSize(w, h);
}

bool EnderDragonPart::hurt(Entity *source, int_t dmg)
{
	return dragon.hurtPart(*this, source, dmg);
}

bool EnderDragonPart::is(const Entity &other) const
{
	return this == &other || &dragon == &other;
}

EnderDragon::EnderDragon(Level &level) : Mob(level)
{
	head = std::make_shared<EnderDragonPart>(*this, u"head", 6.0f, 6.0f);
	body = std::make_shared<EnderDragonPart>(*this, u"body", 8.0f, 8.0f);
	tail1 = std::make_shared<EnderDragonPart>(*this, u"tail", 4.0f, 4.0f);
	tail2 = std::make_shared<EnderDragonPart>(*this, u"tail", 4.0f, 4.0f);
	tail3 = std::make_shared<EnderDragonPart>(*this, u"tail", 4.0f, 4.0f);
	wing1 = std::make_shared<EnderDragonPart>(*this, u"wing", 4.0f, 4.0f);
	wing2 = std::make_shared<EnderDragonPart>(*this, u"wing", 4.0f, 4.0f);
	subEntities = {head, body, tail1, tail2, tail3, wing1, wing2};
	health = MAX_HEALTH;
	textureName = u"/mob/enderdragon/ender.png";
	setSize(16.0f, 8.0f);
	noPhysics = true;
	fireImmune = true;
	yTarget = 100.0;
	noCulling = true;
	dataWatcher.addObject(16, MAX_HEALTH);
}

int_t EnderDragon::getSynchedHealth()
{
	return dataWatcher.getWatchableObjectInt(16);
}

std::array<double, 3> EnderDragon::getLatencyPos(int_t step, float a)
{
	if (health <= 0)
		a = 0.0f;
	a = 1.0f - a;
	int_t p0 = (posPointer - step) & 63;
	int_t p1 = (posPointer - step - 1) & 63;
	std::array<double, 3> result = {};
	double yaw = positions[p0][0];
	double dyaw = positions[p1][0] - yaw;
	while (dyaw < -180.0)
		dyaw += 360.0;
	while (dyaw >= 180.0)
		dyaw -= 360.0;
	result[0] = yaw + dyaw * a;
	double y = positions[p0][1];
	result[1] = y + (positions[p1][1] - y) * a;
	result[2] = positions[p0][2] + (positions[p1][2] - positions[p0][2]) * a;
	return result;
}

void EnderDragon::aiStep()
{
	oFlapTime = flapTime;
	if (!level.isOnline)
		dataWatcher.updateObject(16, health);

	if (health <= 0)
	{
		float ox = (random.nextFloat() - 0.5f) * 8.0f;
		float oy = (random.nextFloat() - 0.5f) * 4.0f;
		float oz = (random.nextFloat() - 0.5f) * 8.0f;
		level.addParticle(u"largeexplode", x + ox, y + 2.0 + oy, z + oz, 0.0, 0.0, 0.0);
		return;
	}

	checkCrystals();
	float flapSpeed = 0.2f / (Mth::sqrt(xd * xd + zd * zd) * 10.0f + 1.0f);
	flapSpeed *= static_cast<float>(std::pow(2.0, yd));
	if (inWall)
		flapTime += flapSpeed * 0.5f;
	else
		flapTime += flapSpeed;

	// 1.5.2: a wing beat sound each time the wings come down
	{
		float now = Mth::cos(flapTime * Mth::PI * 2.0f);
		float before = Mth::cos(oFlapTime * Mth::PI * 2.0f);
		if (before <= -0.3f && now >= -0.3f)
			level.playSoundEffect(x, y, z, u"mob.enderdragon.wings", 5.0f, 0.8f + random.nextFloat() * 0.3f);
	}

	while (yRot >= 180.0f)
		yRot -= 360.0f;
	while (yRot < -180.0f)
		yRot += 360.0f;

	if (posPointer < 0)
	{
		for (auto &pos : positions)
		{
			pos[0] = yRot;
			pos[1] = y;
		}
	}
	if (++posPointer == static_cast<int_t>(positions.size()))
		posPointer = 0;
	positions[posPointer][0] = yRot;
	positions[posPointer][1] = y;

	if (!level.isOnline)
	{
		double dx = xTarget - x;
		double dy = yTarget - y;
		double dz = zTarget - z;
		double dist = dx * dx + dy * dy + dz * dz;
		if (attackTarget != nullptr)
		{
			xTarget = attackTarget->x;
			zTarget = attackTarget->z;
			double tx = xTarget - x;
			double tz = zTarget - z;
			double horizontal = std::sqrt(tx * tx + tz * tz);
			double lift = 0.4f + horizontal / 80.0 - 1.0;
			if (lift > 10.0)
				lift = 10.0;
			yTarget = attackTarget->bb.y0 + lift;
		}
		else
		{
			xTarget += random.nextGaussian() * 2.0;
			zTarget += random.nextGaussian() * 2.0;
		}

		if (newTarget || dist < 100.0 || dist > 22500.0 || horizontalCollision || verticalCollision)
			findNewTarget();

		dy /= Mth::sqrt(dx * dx + dz * dz);
		float maxClimb = 0.6f;
		if (dy < -maxClimb)
			dy = -maxClimb;
		if (dy > maxClimb)
			dy = maxClimb;
		yd += dy * 0.1f;

		while (yRot < -180.0f)
			yRot += 360.0f;
		while (yRot >= 180.0f)
			yRot -= 360.0f;
		double wantYaw = 180.0 - std::atan2(dx, dz) * 180.0 / Mth::PI;
		double turn = wantYaw - yRot;
		while (turn < -180.0)
			turn += 360.0;
		while (turn >= 180.0)
			turn -= 360.0;
		if (turn > 50.0)
			turn = 50.0;
		if (turn < -50.0)
			turn = -50.0;

		Vec3 *toTarget = Vec3::newTemp(xTarget - x, yTarget - y, zTarget - z)->normalize();
		Vec3 *facing = Vec3::newTemp(Mth::sin(yRot * Mth::PI / 180.0f), yd, -Mth::cos(yRot * Mth::PI / 180.0f))->normalize();
		float align = static_cast<float>(facing->dot(*toTarget) + 0.5) / 1.5f;
		if (align < 0.0f)
			align = 0.0f;
		yRotA *= 0.8f;
		float speed = Mth::sqrt(xd * xd + zd * zd) * 1.0f + 1.0f;
		double speedD = std::sqrt(xd * xd + zd * zd) * 1.0 + 1.0;
		if (speedD > 40.0)
			speedD = 40.0;
		yRotA = static_cast<float>(yRotA + turn * (0.7f / speedD / speed));
		yRot += yRotA * 0.1f;
		float inertia = static_cast<float>(2.0 / (speedD + 1.0));
		float accel = 0.06f;
		moveRelative(0.0f, -1.0f, accel * (align * inertia + (1.0f - inertia)));
		if (inWall)
			move(xd * 0.8f, yd * 0.8f, zd * 0.8f);
		else
			move(xd, yd, zd);

		Vec3 *motion = Vec3::newTemp(xd, yd, zd)->normalize();
		float drag = static_cast<float>(motion->dot(*facing) + 1.0) / 2.0f;
		drag = 0.8f + 0.15f * drag;
		xd *= drag;
		zd *= drag;
		yd *= 0.91f;
	}

	yBodyRot = yRot;
	head->bbWidth = head->bbHeight = 3.0f;
	tail1->bbWidth = tail1->bbHeight = 2.0f;
	tail2->bbWidth = tail2->bbHeight = 2.0f;
	tail3->bbWidth = tail3->bbHeight = 2.0f;
	body->bbHeight = 3.0f;
	body->bbWidth = 5.0f;
	wing1->bbHeight = 2.0f;
	wing1->bbWidth = 4.0f;
	wing2->bbHeight = 3.0f;
	wing2->bbWidth = 4.0f;

	float tilt = static_cast<float>(getLatencyPos(5, 1.0f)[1] - getLatencyPos(10, 1.0f)[1]) * 10.0f / 180.0f * Mth::PI;
	float tiltCos = Mth::cos(tilt);
	float tiltSin = -Mth::sin(tilt);
	float yawRad = yRot * Mth::PI / 180.0f;
	float yawSin = Mth::sin(yawRad);
	float yawCos = Mth::cos(yawRad);
	body->tick();
	body->moveTo(x + yawSin * 0.5f, y, z - yawCos * 0.5f, 0.0f, 0.0f);
	wing1->tick();
	wing1->moveTo(x + yawCos * 4.5f, y + 2.0, z + yawSin * 4.5f, 0.0f, 0.0f);
	wing2->tick();
	wing2->moveTo(x - yawCos * 4.5f, y + 2.0, z - yawSin * 4.5f, 0.0f, 0.0f);

	if (!level.isOnline && hurtTime == 0)
	{
		std::vector<std::shared_ptr<Entity>> near1 = level.getEntities(this, *wing1->bb.grow(4.0, 2.0, 4.0)->cloneMove(0.0, -2.0, 0.0));
		knockBack(near1);
		std::vector<std::shared_ptr<Entity>> near2 = level.getEntities(this, *wing2->bb.grow(4.0, 2.0, 4.0)->cloneMove(0.0, -2.0, 0.0));
		knockBack(near2);
		std::vector<std::shared_ptr<Entity>> bitten = level.getEntities(this, *head->bb.grow(1.0, 1.0, 1.0));
		hurtEntities(bitten);
	}

	auto p5 = getLatencyPos(5, 1.0f);
	auto p0 = getLatencyPos(0, 1.0f);
	float headSin = Mth::sin(yRot * Mth::PI / 180.0f - yRotA * 0.01f);
	float headCos = Mth::cos(yRot * Mth::PI / 180.0f - yRotA * 0.01f);
	head->tick();
	head->moveTo(x + headSin * 5.5f * tiltCos, y + (p0[1] - p5[1]) * 1.0 + tiltSin * 5.5f, z - headCos * 5.5f * tiltCos, 0.0f, 0.0f);

	for (int_t i = 0; i < 3; i++)
	{
		EnderDragonPart *part = i == 0 ? tail1.get() : (i == 1 ? tail2.get() : tail3.get());
		auto seg = getLatencyPos(12 + i * 2, 1.0f);
		float angle = yRot * Mth::PI / 180.0f + rotWrap(seg[0] - p5[0]) * Mth::PI / 180.0f * 1.0f;
		float as = Mth::sin(angle);
		float ac = Mth::cos(angle);
		float near = 1.5f;
		float far = (i + 1) * 2.0f;
		part->tick();
		part->moveTo(x - (yawSin * near + as * far) * tiltCos, y + (seg[1] - p5[1]) * 1.0 - (far + near) * tiltSin + 1.5, z + (yawCos * near + ac * far) * tiltCos, 0.0f, 0.0f);
	}

	if (!level.isOnline)
		inWall = checkWalls(head->bb) | checkWalls(body->bb);
}

void EnderDragon::checkCrystals()
{
	if (nearestCrystal != nullptr)
	{
		if (nearestCrystal->removed)
		{
			if (!level.isOnline)
				hurtPart(*head, nullptr, 10, true);
			nearestCrystal = nullptr;
		}
		else if (tickCount % 10 == 0 && health < MAX_HEALTH)
		{
			health++;
		}
	}

	if (random.nextInt(10) == 0)
	{
		float range = 32.0f;
		std::vector<std::shared_ptr<Entity>> found = level.getEntities(this, *bb.grow(range, range, range));
		std::shared_ptr<EnderCrystal> best;
		double bestDist = DBL_MAX;
		for (auto &entity : found)
		{
			auto crystal = std::dynamic_pointer_cast<EnderCrystal>(entity);
			if (crystal == nullptr)
				continue;
			double d = crystal->distanceToSqr(*this);
			if (d < bestDist)
			{
				bestDist = d;
				best = crystal;
			}
		}
		nearestCrystal = best;
	}
}

void EnderDragon::knockBack(const std::vector<std::shared_ptr<Entity>> &entities)
{
	double cx = (body->bb.x0 + body->bb.x1) / 2.0;
	double cz = (body->bb.z0 + body->bb.z1) / 2.0;
	for (auto &entity : entities)
	{
		if (dynamic_cast<Mob *>(entity.get()) == nullptr)
			continue;
		double dx = entity->x - cx;
		double dz = entity->z - cz;
		double d = dx * dx + dz * dz;
		entity->push(dx / d * 4.0, 0.2f, dz / d * 4.0);
	}
}

void EnderDragon::hurtEntities(const std::vector<std::shared_ptr<Entity>> &entities)
{
	for (auto &entity : entities)
	{
		if (dynamic_cast<Mob *>(entity.get()) != nullptr)
			entity->hurt(this, 10);
	}
}

void EnderDragon::findNewTarget()
{
	newTarget = false;
	if (random.nextInt(2) == 0 && !level.players.empty())
	{
		attackTarget = level.players[random.nextInt(static_cast<int_t>(level.players.size()))];
	}
	else
	{
		bool farEnough = false;
		do
		{
			xTarget = 0.0;
			yTarget = 70.0f + random.nextFloat() * 50.0f;
			zTarget = 0.0;
			xTarget += random.nextFloat() * 120.0f - 60.0f;
			zTarget += random.nextFloat() * 120.0f - 60.0f;
			double dx = x - xTarget;
			double dy = y - yTarget;
			double dz = z - zTarget;
			farEnough = dx * dx + dy * dy + dz * dz > 100.0;
		} while (!farEnough);
		attackTarget = nullptr;
	}
}

float EnderDragon::rotWrap(double angle)
{
	while (angle >= 180.0)
		angle -= 360.0;
	while (angle < -180.0)
		angle += 360.0;
	return static_cast<float>(angle);
}

bool EnderDragon::checkWalls(const AABB &box)
{
	int_t x0 = Mth::floor(box.x0);
	int_t y0 = Mth::floor(box.y0);
	int_t z0 = Mth::floor(box.z0);
	int_t x1 = Mth::floor(box.x1);
	int_t y1 = Mth::floor(box.y1);
	int_t z1 = Mth::floor(box.z1);
	bool hitHard = false;
	bool broke = false;
	for (int_t xx = x0; xx <= x1; xx++)
	{
		for (int_t yy = y0; yy <= y1; yy++)
		{
			for (int_t zz = z0; zz <= z1; zz++)
			{
				int_t tile = level.getTile(xx, yy, zz);
				if (tile == 0)
					continue;
				if (tile != Tile::obsidian.id && tile != Tile::endStone.id && tile != Tile::bedrock.id)
				{
					broke = true;
					level.setTile(xx, yy, zz, 0);
				}
				else
				{
					hitHard = true;
				}
			}
		}
	}
	if (broke)
	{
		double px = box.x0 + (box.x1 - box.x0) * random.nextFloat();
		double py = box.y0 + (box.y1 - box.y0) * random.nextFloat();
		double pz = box.z0 + (box.z1 - box.z0) * random.nextFloat();
		level.addParticle(u"largeexplode", px, py, pz, 0.0, 0.0, 0.0);
	}
	return hitHard;
}

bool EnderDragon::hurt(Entity *source, int_t dmg)
{
	// 1.0 BossEntity: only the parts take damage
	(void)source;
	(void)dmg;
	return false;
}

bool EnderDragon::hurtPart(EnderDragonPart &part, Entity *source, int_t dmg, bool explosion)
{
	if (&part != head.get())
		dmg = dmg / 4 + 1;
	float yawRad = yRot * Mth::PI / 180.0f;
	float s = Mth::sin(yawRad);
	float c = Mth::cos(yawRad);
	xTarget = x + s * 5.0f + (random.nextFloat() - 0.5f) * 2.0f;
	yTarget = y + random.nextFloat() * 3.0f + 1.0;
	zTarget = z - c * 5.0f + (random.nextFloat() - 0.5f) * 2.0f;
	attackTarget = nullptr;
	if (dynamic_cast<Player *>(source) != nullptr || explosion)
		Mob::hurt(source, dmg);
	return true;
}

void EnderDragon::tickDeath()
{
	dragonDeathTime++;
	// 1.5.2 global event 1018: everyone hears the death roar from the dragon's direction
	if (dragonDeathTime == 1 && !level.isOnline)
	{
		for (const auto &p : level.players)
		{
			double dx = x - p->x, dy = y - p->y, dz = z - p->z;
			double len = std::sqrt(dx * dx + dy * dy + dz * dz);
			double px = p->x, py = p->y, pz = p->z;
			if (len > 0.0)
			{
				px += dx / len * 2.0;
				py += dy / len * 2.0;
				pz += dz / len * 2.0;
			}
			level.playSoundEffect(px, py, pz, u"mob.enderdragon.end", 5.0f, 1.0f);
		}
	}
	if (dragonDeathTime >= 180 && dragonDeathTime <= 200)
	{
		float ox = (random.nextFloat() - 0.5f) * 8.0f;
		float oy = (random.nextFloat() - 0.5f) * 4.0f;
		float oz = (random.nextFloat() - 0.5f) * 8.0f;
		level.addParticle(u"hugeexplosion", x + ox, y + 2.0 + oy, z + oz, 0.0, 0.0, 0.0);
	}
	if (!level.isOnline && dragonDeathTime > 150 && dragonDeathTime % 5 == 0)
	{
		int_t xp = 1000;
		while (xp > 0)
		{
			int_t orb = ExperienceOrb::getExperienceValue(xp);
			xp -= orb;
			level.addEntity(std::make_shared<ExperienceOrb>(level, x, y, z, orb));
		}
	}
	move(0.0, 0.1f, 0.0);
	yBodyRot = yRot += 20.0f;
	if (dragonDeathTime == 200)
	{
		int_t xp = 10000;
		while (xp > 0)
		{
			int_t orb = ExperienceOrb::getExperienceValue(xp);
			xp -= orb;
			level.addEntity(std::make_shared<ExperienceOrb>(level, x, y, z, orb));
		}
		random.nextInt(2);
		random.nextInt(2);
		random.nextInt(2);
		spawnExitPortal(Mth::floor(x), Mth::floor(z));
		beforeRemove();
		remove();
	}
}

void EnderDragon::spawnExitPortal(int_t px, int_t pz)
{
	int_t py = 128 / 2;
	EndPortalTile::forcePlace = true;
	int_t r = 4;
	for (int_t yy = py - 1; yy <= py + 32; yy++)
	{
		for (int_t xx = px - r; xx <= px + r; xx++)
		{
			for (int_t zz = pz - r; zz <= pz + r; zz++)
			{
				double dx = xx - px;
				double dz = zz - pz;
				double d = Mth::sqrt(dx * dx + dz * dz);
				if (d <= r - 0.5)
				{
					if (yy < py)
					{
						if (!(d > (r - 1) - 0.5))
							level.setTile(xx, yy, zz, Tile::bedrock.id);
					}
					else if (yy > py)
					{
						level.setTile(xx, yy, zz, 0);
					}
					else if (d > (r - 1) - 0.5)
					{
						level.setTile(xx, yy, zz, Tile::bedrock.id);
					}
					else
					{
						level.setTile(xx, yy, zz, Tile::endPortal.id);
					}
				}
			}
		}
	}
	level.setTile(px, py + 0, pz, Tile::bedrock.id);
	level.setTile(px, py + 1, pz, Tile::bedrock.id);
	level.setTile(px, py + 2, pz, Tile::bedrock.id);
	level.setTile(px - 1, py + 2, pz, Tile::torch.id);
	level.setTile(px + 1, py + 2, pz, Tile::torch.id);
	level.setTile(px, py + 2, pz - 1, Tile::torch.id);
	level.setTile(px, py + 2, pz + 1, Tile::torch.id);
	level.setTile(px, py + 3, pz, Tile::bedrock.id);
	level.setTile(px, py + 4, pz, Tile::dragonEgg.id);
	EndPortalTile::forcePlace = false;
}

jstring EnderDragon::getAmbientSound()
{
	return u"mob.enderdragon.growl";
}

jstring EnderDragon::getHurtSound()
{
	return u"mob.enderdragon.hit";
}

float EnderDragon::getSoundVolume()
{
	return 5.0f;
}

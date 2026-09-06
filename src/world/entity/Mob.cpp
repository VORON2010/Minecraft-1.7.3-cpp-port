#include "world/entity/Mob.h"

#include "world/level/Level.h"
#include "world/level/tile/LadderTile.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/item/ItemInstance.h"

#include "util/Mth.h"

Mob::Mob(Level &level) : Entity(level)
{
	blocksBuilding = true;
	footSize = 0.5f;
}

void Mob::defineSynchedData()
{

}

bool Mob::canSee(Entity &entity)
{
	Vec3 *from = Vec3::newTemp(x, y + getHeadHeight(), z);
	Vec3 *to = Vec3::newTemp(entity.x, entity.y + entity.getHeadHeight(), entity.z);
	return level.clip(*from, *to).type == HitResult::Type::NONE;
}

jstring Mob::getTexture()
{
	return textureName;
}

bool Mob::isPickable()
{
	return !removed;
}

bool Mob::isPushable()
{
	return !removed;
}

float Mob::getHeadHeight()
{
	return bbHeight * 0.85f;
}

int_t Mob::getAmbientSoundInterval()
{
	return 80;
}

void Mob::baseTick()
{
	oAttackAnim = attackAnim;

	Entity::baseTick();

	if (random.nextInt(1000) < ambientSoundTime++)
	{
		const jstring &ambientSound = getAmbientSound();
		ambientSoundTime = -getAmbientSoundInterval();
		if (!ambientSound.empty())
		{
			level.playSoundAtEntity(*this, ambientSound, getSoundVolume(),
				(random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
		}
	}

	if (isAlive() && isInWall())
		hurt(nullptr, 1);

	if (fireImmune || level.isOnline)
		onFire = 0;

	if (isAlive() && isUnderLiquid(static_cast<const Material &>(Material::water)) && !isWaterMob())
	{
		airSupply--;
		if (airSupply == -20)
		{
			airSupply = 0;
			for (int_t i = 0; i < 8; ++i)
			{
				float xo = random.nextFloat() - random.nextFloat();
				float yo = random.nextFloat() - random.nextFloat();
				float zo = random.nextFloat() - random.nextFloat();
				level.addParticle(u"bubble", x + xo, y + yo, z + zo, xd, yd, zd);
			}
			hurt(nullptr, 2);
		}

		onFire = 0;
	}
	else
	{
		airSupply = airCapacity;
	}

	oTilt = tilt;
	if (attackTime > 0) attackTime--;
	if (hurtTime > 0) hurtTime--;
	if (invulnerableTime > 0) invulnerableTime--;

	if (health <= 0)
	{
		deathTime++;
		if (deathTime > 20)
		{
			beforeRemove();
			remove();
			for (int_t i = 0; i < 20; ++i)
			{
				double xa = random.nextGaussian() * 0.02;
				double ya = random.nextGaussian() * 0.02;
				double za = random.nextGaussian() * 0.02;
				level.addParticle(
					u"explode",
					x + random.nextFloat() * bbWidth * 2.0f - bbWidth,
					y + random.nextFloat() * bbHeight,
					z + random.nextFloat() * bbWidth * 2.0f - bbWidth,
					xa, ya, za);
			}
		}
	}

	animStepO = animStep;
	yBodyRotO = yBodyRot;
	yRotO = yRot;
	xRotO = xRot;
}

void Mob::playAmbientSound()
{
	const jstring &ambientSound = getAmbientSound();
	if (!ambientSound.empty())
	{
		level.playSoundAtEntity(*this, ambientSound, getSoundVolume(),
			(random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
	}
}

void Mob::spawnAnim()
{
	for (int_t i = 0; i < 20; ++i)
	{
		double xa = random.nextGaussian() * 0.02;
		double ya = random.nextGaussian() * 0.02;
		double za = random.nextGaussian() * 0.02;
		double scale = 10.0;
		level.addParticle(
			u"explode",
			x + random.nextFloat() * bbWidth * 2.0f - bbWidth - xa * scale,
			y + random.nextFloat() * bbHeight - ya * scale,
			z + random.nextFloat() * bbWidth * 2.0f - bbWidth - za * scale,
			xa, ya, za);
	}
}

void Mob::rideTick()
{
	Entity::rideTick();
	oRun = run;
	run = 0.0f;
}

void Mob::lerpTo(double x, double y, double z, float yRot, float xRot, int_t steps)
{
	heightOffset = 0.0f;
	lx = x;
	ly = y;
	lz = z;
	lyr = yRot;
	lxr = xRot;
	lSteps = steps;
}

void Mob::superTick()
{
	Entity::tick();
}

void Mob::tick()
{
	Entity::tick();
	
	aiStep();

	double fxd = x - xo;
	double fzd = z - zo;
	float fd = Mth::sqrt(fxd * fxd + fzd * fzd);

	float targetBodyRot = yBodyRot;
	float speedAnimStep = 0.0f;

	oRun = run;

	float targetRun = 0.0f;
	if (fd > 0.05f)
	{
		targetRun = 1.0f;
		speedAnimStep = fd * 3.0f;
		targetBodyRot = static_cast<float>(std::atan2(fzd, fxd)) * 180.0f / Mth::PI - 90.0f;
	}

	if (attackAnim > 0.0f)
		targetBodyRot = yRot;

	if (!onGround)
		targetRun = 0.0f;

	run += (targetRun - run) * 0.3f;

	float deltaBodyRot = targetBodyRot - yBodyRot;
	while (deltaBodyRot < -180.0f) deltaBodyRot += 360.0f;
	while (deltaBodyRot >= 180.0f) deltaBodyRot -= 360.0f;

	yBodyRot += deltaBodyRot * 0.3f;

	float deltaYRot = yRot - yBodyRot;
	while (deltaYRot < -180.0f) deltaYRot += 360.0f;
	while (deltaYRot >= 180.0f) deltaYRot -= 360.0f;

	bool rotAtLimit = deltaYRot < -90.0f || deltaYRot >= 90.0f;
	if (deltaYRot < -75.0f) deltaYRot = -75.0f;
	if (deltaYRot >= 75.0f) deltaYRot = 75.0f;

	yBodyRot = yRot - deltaYRot;
	if (deltaYRot * deltaYRot > 2500.0f)
		yBodyRot += deltaYRot * 0.2f;

	if (rotAtLimit)
		speedAnimStep *= -1.0f;

	while (yRot - yRotO < -180.0f) yRotO -= 360.0f;
	while (yRot - yRotO >= 180.0f) yRotO += 360.0f;

	while (yBodyRot - yBodyRotO < -180.0f) yBodyRotO -= 360.0f;
	while (yBodyRot - yBodyRotO >= 180.0f) yBodyRotO += 360.0f;

	while (xRot - xRotO < -180.0f) xRotO -= 360.0f;
	while (xRot - xRotO >= 180.0f) xRotO += 360.0f;

	animStep += speedAnimStep;
}

void Mob::setSize(float width, float height)
{
	Entity::setSize(width, height);
}

void Mob::heal(int_t heal)
{
	if (health > 0)
	{
		health += heal;
		if (health > 20)
			health = 20;
		invulnerableTime = invulnerableDuration / 2;
	}
}

bool Mob::hurt(Entity *source, int_t dmg)
{
	if (level.isOnline)
		return false;
	noActionTime = 0;
	if (health <= 0)
		return false;

	walkAnimSpeed = 1.5f;
	bool playEffects = true;

	if (static_cast<float>(invulnerableTime) > static_cast<float>(invulnerableDuration) / 2.0f)
	{
		if (dmg <= lastHurt)
			return false;
		actuallyHurt(dmg - lastHurt);
		lastHurt = dmg;
		playEffects = false;
	}
	else
	{
		lastHurt = dmg;
		lastHealth = health;
		invulnerableTime = invulnerableDuration;
		actuallyHurt(dmg);
		hurtTime = hurtDuration = 10;
	}

	hurtDir = 0.0f;
	if (playEffects)
	{
		markHurt();
		if (source != nullptr)
		{
			double dx = source->x - x;
			double dz = source->z - z;
			while (dx * dx + dz * dz < 1.0E-4)
			{
				dx = (random.nextFloat() - random.nextFloat()) * 0.01;
				dz = (random.nextFloat() - random.nextFloat()) * 0.01;
			}
			hurtDir = static_cast<float>(std::atan2(dz, dx) * 180.0 / Mth::PI) - yRot;
			knockback(*source, dmg, dx, dz);
		}
		else
		{
			hurtDir = static_cast<float>(static_cast<int_t>(random.nextFloat() * 2.0f) * 180);
		}
	}

	if (health <= 0)
	{
		if (playEffects)
		{
			jstring deathSound = getDeathSound();
			if (!deathSound.empty())
				level.playSoundAtEntity(*this, deathSound, getSoundVolume(), (random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
		}
		die(source);
	}
	else if (playEffects)
	{
		jstring hurtSound = getHurtSound();
		if (!hurtSound.empty())
			level.playSoundAtEntity(*this, hurtSound, getSoundVolume(), (random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
	}

	return true;
}

void Mob::animateHurt()
{
	hurtTime = hurtDuration = 10;
	hurtDir = 0.0f;
}

void Mob::actuallyHurt(int_t dmg)
{
	health -= dmg;
}

float Mob::getSoundVolume()
{
	return 1.0f;
}

jstring Mob::getAmbientSound()
{
	return u"";
}

jstring Mob::getHurtSound()
{
	return u"random.hurt";
}

jstring Mob::getDeathSound()
{
	return u"random.hurt";
}

void Mob::knockback(Entity &source, int_t unknown, double x, double z)
{
	float dist = Mth::sqrt(x * x + z * z);
	if (dist <= 0.0f)
		return;
	float strength = 0.4f;

	xd /= 2.0;
	yd /= 2.0;
	zd /= 2.0;

	xd -= x / dist * strength;
	yd += 0.4f;
	zd -= z / dist * strength;

	if (yd > 0.4f)
		yd = 0.4f;
}

void Mob::die(Entity *source)
{
	if (deathScore >= 0 && source != nullptr)
		source->awardKillScore(*this, deathScore);
	dead = true;
	if (!level.isOnline)
		dropDeathLoot();
}

void Mob::dropDeathLoot()
{
	int_t loot = getDeathLoot();
	if (loot <= 0)
		return;

	int_t count = random.nextInt(3);
	for (int_t i = 0; i < count; ++i)
		spawnAtLocation(ItemInstance(loot, 1, 0), 0.0f);
}

int_t Mob::getDeathLoot()
{
	return 0;
}

void Mob::causeFallDamage(float distance)
{
	Entity::causeFallDamage(distance);
	int_t fallDamage = Mth::ceil(distance - 3.0f);
	if (fallDamage <= 0)
		return;

	hurt(nullptr, fallDamage);

	int_t tileX = Mth::floor(x);
	int_t tileY = Mth::floor(y - 0.2 - heightOffset);
	int_t tileZ = Mth::floor(z);
	int_t tile = level.getTile(tileX, tileY, tileZ);
	if (tile <= 0)
		return;

	Tile *landedTile = Tile::tiles[tile];
	if (landedTile == nullptr || landedTile->soundType == nullptr)
		return;

	StepSound *ss = landedTile->soundType;
	level.playSoundAtEntity(*this, ss->getStepResourcePath(), ss->getVolume() * 0.5f, ss->getPitch() * 0.75f);
}

void Mob::travel(float x, float z)
{
	if (isInWater())
	{
		double oy = y;

		moveRelative(x, z, 0.02f);
		move(xd, yd, zd);

		xd *= 0.8;
		yd *= 0.8;
		zd *= 0.8;
		yd -= 0.02;

		if (horizontalCollision && isFree(xd, yd + 0.6 - y + oy, zd))
			yd = 0.3;
	}
	else if (isInLava())
	{
		double oy = y;

		moveRelative(x, z, 0.02f);
		move(xd, yd, zd);

		xd *= 0.5;
		yd *= 0.5;
		zd *= 0.5;
		yd -= 0.02;

		if (horizontalCollision && isFree(xd, yd + 0.6 - y + oy, zd))
			yd = 0.3;
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
		if (isSprinting && onGround) acceleration *= 1.5f;
		// Creative flight has its own air-control speed.  Sprint doubles it,
		// matching the expected accelerated flight behaviour.
		float airAcceleration = isFlying ? (isSprinting ? 0.10f : 0.05f)
			: (isSprinting ? 0.025f : 0.02f);
		moveRelative(x, z, onGround ? (0.1f * acceleration) : airAcceleration);

		friction = 0.91f;
		if (onGround)
		{
			friction = 0.546f;
			int_t tile = level.getTile(Mth::floor(this->x), Mth::floor(bb.y0) - 1, Mth::floor(this->z));
			if (tile > 0)
				friction = Tile::tiles[tile]->friction * 0.91f;
		}

		if (onLadder())
		{
			float clamp = 0.15f;
			if (xd < -clamp) xd = -clamp;
			if (xd > clamp) xd = clamp;
			if (zd < -clamp) zd = -clamp;
			if (zd > clamp) zd = clamp;
			fallDistance = 0.0f;
			if (yd < -0.15) yd = -0.15;
			if (isSneaking() && yd < 0.0) yd = 0.0;
		}

		move(xd, yd, zd);

		if (horizontalCollision && onLadder())
			yd = 0.2;

		if (!isFlying) {
			yd -= 0.08;
			yd *= 0.98;
		} else {
			yd *= 0.8; // Dampen vertical speed slightly
		}
		xd *= friction;
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

bool Mob::onLadder()
{
	int_t tx = Mth::floor(x);
	int_t ty = Mth::floor(bb.y0);
	int_t tz = Mth::floor(z);
	return level.getTile(tx, ty, tz) == Tile::ladder.id;
}

bool Mob::isShootable()
{
	return true;
}

void Mob::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putShort(u"Health", static_cast<short_t>(health));
	tag.putShort(u"HurtTime", static_cast<short_t>(hurtTime));
	tag.putShort(u"DeathTime", static_cast<short_t>(deathTime));
	tag.putShort(u"AttackTime", static_cast<short_t>(attackTime));
}

void Mob::readAdditionalSaveData(CompoundTag &tag)
{
	health = tag.getShort(u"Health");
	if (!tag.contains(u"Health"))
		health = 10;
	hurtTime = tag.getShort(u"HurtTime");
	deathTime = tag.getShort(u"DeathTime");
	attackTime = tag.getShort(u"AttackTime");
}

bool Mob::isAlive()
{
	return !removed && health > 0;
}

bool Mob::isWaterMob()
{
	return false;
}

void Mob::aiStep()
{
	if (lSteps > 0)
	{
		double xd = x + (lx - x) / lSteps;
		double yd = y + (ly - y) / lSteps;
		double zd = z + (lz - z) / lSteps;

		double deltaYRot = lyr - yRot;
		while (deltaYRot < -180.0f) deltaYRot += 360.0f;
		while (deltaYRot >= 180.0f) deltaYRot -= 360.0f;

		yRot = static_cast<float>(yRot + deltaYRot / lSteps);
		xRot = static_cast<float>(xRot + (lxr - xRot) / lSteps);

		--lSteps;

		setPos(xd, yd, zd);
		setRot(yRot, xRot);
	}

	if (health <= 0)
	{
		jumping = false;
		xxa = 0.0f;
		yya = 0.0f;
		yRotA = 0.0f;
	}
	else if (!interpolateOnly)
	{
		updateAi();
	}

	
	bool inWater = isInWater();
	bool inLava = isInLava();

	if (jumping)
	{
		if (inWater)
			yd += 0.04;
		else if (inLava)
			yd += 0.04;
		else if (onGround)
			jumpFromGround();
	}

	
	xxa *= 0.98f;
	yya *= 0.98f;
	yRotA *= 0.9f;
	travel(xxa, yya);

	const auto &entities = level.getEntities(this, *bb.grow(0.2, 0.0f, 0.2));
	if (!entities.empty())
	{
		for (const auto &entity : entities)
		{
			if (entity->isPushable())
				entity->push(*this);
		}
	}
}

void Mob::jumpFromGround()
{
	yd = 0.42;
}

void Mob::updateAi()
{
	noActionTime++;
	std::shared_ptr<Player> nearestPlayer = level.getNearestPlayer(*this, -1.0);
	if (canDespawn() && nearestPlayer != nullptr)
	{
		double dx = nearestPlayer->x - x;
		double dy = nearestPlayer->y - y;
		double dz = nearestPlayer->z - z;
		double distanceSqr = dx * dx + dy * dy + dz * dz;
		if (distanceSqr > 16384.0)
			remove();

		if (noActionTime > 600 && random.nextInt(800) == 0)
		{
			if (distanceSqr < 1024.0)
				noActionTime = 0;
			else
				remove();
		}
	}

	xxa = 0.0f;
	yya = 0.0f;
	float lookRange = 8.0f;
	if (random.nextFloat() < 0.02f)
	{
		nearestPlayer = level.getNearestPlayer(*this, lookRange);
		if (nearestPlayer != nullptr)
		{
			lookingAt = nearestPlayer;
			lookTime = 10 + random.nextInt(20);
		}
		else
		{
			yRotA = (random.nextFloat() - 0.5f) * 20.0f;
		}
	}

	if (lookingAt != nullptr)
	{
		lookAt(*lookingAt, 10.0f, static_cast<float>(getMaxHeadXRot()));
		if (lookTime-- <= 0 || lookingAt->removed || lookingAt->distanceToSqr(*this) > lookRange * lookRange)
			lookingAt = nullptr;
	}
	else
	{
		if (random.nextFloat() < 0.05f)
			yRotA = (random.nextFloat() - 0.5f) * 20.0f;

		yRot += yRotA;
		xRot = defaultLookAngle;
	}

	bool inWater = isInWater();
	bool inLava = isInLava();
	if (inWater || inLava)
		jumping = random.nextFloat() < 0.8f;
}

bool Mob::canDespawn()
{
	return true;
}

void Mob::lookAt(Entity &entity, float yawSpeed, float pitchSpeed)
{
	double dx = entity.x - x;
	double dz = entity.z - z;
	double dy;
	if (Mob *mob = dynamic_cast<Mob *>(&entity))
		dy = y + getHeadHeight() - (mob->y + mob->getHeadHeight());
	else
		dy = (entity.bb.y0 + entity.bb.y1) / 2.0 - (y + getHeadHeight());

	double flatDistance = Mth::sqrt(dx * dx + dz * dz);
	float targetYRot = static_cast<float>(std::atan2(dz, dx) * 180.0 / Mth::PI) - 90.0f;
	float targetXRot = static_cast<float>(-(std::atan2(dy, flatDistance) * 180.0 / Mth::PI));
	xRot = -rotlerp(xRot, targetXRot, pitchSpeed);
	yRot = rotlerp(yRot, targetYRot, yawSpeed);
}

int_t Mob::getMaxHeadXRot()
{
	return 40;
}

float Mob::rotlerp(float from, float to, float speed)
{
	float delta = to - from;
	while (delta < -180.0f) delta += 360.0f;
	while (delta >= 180.0f) delta -= 360.0f;

	if (delta > speed) delta = speed;
	if (delta < -speed) delta = -speed;
	return from + delta;
}

void Mob::beforeRemove()
{

}

bool Mob::canSpawn()
{
	return level.isUnobstructed(bb) && level.getCubes(*this, bb).empty() && !level.containsAnyLiquid(bb);
}

void Mob::outOfWorld()
{
	hurt(nullptr, 4);
}

float Mob::getAttackAnim(float a)
{
	float f = attackAnim - oAttackAnim;
	if (f < 0.0f) f++;
	return oAttackAnim + f * a;
}

Vec3 *Mob::getPos(float a)
{
	if (a == 1.0f)
		return Vec3::newTemp(x, y, z);

	double xd = xo + (x - xo) * a;
	double yd = yo + (y - yo) * a;
	double zd = zo + (z - zo) * a;
	return Vec3::newTemp(xd, yd, zd);
}

Vec3 *Mob::getLookAngle()
{
	return getViewVector(1.0f);
}

Vec3 *Mob::getViewVector(float a)
{
	if (a == 1.0f)
	{
		float cy = Mth::cos(-yRot * Mth::DEGRAD - Mth::PI);
		float sy = Mth::sin(-yRot * Mth::DEGRAD - Mth::PI);
		float cx = -Mth::cos(-xRot * Mth::DEGRAD);
		float sx = Mth::sin(-xRot * Mth::DEGRAD);
		return Vec3::newTemp(sy * cx, sx, cy * cx);
	}
	else
	{
		float xa = yRotO + (yRot - yRotO) * a;
		float ya = xRotO + (xRot - xRotO) * a;
		float cy = Mth::cos(-xa * Mth::DEGRAD - Mth::PI);
		float sy = Mth::sin(-xa * Mth::DEGRAD - Mth::PI);
		float cx = -Mth::cos(-ya * Mth::DEGRAD);
		float sx = Mth::sin(-ya * Mth::DEGRAD);
		return Vec3::newTemp(sy * cx, sx, cy * cx);
	}
}

HitResult Mob::pick(double length, float a)
{
	Vec3 *pos = getPos(a);
	Vec3 *look = getViewVector(a);
	Vec3 *to = pos->add(look->x * length, look->y * length, look->z * length);
	return level.clip(*pos, *to);
}

int_t Mob::getMaxSpawnClusterSize()
{
	return 4;
}

ItemInstance *Mob::getCarriedItem()
{
	return nullptr;
}

bool Mob::hasCurrentTarget() const
{
	return lookingAt != nullptr;
}

std::shared_ptr<Entity> Mob::getCurrentTarget() const
{
	return lookingAt;
}

void Mob::handleEntityEvent(byte_t event)
{
	if (event == 2)
	{
		walkAnimSpeed = 1.5f;
		invulnerableTime = invulnerableDuration;
		hurtTime = hurtDuration = 10;
		hurtDir = 0.0f;
		jstring hurtSound = getHurtSound();
		if (!hurtSound.empty())
			level.playSoundAtEntity(*this, hurtSound, getSoundVolume(),
				(random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
		hurt(nullptr, 0);
	}
	else if (event == 3)
	{
		jstring deathSound = getDeathSound();
		if (!deathSound.empty())
			level.playSoundAtEntity(*this, deathSound, getSoundVolume(),
				(random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
		health = 0;
		die(nullptr);
	}
	else
	{
		Entity::handleEntityEvent(event);
	}
}

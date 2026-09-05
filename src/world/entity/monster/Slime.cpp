#include "world/entity/monster/Slime.h"

#include "nbt/CompoundTag.h"
#include "world/entity/player/Player.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/level/Level.h"
#include "util/Mth.h"
#include "java/Random.h"

Slime::Slime(Level &level) : Mob(level)
{
	dataWatcher.addObject(16, static_cast<byte_t>(1));
	textureName = u"/mob/slime.png";
	renderOffset = 0.0f;
	int_t size = 1 << random.nextInt(3);
	slimeJumpDelay = random.nextInt(20) + 10;
	setSlimeSize(size);
}

void Slime::setSlimeSize(int_t size)
{
	dataWatcher.updateObject(16, static_cast<byte_t>(size));
	setSize(0.6f * size, 0.6f * size);
	health = size * size;
	setPos(x, y, z);
}

int_t Slime::getSlimeSize() const
{
	return dataWatcher.getWatchableObjectByte(16);
}

void Slime::tick()
{
	squishOld = squish;
	bool wasOnGround = onGround;
	Mob::tick();
	if (onGround && !wasOnGround)
	{
		int_t size = getSlimeSize();
		for (int_t i = 0; i < size * 8; ++i)
		{
			float angle = random.nextFloat() * Mth::PI * 2.0f;
			float dist = random.nextFloat() * 0.5f + 0.5f;
			float px = Mth::sin(angle) * size * 0.5f * dist;
			float pz = Mth::cos(angle) * size * 0.5f * dist;
			level.addParticle(u"slime", x + px, bb.y0, z + pz, 0.0, 0.0, 0.0);
		}
		if (size > 2)
			level.playSoundAtEntity(*this, u"mob.slime", getSoundVolume(), ((random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f) / 0.8f);
		squish = -0.5f;
	}
	squish *= 0.6f;
}

void Slime::playerTouch(Player &player)
{
	int_t size = getSlimeSize();
	if (size > 1 && canSee(player) && distanceTo(player) < 0.6f * size && player.hurt(this, size))
		level.playSoundAtEntity(*this, u"mob.slimeattack", 1.0f, (random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
}

void Slime::die(Entity *source)
{
	Mob::die(source);
}

bool Slime::canSpawn()
{
	int_t chunkX = Mth::floor(x) >> 4;
	int_t chunkZ = Mth::floor(z) >> 4;
	Random chunkRandom(level.seed + chunkX * chunkX * 4987142 + chunkX * 5947611 + chunkZ * chunkZ * 4392871LL + chunkZ * 389711 ^ 987234911L);
	return (getSlimeSize() == 1 || level.difficulty > 0) && random.nextInt(10) == 0 && chunkRandom.nextInt(10) == 0 && y < 16.0;
}

void Slime::addAdditionalSaveData(CompoundTag &tag)
{
	Mob::addAdditionalSaveData(tag);
	tag.putInt(u"Size", getSlimeSize() - 1);
}

void Slime::readAdditionalSaveData(CompoundTag &tag)
{
	Mob::readAdditionalSaveData(tag);
	setSlimeSize(tag.getInt(u"Size") + 1);
}

void Slime::updateAi()
{
	Player *target = nullptr;
	auto nearest = level.getNearestPlayer(*this, 16.0);
	if (nearest != nullptr)
		target = nearest.get();
	if (target != nullptr)
		lookAt(*target, 10.0f, 20.0f);
	if (onGround && slimeJumpDelay-- <= 0)
	{
		slimeJumpDelay = random.nextInt(20) + 10;
		if (target != nullptr)
			slimeJumpDelay /= 3;
		jumping = true;
		if (getSlimeSize() > 1)
			level.playSoundAtEntity(*this, u"mob.slime", getSoundVolume(), ((random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f) * 0.8f);
		squish = 1.0f;
		xxa = 1.0f - random.nextFloat() * 2.0f;
		yya = static_cast<float>(getSlimeSize());
	}
	else
	{
		jumping = false;
		if (onGround)
			xxa = yya = 0.0f;
	}
}

void Slime::beforeRemove()
{
	int_t size = getSlimeSize();
	if (!level.isOnline && size > 1 && health == 0)
	{
		for (int_t i = 0; i < 4; ++i)
		{
			float dx = (i % 2 - 0.5f) * size / 4.0f;
			float dz = (i / 2 - 0.5f) * size / 4.0f;
			auto slime = std::make_shared<Slime>(level);
			slime->setSlimeSize(size / 2);
			slime->moveTo(x + dx, y + 0.5, z + dz, random.nextFloat() * 360.0f, 0.0f);
			level.addEntity(slime);
		}
	}
}

jstring Slime::getHurtSound()
{
	return u"mob.slime";
}

jstring Slime::getDeathSound()
{
	return u"mob.slime";
}

int_t Slime::getDeathLoot()
{
	return getSlimeSize() == 1 ? Items::slimeball->getShiftedIndex() : 0;
}

float Slime::getSoundVolume()
{
	return 0.6f;
}

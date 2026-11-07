#include "world/entity/item/ExperienceOrb.h"

#include <cmath>

#include "java/Math.h"
#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/Tile.h"

ExperienceOrb::ExperienceOrb(Level &level) : Entity(level)
{
	setSize(0.25f, 0.25f);
	heightOffset = bbHeight / 2.0f;
	makeStepSound = false;
}

ExperienceOrb::ExperienceOrb(Level &level, double x, double y, double z, int_t value) : Entity(level)
{
	setSize(0.5f, 0.5f);
	heightOffset = bbHeight / 2.0f;
	makeStepSound = false;
	setPos(x, y, z);
	yRot = static_cast<float>(Math::random() * 360.0);
	xd = static_cast<float>(Math::random() * 0.2 - 0.1) * 2.0f;
	yd = static_cast<float>(Math::random() * 0.2) * 2.0f;
	zd = static_cast<float>(Math::random() * 0.2 - 0.1) * 2.0f;
	this->value = value;
}

float ExperienceOrb::getBrightness(float a)
{
	// orbs glow a little in the dark
	float br = Entity::getBrightness(a);
	return br < 0.5f ? 0.5f + br * 0.5f : br;
}

void ExperienceOrb::tick()
{
	Entity::tick();
	if (throwTime > 0)
		throwTime--;

	xo = x;
	yo = y;
	zo = z;
	yd -= 0.03;
	if (&level.getMaterial(Mth::floor(x), Mth::floor(y), Mth::floor(z)) == static_cast<const Material *>(&Material::lava))
	{
		yd = 0.2;
		xd = (random.nextFloat() - random.nextFloat()) * 0.2f;
		zd = (random.nextFloat() - random.nextFloat()) * 0.2f;
		level.playSoundAtEntity(*this, u"random.fizz", 0.4f, 2.0f + random.nextFloat() * 0.4f);
	}

	double range = 8.0;
	std::shared_ptr<Player> player = level.getNearestPlayer(*this, range);
	if (player != nullptr)
	{
		double dx = (player->x - x) / range;
		double dy = (player->y + player->getHeadHeight() - y) / range;
		double dz = (player->z - z) / range;
		double dist = std::sqrt(dx * dx + dy * dy + dz * dz);
		double pull = 1.0 - dist;
		if (pull > 0.0 && dist > 0.0)
		{
			pull *= pull;
			xd += dx / dist * pull * 0.1;
			yd += dy / dist * pull * 0.1;
			zd += dz / dist * pull * 0.1;
		}
	}

	move(xd, yd, zd);
	float friction = 0.98f;
	if (onGround)
	{
		friction = 0.58800006f;
		int_t below = level.getTile(Mth::floor(x), Mth::floor(bb.y0) - 1, Mth::floor(z));
		if (below > 0 && Tile::tiles[below] != nullptr)
			friction = Tile::tiles[below]->friction * 0.98f;
	}

	xd *= friction;
	yd *= 0.98;
	zd *= friction;
	if (onGround)
		yd *= -0.9;

	tickCount2++;
	age++;
	if (age >= 6000)
		remove();
}

bool ExperienceOrb::hurt(Entity *source, int_t damage)
{
	(void)source;
	markHurt();
	health -= damage;
	if (health <= 0)
		remove();
	return false;
}

void ExperienceOrb::playerTouch(Player &player)
{
	if (level.isOnline)
		return;
	if (throwTime != 0 || player.takeXpDelay != 0)
		return;
	player.takeXpDelay = 2;
	level.playSoundAtEntity(*this, u"random.orb", 0.1f, 0.5f * ((random.nextFloat() - random.nextFloat()) * 0.7f + 1.8f));
	player.take(*this, 1);
	player.increaseXp(value);
	remove();
}

void ExperienceOrb::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putShort(u"Health", static_cast<short_t>(static_cast<byte_t>(health)));
	tag.putShort(u"Age", static_cast<short_t>(age));
	tag.putShort(u"Value", static_cast<short_t>(value));
}

void ExperienceOrb::readAdditionalSaveData(CompoundTag &tag)
{
	health = tag.getShort(u"Health") & 255;
	age = tag.getShort(u"Age");
	value = tag.getShort(u"Value");
}

int_t ExperienceOrb::getIcon() const
{
	if (value >= 2477) return 10;
	if (value >= 1237) return 9;
	if (value >= 617) return 8;
	if (value >= 307) return 7;
	if (value >= 149) return 6;
	if (value >= 73) return 5;
	if (value >= 37) return 4;
	if (value >= 17) return 3;
	if (value >= 7) return 2;
	return value >= 3 ? 1 : 0;
}

int_t ExperienceOrb::getExperienceValue(int_t maxValue)
{
	if (maxValue >= 2477) return 2477;
	if (maxValue >= 1237) return 1237;
	if (maxValue >= 617) return 617;
	if (maxValue >= 307) return 307;
	if (maxValue >= 149) return 149;
	if (maxValue >= 73) return 73;
	if (maxValue >= 37) return 37;
	if (maxValue >= 17) return 17;
	if (maxValue >= 7) return 7;
	return maxValue >= 3 ? 3 : 1;
}

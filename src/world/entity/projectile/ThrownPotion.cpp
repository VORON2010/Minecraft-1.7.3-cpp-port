#include "world/entity/projectile/ThrownPotion.h"

#include <cmath>

#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/entity/Mob.h"
#include "world/item/Items.h"
#include "world/item/PotionItem.h"
#include "world/level/Level.h"
#include "world/phys/HitResult.h"

ThrownPotion::ThrownPotion(Level &level, Mob &owner, int_t potionValue) : EntitySnowball(level, owner), potionValue(potionValue)
{
	// 1.0 ThrownEntity with getPower 0.5 and getStartPitchOffset -20
	float speed = 0.4f;
	xd = -Mth::sin(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD) * speed;
	zd = Mth::cos(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD) * speed;
	yd = -Mth::sin((xRot - 20.0f) * Mth::DEGRAD) * speed;
	shoot(xd, yd, zd, 0.5f, 1.0f);
}

ThrownPotion::ThrownPotion(Level &level, double x, double y, double z, int_t potionValue) : EntitySnowball(level, x, y, z), potionValue(potionValue)
{
}

void ThrownPotion::onHit(HitResult &hit)
{
	if (level.isOnline)
		return;
	const PotionItem *potion = static_cast<const PotionItem *>(Items::potion);
	const std::vector<MobEffectInstance> &effects = potion->getMobEffects(potionValue);
	if (!effects.empty())
	{
		AABB *area = bb.grow(4.0, 2.0, 4.0);
		std::vector<std::shared_ptr<Entity>> near = level.getEntities(nullptr, *area);
		std::shared_ptr<Entity> thrower = owner.lock();
		for (auto &entity : near)
		{
			Mob *mob = dynamic_cast<Mob *>(entity.get());
			if (mob == nullptr)
				continue;
			double d2 = distanceToSqr(*entity);
			if (d2 >= 16.0)
				continue;
			double scale = 1.0 - std::sqrt(d2) / 4.0;
			if (entity == hit.entity)
				scale = 1.0;
			for (const MobEffectInstance &e : effects)
			{
				const MobEffect *effect = MobEffect::effects[e.getId()];
				if (effect->isInstantenous())
					effect->applyInstantenousEffect(dynamic_cast<Mob *>(thrower.get()), *mob, e.getAmplifier(), scale);
				else
				{
					int_t duration = static_cast<int_t>(scale * e.getDuration() + 0.5);
					if (duration > 20)
						mob->addEffect(MobEffectInstance(e.getId(), duration, e.getAmplifier()));
				}
			}
		}
	}
	level.levelEvent(2002, static_cast<int_t>(std::lround(x)), static_cast<int_t>(std::lround(y)), static_cast<int_t>(std::lround(z)), potionValue);
	remove();
}

void ThrownPotion::addAdditionalSaveData(CompoundTag &tag)
{
	EntitySnowball::addAdditionalSaveData(tag);
	tag.putInt(u"potionValue", potionValue);
}

void ThrownPotion::readAdditionalSaveData(CompoundTag &tag)
{
	EntitySnowball::readAdditionalSaveData(tag);
	potionValue = tag.getInt(u"potionValue");
}

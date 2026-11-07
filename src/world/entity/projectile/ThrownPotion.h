#pragma once

#include "world/entity/projectile/EntitySnowball.h"

// 1.0 PotionEntity: a splash potion, applies its effects to mobs within 4 blocks
class ThrownPotion : public EntitySnowball
{
	int_t potionValue = 0;

public:
	explicit ThrownPotion(Level &level) : EntitySnowball(level) {}
	ThrownPotion(Level &level, Mob &owner, int_t potionValue);
	ThrownPotion(Level &level, double x, double y, double z, int_t potionValue);

	jstring getEncodeId() const override { return u"ThrownPotion"; }
	int_t getPotionValue() const { return potionValue; }

protected:
	float getGravity() const override { return 0.05f; }
	void onHit(HitResult &hit) override;
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};

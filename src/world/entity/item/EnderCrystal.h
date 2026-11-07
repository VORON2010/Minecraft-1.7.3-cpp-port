#pragma once

#include "world/entity/Entity.h"

// 1.0 EnderCrystalEntity: sits on the obsidian pillars, heals the dragon, blows up when hit
class EnderCrystal : public Entity
{
public:
	int_t time = 0;
	int_t life = 5;

	EnderCrystal(Level &level);
	EnderCrystal(Level &level, double x, double y, double z);
	jstring getEncodeId() const override { return u"EnderCrystal"; }

	void tick() override;
	bool isPickable() override;
	bool hurt(Entity *source, int_t dmg) override;
	float getShadowHeightOffs() override { return 0.0f; }

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};

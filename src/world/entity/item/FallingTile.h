#pragma once

#include "world/entity/Entity.h"

class Level;
class CompoundTag;

class FallingTile : public Entity
{
public:
	jstring getEncodeId() const override { return u"FallingSand"; }

	int_t tile = 0;
	int_t time = 0;

public:
	FallingTile(Level &level);
	FallingTile(Level &level, double x, double y, double z, int_t tile);

protected:
	virtual void defineSynchedData();

public:
	bool isPickable() override;
	void tick() override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;

public:
	float getShadowHeightOffs() override;
	Level &getLevel() { return level; }
};

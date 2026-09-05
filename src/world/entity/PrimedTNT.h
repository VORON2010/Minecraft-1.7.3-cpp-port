#pragma once

#include "world/entity/Entity.h"

class PrimedTNT : public Entity
{
public:
	int_t fuse = 80;

	PrimedTNT(Level &level);
	PrimedTNT(Level &level, double x, double y, double z);

	void tick() override;
	jstring getEncodeId() const override { return u"PrimedTnt"; }

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};

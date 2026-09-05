#pragma once

#include "world/level/tile/entity/TileEntity.h"

#include "java/Type.h"
#include "java/String.h"

class MobSpawnerTileEntity : public TileEntity
{
public:
	int_t spawnDelay = -1;
	jstring entityId = u"Pig";
	double spin = 0.0;
	double oSpin = 0.0;

	MobSpawnerTileEntity();

	jstring getEncodeId() const override { return u"MobSpawner"; }

	jstring getEntityId() const { return entityId; }
	void setEntityId(const jstring &id) { entityId = id; }

	bool isNearPlayer() const;

	void tick() override;
	void load(CompoundTag &tag) override;
	void save(CompoundTag &tag) override;

private:
	void resetDelay();
};

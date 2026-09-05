#pragma once

#include "world/level/tile/entity/TileEntity.h"
#include "java/Type.h"

class Entity;
class AABB;

class PistonTileEntity : public TileEntity
{
public:
	jstring getEncodeId() const override { return u"Piston"; }

	PistonTileEntity() = default;
	PistonTileEntity(int_t blockId, int_t blockData, int_t facing, bool extending, bool renderHead);

	int_t getStoredBlockID() const { return storedBlockID; }
	int_t getBlockMetadata() const { return storedData; }
	bool isExtending() const { return extending; }
	int_t getDirection() const { return direction; }
	bool shouldRenderHead() const { return renderHead; }

	float getProgress(float partialTick) const;
	float getOffsetX(float partialTick) const;
	float getOffsetY(float partialTick) const;
	float getOffsetZ(float partialTick) const;

	void finishMovement();

	void tick() override;
	void load(CompoundTag &tag) override;
	void save(CompoundTag &tag) override;

private:
	void pushEntities(float progress, float delta);

	int_t storedBlockID = 0;
	int_t storedData = 0;
	int_t direction = 0;
	bool extending = false;
	bool renderHead = false;
	float progress = 0.0f;
	float lastProgress = 0.0f;
};

#include "world/entity/item/FallingTile.h"

#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/level/Level.h"

FallingTile::FallingTile(Level &level) : Entity(level)
{
}

FallingTile::FallingTile(Level &level, double x, double y, double z, int_t tile) : FallingTile(level)
{
	this->tile = tile;
	blocksBuilding = true;
	setSize(0.98f, 0.98f);
	heightOffset = bbHeight / 2.0f;
	setPos(x, y, z);
	xd = 0.0;
	yd = 0.0;
	zd = 0.0;
	makeStepSound = false;
	xo = x;
	yo = y;
	zo = z;
	xOld = x;
	yOld = y;
	zOld = z;
}

void FallingTile::defineSynchedData()
{
}

bool FallingTile::isPickable()
{
	return !removed;
}

void FallingTile::tick()
{
	if (tile == 0)
	{
		remove();
		return;
	}

	int_t xTile = Mth::floor(x);
	int_t yTile = Mth::floor(y);
	int_t zTile = Mth::floor(z);
	if (level.getTile(xTile, yTile, zTile) == tile)
		level.setTile(xTile, yTile, zTile, 0);

	xo = x;
	yo = y;
	zo = z;
	time++;
	yd -= 0.04f;
	move(xd, yd, zd);
	xd *= 0.98f;
	yd *= 0.98f;
	zd *= 0.98f;

	xTile = Mth::floor(x);
	yTile = Mth::floor(y);
	zTile = Mth::floor(z);

	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
		yd *= -0.5;
		remove();
		if (!cancelDrop && level.isEmptyTile(xTile, yTile, zTile))
		{
			level.setTileAndData(xTile, yTile, zTile, tile, data);
			if (tile == 145)
				level.levelEvent(1022, xTile, yTile, zTile, 0);
		}
	}
	else if (time > 100 && !level.isOnline)
	{
		remove();
	}
}

void FallingTile::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putByte(u"Tile", (byte_t)tile);
	tag.putByte(u"Data", (byte_t)data);
	tag.putBoolean(u"HurtEntities", hurtEntities);
}

void FallingTile::readAdditionalSaveData(CompoundTag &tag)
{
	tile = tag.getByte(u"Tile") & 255;
	data = tag.getByte(u"Data") & 255;
	hurtEntities = tag.contains(u"HurtEntities") ? tag.getBoolean(u"HurtEntities") : tile == 145;
}

float FallingTile::getShadowHeightOffs()
{
	return 0.0f;
}

void FallingTile::causeFallDamage(float distance)
{
	// 1.4.7: a falling anvil hurts what it lands on and can wear out
	if (!hurtEntities)
		return;
	int_t dmg = Mth::ceil(distance - 1.0f);
	if (dmg <= 0)
		return;
	std::vector<std::shared_ptr<Entity>> hit = level.getEntities(this, bb);
	int_t amount = Mth::floor(dmg * 2.0f);
	if (amount > 40)
		amount = 40;
	for (auto &e : hit)
		e->hurt(nullptr, amount);
	if (tile == 145 && random.nextFloat() < 0.05f + dmg * 0.05)
	{
		int_t wear = (data >> 2) + 1;
		if (wear > 2)
			cancelDrop = true;
		else
			data = (data & 3) | wear << 2;
	}
}

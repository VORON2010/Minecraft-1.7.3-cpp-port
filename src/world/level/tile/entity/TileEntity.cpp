#include "world/level/tile/entity/TileEntity.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/FurnaceTileEntity.h"
#include "world/level/tile/entity/NoteTileEntity.h"
#include "world/level/tile/entity/RecordPlayerTileEntity.h"
#include "world/level/tile/entity/DispenserTileEntity.h"
#include "world/level/tile/entity/ChestTileEntity.h"
#include "world/level/tile/entity/SignTileEntity.h"
#include "world/level/tile/entity/PistonTileEntity.h"
#include "world/level/tile/entity/MobSpawnerTileEntity.h"

void TileEntity::load(CompoundTag &tag)
{
	x = tag.getInt(u"x");
	y = tag.getInt(u"y");
	z = tag.getInt(u"z");
}

void TileEntity::save(CompoundTag &tag)
{
	jstring id = getEncodeId();
	if (id.empty())
		throw std::runtime_error("TileEntity is missing a mapping! This is a bug!");

	tag.putString(u"id", id);
	tag.putInt(u"x", x);
	tag.putInt(u"y", y);
	tag.putInt(u"z", z);
}

void TileEntity::tick()
{

}

TileEntity *TileEntity::loadStatic(CompoundTag &tag)
{
	jstring id = tag.getString(u"id");
	TileEntity *tileEntity = nullptr;
	if (id == u"Furnace")
		tileEntity = new FurnaceTileEntity();
	if (id == u"Music")
		tileEntity = new NoteTileEntity();
	if (id == u"RecordPlayer")
		tileEntity = new RecordPlayerTileEntity();
	if (id == u"Trap")
		tileEntity = new DispenserTileEntity();
	if (id == u"Sign")
		tileEntity = new SignTileEntity();
	if (id == u"Chest")
		tileEntity = new ChestTileEntity();
	if (id == u"Piston")
		tileEntity = new PistonTileEntity();
	if (id == u"MobSpawner")
		tileEntity = new MobSpawnerTileEntity();
	if (tileEntity != nullptr)
		tileEntity->load(tag);
	return tileEntity;
}

int_t TileEntity::getData()
{
	return level->getData(x, y, z);
}

void TileEntity::setData(int_t data)
{
	level->setData(x, y, z, data);
}

void TileEntity::setChanged()
{
	if (level != nullptr)
		level->tileEntityChanged(x, y, z, shared_from_this());
}

double TileEntity::distanceToSqr(double x, double y, double z)
{
	double dx = this->x + 0.5 - x;
	double dy = this->y + 0.5 - y;
	double dz = this->z + 0.5 - z;
	return dx * dx + dy * dy + dz * dz;
}

Tile &TileEntity::getTile()
{
	return *Tile::tiles[level->getTile(x, y, z)];
}

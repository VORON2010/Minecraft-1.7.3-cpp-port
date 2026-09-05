#include "world/level/tile/NoteTile.h"

#include "util/Memory.h"
#include "world/level/Level.h"
#include "world/level/tile/entity/NoteTileEntity.h"

NoteTile::NoteTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
	Tile::isEntityTile[id] = true;
}

void NoteTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	level.setTileEntity(x, y, z, Util::make_shared<NoteTileEntity>());
}

void NoteTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	level.removeTileEntity(x, y, z);
}

bool NoteTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	if (level.isOnline)
		return true;
	auto noteEntity = std::dynamic_pointer_cast<NoteTileEntity>(level.getTileEntity(x, y, z));
	if (noteEntity == nullptr)
		return false;
	noteEntity->changePitch();
	noteEntity->triggerNote(level, x, y, z);
	return true;
}

void NoteTile::attack(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	auto noteEntity = std::dynamic_pointer_cast<NoteTileEntity>(level.getTileEntity(x, y, z));
	if (noteEntity != nullptr)
		noteEntity->triggerNote(level, x, y, z);
}

void NoteTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	if (tile > 0 && Tile::tiles[tile] != nullptr && Tile::tiles[tile]->isSignalSource())
	{
		bool powered = level.hasDirectSignal(x, y, z);
		auto noteEntity = std::dynamic_pointer_cast<NoteTileEntity>(level.getTileEntity(x, y, z));
		if (noteEntity != nullptr && noteEntity->previousRedstoneState != powered)
		{
			if (powered)
				noteEntity->triggerNote(level, x, y, z);
			noteEntity->previousRedstoneState = powered;
		}
	}
}
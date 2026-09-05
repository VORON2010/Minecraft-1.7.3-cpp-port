#include "world/level/tile/JukeboxTile.h"

#include "util/Memory.h"
#include "world/entity/item/EntityItem.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/entity/RecordPlayerTileEntity.h"

JukeboxTile::JukeboxTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
	Tile::isEntityTile[id] = true;
}

int_t JukeboxTile::getTexture(Facing face, int_t data)
{
	(void)data;
	return tex + (face == Facing::UP ? 1 : 0);
}

void JukeboxTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	level.setTileEntity(x, y, z, Util::make_shared<RecordPlayerTileEntity>());
}

void JukeboxTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	ejectRecord(level, x, y, z);
	level.removeTileEntity(x, y, z);
}

bool JukeboxTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	if (level.getData(x, y, z) == 0)
		return false;
	ejectRecord(level, x, y, z);
	return true;
}

void JukeboxTile::insertRecord(Level &level, int_t x, int_t y, int_t z, int_t recordId)
{
	if (level.isOnline)
		return;
	auto jukebox = std::dynamic_pointer_cast<RecordPlayerTileEntity>(level.getTileEntity(x, y, z));
	if (jukebox == nullptr)
		return;
	jukebox->record = recordId;
	jukebox->setChanged();
	level.setData(x, y, z, 1);
}

void JukeboxTile::ejectRecord(Level &level, int_t x, int_t y, int_t z)
{
	if (level.isOnline)
		return;
	auto jukebox = std::dynamic_pointer_cast<RecordPlayerTileEntity>(level.getTileEntity(x, y, z));
	if (jukebox == nullptr || jukebox->record == 0)
		return;
	level.playRecord(jstring(), x, y, z);
	int_t recordId = jukebox->record;
	jukebox->record = 0;
	jukebox->setChanged();
	level.setData(x, y, z, 0);
	float spread = 0.7f;
	double xo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
	double yo = level.random.nextFloat() * spread + (1.0f - spread) * 0.2f + 0.6f;
	double zo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
	ItemInstance stack(recordId, 1, 0);
	auto entity = std::make_shared<EntityItem>(level, x + xo, y + yo, z + zo, stack);
	entity->throwTime = 10;
	level.addEntity(entity);
}
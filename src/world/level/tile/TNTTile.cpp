#include "world/level/tile/TNTTile.h"

#include "world/level/Level.h"
#include "world/entity/PrimedTNT.h"
#include "world/entity/item/EntityItem.h"
#include "world/item/Items.h"
#include "world/item/ItemInstance.h"
#include "world/item/ItemFlintAndSteel.h"
#include "java/Random.h"

TNTTile::TNTTile(int_t id, int_t tex) : Tile(id, tex, Material::tnt)
{
	updateCachedProperties();
}

int_t TNTTile::getTexture(Facing face, int_t data)
{
	(void)data;
	if (face == Facing::DOWN)
		return tex + 2;
	if (face == Facing::UP)
		return tex + 1;
	return tex;
}

void TNTTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	Tile::onPlace(level, x, y, z);
	if (level.hasNeighborSignal(x, y, z))
	{
		playerDestroy(level, x, y, z, 1);
		level.setTile(x, y, z, 0);
	}
}

void TNTTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	if (tile > 0 && Tile::tiles[tile]->isSignalSource() && level.hasNeighborSignal(x, y, z))
	{
		playerDestroy(level, x, y, z, 1);
		level.setTile(x, y, z, 0);
	}
}

void TNTTile::attack(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	ItemInstance *selected = player.getSelectedItem();
	if (selected != nullptr && selected->itemID == Items::flintAndSteel->getShiftedIndex())
	{
		level.setData(x, y, z, 1);
	}
	Tile::attack(level, x, y, z, player);
}

int_t TNTTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

void TNTTile::playerDestroy(Level &level, int_t x, int_t y, int_t z, int_t data)
{
	if (level.isOnline)
		return;

	if ((data & 1) == 0)
	{
		float spread = 0.7f;
		double xo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
		double yo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
		double zo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
		auto item = std::make_shared<EntityItem>(level, x + xo, y + yo, z + zo, ItemInstance(id, 1, 0));
		item->throwTime = 10;
		level.addEntity(item);
	}
	else
	{
		auto entity = std::make_shared<PrimedTNT>(level, x + 0.5, y + 0.5, z + 0.5);
		level.addEntity(entity);
		level.playSoundEffect(x + 0.5, y + 0.5, z + 0.5, u"random.fuse", 1.0f, 1.0f);
	}
}

void TNTTile::onBlockDestroyedByExplosion(Level &level, int_t x, int_t y, int_t z)
{
	auto entity = std::make_shared<PrimedTNT>(level, x + 0.5, y + 0.5, z + 0.5);
	entity->fuse = level.random.nextInt(entity->fuse / 4) + entity->fuse / 8;
	level.addEntity(entity);
}

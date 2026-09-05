#include "world/item/ItemHoe.h"

#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/Tile.h"

namespace
{
	constexpr int_t FARMLAND_TILE_ID = 60;
}

ItemHoe::ItemHoe(int_t baseId, ToolMaterialType material) : Item(baseId)
{
	setMaxStackSize(1);
	setMaxDamage(getToolMaterialData(material).maxUses);
}

bool ItemHoe::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	int_t tile = level.getTile(x, y, z);
	int_t aboveTile = level.getTile(x, y + 1, z);
	if ((face == Facing::DOWN || aboveTile != 0 || tile != Tile::grass.id) && tile != Tile::dirt.id)
		return false;

	Tile *farmlandTile = Tile::tiles[FARMLAND_TILE_ID];
	if (farmlandTile == nullptr || farmlandTile->soundType == nullptr)
		return false;

	StepSound *sound = farmlandTile->soundType;
	level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5,
		sound->getStepResourcePath(), (sound->getVolume() + 1.0f) / 2.0f, sound->getPitch() * 0.8f);
	if (level.isOnline)
		return true;

	if (!level.setTile(x, y, z, FARMLAND_TILE_ID))
		return false;
	stack.damageItem(1, player);
	return true;
}

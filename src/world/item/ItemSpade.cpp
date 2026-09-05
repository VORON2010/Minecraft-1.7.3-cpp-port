#include "world/item/ItemSpade.h"

#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/GravelTile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/SnowBlockTile.h"
#include "world/level/tile/Tile.h"

namespace
{
	constexpr int_t FARMLAND_TILE_ID = 60;

	struct EffectiveTiles
	{
		const int_t *tiles;
		std::size_t count;
	};

	EffectiveTiles getEffectiveTiles()
	{
		static int_t EFFECTIVE_TILES[] = {
			Tile::grass.id,
			Tile::dirt.id,
			Tile::sand.id,
			Tile::gravel.id,
			Tile::snow.id,
			Tile::snowBlock.id,
			Tile::clay.id,
			FARMLAND_TILE_ID,
		};
		return EffectiveTiles{EFFECTIVE_TILES, sizeof(EFFECTIVE_TILES) / sizeof(EFFECTIVE_TILES[0])};
	}
}

ItemSpade::ItemSpade(int_t baseId, ToolMaterialType material)
	: ItemTool(baseId, 1, material, getEffectiveTiles().tiles, getEffectiveTiles().count)
{
}

bool ItemSpade::canDestroySpecial(const ItemInstance &stack, Tile &tile) const
{
	(void)stack;
	return tile.id == Tile::snow.id || tile.id == Tile::snowBlock.id;
}

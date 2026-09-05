#include "world/item/ItemAxe.h"

#include "world/level/tile/WoodTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/BookshelfTile.h"
#include "world/level/tile/Tile.h"

namespace
{
	struct EffectiveTiles
	{
		const int_t *tiles;
		std::size_t count;
	};

	EffectiveTiles getEffectiveTiles()
	{
		static int_t EFFECTIVE_TILES[] = {
			Tile::wood.id,
			Tile::treeTrunk.id,
			Tile::bookshelf.id,
		};
		return EffectiveTiles{EFFECTIVE_TILES, sizeof(EFFECTIVE_TILES) / sizeof(EFFECTIVE_TILES[0])};
	}
}

ItemAxe::ItemAxe(int_t baseId, ToolMaterialType material)
	: ItemTool(baseId, 3, material, getEffectiveTiles().tiles, getEffectiveTiles().count)
{
}

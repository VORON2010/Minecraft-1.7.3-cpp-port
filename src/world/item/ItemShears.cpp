#include "world/item/ItemShears.h"
#include "world/item/ItemInstance.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/WebTile.h"
#include "world/level/tile/LeafTile.h"
#include "world/level/tile/ClothTile.h"

ItemShears::ItemShears(int_t baseId) : Item(baseId)
{
    setMaxStackSize(1);
    setMaxDamage(238);
}

float ItemShears::getDestroySpeed(const ItemInstance &stack, Tile &tile) const
{
    (void)stack;
    if (tile.id == Tile::cobweb.id || tile.id == Tile::leaves.id)
        return 15.0f;
    if (tile.id == Tile::wool.id)
        return 5.0f;
    return Item::getDestroySpeed(stack, tile);
}

bool ItemShears::canDestroySpecial(const ItemInstance &stack, Tile &tile) const
{
    (void)stack;
    return tile.id == Tile::cobweb.id;
}

bool ItemShears::mineBlock(ItemInstance &stack, int_t tileId, int_t x, int_t y, int_t z, Entity &miner) const
{
    (void)x; (void)y; (void)z; (void)miner;
    if (tileId == Tile::cobweb.id || tileId == Tile::leaves.id)
        stack.damageItem(1, miner);
    return true;
}

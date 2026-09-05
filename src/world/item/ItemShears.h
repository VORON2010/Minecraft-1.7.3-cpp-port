#pragma once
#include "world/item/Item.h"
class ItemShears : public Item
{
public:
    ItemShears(int_t baseId);
    float getDestroySpeed(const ItemInstance &stack, Tile &tile) const override;
    bool canDestroySpecial(const ItemInstance &stack, Tile &tile) const override;
    bool mineBlock(ItemInstance &stack, int_t tile, int_t x, int_t y, int_t z, Entity &miner) const override;
};

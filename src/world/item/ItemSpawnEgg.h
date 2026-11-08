#pragma once

#include "world/item/Item.h"

// 1.5.2 SpawnEggItem (383): the entity id is the item data, the egg is drawn in two tinted layers
class ItemSpawnEgg : public Item
{
public:
	explicit ItemSpawnEgg(int_t baseId);

	jstring getHoverName(const ItemInstance &stack) const override;
	bool hasMultipleLayers() const override { return true; }
	int_t getLayerIcon(const ItemInstance &stack, int_t layer) const override;
	int_t getLayerColor(const ItemInstance &stack, int_t layer) const override;
	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;

	static std::shared_ptr<Entity> spawnEntity(Level &level, int_t id, double x, double y, double z);
};

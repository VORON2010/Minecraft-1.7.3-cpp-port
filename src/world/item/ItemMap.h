#pragma once

#include "world/item/Item.h"

class Level;
class Entity;
class MapData;
class ItemInstance;
class Player;

class ItemMap : public Item
{
public:
	ItemMap(int_t baseId);

	static MapData *getMapData(short_t mapId, Level &level);
	MapData *getMapData(ItemInstance &item, Level &level);
	void updateMapData(Level &level, Entity &entity, MapData &data);

	void onUpdate(ItemInstance &stack, Level &level, Entity &entity, int_t slot, bool isHeld) override;
	void onCreated(ItemInstance &stack, Level &level, Player &player) override;
};

#include "world/item/crafting/FurnaceRecipes.h"

#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/tile/CactusTile.h"
#include "world/level/tile/OreTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/TreeTile.h"

#include "world/level/tile/Tile.h"


FurnaceRecipes &FurnaceRecipes::getInstance()
{
	static FurnaceRecipes instance;
	return instance;
}

FurnaceRecipes::FurnaceRecipes()
{
	recipes.emplace(Tile::ironOre.id, ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0));
	recipes.emplace(Tile::goldOre.id, ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0));
	recipes.emplace(Tile::diamondOre.id, ItemInstance(Items::diamond->getShiftedIndex(), 1, 0));
	recipes.emplace(Tile::sand.id, ItemInstance(20, 1, 0)); 
	recipes.emplace(Tile::cobblestone.id, ItemInstance(Tile::rock.id, 1, 0)); 
	recipes.emplace(Items::clayItem->getShiftedIndex(), ItemInstance(Items::brick->getShiftedIndex(), 1, 0)); 
	
	recipes.emplace(Tile::cactus.id, ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 2));
	
	recipes.emplace(Tile::treeTrunk.id, ItemInstance(Items::coal->getShiftedIndex(), 1, 1));
	
	recipes.emplace(Items::porkchopRaw->getShiftedIndex(), ItemInstance(Items::porkchopCooked->getShiftedIndex(), 1, 0));
	
	recipes.emplace(Items::fishRaw->getShiftedIndex(), ItemInstance(Items::fishCooked->getShiftedIndex(), 1, 0));
}

ItemInstance FurnaceRecipes::getResult(const ItemInstance &input) const
{
	auto it = recipes.find(input.itemID);
	if (it == recipes.end())
		return ItemInstance();
	return it->second;
}

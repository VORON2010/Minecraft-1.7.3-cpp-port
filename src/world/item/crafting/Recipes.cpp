#include "world/item/crafting/Recipes.h"

#include <algorithm>
#include <unordered_map>

#include "world/item/crafting/CraftingContainer.h"
#include "world/item/Item.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/Items.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/WoodTile.h"
#include "world/level/tile/WorkbenchTile.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/SlabTile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/TNTTile.h"
#include "world/level/tile/SandTile.h"
#include "world/item/ItemSign.h"
#include "world/level/tile/RailTile.h"
#include "world/level/tile/DetectorRailTile.h"
#include "world/level/tile/ChestTile.h"
#include "world/level/tile/ButtonTile.h"
#include "world/level/tile/LeverTile.h"
#include "world/level/tile/PressurePlateTile.h"
#include "world/level/tile/NotGateTile.h"
#include "world/level/tile/PistonBaseTile.h"
#include "world/level/tile/MushroomTile.h"
#include "world/level/tile/ClothTile.h"
#include "world/item/ItemArmor.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/FenceTile.h"
#include "world/level/tile/JukeboxTile.h"
#include "world/level/tile/NoteTile.h"
#include "world/level/tile/BookshelfTile.h"
#include "world/level/tile/SnowBlockTile.h"
#include "world/level/tile/GlowStoneTile.h"
#include "world/level/tile/LadderTile.h"
#include "world/level/tile/TrapDoorTile.h"
#include "world/level/tile/StairTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/level/tile/TorchTile.h"
#include "world/level/tile/DispenserTile.h"
#include "world/level/tile/FlowerTile.h"

namespace
{
	bool matchesAt(const Recipes::Recipe &recipe, const CraftingContainer &container, int_t offsetX, int_t offsetY, bool mirror)
	{
		for (int_t x = 0; x < 3; ++x)
		{
			for (int_t y = 0; y < 3; ++y)
			{
				int_t recipeX = x - offsetX;
				int_t recipeY = y - offsetY;
				ItemInstance recipeItem;
				if (recipeX >= 0 && recipeY >= 0 && recipeX < recipe.width && recipeY < recipe.height)
				{
					int_t index = mirror ? (recipe.width - recipeX - 1) + recipeY * recipe.width : recipeX + recipeY * recipe.width;
					recipeItem = recipe.items[index];
				}

				ItemInstance containerItem = container.getItem(x, y);
				if (recipeItem.isEmpty() && containerItem.isEmpty())
					continue;
				if (recipeItem.isEmpty() != containerItem.isEmpty())
					return false;
				if (recipeItem.itemID != containerItem.itemID)
					return false;
				if (recipeItem.itemDamage != -1 && recipeItem.itemDamage != containerItem.itemDamage)
					return false;
			}
		}
		return true;
	}

	bool matchesShaped(const Recipes::Recipe &recipe, const CraftingContainer &container)
	{
		for (int_t offsetX = 0; offsetX <= 3 - recipe.width; ++offsetX)
		{
			for (int_t offsetY = 0; offsetY <= 3 - recipe.height; ++offsetY)
			{
				if (matchesAt(recipe, container, offsetX, offsetY, true))
					return true;
				if (matchesAt(recipe, container, offsetX, offsetY, false))
					return true;
			}
		}
		return false;
	}

	bool matchesShapeless(const Recipes::Recipe &recipe, const CraftingContainer &container)
	{
		std::vector<ItemInstance> remaining = recipe.items;
		for (int_t y = 0; y < 3; ++y)
		{
			for (int_t x = 0; x < 3; ++x)
			{
				ItemInstance containerItem = container.getItem(x, y);
				if (containerItem.isEmpty())
					continue;

				bool found = false;
				for (auto it = remaining.begin(); it != remaining.end(); ++it)
				{
					if (containerItem.itemID == it->itemID && (it->itemDamage == -1 || containerItem.itemDamage == it->itemDamage))
					{
						found = true;
						remaining.erase(it);
						break;
					}
				}

				if (!found)
					return false;
			}
		}
		return remaining.empty();
	}
}

Recipes &Recipes::getInstance()
{
	static Recipes instance;
	return instance;
}

Recipes::Recipes()
{
	ItemInstance stick(Items::stick->getShiftedIndex(), 1, 0);

	
	{
		const std::vector<std::string> patterns[4] = {
			{"XXX", " # ", " # "},
			{"X", "#", "#"},
			{"XX", "X#", " #"},
			{"XX", " #", " #"}
		};
		const ItemInstance materials[5] = {
			ItemInstance(Tile::wood.id, 1, -1),
			ItemInstance(Tile::cobblestone.id, 1, -1),
			ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0),
			ItemInstance(Items::diamond->getShiftedIndex(), 1, 0),
			ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0)
		};
		Item *tools[4][5] = {
			{Items::pickaxeWood, Items::pickaxeStone, Items::pickaxeIron, Items::pickaxeDiamond, Items::pickaxeGold},
			{Items::shovelWood, Items::shovelStone, Items::shovelIron, Items::shovelDiamond, Items::shovelGold},
			{Items::axeWood, Items::axeStone, Items::axeIron, Items::axeDiamond, Items::axeGold},
			{Items::hoeWood, Items::hoeStone, Items::hoeIron, Items::hoeDiamond, Items::hoeGold}
		};
		for (int_t m = 0; m < 5; ++m)
		{
			for (int_t t = 0; t < 4; ++t)
			{
				addShapedRecipe(ItemInstance(tools[t][m]->getShiftedIndex(), 1, 0), patterns[t], {
					{'#', stick},
					{'X', materials[m]}
				});
			}
		}
		addShapedRecipe(ItemInstance(Items::shears->getShiftedIndex(), 1, 0), {" #", "# "}, {
			{'#', ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0)}
		});
	}

	
	{
		const ItemInstance materials[5] = {
			ItemInstance(Tile::wood.id, 1, -1),
			ItemInstance(Tile::cobblestone.id, 1, -1),
			ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0),
			ItemInstance(Items::diamond->getShiftedIndex(), 1, 0),
			ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0)
		};
		Item *swords[5] = {Items::swordWood, Items::swordStone, Items::swordIron, Items::swordDiamond, Items::swordGold};
		for (int_t m = 0; m < 5; ++m)
		{
			addShapedRecipe(ItemInstance(swords[m]->getShiftedIndex(), 1, 0), {"X", "X", "#"}, {
				{'#', stick},
				{'X', materials[m]}
			});
		}
		addShapedRecipe(ItemInstance(Items::bow->getShiftedIndex(), 1, 0), {" #X", "# X", " #X"}, {
			{'X', ItemInstance(Items::silk->getShiftedIndex(), 1, 0)},
			{'#', stick}
		});
		addShapedRecipe(ItemInstance(Items::arrow->getShiftedIndex(), 4, 0), {"X", "#", "Y"}, {
			{'Y', ItemInstance(Items::feather->getShiftedIndex(), 1, 0)},
			{'X', ItemInstance(Items::flint->getShiftedIndex(), 1, 0)},
			{'#', stick}
		});
	}

	
	{
		Tile *blocks[4] = {&Tile::goldBlock, &Tile::ironBlock, &Tile::diamondBlock, &Tile::lapisBlock};
		const ItemInstance stacks[4] = {
			ItemInstance(Items::ingotGold->getShiftedIndex(), 9, 0),
			ItemInstance(Items::ingotIron->getShiftedIndex(), 9, 0),
			ItemInstance(Items::diamond->getShiftedIndex(), 9, 0),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 9, 4)
		};
		for (int_t i = 0; i < 4; ++i)
		{
			addShapedRecipe(ItemInstance(blocks[i]->id, 1, 0), {"###", "###", "###"}, {{'#', stacks[i]}});
			addShapedRecipe(stacks[i], {"#"}, {{'#', ItemInstance(blocks[i]->id, 1, -1)}});
		}
	}

	
	addShapedRecipe(ItemInstance(Items::bowlSoup->getShiftedIndex(), 1, 0), {"Y", "X", "#"}, {
		{'X', ItemInstance(Tile::brownMushroom.id, 1, -1)},
		{'Y', ItemInstance(Tile::redMushroom.id, 1, -1)},
		{'#', ItemInstance(Items::bowlEmpty->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::bowlSoup->getShiftedIndex(), 1, 0), {"Y", "X", "#"}, {
		{'X', ItemInstance(Tile::redMushroom.id, 1, -1)},
		{'Y', ItemInstance(Tile::brownMushroom.id, 1, -1)},
		{'#', ItemInstance(Items::bowlEmpty->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::cookie->getShiftedIndex(), 8, 0), {"#X#"}, {
		{'X', ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 3)},
		{'#', ItemInstance(Items::wheat->getShiftedIndex(), 1, 0)}
	});

	
	addShapedRecipe(ItemInstance(Tile::chest.id, 1, 0), {"###", "# #", "###"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::furnace.id, 1, 0), {"###", "# #", "###"}, {
		{'#', ItemInstance(Tile::cobblestone.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::workBench.id, 1, 0), {"##", "##"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::sandstone.id, 1, 0), {"##", "##"}, {
		{'#', ItemInstance(Tile::sand.id, 1, -1)}
	});

	
	{
		const std::vector<std::string> patterns[4] = {
			{"XXX", "X X"},
			{"X X", "XXX", "XXX"},
			{"XXX", "X X", "X X"},
			{"X X", "X X"}
		};
		const ItemInstance materials[5] = {
			ItemInstance(Items::leather->getShiftedIndex(), 1, 0),
			ItemInstance(Tile::fire.id, 1, -1),
			ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0),
			ItemInstance(Items::diamond->getShiftedIndex(), 1, 0),
			ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0)
		};
		Item *pieces[4][5] = {
			{Items::helmetLeather, Items::helmetChain, Items::helmetIron, Items::helmetDiamond, Items::helmetGold},
			{Items::plateLeather, Items::plateChain, Items::plateIron, Items::plateDiamond, Items::plateGold},
			{Items::legsLeather, Items::legsChain, Items::legsIron, Items::legsDiamond, Items::legsGold},
			{Items::bootsLeather, Items::bootsChain, Items::bootsIron, Items::bootsDiamond, Items::bootsGold}
		};
		for (int_t m = 0; m < 5; ++m)
		{
			for (int_t p = 0; p < 4; ++p)
			{
				addShapedRecipe(ItemInstance(pieces[p][m]->getShiftedIndex(), 1, 0), patterns[p], {
					{'X', materials[m]}
				});
			}
		}
	}

	
	{
		for (int_t i = 0; i < 16; ++i)
		{
			
			addShapelessRecipe(ItemInstance(Tile::wool.id, 1, ~i & 15), {
				ItemInstance(Items::dyePowder->getShiftedIndex(), 1, i),
				ItemInstance(Tile::wool.id, 1, 0)
			});
		}
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 11), {ItemInstance(Tile::flower.id, 1, 0)});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 1), {ItemInstance(Tile::rose.id, 1, 0)});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 3, 15), {ItemInstance(Items::bone->getShiftedIndex(), 1, 0)});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 9), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 1),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 15)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 14), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 1),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 11)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 10), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 2),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 15)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 8), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 0),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 15)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 7), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 8),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 15)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 3, 7), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 0),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 15),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 15)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 12), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 4),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 15)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 6), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 4),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 2)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 5), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 4),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 1)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 2, 13), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 5),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 9)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 3, 13), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 4),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 1),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 9)
		});
		addShapelessRecipe(ItemInstance(Items::dyePowder->getShiftedIndex(), 4, 13), {
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 4),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 1),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 1),
			ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 15)
		});
	}

	
	addShapedRecipe(ItemInstance(Items::paper->getShiftedIndex(), 3, 0), {"###"}, {
		{'#', ItemInstance(Items::reed->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::book->getShiftedIndex(), 1, 0), {"#", "#", "#"}, {
		{'#', ItemInstance(Items::paper->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::fence.id, 2, 0), {"###", "###"}, {
		{'#', stick}
	});
	addShapedRecipe(ItemInstance(Tile::jukebox.id, 1, 0), {"###", "#X#", "###"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)},
		{'X', ItemInstance(Items::diamond->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::noteBlock.id, 1, 0), {"###", "#X#", "###"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)},
		{'X', ItemInstance(Items::redstone->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::bookshelf.id, 1, 0), {"###", "XXX", "###"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)},
		{'X', ItemInstance(Items::book->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::snowBlock.id, 1, 0), {"##", "##"}, {
		{'#', ItemInstance(Items::snowball->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::clay.id, 1, 0), {"##", "##"}, {
		{'#', ItemInstance(Items::clayItem->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::brick.id, 1, 0), {"##", "##"}, {
		{'#', ItemInstance(Items::brick->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::glowstone.id, 1, 0), {"##", "##"}, {
		{'#', ItemInstance(Items::glowstoneDust->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::wool.id, 1, 0), {"##", "##"}, {
		{'#', ItemInstance(Items::silk->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::tnt.id, 1, 0), {"X#X", "#X#", "X#X"}, {
		{'X', ItemInstance(Items::gunpowder->getShiftedIndex(), 1, 0)},
		{'#', ItemInstance(Tile::sand.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::slabSingle.id, 3, 3), {"###"}, {{'#', ItemInstance(Tile::cobblestone.id, 1, -1)}});
	addShapedRecipe(ItemInstance(Tile::slabSingle.id, 3, 0), {"###"}, {{'#', ItemInstance(Tile::rock.id, 1, -1)}});
	addShapedRecipe(ItemInstance(Tile::slabSingle.id, 3, 1), {"###"}, {{'#', ItemInstance(Tile::sandstone.id, 1, -1)}});
	addShapedRecipe(ItemInstance(Tile::slabSingle.id, 3, 2), {"###"}, {{'#', ItemInstance(Tile::wood.id, 1, -1)}});
	addShapedRecipe(ItemInstance(Tile::ladder.id, 2, 0), {"# #", "###", "# #"}, {
		{'#', stick}
	});
	addShapedRecipe(ItemInstance(Items::doorWood->getShiftedIndex(), 1, 0), {"##", "##", "##"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::trapdoor.id, 2, 0), {"###", "###"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Items::doorIron->getShiftedIndex(), 1, 0), {"##", "##", "##"}, {
		{'#', ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::sign->getShiftedIndex(), 1, 0), {"###", "###", " X "}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)},
		{'X', stick}
	});
	addShapedRecipe(ItemInstance(Items::cake->getShiftedIndex(), 1, 0), {"AAA", "BEB", "CCC"}, {
		{'A', ItemInstance(Items::bucketMilk->getShiftedIndex(), 1, 0)},
		{'B', ItemInstance(Items::sugar->getShiftedIndex(), 1, 0)},
		{'C', ItemInstance(Items::wheat->getShiftedIndex(), 1, 0)},
		{'E', ItemInstance(Items::egg->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::sugar->getShiftedIndex(), 1, 0), {"#"}, {
		{'#', ItemInstance(Items::reed->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::wood.id, 4, 0), {"#"}, {{'#', ItemInstance(Tile::treeTrunk.id, 1, -1)}});
	addShapedRecipe(ItemInstance(Items::stick->getShiftedIndex(), 4, 0), {"#", "#"}, {{'#', ItemInstance(Tile::wood.id, 1, -1)}});
	addShapedRecipe(ItemInstance(Tile::torch.id, 4, 0), {"X", "#"}, {
		{'X', ItemInstance(Items::coal->getShiftedIndex(), 1, 0)},
		{'#', stick}
	});
	addShapedRecipe(ItemInstance(Tile::torch.id, 4, 0), {"X", "#"}, {
		{'X', ItemInstance(Items::coal->getShiftedIndex(), 1, 1)},
		{'#', stick}
	});
	addShapedRecipe(ItemInstance(Items::bowlEmpty->getShiftedIndex(), 4, 0), {"# #", " # "}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::rail.id, 16, 0), {"X X", "X#X", "X X"}, {
		{'X', ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0)},
		{'#', stick}
	});
	addShapedRecipe(ItemInstance(Tile::railPowered.id, 6, 0), {"X X", "X#X", "XRX"}, {
		{'X', ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0)},
		{'R', ItemInstance(Items::redstone->getShiftedIndex(), 1, 0)},
		{'#', stick}
	});
	addShapedRecipe(ItemInstance(Tile::railDetector.id, 6, 0), {"X X", "X#X", "XRX"}, {
		{'X', ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0)},
		{'R', ItemInstance(Items::redstone->getShiftedIndex(), 1, 0)},
		{'#', ItemInstance(Tile::pressurePlateStone.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Items::minecart->getShiftedIndex(), 1, 0), {"# #", "###"}, {
		{'#', ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::jackOLantern.id, 1, 0), {"A", "B"}, {
		{'A', ItemInstance(Tile::pumpkin.id, 1, -1)},
		{'B', ItemInstance(Tile::torch.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Items::minecartChest->getShiftedIndex(), 1, 0), {"A", "B"}, {
		{'A', ItemInstance(Tile::chest.id, 1, -1)},
		{'B', ItemInstance(Items::minecart->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::minecartPowered->getShiftedIndex(), 1, 0), {"A", "B"}, {
		{'A', ItemInstance(Tile::furnace.id, 1, -1)},
		{'B', ItemInstance(Items::minecart->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::boat->getShiftedIndex(), 1, 0), {"# #", "###"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Items::bucketEmpty->getShiftedIndex(), 1, 0), {"# #", " # "}, {
		{'#', ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::flintAndSteel->getShiftedIndex(), 1, 0), {"A ", " B"}, {
		{'A', ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0)},
		{'B', ItemInstance(Items::flint->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::bread->getShiftedIndex(), 1, 0), {"###"}, {
		{'#', ItemInstance(Items::wheat->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::stairsWood.id, 4, 0), {"#  ", "## ", "###"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Items::fishingRod->getShiftedIndex(), 1, 0), {"  #", " #X", "# X"}, {
		{'#', stick},
		{'X', ItemInstance(Items::silk->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::stairsStone.id, 4, 0), {"#  ", "## ", "###"}, {
		{'#', ItemInstance(Tile::cobblestone.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Items::painting->getShiftedIndex(), 1, 0), {"###", "#X#", "###"}, {
		{'#', stick},
		{'X', ItemInstance(Tile::wool.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Items::appleGold->getShiftedIndex(), 1, 0), {"###", "#X#", "###"}, {
		{'#', ItemInstance(Tile::goldBlock.id, 1, -1)},
		{'X', ItemInstance(Items::apple->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::lever.id, 1, 0), {"X", "#"}, {
		{'#', ItemInstance(Tile::cobblestone.id, 1, -1)},
		{'X', stick}
	});
	addShapedRecipe(ItemInstance(Tile::torchRedstoneActive.id, 1, 0), {"X", "#"}, {
		{'#', stick},
		{'X', ItemInstance(Items::redstone->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::redstoneRepeater->getShiftedIndex(), 1, 0), {"#X#", "III"}, {
		{'#', ItemInstance(Tile::torchRedstoneActive.id, 1, -1)},
		{'X', ItemInstance(Items::redstone->getShiftedIndex(), 1, 0)},
		{'I', ItemInstance(Tile::rock.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Items::clock->getShiftedIndex(), 1, 0), {" # ", "#X#", " # "}, {
		{'#', ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0)},
		{'X', ItemInstance(Items::redstone->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::compass->getShiftedIndex(), 1, 0), {" # ", "#X#", " # "}, {
		{'#', ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0)},
		{'X', ItemInstance(Items::redstone->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Items::map->getShiftedIndex(), 1, 0), {"###", "#X#", "###"}, {
		{'#', ItemInstance(Items::paper->getShiftedIndex(), 1, 0)},
		{'X', ItemInstance(Items::compass->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::buttonStone.id, 1, 0), {"#", "#"}, {
		{'#', ItemInstance(Tile::rock.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::pressurePlateStone.id, 1, 0), {"##"}, {
		{'#', ItemInstance(Tile::rock.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::pressurePlateWood.id, 1, 0), {"##"}, {
		{'#', ItemInstance(Tile::wood.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::dispenser.id, 1, 0), {"###", "#X#", "#R#"}, {
		{'#', ItemInstance(Tile::cobblestone.id, 1, -1)},
		{'X', ItemInstance(Items::bow->getShiftedIndex(), 1, 0)},
		{'R', ItemInstance(Items::redstone->getShiftedIndex(), 1, 0)}
	});
	addShapedRecipe(ItemInstance(Tile::pistonBase.id, 1, 0), {"TTT", "#X#", "#R#"}, {
		{'#', ItemInstance(Tile::cobblestone.id, 1, -1)},
		{'X', ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0)},
		{'R', ItemInstance(Items::redstone->getShiftedIndex(), 1, 0)},
		{'T', ItemInstance(Tile::wood.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Tile::pistonStickyBase.id, 1, 0), {"S", "P"}, {
		{'S', ItemInstance(Items::slimeball->getShiftedIndex(), 1, 0)},
		{'P', ItemInstance(Tile::pistonBase.id, 1, -1)}
	});
	addShapedRecipe(ItemInstance(Items::bed->getShiftedIndex(), 1, 0), {"###", "XXX"}, {
		{'#', ItemInstance(Tile::wool.id, 1, -1)},
		{'X', ItemInstance(Tile::wood.id, 1, -1)}
	});

	
	std::stable_sort(recipes.begin(), recipes.end(), [](const Recipe &a, const Recipe &b) {
		if (!a.shapeless && b.shapeless)
			return true;
		if (a.shapeless && !b.shapeless)
			return false;
		return a.size() > b.size();
	});
}

ItemInstance Recipes::getItemFor(const CraftingContainer &container) const
{
	for (const Recipe &recipe : recipes)
	{
		if (recipe.shapeless ? matchesShapeless(recipe, container) : matchesShaped(recipe, container))
			return ItemInstance(recipe.result.itemID, recipe.result.stackSize, recipe.result.itemDamage);
	}
	return ItemInstance();
}

void Recipes::addShapedRecipe(const ItemInstance &result, const std::vector<std::string> &pattern,
	const std::vector<std::pair<char, ItemInstance>> &mapping)
{
	if (pattern.empty())
		return;

	int_t width = static_cast<int_t>(pattern[0].size());
	int_t height = static_cast<int_t>(pattern.size());
	std::unordered_map<char, ItemInstance> itemMap;
	for (const auto &entry : mapping)
		itemMap[entry.first] = entry.second;

	Recipe recipe;
	recipe.width = width;
	recipe.height = height;
	recipe.result = result;
	recipe.items.resize(width * height);

	for (int_t y = 0; y < height; ++y)
	{
		for (int_t x = 0; x < width; ++x)
		{
			char key = pattern[y][x];
			auto it = itemMap.find(key);
			if (it != itemMap.end())
				recipe.items[x + y * width] = it->second;
			else
				recipe.items[x + y * width] = ItemInstance();
		}
	}

	recipes.push_back(std::move(recipe));
}

void Recipes::addShapelessRecipe(const ItemInstance &result, const std::vector<ItemInstance> &ingredients)
{
	Recipe recipe;
	recipe.shapeless = true;
	recipe.result = result;
	recipe.items = ingredients;
	recipes.push_back(std::move(recipe));
}

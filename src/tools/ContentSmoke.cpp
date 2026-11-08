#include "tools/ContentSmoke.h"

#include <iostream>
#include <memory>

#include "java/File.h"
#include "world/item/CreativeModeTab.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/AllTiles.h"
#include "world/level/tile/AnvilTile.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/ItemArmor.h"
#include "world/stats/StatList.h"

namespace
{
	bool expect(bool condition, const char *message)
	{
		if (!condition)
			std::cerr << "FAIL: " << message << std::endl;
		return condition;
	}

	bool contains(const std::vector<ItemInstance> &list, int_t id, int_t data)
	{
		for (const ItemInstance &s : list)
			if (s.itemID == id && (data < 0 || s.itemDamage == data))
				return true;
		return false;
	}

	struct TestPlayer : public Player
	{
		using Player::Player;
	};
}

int runContentSmoke()
{
	Tile::initTiles();
	Items::initItems();
	StatList::init();
	bool ok = true;

	std::cerr << "content-smoke: creative tabs" << std::endl;
	{
		using namespace CreativeModeTab;
		std::vector<ItemInstance> building, deco, redstone, transport, misc, food, tools, combat, brewing, materials;
		addItems(BUILDING_BLOCKS, building);
		addItems(DECORATIONS, deco);
		addItems(REDSTONE, redstone);
		addItems(TRANSPORTATION, transport);
		addItems(MISC, misc);
		addItems(FOOD, food);
		addItems(TOOLS, tools);
		addItems(COMBAT, combat);
		addItems(BREWING, brewing);
		addItems(MATERIALS, materials);
		ok &= expect(contains(building, Tile::rock.id, 0) && contains(building, Tile::wool.id, 15) && contains(building, Tile::wood.id, 3) && contains(building, Tile::sandstone.id, 2), "building blocks: stone, jungle planks, smooth sandstone");
		ok &= expect(contains(building, Tile::slabSingle.id, 6) && !contains(building, Tile::slabSingle.id, 2), "nether brick slab is there, the old wooden slab is not");
		ok &= expect(contains(deco, Tile::sapling.id, 3) && contains(deco, Tile::torch.id, 0) && contains(deco, Tile::anvil.id, 2), "decorations: jungle sapling, torch, damaged anvil");
		ok &= expect(contains(redstone, Tile::dispenser.id, 0) && contains(redstone, Items::redstone->getShiftedIndex(), 0), "redstone tab");
		ok &= expect(contains(transport, Tile::rail.id, 0) && contains(transport, Items::boat->getShiftedIndex(), 0), "transportation tab");
		ok &= expect(contains(misc, Items::spawnEgg->getShiftedIndex(), 120) && contains(misc, Items::expBottle->getShiftedIndex(), 0), "misc: villager egg and bottle o' enchanting");
		ok &= expect(contains(food, Items::beefCooked->getShiftedIndex(), 0) && contains(food, Items::appleGold->getShiftedIndex(), 1), "food: steak and the enchanted golden apple");
		ok &= expect(contains(tools, Items::pickaxeDiamond->getShiftedIndex(), 0) && contains(tools, Items::enchantedBook->getShiftedIndex(), -1), "tools with digging books");
		ok &= expect(contains(combat, Items::swordGold->getShiftedIndex(), 0) && contains(combat, Items::plateDiamond->getShiftedIndex(), 0), "combat tab");
		ok &= expect(contains(brewing, Items::potion->getShiftedIndex(), 8193) && contains(brewing, Items::potion->getShiftedIndex(), 16385), "brewing: regeneration and its splash");
		ok &= expect(contains(materials, Items::emerald->getShiftedIndex(), 0) && contains(materials, Items::stick->getShiftedIndex(), 0), "materials: emerald, stick");
		ok &= expect(getTab(Tile::water.id) < 0 && getTab(Items::writtenBook->getShiftedIndex()) < 0, "water and written books are not in the tabs");
		std::vector<ItemInstance> all;
		addAllItems(all);
		ok &= expect(all.size() > building.size() + food.size() + 100, "search goes through every item");
	}

	std::cerr << "content-smoke: block sounds" << std::endl;
	ok &= expect(Tile::glass.soundType->getBreakSound() == u"random.glass" && Tile::glass.soundType->getPlaceSound() == u"step.stone", "glass shatters and is placed like stone");
	ok &= expect(Tile::sand.soundType->getBreakSound() == u"dig.sand" && Tile::sand.soundType->getStepSound() == u"step.sand", "sand digs and steps as sand");
	ok &= expect(Tile::ladder.soundType->getBreakSound() == u"dig.wood" && Tile::ladder.soundType->getStepSound() == u"step.ladder", "ladders");
	ok &= expect(Tile::snow.soundType->getStepSound() == u"step.snow" && Tile::anvil.soundType->getPlaceSound() == u"random.anvil_land", "snow and anvil");
	ok &= expect(Tile::rock.soundType->getBreakSound() == u"dig.stone", "stone breaks with dig.stone");

	std::cerr << "content-smoke: items" << std::endl;
	ok &= expect(Items::fireCharge->getShiftedIndex() == 385, "fire charge is 385");
	ok &= expect(Tile::wood.getTexture(Facing::NORTH, 3) == 199 && Tile::sandstone.getTexture(Facing::NORTH, 1) == 229, "jungle planks and chiseled sandstone textures");

	{
		std::unique_ptr<File> workdir(File::open(u"build/content-smoke-workdir"));
		Level::deleteLevel(*workdir, u"content-smoke-world");
	}
	{
		Level level(File::open(u"build/content-smoke-workdir"), u"content-smoke-world", 3LL);
		level.getChunk(0, 0);
		auto player = std::make_shared<TestPlayer>(level);
		ok &= expect(player->getXpNeededForNextLevel() == 17, "1.5.2: the first levels need 17 xp");
		player->increaseXp(17 * 15);
		ok &= expect(player->experienceLevel == 15 && player->getXpNeededForNextLevel() == 17, "fifteen levels of 17");
		player->experienceLevel = 30;
		ok &= expect(player->getXpNeededForNextLevel() == 62, "level 30 needs 62");
	}
	{
		std::unique_ptr<File> workdir(File::open(u"build/content-smoke-workdir"));
		Level::deleteLevel(*workdir, u"content-smoke-world");
	}

	std::cerr << (ok ? "content-smoke: PASS" : "content-smoke: FAIL") << std::endl;
	return ok ? 0 : 1;
}

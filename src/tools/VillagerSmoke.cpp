#include "tools/VillagerSmoke.h"

#include <iostream>
#include <memory>

#include "java/File.h"
#include "nbt/CompoundTag.h"
#include "world/entity/EntityIO.h"
#include "world/entity/ai/PathNavigation.h"
#include "world/entity/npc/Villager.h"
#include "world/entity/player/Player.h"
#include "world/inventory/MerchantMenu.h"
#include "world/inventory/Slot.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/ItemSpawnEgg.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/tile/AllTiles.h"
#include "world/stats/StatList.h"
#include "world/village/Village.h"

namespace
{
	bool expect(bool condition, const char *message)
	{
		if (!condition)
			std::cerr << "FAIL: " << message << std::endl;
		return condition;
	}

	struct TestPlayer : public Player
	{
		using Player::Player;
	};

	void platform(Level &level, int_t x0, int_t z0, int_t x1, int_t z1, int_t y)
	{
		for (int_t x = x0; x <= x1; ++x)
			for (int_t z = z0; z <= z1; ++z)
			{
				level.setTile(x, y, z, Tile::rock.id);
				for (int_t yy = y + 1; yy < y + 8; ++yy)
					level.setTile(x, yy, z, 0);
			}
	}

	// a door facing x with a roof on its +x side, like a house wall
	void house(Level &level, int_t x, int_t y, int_t z)
	{
		level.setTileAndData(x, y, z, Tile::doorWood.id, 0);
		level.setTileAndData(x, y + 1, z, Tile::doorWood.id, 8);
		for (int_t xx = x + 1; xx <= x + 6; ++xx)
			for (int_t zz = z - 1; zz <= z + 1; ++zz)
				level.setTile(xx, y + 3, zz, Tile::rock.id);
	}
}

int runVillagerSmoke()
{
	Tile::initTiles();
	Items::initItems();
	StatList::init();
	bool ok = true;

	std::cerr << "villager-smoke: 1.5.2 item ids" << std::endl;
	ok &= expect(Items::melon->getShiftedIndex() == 360 && Items::melonSeeds->getShiftedIndex() == 362, "melon 360, melon seeds 362");
	ok &= expect(Items::beefRaw->getShiftedIndex() == 363 && Items::chickenCooked->getShiftedIndex() == 366, "beef 363, cooked chicken 366");
	ok &= expect(Items::spawnEgg->getShiftedIndex() == 383 && Items::expBottle->getShiftedIndex() == 384, "spawn egg 383, bottle o' enchanting 384");
	ok &= expect(Items::writtenBook->getShiftedIndex() == 387 && Items::emerald->getShiftedIndex() == 388, "written book 387, emerald 388");
	ok &= expect(Items::enchantedBook->getShiftedIndex() == 403 && Item::items[2258] != nullptr && Item::items[2267] != nullptr, "enchanted book 403, all twelve discs");
	ok &= expect(Items::diamond->getDescriptionId(ItemInstance(Items::diamond->getShiftedIndex(), 1, 0)) == u"item.diamond", "diamond is no longer called emerald");

	{
		std::unique_ptr<File> workdir(File::open(u"build/villager-smoke-workdir"));
		Level::deleteLevel(*workdir, u"villager-smoke-world");
	}
	{
		Level level(File::open(u"build/villager-smoke-workdir"), u"villager-smoke-world", 11LL);
		for (int_t cx = -1; cx <= 3; ++cx)
			for (int_t cz = -1; cz <= 2; ++cz)
				level.getChunk(cx, cz);
		platform(level, -6, -6, 40, 26, 100);

		auto player = std::make_shared<TestPlayer>(level);
		player->name = u"Tester";
		player->moveTo(2.5, 101.0, 2.5, 0.0f, 0.0f);
		level.addEntity(player);

		std::cerr << "villager-smoke: spawn egg" << std::endl;
		{
			ItemInstance egg(Items::spawnEgg->getShiftedIndex(), 1, 120);
			jstring name = Items::spawnEgg->getHoverName(egg);
			ok &= expect(name.find(u"Villager") != jstring::npos, "the egg is called after the villager");
			ok &= expect(Items::spawnEgg->hasMultipleLayers() && Items::spawnEgg->getLayerColor(egg, 0) == 5651507, "the villager egg has its 1.5.2 colours");
			std::shared_ptr<Entity> spawned = ItemSpawnEgg::spawnEntity(level, 120, 30.5, 101.0, 20.5);
			ok &= expect(dynamic_cast<Villager *>(spawned.get()) != nullptr, "a villager egg spawns a villager");
			if (spawned != nullptr)
				spawned->remove();
		}

		std::cerr << "villager-smoke: professions and offers" << std::endl;
		for (int_t profession = 0; profession < 5; ++profession)
		{
			Villager villager(level, profession);
			MerchantRecipeList *offers = villager.getOffers(*player);
			bool hasEmerald = false;
			for (auto &r : offers->recipes)
				hasEmerald |= r->getBuyA().itemID == Items::emerald->getShiftedIndex() || r->getSellItem().itemID == Items::emerald->getShiftedIndex()
					|| (r->getBuyB() != nullptr && r->getBuyB()->itemID == Items::emerald->getShiftedIndex());
			ok &= expect(offers->size() == 1 && hasEmerald, "every profession starts with one emerald offer");
		}

		std::cerr << "villager-smoke: trading" << std::endl;
		{
			auto villager = std::make_shared<Villager>(level, 3);
			villager->moveTo(3.5, 101.0, 2.5, 0.0f, 0.0f);
			level.addEntity(villager);
			villager->setTradingPlayer(player.get());
			MerchantMenu menu(player->inventory, *villager, level, *player);
			MerchantRecipe &recipe = (*villager->getOffers(*player))[0];
			ItemInstance pay = recipe.getBuyA();
			pay.stackSize = pay.stackSize + 3;
			menu.tradeContainer.setInventorySlotContents(0, pay);
			if (recipe.getBuyB() != nullptr)
				menu.tradeContainer.setInventorySlotContents(1, *recipe.getBuyB());
			const ItemInstance *out = menu.tradeContainer.getStackInSlot(2);
			ok &= expect(out != nullptr && out->itemID == recipe.getSellItem().itemID, "paying shows the offer's result");
			Slot &resultSlot = *menu.slots[2];
			ItemInstance taken = resultSlot.remove(64);
			resultSlot.onPickup(taken);
			const ItemInstance *left = menu.tradeContainer.getStackInSlot(0);
			ok &= expect(taken.itemID == recipe.getSellItem().itemID, "the result can be taken");
			ok &= expect(left != nullptr && left->stackSize == 3, "the price is taken from the payment slot");
			ok &= expect(recipe.getUses() == 1, "the offer counts the use");

			for (int_t i = 0; i < 6; ++i)
				recipe.increaseUses();
			ok &= expect(recipe.isDeprecated(), "after seven uses the offer is locked");
			menu.tradeContainer.setInventorySlotContents(0, pay);
			ok &= expect(menu.tradeContainer.getStackInSlot(2) == nullptr, "a locked offer gives nothing");

			menu.onClosed(*player);
			ok &= expect(villager->getTradingPlayer() == nullptr, "closing the menu ends the trade");

			// the new offer comes 40 ticks after trading the last one
			for (int_t i = 0; i < 60; ++i)
				villager->tick();
			ok &= expect(villager->getOffers(*player)->size() == 2, "a villager unlocks a new offer after its last one is used");

			CompoundTag tag;
			villager->save(tag);
			auto copy = std::make_shared<Villager>(level);
			copy->load(tag);
			ok &= expect(copy->getProfession() == 3 && copy->getOffers(*player)->size() == 2, "profession and offers survive saving");
			ok &= expect((*copy->getOffers(*player))[0].getUses() == recipe.getUses(), "offer uses survive saving");
			villager->remove();
		}

		std::cerr << "villager-smoke: villages from doors" << std::endl;
		{
			house(level, 10, 101, 10);
			house(level, 10, 101, 14);
			house(level, 10, 101, 18);
			Villages &villages = level.getVillages();
			villages.queryUpdateAround(10, 101, 14);
			for (int_t i = 0; i < 5; ++i)
				villages.tick();
			ok &= expect(villages.getVillages().size() == 1, "three doors make one village");
			if (!villages.getVillages().empty())
			{
				Village &village = *villages.getVillages()[0];
				ok &= expect(village.getDoorCount() == 3, "the village has three doors");
				ok &= expect(village.getRadius() == 32, "a small village has radius 32");
				DoorInfo *door = village.getClosestDoorInfo(10, 101, 10);
				ok &= expect(door != nullptr && door->insideDx == 2 && door->insideDz == 0, "the roofed side of the door is inside");
				village.modifyStanding(u"Tester", -40);
				ok &= expect(village.getStanding(u"Tester") == -30 && village.isVeryBadStanding(u"Tester"), "standing is clamped to -30");
			}
		}

		std::cerr << "villager-smoke: walking a path" << std::endl;
		{
			auto walker = std::make_shared<Villager>(level, 0);
			walker->moveTo(30.5, 101.0, 2.5, 0.0f, 0.0f);
			level.addEntity(walker);
			for (int_t i = 0; i < 5; ++i)
				walker->tick();
			std::shared_ptr<NavPath> path = level.findPath(*walker, 38, 101, 2, 16.0f, true, false, false, false);
			ok &= expect(path != nullptr && path->getSize() >= 8, "the path finder finds a straight path");
			bool started = walker->getNavigation().moveTo(38.5, 101.0, 2.5, 0.3f);
			ok &= expect(started, "navigation accepts the target");
			for (int_t i = 0; i < 200 && walker->x < 37.0; ++i)
				walker->tick();
			ok &= expect(walker->x > 36.0, "the villager walks to the target");
		}
	}
	{
		std::unique_ptr<File> workdir(File::open(u"build/villager-smoke-workdir"));
		Level::deleteLevel(*workdir, u"villager-smoke-world");
	}

	std::cerr << (ok ? "villager-smoke: PASS" : "villager-smoke: FAIL") << std::endl;
	return ok ? 0 : 1;
}

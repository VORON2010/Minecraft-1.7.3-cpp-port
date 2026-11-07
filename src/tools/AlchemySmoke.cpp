#include "tools/AlchemySmoke.h"

#include <iostream>
#include <memory>

#include "java/File.h"
#include "nbt/CompoundTag.h"
#include "world/entity/effect/MobEffect.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/item/PotionItem.h"
#include "world/inventory/ContainerRepair.h"
#include "world/inventory/ContainerMenus.h"
#include "world/entity/player/Player.h"
#include "world/level/tile/AnvilTile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/EnchantmentTableTile.h"
#include "world/item/ItemSword.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/enchantment/Enchantment.h"
#include "world/item/alchemy/PotionBrewing.h"
#include "world/level/Level.h"
#include "world/level/tile/BrewingStandTile.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/BrewingStandTileEntity.h"
#include "world/stats/StatList.h"

namespace
{
	bool expect(bool condition, const char *message)
	{
		if (!condition)
			std::cerr << "FAIL: " << message << std::endl;
		return condition;
	}

	bool hasEffect(int_t data, const MobEffect &effect, int_t amplifier = -1)
	{
		for (const MobEffectInstance &e : PotionBrewing::getEffects(data, false))
			if (e.getId() == effect.id && (amplifier < 0 || e.getAmplifier() == amplifier))
				return true;
		return false;
	}
}

int runAlchemySmoke()
{
	Tile::initTiles();
	Items::initItems();
	StatList::init();
	bool ok = true;

	std::cerr << "alchemy-smoke: brewing formulas" << std::endl;
	{
		int_t awkward = PotionBrewing::applyBrew(0, PotionBrewing::NETHER_WART);
		ok &= expect(awkward == 16, "nether wart turns water into an awkward potion (16)");
		ok &= expect(PotionBrewing::getEffects(awkward, false).empty(), "awkward potions have no effect");
		ok &= expect(PotionBrewing::getAppearanceName(awkward) == u"potion.prefix.awkward", "data 16 is called awkward");
		int_t strength = PotionBrewing::applyBrew(awkward, PotionBrewing::BLAZE_POWDER);
		ok &= expect(hasEffect(strength, MobEffect::damageBoost, 0), "blaze powder makes strength");
		int_t strength2 = PotionBrewing::applyBrew(strength, PotionBrewing::GLOWSTONE);
		ok &= expect(hasEffect(strength2, MobEffect::damageBoost, 1), "glowstone makes strength II");
		int_t healing = PotionBrewing::applyBrew(awkward, PotionBrewing::GLISTERING_MELON);
		ok &= expect(hasEffect(healing, MobEffect::heal), "glistering melon makes healing");
		int_t harming = PotionBrewing::applyBrew(healing, PotionBrewing::FERMENTED_SPIDER_EYE);
		ok &= expect(hasEffect(harming, MobEffect::harm), "a fermented eye turns healing into harming");
		int_t splash = PotionBrewing::applyBrew(healing, PotionBrewing::GUNPOWDER);
		ok &= expect(PotionItem::isThrowable(splash) && hasEffect(splash, MobEffect::heal), "gunpowder makes a splash potion");
		int_t swift = PotionBrewing::applyBrew(awkward, PotionBrewing::SUGAR);
		auto effects = PotionBrewing::getEffects(swift, false);
		ok &= expect(!effects.empty() && effects[0].getId() == MobEffect::movementSpeed.id && effects[0].getDuration() == 3600, "swiftness lasts 3:00");
		int_t swiftLong = PotionBrewing::applyBrew(swift, PotionBrewing::REDSTONE);
		effects = PotionBrewing::getEffects(swiftLong, false);
		ok &= expect(!effects.empty() && effects[0].getDuration() == 9600, "redstone extends swiftness to 8:00");
		ok &= expect(PotionBrewing::getColorValue(0, false) == 3694022, "water bottles are blue");
	}

	std::cerr << "alchemy-smoke: brewing stand" << std::endl;
	{
		{
			std::unique_ptr<File> workdir(File::open(u"build/alchemy-smoke-workdir"));
			Level::deleteLevel(*workdir, u"alchemy-smoke-world");
		}
		{
			Level level(File::open(u"build/alchemy-smoke-workdir"), u"alchemy-smoke-world", 42LL);
			level.getChunk(0, 0);
			level.setTile(1, 100, 1, Tile::brewingStand.id);
			auto stand = std::dynamic_pointer_cast<BrewingStandTileEntity>(level.getTileEntity(1, 100, 1));
			ok &= expect(stand != nullptr, "the brewing stand has a tile entity");
			if (stand != nullptr)
			{
				int_t potionId = Items::potion->getShiftedIndex();
				stand->setInventorySlotContents(0, ItemInstance(potionId, 1, 0));
				stand->setInventorySlotContents(2, ItemInstance(potionId, 1, 0));
				stand->setInventorySlotContents(3, ItemInstance(Items::netherWart->getShiftedIndex(), 2, 0));
				stand->tick();
				ok &= expect(stand->getBrewTime() == 600, "brewing takes 600 ticks");
				for (int_t i = 0; i < 600; ++i)
					stand->tick();
				ok &= expect(stand->getItem(0).itemDamage == 16 && stand->getItem(2).itemDamage == 16, "both water bottles became awkward potions");
				ok &= expect(stand->getItem(3).stackSize == 1, "one nether wart was used");
				ok &= expect(level.getData(1, 100, 1) == 5, "the stand shows bottles in holders 0 and 2");
				stand->tick();
				ok &= expect(stand->getBrewTime() == 0, "nether wart does nothing more to awkward potions");

				CompoundTag tag;
				stand->save(tag);
				BrewingStandTileEntity copy;
				copy.load(tag);
				ok &= expect(copy.getItem(0).itemDamage == 16 && copy.getItem(3).stackSize == 1, "the stand saves its bottles");
			}
		}
		std::unique_ptr<File> workdir(File::open(u"build/alchemy-smoke-workdir"));
		Level::deleteLevel(*workdir, u"alchemy-smoke-world");
	}

	std::cerr << "alchemy-smoke: enchanting" << std::endl;
	{
		ItemInstance sword(Items::swordDiamond->getShiftedIndex(), 1, 0);
		ItemInstance stick(Items::stick->getShiftedIndex(), 1, 0);
		ok &= expect(sword.isEnchantable() && !stick.isEnchantable(), "swords can be enchanted, sticks cannot");
		Random random(1234);
		int_t maxCost = 0;
		for (int_t i = 0; i < 200; ++i)
		{
			int_t c = EnchantmentHelper::getEnchantmentCost(random, 2, 30, sword);
			if (c > maxCost)
				maxCost = c;
			ok &= expect(c >= 1 && c <= 50, "the third slot costs 1..50 levels");
		}
		ok &= expect(maxCost == 50 || maxCost >= 40, "thirty bookshelves reach level 40 and more");
		int_t picked = 0;
		bool sawSharpness = false;
		for (int_t i = 0; i < 200; ++i)
		{
			auto chosen = EnchantmentHelper::selectEnchantment(random, sword, 30);
			picked += chosen.empty() ? 0 : 1;
			for (auto &e : chosen)
			{
				ok &= expect(canEnchant(e.enchantment->category, *sword.getItem()), "only weapon enchantments go on a sword");
				sawSharpness |= e.enchantment == &Enchantment::damageAll;
			}
		}
		ok &= expect(picked == 200 && sawSharpness, "level 30 always gives a sword something, sharpness among them");
		ItemInstance pick(Items::pickaxeIron->getShiftedIndex(), 1, 0);
		pick.enchant(Enchantment::digging.id, 3);
		CompoundTag tag;
		pick.save(tag);
		ItemInstance loaded(tag);
		ok &= expect(loaded.enchantments.size() == 1 && loaded.enchantments[0].first == 32 && loaded.enchantments[0].second == 3, "enchantments are saved in tag.ench");
		ok &= expect(loaded.isFoil() && !loaded.isEnchantable(), "an enchanted pickaxe glints and cannot be enchanted again");
		ok &= expect(Enchantment::damageAll.getFullname(5) == u"Sharpness V", "Sharpness V is named like in 1.0");
		ok &= expect(!Enchantment::untouching.isCompatibleWith(Enchantment::lootBonusDigger), "silk touch and fortune exclude each other");
		ok &= expect(Enchantment::protection.getMinCost(4) == 49 && Enchantment::digging.getMaxCost(1) == 61, "costs follow the 1.0 formulas");
	}

	std::cerr << "alchemy-smoke: anvil and books" << std::endl;
	{
		{
			std::unique_ptr<File> workdir(File::open(u"build/alchemy-smoke-workdir"));
			Level::deleteLevel(*workdir, u"alchemy-anvil-world");
		}
		{
			Level level(File::open(u"build/alchemy-smoke-workdir"), u"alchemy-anvil-world", 7LL);
			level.getChunk(0, 0);
			struct TestPlayer : public Player
			{
				using Player::Player;
			};
			auto player = std::make_shared<TestPlayer>(level);
			player->moveTo(0.5, 101.0, 0.5, 0.0f, 0.0f);
			level.addEntity(player);
			level.setTile(1, 100, 1, Tile::anvil.id);
			ok &= expect(level.getTile(1, 100, 1) == Tile::anvil.id, "the anvil is placed");
			ContainerRepair menu(player->inventory, level, 1, 100, 1, *player);

			int_t pickId = Items::pickaxeIron->getShiftedIndex();
			ItemInstance worn(pickId, 1, 200);
			menu.inputs.setInventorySlotContents(0, worn);
			menu.inputs.setInventorySlotContents(1, ItemInstance(Items::ingotIron->getShiftedIndex(), 2, 0));
			const ItemInstance *out = menu.result.getStackInSlot(0);
			ok &= expect(out != nullptr && out->itemDamage == 200 - 2 * (250 / 4) && menu.repairItemCountCost == 2, "two iron ingots repair a quarter of the pickaxe each");
			ok &= expect(menu.cost > 0 && out != nullptr && out->repairCost == 2, "repairing costs levels and raises the repair cost");

			ItemInstance a1(Items::swordIron->getShiftedIndex(), 1, 0);
			a1.enchant(Enchantment::damageAll.id, 1);
			ItemInstance a2 = a1;
			menu.inputs.setInventorySlotContents(0, a1);
			menu.inputs.setInventorySlotContents(1, a2);
			out = menu.result.getStackInSlot(0);
			ok &= expect(out != nullptr && EnchantmentHelper::getEnchantmentLevel(Enchantment::damageAll.id, out) == 2, "two Sharpness I swords make Sharpness II");

			ItemInstance book(Items::enchantedBook->getShiftedIndex(), 1, 0);
			EnchantedBookItem::addEnchantment(book, Enchantment::knockback.id, 2);
			menu.inputs.setInventorySlotContents(0, ItemInstance(Items::swordIron->getShiftedIndex(), 1, 0));
			menu.inputs.setInventorySlotContents(1, book);
			out = menu.result.getStackInSlot(0);
			ok &= expect(out != nullptr && EnchantmentHelper::getEnchantmentLevel(Enchantment::knockback.id, out) == 2, "an enchanted book puts Knockback II on a sword");
			int_t bookCost = menu.cost;

			player->experienceLevel = 0;
			Slot &resultSlot = *menu.slots[2];
			ok &= expect(!resultSlot.mayPickup(*player), "without levels the result cannot be taken");
			player->experienceLevel = 30;
			ok &= expect(resultSlot.mayPickup(*player), "with enough levels it can");
			ItemInstance taken = resultSlot.remove(1);
			resultSlot.onPickup(taken);
			ok &= expect(player->experienceLevel == 30 - bookCost, "taking the result spends the levels");
			ok &= expect(menu.inputs.getStackInSlot(0) == nullptr && menu.inputs.getStackInSlot(1) == nullptr, "the inputs are used up");

			menu.inputs.setInventorySlotContents(0, ItemInstance(Tile::rock.id, 10, 0));
			menu.inputs.setInventorySlotContents(1, ItemInstance());
			menu.setItemName(u"Hello");
			out = menu.result.getStackInSlot(0);
			ok &= expect(out != nullptr && out->customName == u"Hello" && menu.cost == 39, "renaming ten stone costs 5 a piece, capped at 39");
			
			ItemInstance named = *out;
			CompoundTag tag;
			named.save(tag);
			ItemInstance loaded(tag);
			ok &= expect(loaded.customName == u"Hello" && loaded.repairCost == named.repairCost, "names and repair cost are saved");
			ItemInstance part = loaded.remove(3);
			ok &= expect(part.customName == u"Hello", "splitting a stack keeps its name");

			ContainerEnchantment table(player->inventory, level, 5, 100, 5);
			ok &= expect(ItemInstance(Items::book->getShiftedIndex(), 1, 0).isEnchantable(), "a single book can be enchanted");
			level.setTile(5, 100, 5, Tile::enchantTable.id);
			table.enchantSlots.setInventorySlotContents(0, ItemInstance(Items::book->getShiftedIndex(), 1, 0));
			ok &= expect(table.costs[0] > 0, "the table offers enchantments for a book");
			player->experienceLevel = 30;
			ok &= expect(table.clickMenuButton(*player, 0), "the book can be enchanted");
			const ItemInstance *enchanted = table.enchantSlots.getStackInSlot(0);
			ok &= expect(enchanted != nullptr && enchanted->itemID == Items::enchantedBook->getShiftedIndex() && enchanted->storedEnchantments.size() == 1 && enchanted->enchantments.empty(), "it turns into an enchanted book with one stored enchantment");
		}
		std::unique_ptr<File> workdir(File::open(u"build/alchemy-smoke-workdir"));
		Level::deleteLevel(*workdir, u"alchemy-anvil-world");
	}

	std::cerr << "alchemy-smoke: " << (ok ? "PASS" : "FAIL") << std::endl;
	return ok ? 0 : 1;
}

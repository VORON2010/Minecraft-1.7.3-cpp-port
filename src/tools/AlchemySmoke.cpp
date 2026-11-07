#include "tools/AlchemySmoke.h"

#include <iostream>
#include <memory>

#include "java/File.h"
#include "nbt/CompoundTag.h"
#include "world/entity/effect/MobEffect.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/item/PotionItem.h"
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

	std::cerr << "alchemy-smoke: " << (ok ? "PASS" : "FAIL") << std::endl;
	return ok ? 0 : 1;
}

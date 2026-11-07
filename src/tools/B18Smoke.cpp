#include "tools/B18Smoke.h"

#include <array>
#include <iostream>
#include <memory>

#include "nbt/CompoundTag.h"
#include "world/entity/EntityIO.h"
#include "world/entity/effect/MobEffect.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/item/ExperienceOrb.h"
#include "world/entity/monster/CaveSpider.h"
#include "world/entity/monster/Enderman.h"
#include "world/entity/monster/Silverfish.h"
#include "world/entity/monster/Zombie.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/item/crafting/CraftingContainer.h"
#include "world/item/crafting/Recipes.h"
#include "world/level/Level.h"
#include "world/level/tile/FenceGateTile.h"
#include "world/level/tile/GlassTile.h"
#include "world/level/tile/HugeMushroomTile.h"
#include "world/level/tile/InfestedTile.h"
#include "world/level/tile/MelonTile.h"
#include "world/level/tile/PaneTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/level/tile/StemTile.h"
#include "world/level/tile/StoneBrickTile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/VineTile.h"
#include "world/level/tile/WoodTile.h"
#include "world/level/tile/FarmlandTile.h"
#include "world/level/tile/SlabTile.h"
#include "world/level/tile/StairTile.h"
#include "world/level/tile/GrassTile.h"

namespace
{
	bool check(bool condition, const char *message)
	{
		if (!condition)
			std::cout << "FAIL: " << message << std::endl;
		return condition;
	}

	struct GridCraftingContainer : public CraftingContainer
	{
		std::array<ItemInstance, 9> slots;

		ItemInstance getItem(int_t x, int_t y) const override
		{
			return slots[static_cast<std::size_t>(x + y * 3)];
		}
	};

	struct EffectZombie : public Zombie
	{
		explicit EffectZombie(Level &level) : Zombie(level) {}
		void tickEffectsForSmoke() { tickEffects(); }
	};

	struct EffectPlayer : public Player
	{
		explicit EffectPlayer(Level &level) : Player(level) {}
		void tickEffectsForSmoke() { tickEffects(); }
		void jumpForSmoke() { jumpFromGround(); }
	};

	ItemInstance craft(std::initializer_list<std::pair<int_t, ItemInstance>> slots)
	{
		GridCraftingContainer grid{};
		for (const auto &slot : slots)
			grid.slots[slot.first] = slot.second;
		return Recipes::getInstance().getItemFor(grid);
	}

	void clearArea(Level &level, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1)
	{
		for (int_t x = x0; x <= x1; ++x)
			for (int_t y = y0; y <= y1; ++y)
				for (int_t z = z0; z <= z1; ++z)
					level.setTile(x, y, z, 0);
	}
}

bool runB18Smoke(Level &level, Player &player, int_t baseY)
{
	bool ok = true;
	(void)player;
	std::cerr << "block-smoke: b1.8 tiles" << std::endl;

	const int_t ids[] = {97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109};
	for (int_t id : ids)
		ok &= check(Tile::tiles[id] != nullptr, "every b1.8 tile id 97-109 should be registered");
	ok &= check(Tile::glassPane.getRenderShape() == Tile::SHAPE_PANE && Tile::ironBars.getRenderShape() == Tile::SHAPE_PANE,
		"glass pane and iron bars should use the pane render shape");
	ok &= check(Tile::vine.getRenderShape() == Tile::SHAPE_VINE, "vines should use the vine render shape");
	ok &= check(Tile::pumpkinStem.getRenderShape() == Tile::SHAPE_STEM && Tile::melonStem.getRenderShape() == Tile::SHAPE_STEM,
		"stems should use the stem render shape");
	ok &= check(Tile::fenceGate.getRenderShape() == Tile::SHAPE_FENCE_GATE, "fence gate should use the gate render shape");
	ok &= check(Tile::stoneBrick.getTexture(Facing::NORTH, 0) == 54 && Tile::stoneBrick.getTexture(Facing::NORTH, 1) == 100 &&
		Tile::stoneBrick.getTexture(Facing::NORTH, 2) == 101, "stone brick variants should use the b1.8 terrain slots");
	ok &= check(Tile::monsterEgg.getTexture(Facing::NORTH, 0) == Tile::rock.tex && Tile::monsterEgg.getTexture(Facing::NORTH, 2) == 54,
		"monster egg should look like the block it hides in");
	ok &= check(Tile::melon.getTexture(Facing::UP) == 137 && Tile::melon.getTexture(Facing::NORTH) == 136, "melon top and side textures");
	ok &= check(Tile::hugeMushroomBrown.getTexture(Facing::UP, 5) == 126 && Tile::hugeMushroomRed.getTexture(Facing::UP, 5) == 125 &&
		Tile::hugeMushroomBrown.getTexture(Facing::NORTH, 10) == 141, "huge mushroom cap and stem textures");

	std::cerr << "block-smoke: b1.8 recipes" << std::endl;
	ItemInstance stone(Tile::rock.id, 1, 0);
	ItemInstance bricksOut = craft({{0, stone}, {1, stone}, {3, stone}, {4, stone}});
	ok &= check(bricksOut.itemID == Tile::stoneBrick.id && bricksOut.stackSize == 4, "4 stone should craft 4 stone bricks");
	ItemInstance glass(Tile::glass.id, 1, 0);
	ItemInstance paneOut = craft({{0, glass}, {1, glass}, {2, glass}, {3, glass}, {4, glass}, {5, glass}});
	ok &= check(paneOut.itemID == Tile::glassPane.id && paneOut.stackSize == 16, "6 glass should craft 16 panes");
	ItemInstance iron(Items::ingotIron->getShiftedIndex(), 1, 0);
	ItemInstance barsOut = craft({{0, iron}, {1, iron}, {2, iron}, {3, iron}, {4, iron}, {5, iron}});
	ok &= check(barsOut.itemID == Tile::ironBars.id && barsOut.stackSize == 16, "6 iron should craft 16 iron bars");
	ItemInstance stick(Items::stick->getShiftedIndex(), 1, 0);
	ItemInstance planks(Tile::wood.id, 1, 0);
	ItemInstance gateOut = craft({{0, stick}, {1, planks}, {2, stick}, {3, stick}, {4, planks}, {5, stick}});
	ok &= check(gateOut.itemID == Tile::fenceGate.id, "sticks and planks should craft a fence gate");
	ItemInstance slice(Items::melon->getShiftedIndex(), 1, 0);
	ItemInstance melonOut = craft({{0, slice}, {1, slice}, {2, slice}, {3, slice}, {4, slice}, {5, slice}, {6, slice}, {7, slice}, {8, slice}});
	ok &= check(melonOut.itemID == Tile::melon.id, "9 melon slices should craft a melon block");
	ItemInstance seedOut = craft({{4, slice}});
	ok &= check(seedOut.itemID == Items::melonSeeds->getShiftedIndex(), "a melon slice should craft melon seeds");
	ItemInstance brick(Tile::brick.id, 1, 0);
	ItemInstance slabOut = craft({{0, brick}, {1, brick}, {2, brick}});
	ok &= check(slabOut.itemID == Tile::slabSingle.id && slabOut.itemDamage == 4 && slabOut.stackSize == 3, "bricks should craft brick slabs");
	ItemInstance stairOut = craft({{0, brick}, {3, brick}, {4, brick}, {6, brick}, {7, brick}, {8, brick}});
	ok &= check(stairOut.itemID == Tile::stairsBrick.id && stairOut.stackSize == 4, "bricks should craft brick stairs");

	std::cerr << "block-smoke: b1.8 pane" << std::endl;
	int_t px = 300, py = baseY + 1, pz = 40;
	clearArea(level, px - 2, py - 1, pz - 2, px + 2, py + 2, pz + 2);
	level.setTile(px, py - 1, pz, Tile::rock.id);
	level.setTile(px, py, pz, Tile::glassPane.id);
	Tile::glassPane.updateShape(level, px, py, pz);
	ok &= check(Tile::glassPane.xx0 == 0.0 && Tile::glassPane.xx1 == 1.0 && Tile::glassPane.zz0 == 0.0 && Tile::glassPane.zz1 == 1.0,
		"a lone pane should be a full cross");
	level.setTile(px - 1, py, pz, Tile::glass.id);
	level.setTile(px + 1, py, pz, Tile::glass.id);
	Tile::glassPane.updateShape(level, px, py, pz);
	ok &= check(Tile::glassPane.xx0 == 0.0 && Tile::glassPane.xx1 == 1.0 && Tile::glassPane.zz0 == 0.4375 && Tile::glassPane.zz1 == 0.5625,
		"a pane between two glass blocks should be a thin east-west sheet");

	std::cerr << "block-smoke: b1.8 vine" << std::endl;
	int_t vx = 306, vy = baseY + 2, vz = 40;
	clearArea(level, vx - 2, vy - 2, vz - 2, vx + 2, vy + 2, vz + 2);
	level.setTile(vx, vy, vz + 1, Tile::rock.id);
	ok &= check(Tile::vine.mayPlaceOnFace(level, vx, vy, vz, Facing::NORTH), "vine should attach to a solid block behind it");
	ok &= check(!Tile::vine.mayPlaceOnFace(level, vx, vy, vz, Facing::SOUTH), "vine should not float without support");
	level.setTile(vx, vy, vz, Tile::vine.id);
	Tile::vine.setPlacedOnFace(level, vx, vy, vz, Facing::NORTH);
	ok &= check(level.getData(vx, vy, vz) == 1, "vine placed on a north face should hang on its south side");
	ok &= check(Tile::vine.getAABB(level, vx, vy, vz) == nullptr, "vines should have no collision box");
	level.setTile(vx, vy, vz + 1, 0);
	Tile::vine.neighborChanged(level, vx, vy, vz, 0);
	ok &= check(level.getTile(vx, vy, vz) == 0, "vine should drop when its support is removed");

	std::cerr << "block-smoke: b1.8 fence gate" << std::endl;
	int_t gx = 310, gy = baseY + 1, gz = 40;
	clearArea(level, gx - 1, gy - 1, gz - 1, gx + 1, gy + 2, gz + 1);
	ok &= check(!Tile::fenceGate.mayPlace(level, gx, gy, gz), "fence gate needs a solid block below");
	level.setTile(gx, gy - 1, gz, Tile::rock.id);
	ok &= check(Tile::fenceGate.mayPlace(level, gx, gy, gz), "fence gate should place on stone");
	level.setTileAndData(gx, gy, gz, Tile::fenceGate.id, 0);
	ok &= check(Tile::fenceGate.getAABB(level, gx, gy, gz) != nullptr, "a closed gate should block movement");
	Tile::fenceGate.use(level, gx, gy, gz, player);
	ok &= check(FenceGateTile::isOpen(level.getData(gx, gy, gz)), "using a gate should open it");
	ok &= check(Tile::fenceGate.getAABB(level, gx, gy, gz) == nullptr, "an open gate should not collide");
	Tile::fenceGate.use(level, gx, gy, gz, player);
	ok &= check(!FenceGateTile::isOpen(level.getData(gx, gy, gz)), "using an open gate should close it");

	std::cerr << "block-smoke: b1.8 stems" << std::endl;
	int_t sx = 314, sy = baseY + 1, sz = 40;
	clearArea(level, sx - 2, sy - 1, sz - 2, sx + 2, sy + 2, sz + 2);
	for (int_t x = sx - 1; x <= sx + 1; ++x)
		for (int_t z = sz - 1; z <= sz + 1; ++z)
			level.setTile(x, sy - 1, z, Tile::farmland.id);
	level.setTile(sx, sy, sz, Tile::melonStem.id);
	ok &= check(level.getTile(sx, sy, sz) == Tile::melonStem.id, "melon stem should be placeable on farmland");
	Tile::melonStem.growFully(level, sx, sy, sz);
	ok &= check(level.getData(sx, sy, sz) == 7, "bonemealed stem should be fully grown");
	ok &= check(Tile::melonStem.getConnectDirection(level, sx, sy, sz) == -1, "stem without fruit should not bend");
	level.setTile(sx + 1, sy, sz, Tile::melon.id);
	ok &= check(Tile::melonStem.getConnectDirection(level, sx, sy, sz) == 1, "grown stem should bend towards its melon");
	ok &= check(Tile::melonStem.getItemColor(7) == (224 << 16 | 199 << 8 | 28), "fully grown stem tint should match b1.8");

	std::cerr << "block-smoke: b1.8 items" << std::endl;
	ok &= check(Items::melon != nullptr && Items::pumpkinSeeds != nullptr && Items::melonSeeds != nullptr && Items::enderPearl != nullptr,
		"b1.8 items should exist");
	ok &= check(Items::enderPearl->getMaxStackSize() == 16, "ender pearls should stack to 16");
	{
		ItemInstance seeds(Items::pumpkinSeeds->getShiftedIndex(), 2, 0);
		int_t fx = 318, fy = baseY + 1, fz = 40;
		clearArea(level, fx, fy, fz, fx, fy + 1, fz);
		level.setTile(fx, fy, fz, Tile::farmland.id);
		ok &= check(seeds.useOn(player, level, fx, fy, fz, Facing::UP), "pumpkin seeds should plant on farmland");
		ok &= check(level.getTile(fx, fy + 1, fz) == Tile::pumpkinStem.id && seeds.stackSize == 1, "planting should place a pumpkin stem");
	}

	std::cerr << "block-smoke: b1.8 status effects" << std::endl;
	{
		EffectPlayer effectPlayer(level);
		effectPlayer.health = 10;
		effectPlayer.addEffect(MobEffectInstance(MobEffect::poison.id, 100, 0));
		ok &= check(effectPlayer.hasEffect(MobEffect::poison), "poison should be applied");
		for (int_t i = 0; i < 100; ++i)
		{
			effectPlayer.invulnerableTime = 0;
			effectPlayer.tickEffectsForSmoke();
		}
		ok &= check(effectPlayer.health == 6, "poison I should deal 1 damage every 25 ticks for 5 seconds");
		ok &= check(!effectPlayer.hasEffect(MobEffect::poison), "poison should expire");

		effectPlayer.health = 1;
		effectPlayer.addEffect(MobEffectInstance(MobEffect::poison.id, 200, 1));
		for (int_t i = 0; i < 200; ++i)
		{
			effectPlayer.invulnerableTime = 0;
			effectPlayer.tickEffectsForSmoke();
		}
		ok &= check(effectPlayer.health == 1, "poison should never kill");

		effectPlayer.health = 10;
		effectPlayer.addEffect(MobEffectInstance(MobEffect::regeneration.id, 50, 0));
		for (int_t i = 0; i < 50; ++i)
			effectPlayer.tickEffectsForSmoke();
		ok &= check(effectPlayer.health == 12, "regeneration I should heal every 25 ticks");

		effectPlayer.addEffect(MobEffectInstance(MobEffect::movementSpeed.id, 100, 1));
		ok &= check(effectPlayer.getSpeedModifier() > 1.39f && effectPlayer.getSpeedModifier() < 1.41f, "speed II should be +40%");
		effectPlayer.addEffect(MobEffectInstance(MobEffect::jump.id, 100, 0));
		effectPlayer.jumpForSmoke();
		ok &= check(effectPlayer.yd > 0.519 && effectPlayer.yd < 0.521, "jump boost I should add 0.1 to jump velocity");

		effectPlayer.addEffect(MobEffectInstance(MobEffect::movementSpeed.id, 50, 0));
		ok &= check(effectPlayer.getEffect(MobEffect::movementSpeed)->getAmplifier() == 1, "a weaker effect should not replace a stronger one");

		CompoundTag saved;
		effectPlayer.saveWithoutId(saved);
		EffectPlayer loaded(level);
		loaded.load(saved);
		ok &= check(loaded.hasEffect(MobEffect::movementSpeed) && loaded.getEffect(MobEffect::movementSpeed)->getAmplifier() == 1,
			"active effects should survive save/load");

		EffectZombie zombie(level);
		zombie.addEffect(MobEffectInstance(MobEffect::poison.id, 100, 0));
		ok &= check(!zombie.hasEffect(MobEffect::poison), "undead should be immune to poison");
		zombie.health = 20;
		zombie.addEffect(MobEffectInstance(MobEffect::heal.id, 1, 0));
		zombie.invulnerableTime = 0;
		zombie.tickEffectsForSmoke();
		ok &= check(zombie.health == 14, "instant health should hurt undead");

		CaveSpider caveSpider(level);
		caveSpider.addEffect(MobEffectInstance(MobEffect::poison.id, 100, 0));
		ok &= check(!caveSpider.hasEffect(MobEffect::poison), "spiders should be immune to poison");
	}

	std::cerr << "block-smoke: b1.8 mobs" << std::endl;
	for (const char16_t *name : {u"Enderman", u"CaveSpider", u"Silverfish"})
	{
		std::shared_ptr<Entity> created = EntityIO::newEntity(jstring(name), level);
		ok &= check(created != nullptr && created->getEncodeId() == name, "new b1.8 mobs should be creatable by save id");
		if (created == nullptr)
			continue;
		created->moveTo(330.5, baseY + 2, 40.5, 0.0f, 0.0f);
		CompoundTag tag;
		ok &= check(created->save(tag), "new b1.8 mobs should save");
		std::shared_ptr<Entity> entity = EntityIO::loadStatic(tag, level);
		ok &= check(entity != nullptr && entity->getEncodeId() == name, "new b1.8 mobs should load back from a save");
	}
	{
		Enderman enderman(level);
		ok &= check(enderman.health == 40, "endermen should have 40 health");
		enderman.setCarriedTile(Tile::grass.id);
		enderman.setCarriedData(0);
		CompoundTag saved;
		enderman.saveWithoutId(saved);
		Enderman loaded(level);
		loaded.load(saved);
		ok &= check(loaded.getCarriedTile() == Tile::grass.id, "enderman carried block should survive save/load");
	}
	{
		int_t ex = 322, ey = baseY + 1, ez = 40;
		clearArea(level, ex - 1, ey, ez - 1, ex + 1, ey + 2, ez + 1);
		level.setTile(ex, ey - 1, ez, Tile::rock.id);
		size_t before = level.entities.size();
		Tile::monsterEgg.destroy(level, ex, ey, ez, 0);
		bool spawned = false;
		for (size_t i = before; i < level.entities.size(); ++i)
			if (dynamic_cast<Silverfish *>(level.entities[i].get()) != nullptr)
				spawned = true;
		ok &= check(spawned, "breaking a monster egg should release a silverfish");
		ok &= check(InfestedTile::canBeInfested(Tile::rock.id) && InfestedTile::getVariant(Tile::stoneBrick.id) == 2,
			"silverfish can hide in stone and stone bricks");
	}
	{
		EffectPlayer eater(level);
		eater.foodLevel = 10;
		ItemInstance flesh(Items::rottenFlesh->getShiftedIndex(), 20, 0);
		int_t hungerHits = 0;
		for (int_t i = 0; i < 20; ++i)
		{
			eater.removeAllEffects();
			eater.foodLevel = 10;
			flesh.useTimeDepleted(level, eater);
			if (eater.hasEffect(MobEffect::hunger))
				hungerHits++;
		}
		ok &= check(hungerHits > 8 && hungerHits < 20, "rotten flesh should usually (80%) cause hunger");
	}

	std::cerr << "block-smoke: b1.8 experience" << std::endl;
	{
		EffectPlayer xpPlayer(level);
		ok &= check(xpPlayer.getXpNeededForNextLevel() == 7, "level 0 needs 7 xp");
		xpPlayer.increaseXp(7);
		ok &= check(xpPlayer.experienceLevel == 1 && xpPlayer.experienceProgress < 0.001f, "7 xp should reach level 1");
		ok &= check(xpPlayer.getXpNeededForNextLevel() == 10, "level 1 needs 10 xp");
		xpPlayer.increaseXp(5);
		ok &= check(xpPlayer.experienceLevel == 1 && xpPlayer.experienceProgress > 0.49f && xpPlayer.experienceProgress < 0.51f, "5 of 10 xp is half a level");
		CompoundTag saved;
		xpPlayer.saveWithoutId(saved);
		EffectPlayer loadedXp(level);
		loadedXp.load(saved);
		ok &= check(loadedXp.experienceLevel == 1 && loadedXp.totalExperience == 12, "experience should survive save/load");

		ok &= check(ExperienceOrb::getExperienceValue(5) == 3 && ExperienceOrb::getExperienceValue(20) == 17, "orb values should split like vanilla");

		auto victim = std::make_shared<Zombie>(level);
		victim->moveTo(334.5, baseY + 2, 40.5, 0.0f, 0.0f);
		level.addEntity(victim);
		auto killer = std::make_shared<Player>(level);
		killer->moveTo(336.5, baseY + 2, 40.5, 0.0f, 0.0f);
		victim->hurt(killer.get(), 100);
		size_t before = level.entities.size();
		for (int_t i = 0; i < 21; ++i)
			victim->baseTick();
		int_t orbXp = 0;
		for (size_t i = before; i < level.entities.size(); ++i)
			if (auto *orb = dynamic_cast<ExperienceOrb *>(level.entities[i].get()))
				orbXp += orb->getValue();
		ok &= check(orbXp == 5, "a zombie killed by a player should drop 5 xp in orbs");
	}

	return ok;
}

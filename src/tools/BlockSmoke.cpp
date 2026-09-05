#include "tools/BlockSmoke.h"

#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <thread>
#include <unordered_map>
#include <vector>
#include "client/particle/TerrainParticle.h"
#include "client/particle/NoteParticle.h"
#include "client/particle/RainParticle.h"
#include "client/model/WolfModel.h"
#include "client/Minecraft.h"
#include "client/User.h"
#include "client/renderer/TileRenderer.h"
#include "client/renderer/texturefx/TextureCompassFX.h"
#include "client/renderer/texturefx/TextureWatchFX.h"
#include "client/renderer/texturefx/TextureFlamesFX.h"
#include "client/renderer/texturefx/TexturePortalFX.h"
#include "client/renderer/texturefx/TileSize.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/ItemInstance.h"
#include "world/item/ItemArmor.h"
#include "world/item/crafting/CraftingContainer.h"
#include "world/item/crafting/Recipes.h"
#include "world/entity/player/Player.h"
#include "world/entity/Mob.h"
#include "world/entity/EntityIO.h"
#include "world/entity/monster/Monster.h"
#include "world/entity/animal/Cow.h"
#include "world/entity/animal/Pig.h"
#include "world/entity/animal/Sheep.h"
#include "world/entity/animal/Chicken.h"
#include "world/entity/animal/Squid.h"
#include "world/entity/animal/Wolf.h"
#include "world/entity/monster/Zombie.h"
#include "world/entity/monster/Skeleton.h"
#include "world/entity/monster/PigZombie.h"
#include "world/entity/monster/Spider.h"
#include "world/entity/monster/Creeper.h"
#include "world/entity/projectile/EntityArrow.h"
#include "world/entity/projectile/EntityFish.h"
#include "world/entity/projectile/EntitySnowball.h"
#include "world/entity/projectile/EntityThrownEgg.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/item/EntityPainting.h"
#include "world/entity/EntityLightningBolt.h"
#include "world/level/pathfinder/PathEntity.h"
#include "world/level/pathfinder/Pathfinder.h"
#include "world/level/Level.h"
#include "world/level/SpawnerAnimals.h"
#include "world/level/material/GasMaterial.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"
#include "world/level/LevelListener.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/GlassTile.h"
#include "world/level/tile/ClothTile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/WoodTile.h"
#include "world/level/tile/RedStoneDustTile.h"
#include "world/level/tile/LeverTile.h"
#include "world/level/tile/ButtonTile.h"
#include "world/level/tile/PressurePlateTile.h"
#include "world/level/tile/NotGateTile.h"
#include "world/level/tile/RepeaterTile.h"
#include "world/level/tile/RailTile.h"
#include "world/level/tile/DetectorRailTile.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/PortalTile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/entity/item/EntityMinecart.h"
#include "world/level/tile/ChestTile.h"
#include "world/level/tile/entity/ChestTileEntity.h"
#include "world/level/tile/LockedChestTile.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/NoteTile.h"
#include "world/level/tile/DispenserTile.h"
#include "world/level/tile/DeadBushTile.h"
#include "world/level/tile/entity/DispenserTileEntity.h"
#include "world/level/tile/entity/SignTileEntity.h"
#include "nbt/CompoundTag.h"
#include "world/level/tile/entity/NoteTileEntity.h"
#include "world/level/tile/PistonBaseTile.h"
#include "world/level/tile/PistonExtensionTile.h"
#include "world/level/tile/PistonMovingTile.h"
#include "world/level/tile/LeafTile.h"
#include "world/level/tile/WebTile.h"
#include "world/level/tile/TNTTile.h"
#include "java/Random.h"
#include "java/File.h"
#include "java/String.h"
#include "world/stats/Achievement.h"
#include "world/stats/AchievementList.h"
#include "world/stats/StatFileWriter.h"
#include "world/stats/StatList.h"
#include "world/stats/StatCrafting.h"

namespace
{
	struct StatCapturePlayer : public Player
	{
		std::unordered_map<const StatBase *, int_t> capturedStats;

		explicit StatCapturePlayer(Level &level) : Player(level) {}

		void addStat(const StatBase &stat, int_t amount) override
		{
			capturedStats[&stat] += amount;
		}

		int_t getCapturedStat(const StatBase *stat) const
		{
			auto it = capturedStats.find(stat);
			return it == capturedStats.end() ? 0 : it->second;
		}

		void jumpForSmoke()
		{
			jumpFromGround();
		}

		void fallForSmoke(float distance)
		{
			causeFallDamage(distance);
		}

		void setCachedWaterForSmoke(bool value)
		{
			wasInWater = value;
		}
	};

	struct CaptureListener : public LevelListener
	{
		std::vector<jstring> sounds;
		std::vector<jstring> particles;
		std::vector<jstring> streams;
		int_t dirtyCalls = 0;

		void tileChanged(int_t, int_t, int_t) override {}
		void setTilesDirty(int_t, int_t, int_t, int_t, int_t, int_t) override { dirtyCalls++; }
		void allChanged() override {}
		void playSound(const jstring &name, double, double, double, float, float) override { sounds.push_back(name); }
		void addParticle(const jstring &name, double, double, double, double, double, double) override { particles.push_back(name); }
		void playMusic(const jstring &, double, double, double, float) override {}
		void entityAdded(std::shared_ptr<Entity>) override {}
		void entityRemoved(std::shared_ptr<Entity>) override {}
		void skyColorChanged() override {}
		void playStreamingMusic(const jstring &name, int_t, int_t, int_t) override { streams.push_back(name); }
		void tileEntityChanged(int_t, int_t, int_t, std::shared_ptr<TileEntity>) override {}
		void levelEvent(Player *, int_t, int_t, int_t, int_t, int_t) override {}

		void clear()
		{
			sounds.clear();
			particles.clear();
			streams.clear();
			dirtyCalls = 0;
		}
	};

	struct InspectableTerrainParticle : public TerrainParticle
	{
		using TerrainParticle::TerrainParticle;
		int_t textureIndex() const { return tex; }
	};

struct InspectableNoteParticle : public NoteParticle
	{
		using NoteParticle::NoteParticle;
		float currentSize() const { return size; }
	};

	struct InspectableRainParticle : public RainParticle
	{
		using RainParticle::RainParticle;
		int_t textureIndex() const { return tex; }
		int_t remainingLifetime() const { return lifetime; }
		float particleGravity() const { return gravity; }
	};

	struct TestLevelSource : public LevelSource
	{
		Level &level;
		std::unordered_map<long_t, int_t> tiles;

		explicit TestLevelSource(Level &level) : level(level) {}

		static long_t key(int_t x, int_t y, int_t z)
		{
			return (static_cast<long_t>(x) << 32) ^ (static_cast<long_t>(z) << 16) ^ static_cast<long_t>(y);
		}

		void setTile(int_t x, int_t y, int_t z, int_t tile)
		{
			tiles[key(x, y, z)] = tile;
		}

		int_t getTile(int_t x, int_t y, int_t z) override
		{
			auto it = tiles.find(key(x, y, z));
			return it == tiles.end() ? 0 : it->second;
		}

		std::shared_ptr<TileEntity> getTileEntity(int_t, int_t, int_t) override { return nullptr; }
		float getBrightness(int_t, int_t, int_t) override { return 1.0f; }
		int_t getData(int_t, int_t, int_t) override { return 0; }
		const Material &getMaterial(int_t x, int_t y, int_t z) override
		{
			int_t tile = getTile(x, y, z);
			return tile == 0 ? static_cast<const Material &>(Material::air) : Tile::tiles[tile]->material;
		}
		bool isSolidTile(int_t x, int_t y, int_t z) override
		{
			Tile *tile = Tile::tiles[getTile(x, y, z)];
			return tile != nullptr && tile->isSolidRender();
		}
		bool isBlockNormalCube(int_t x, int_t y, int_t z) override
		{
			Tile *tile = Tile::tiles[getTile(x, y, z)];
			return tile != nullptr && tile->isCubeShaped();
		}
		BiomeSource &getBiomeSource() override { return level.getBiomeSource(); }
	};

	struct NaturalSpawnerTestLevel : public Level
	{
		std::vector<std::shared_ptr<Entity>> spawnedEntities;

		explicit NaturalSpawnerTestLevel(int_t dimension)
			: Level(File::open(u"build/block-smoke-workdir"),
				dimension == -1 ? u"natural-spawner-smoke-hell" : u"natural-spawner-smoke-overworld",
				2468LL, dimension)
		{
			xSpawn = 1000000;
			ySpawn = 64;
			zSpawn = 1000000;
		}

		bool isWaterColumn(int_t x, int_t z) const
		{
			return Mth::floor(x / 16.0) == 4 && Mth::floor(z / 16.0) == 4;
		}

		int_t getTile(int_t x, int_t y, int_t z) override
		{
			return y >= 0 && y < DEPTH && isWaterColumn(x, z) ? Tile::water.id : 0;
		}

		const Material &getMaterial(int_t x, int_t y, int_t z) override
		{
			return getTile(x, y, z) == Tile::water.id
				? static_cast<const Material &>(Material::water)
				: static_cast<const Material &>(Material::air);
		}

		bool addEntity(std::shared_ptr<Entity> entity) override
		{
			spawnedEntities.push_back(std::move(entity));
			return true;
		}
	};
	
	struct GridCraftingContainer : public CraftingContainer
	{
		std::array<ItemInstance, 9> slots;

		ItemInstance getItem(int_t x, int_t y) const override
		{
			return slots[static_cast<std::size_t>(x + y * 3)];
		}
	};

	struct InspectablePlayer : public Player
	{
		using Player::Player;
		void applyArmorDamage(int_t damage) { actuallyHurt(damage); }
	};

	struct InspectableCow : public Cow
	{
		using Cow::Cow;
		using Cow::getDeathLoot;
	};

	struct InspectableZombie : public Zombie
	{
		using Zombie::Zombie;
		using Monster::checkHurtTarget;
		using Monster::findAttackTarget;
	};

	struct InspectablePig : public Pig
	{
		using Pig::Pig;
	};

	struct InspectableSheep : public Sheep
	{
		using Sheep::Sheep;
	};

	struct InspectableChicken : public Chicken
	{
		using Chicken::Chicken;
		using Chicken::getDeathLoot;
	};

	struct InspectableSpider : public Spider
	{
		using Spider::Spider;
		using Spider::getDeathLoot;
		using Spider::findAttackTarget;
	};

	struct InspectableCreeper : public Creeper
	{
		using Creeper::Creeper;
		using Creeper::checkHurtTarget;
		using Creeper::getDeathLoot;
	};

	struct InspectableSkeleton : public Skeleton
	{
		using Skeleton::Skeleton;
		using Skeleton::checkHurtTarget;
		using Skeleton::getDeathLoot;
	};

	struct InspectablePigZombie : public PigZombie
	{
		using PigZombie::PigZombie;
		using PigZombie::findAttackTarget;
	};

	struct InspectableArrow : public EntityArrow
	{
		using EntityArrow::EntityArrow;
		void plantForPickup() { inGround = true; doesArrowBelongToPlayer = true; arrowShake = 0; }
	};
	
	bool expect(bool condition, const char *message)
	{
		if (!condition)
		{
			std::cerr << "FAIL: " << message << std::endl;
			return false;
		}
		return true;
	}

	bool expectNear(double actual, double expected, double epsilon, const char *message)
	{
		return expect(std::abs(actual - expected) <= epsilon, message);
	}

	void logEvents(const char *label, const std::vector<jstring> &values)
	{
		std::cerr << label << "[" << values.size() << "]";
		for (std::size_t i = 0; i < values.size(); ++i)
			std::cerr << (i == 0 ? " " : ", ") << String::toUTF8(values[i]);
		std::cerr << std::endl;
	}

	bool containsPrefix(const std::vector<jstring> &values, const jstring &prefix)
	{
		for (std::size_t i = 0; i < values.size(); ++i)
		{
			if (values[i].find(prefix) == 0)
				return true;
		}
		return false;
	}

	void logRepeatedDoorState(Level &level, const CaptureListener &listener, const char *phase, int_t doorX, int_t doorY, int_t doorZ, int_t supportX, int_t supportY, int_t supportZ, int_t leverX, int_t leverY, int_t leverZ)
	{
		std::cerr << "diag door " << phase
			<< ": leverData=" << level.getData(leverX, leverY, leverZ)
			<< " supportPowered=" << level.hasNeighborSignal(supportX, supportY, supportZ)
			<< " doorData=" << level.getData(doorX, doorY, doorZ)
			<< " doorPowered=" << level.hasNeighborSignal(doorX, doorY, doorZ)
			<< std::endl;
		logEvents("diag door sounds", listener.sounds);
		logEvents("diag door particles", listener.particles);
	}

	void logRepeatedNoteState(Level &level, const CaptureListener &listener, const char *phase, int_t noteX, int_t noteY, int_t noteZ, int_t leverX, int_t leverY, int_t leverZ, NoteTileEntity *noteEntity)
	{
		std::cerr << "diag note " << phase
			<< ": leverData=" << level.getData(leverX, leverY, leverZ)
			<< " notePowered=" << level.hasDirectSignal(noteX, noteY, noteZ)
			<< " leverProvidesEast=" << level.getDirectSignal(leverX, leverY, leverZ, 5)
			<< " aboveTile=" << level.getTile(noteX, noteY + 1, noteZ)
			<< " belowTile=" << level.getTile(noteX, noteY - 1, noteZ)
			<< " notePrev=" << (noteEntity != nullptr ? noteEntity->previousRedstoneState : false)
			<< std::endl;
		logEvents("diag note sounds", listener.sounds);
		logEvents("diag note particles", listener.particles);
	}
}

int runBlockSmoke()
{
	try
	{
		std::cerr << "block-smoke: init" << std::endl;
		Tile::initTiles();
		Items::initItems();
		StatList::init();

		bool ok = true;
		Random random;
		std::cerr << "block-smoke: static assertions" << std::endl;
		ok &= expect(StatList::generalStats.size() == 23, "general stat registry should contain the 23 Beta 1.7.3 counters");
		ok &= expect(StatList::startGameStat->statId == 1000 && StatList::minutesPlayedStat->statId == 1100 &&
			StatList::distanceWalkedStat->statId == 2000 && StatList::fishCaughtStat->statId == 2025,
			"general stat ids should match Beta 1.7.3");
		ok &= expect(StatList::startGameStat->independent && StatList::minutesPlayedStat->independent &&
			!StatList::damageDealtStat->independent,
			"only the Beta 1.7.3 independent counters should be marked independent");
		ok &= expect(StatList::jumpStat->format(1234567) == u"1,234,567" &&
			StatList::minutesPlayedStat->format(20) == u"1.0 s" &&
			StatList::minutesPlayedStat->format(21) == u"1.05 s" &&
			StatList::minutesPlayedStat->format(720) == u"0.60 m" &&
			StatList::minutesPlayedStat->format(std::numeric_limits<int_t>::min()) == u"-1.073741824E8 s" &&
			StatList::distanceWalkedStat->format(100) == u"1.00 m" &&
			StatList::distanceWalkedStat->format(100000) == u"1.00 km",
			"stat formatters should match the Beta number, time, and distance formats");
		ok &= expect(StatList::mineBlockStats[Tile::rock.id] != nullptr &&
			StatList::mineBlockStats[Tile::rock.id]->statId == 16777216 + Tile::rock.id &&
			StatList::useItemStats[Items::pickaxeWood->getShiftedIndex()] != nullptr &&
			StatList::breakItemStats[Items::pickaxeWood->getShiftedIndex()] != nullptr &&
			StatList::craftItemStats[Items::pickaxeWood->getShiftedIndex()] != nullptr,
			"block mining and item use, break, and craft counters should use the Beta id ranges");
		ok &= expect(StatList::mineBlockStats[18] == nullptr && StatList::mineBlockStats[9] == StatList::mineBlockStats[8] &&
			StatList::mineBlockStats[62] == StatList::mineBlockStats[61] &&
			StatList::mineBlockStats[2] == StatList::mineBlockStats[3] &&
			StatList::mineBlockStats[60] == StatList::mineBlockStats[3],
			"disabled and equivalent block mining counters should match the Beta registry aliases");
		ok &= expect(AchievementList::achievements.size() == 16, "achievement registry should contain the 16 Beta 1.7.3 achievements");
		ok &= expect(AchievementList::openInventory->statId == 5242880 && AchievementList::flyPig->statId == 5242895,
			"achievement stat ids should occupy the Beta 1.7.3 range");
		ok &= expect(StatList::getStat(5242880) == AchievementList::openInventory && StatList::getStat(5242895) == AchievementList::flyPig,
			"achievement stat ids should resolve through StatList");
		ok &= expect(AchievementList::openInventory->independent, "Taking Inventory should be marked independent");
		ok &= expect(AchievementList::mineWood->parentAchievement == AchievementList::openInventory,
			"Getting Wood should require Taking Inventory");
		ok &= expect(AchievementList::buildFurnace->parentAchievement == AchievementList::buildPickaxe,
			"Hot Topic should require Time to Mine");
		ok &= expect(AchievementList::flyPig->parentAchievement == AchievementList::killCow,
			"When Pigs Fly should require Cow Tipper");
		ok &= expect(AchievementList::onARail->isSpecial() && AchievementList::flyPig->isSpecial(),
			"On A Rail and When Pigs Fly should use special achievement frames");
		ok &= expect(AchievementList::minDisplayColumn == -1 && AchievementList::minDisplayRow == -5 &&
			AchievementList::maxDisplayColumn == 8 && AchievementList::maxDisplayRow == 6,
			"achievement display bounds should match the Beta 1.7.3 tree");
		ok &= expect(!AchievementList::openInventory->statGuid.empty() && !AchievementList::flyPig->statGuid.empty(),
			"achievement GUIDs should load from achievement/map.txt");
		StatMap achievementStats;
		achievementStats[AchievementList::openInventory] = 1;
		achievementStats[AchievementList::mineWood] = 2;
		jstring smokeUser = u"Smoke";
		jstring localSession = u"local";
		std::unique_ptr<StatMap> parsedAchievementStats = StatFileWriter::parse(
			StatFileWriter::serialize(&smokeUser, &localSession, achievementStats));
		ok &= expect(parsedAchievementStats != nullptr && parsedAchievementStats->at(AchievementList::openInventory) == 1 &&
			parsedAchievementStats->at(AchievementList::mineWood) == 2,
			"achievement stats should survive the Beta checksum serialization round trip");
		StatMap counterStats;
		counterStats[StatList::minutesPlayedStat] = 24000;
		counterStats[StatList::mineBlockStats[Tile::rock.id]] = 37;
		counterStats[StatList::craftItemStats[Items::pickaxeWood->getShiftedIndex()]] = 4;
		std::string serializedCounterStats = StatFileWriter::serialize(&smokeUser, &localSession, counterStats);
		std::unique_ptr<StatMap> parsedCounterStats = StatFileWriter::parse(
			serializedCounterStats);
		ok &= expect(parsedCounterStats != nullptr && parsedCounterStats->at(StatList::minutesPlayedStat) == 24000 &&
			parsedCounterStats->at(StatList::mineBlockStats[Tile::rock.id]) == 37 &&
			parsedCounterStats->at(StatList::craftItemStats[Items::pickaxeWood->getShiftedIndex()]) == 4,
			"non-achievement counters should survive the Beta checksum serialization round trip");
		std::string malformedCounterStats = serializedCounterStats;
		malformedCounterStats.insert(malformedCounterStats.find("\r\n  ],"), ",");
		std::unique_ptr<StatMap> malformedResult = StatFileWriter::parse(malformedCounterStats);
		ok &= expect(malformedResult != nullptr && malformedResult->empty(),
			"invalid Beta stats JSON syntax should return an empty map");
		std::string mismatchedCounterStats = serializedCounterStats;
		const std::string checksumMarker = "\"checksum\":\"";
		size_t checksumPosition = mismatchedCounterStats.find(checksumMarker) + checksumMarker.size();
		mismatchedCounterStats[checksumPosition] = mismatchedCounterStats[checksumPosition] == '0' ? '1' : '0';
		ok &= expect(StatFileWriter::parse(mismatchedCounterStats) == nullptr,
			"Beta stats checksum mismatches should return null");
		bool wrongShapeThrew = false;
		try
		{
			StatFileWriter::parse("{\"stats-change\":{},\"checksum\":\"\"}");
		}
		catch (const std::exception &)
		{
			wrongShapeThrew = true;
		}
		ok &= expect(wrongShapeThrew, "wrong Beta stats JSON shapes should propagate from the parser");
		bool overflowValueThrew = false;
		try
		{
			StatFileWriter::parse("{\"stats-change\":[{\"1000\":1},{\"1001\":2147483648}],\"checksum\":\"\"}");
		}
		catch (const std::out_of_range &)
		{
			overflowValueThrew = true;
		}
		ok &= expect(overflowValueThrew, "overflowing Beta stats values should propagate without returning a partial map");
		ok &= expect(Items::reed->getDescriptionId() == u"item.reeds" &&
			Items::minecartPowered->getDescriptionId() == u"item.minecartFurnace" &&
			Items::diamond->getDescriptionId() == u"item.emerald" &&
			Tile::deadBush.descriptionId == u"tile.deadbush",
			"corrected item and tile description ids should match Beta 1.7.3");
		ok &= expect(Tile::lightBlock[65] == 0, "ladder should stay transparent for light sampling like beta");
		ok &= expect(Tile::lightBlock[53] == 255, "wood stairs should block light like beta");
		ok &= expect(Tile::lightBlock[67] == 255, "stone stairs should block light like beta");
		ok &= expect(Tile::lightBlock[96] == 0, "trapdoor should not block light");
		ok &= expect(Tile::lightBlock[63] == 0, "sign post should not block light");
		ok &= expect(Tile::lightBlock[68] == 0, "wall sign should not block light");
		ok &= expect(Items::sign->getShiftedIndex() == 323, "sign item should use the beta shifted id 323");
		ok &= expect(ItemInstance(Items::sign->getShiftedIndex(), 1, 0).getIcon() == 42, "sign item should use the beta sign icon");
		ok &= expect(Tile::tiles[30]->getResource(0, random) == Items::silk->getShiftedIndex(), "cobweb should drop string");
		ok &= expect(Tile::tiles[89]->getResource(0, random) == Items::glowstoneDust->getShiftedIndex(), "glowstone should drop glowstone dust");
		ok &= expect(Tile::tiles[80]->getResource(0, random) == Items::snowball->getShiftedIndex(), "snow block should drop snowballs");
		ok &= expect(Tile::tiles[64]->getResource(0, random) == Items::doorWood->getShiftedIndex(), "wood door bottom half should drop wood door item");
		ok &= expect(Tile::tiles[64]->getResource(8, random) == 0, "wood door top half should not drop an item");
		ItemInstance woodAxe(Items::axeWood->getShiftedIndex(), 1, 0);
		ok &= expect(woodAxe.getDestroySpeed(*Tile::tiles[47]) > 1.0f, "axe should be the preferred tool for bookshelves");
		ok &= expect(woodAxe.getDestroySpeed(*Tile::tiles[58]) == 1.0f, "workbench should not get an axe speed bonus in b173");
		ItemInstance stoneShovel(Items::shovelStone->getShiftedIndex(), 1, 0);
		ok &= expect(stoneShovel.getDestroySpeed(*Tile::tiles[80]) > 1.0f, "shovel should be effective against snow blocks");
		ok &= expect(stoneShovel.canDestroySpecial(*Tile::tiles[80]), "shovel should harvest snow blocks as a special case");
		ItemInstance woodPick(Items::pickaxeWood->getShiftedIndex(), 1, 0);
		ok &= expect(!woodPick.canDestroySpecial(*Tile::tiles[41]), "wood pickaxe should not harvest gold blocks");
		ItemInstance stonePick(Items::pickaxeStone->getShiftedIndex(), 1, 0);
		ok &= expect(stonePick.getDestroySpeed(*Tile::tiles[41]) > 1.0f, "pickaxe should be the preferred tool for gold blocks");
		ok &= expect(!stonePick.canDestroySpecial(*Tile::tiles[41]), "stone pickaxe should not harvest gold blocks");
		ItemInstance ironPick(Items::pickaxeIron->getShiftedIndex(), 1, 0);
		ok &= expect(ironPick.canDestroySpecial(*Tile::tiles[41]), "iron pickaxe should harvest gold blocks");
		ItemInstance diamondPick(Items::pickaxeDiamond->getShiftedIndex(), 1, 0);
		ok &= expect(diamondPick.canDestroySpecial(*Tile::tiles[49]), "diamond pickaxe should harvest obsidian");
		ItemInstance woodSword(Items::swordWood->getShiftedIndex(), 1, 0);
		ok &= expect(woodSword.getDestroySpeed(*Tile::tiles[30]) == 15.0f, "sword should be the preferred tool for cobwebs before shears");
		ok &= expect(woodSword.canDestroySpecial(*Tile::tiles[30]), "sword should harvest cobwebs as a special case");

		Vec3::resetPool();
		Vec3 *xRotVec = Vec3::newTemp(0.0, 1.0, 0.0);
		xRotVec->xRot(Mth::PI * 0.5f);
		ok &= expectNear(xRotVec->x, 0.0, 1.0e-6, "Vec3 xRot should preserve x");
		ok &= expectNear(xRotVec->y, 0.0, 1.0e-6, "Vec3 xRot should rotate y to zero at 90 degrees");
		ok &= expectNear(xRotVec->z, -1.0, 1.0e-6, "Vec3 xRot should rotate +Y to -Z");
		Vec3 *yRotVec = Vec3::newTemp(1.0, 0.0, 0.0);
		yRotVec->yRot(Mth::PI * 0.5f);
		ok &= expectNear(yRotVec->x, 0.0, 1.0e-6, "Vec3 yRot should rotate x to zero at 90 degrees");
		ok &= expectNear(yRotVec->y, 0.0, 1.0e-6, "Vec3 yRot should preserve y");
		ok &= expectNear(yRotVec->z, -1.0, 1.0e-6, "Vec3 yRot should rotate +X to -Z");
		Vec3 *zRotVec = Vec3::newTemp(1.0, 0.0, 0.0);
		zRotVec->zRot(Mth::PI * 0.5f);
		ok &= expectNear(zRotVec->x, 0.0, 1.0e-6, "Vec3 zRot should rotate x to zero at 90 degrees");
		ok &= expectNear(zRotVec->y, -1.0, 1.0e-6, "Vec3 zRot should rotate +X to -Y");
		ok &= expectNear(zRotVec->z, 0.0, 1.0e-6, "Vec3 zRot should preserve z");
		ok &= expect(Tile::redstoneWire.getTexture(Facing::NORTH, 0) == 164, "redstone wire should use the b173 texture index");
		ok &= expect(Tile::redstoneWire.getTexture(Facing::NORTH, 1) == 164, "powered redstone wire should keep the same tinted texture");
		ok &= expect(Tile::lever.tex == 96, "lever should use the beta lever texture");
		ok &= expect(Tile::lightEmission[75] == 0, "redstone torch off should not emit light");
		ok &= expect(Tile::lightEmission[76] == 7, "redstone torch on should emit beta redstone torch light");
		ok &= expect(Tile::buttonStone.getRenderShape() == Tile::SHAPE_BLOCK, "stone button should render as a bounded block shape");
		ok &= expect(Tile::pressurePlateStone.getRenderShape() == Tile::SHAPE_BLOCK, "pressure plate should render as a bounded block shape");
		ok &= expect(!TileRenderer::canRender(Tile::SHAPE_RED_DUST), "redstone wire item should use flat icon rendering");
		ok &= expect(!TileRenderer::canRender(Tile::SHAPE_LEVER), "lever item should use flat icon rendering");
		ok &= expect(TileRenderer::canRender(Tile::SHAPE_FENCE), "fence items should use the Beta 3D inventory renderer");
		ok &= expect(!TileRenderer::canRender(Tile::SHAPE_BED), "bed items should use flat icon rendering");
		ok &= expect(!TileRenderer::canRender(Tile::SHAPE_PISTON_EXTENSION), "piston extension items should use flat icon rendering");
		ok &= expect(Tile::lightBlock[55] == 0, "redstone wire should not block light");
		ok &= expect(Tile::lightBlock[69] == 0, "lever should not block light");
		ok &= expect(Tile::lightBlock[70] == 0, "stone pressure plate should not block light");
		ok &= expect(Tile::lightBlock[72] == 0, "wood pressure plate should not block light");
		ok &= expect(Tile::lightBlock[75] == 0, "redstone torch off should not block light");
		ok &= expect(Tile::lightBlock[76] == 0, "redstone torch on should not block light");
		ok &= expect(Tile::lightBlock[77] == 0, "stone button should not block light");
		ok &= expect(ItemInstance(Tile::lever.id, 1, 0).getIcon() == 96, "lever block item should use lever texture icon");
		ok &= expect(Tile::torchRedstoneIdle.descriptionId == u"tile.notGate", "redstone torch off should use localized notGate key");
		ok &= expect(Tile::torchRedstoneActive.descriptionId == u"tile.notGate", "redstone torch on should use localized notGate key");
		ok &= expect(Tile::repeaterIdle.getRenderShape() == Tile::SHAPE_REPEATER, "repeater should use the custom repeater render shape");
		ok &= expect(Tile::repeaterIdle.getTexture(Facing::UP, 0) == 131, "idle repeater should use the beta top texture");
		ok &= expect(Tile::repeaterActive.getTexture(Facing::UP, 0) == 147, "active repeater should use the lit beta top texture");
		ok &= expect(Tile::repeaterIdle.getTexture(Facing::DOWN, 0) == 115, "idle repeater torches should use the off texture");
		ok &= expect(Tile::repeaterActive.getTexture(Facing::DOWN, 0) == 99, "active repeater torches should use the on texture");
		ok &= expect(Tile::lightEmission[94] == 9, "active repeater should emit the beta light level");
		ok &= expect(Tile::lightBlock[93] == 0, "idle repeater should not block light");
		ok &= expect(Tile::lightBlock[94] == 0, "active repeater should not block light");
		ok &= expect(Tile::repeaterIdle.descriptionId == u"tile.diode", "idle repeater should use the diode localization key");
		ok &= expect(Tile::repeaterActive.descriptionId == u"tile.diode", "active repeater should use the diode localization key");
		ok &= expect(Items::redstoneRepeater->getShiftedIndex() == 356, "repeater item should use the beta shifted id 356");
		ok &= expect(ItemInstance(Items::redstoneRepeater->getShiftedIndex(), 1, 0).getIcon() == 86, "repeater item should use the beta diode icon");
		GridCraftingContainer redstoneTorchRecipe{};
		redstoneTorchRecipe.slots[0] = ItemInstance(Items::redstone->getShiftedIndex(), 1, 0);
		redstoneTorchRecipe.slots[3] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		ItemInstance redstoneTorchOut = Recipes::getInstance().getItemFor(redstoneTorchRecipe);
		ok &= expect(redstoneTorchOut.itemID == Tile::torchRedstoneActive.id, "redstone torch recipe should match b173");
		GridCraftingContainer repeaterRecipe{};
		repeaterRecipe.slots[0] = ItemInstance(Tile::torchRedstoneActive.id, 1, 0);
		repeaterRecipe.slots[1] = ItemInstance(Items::redstone->getShiftedIndex(), 1, 0);
		repeaterRecipe.slots[2] = ItemInstance(Tile::torchRedstoneActive.id, 1, 0);
		repeaterRecipe.slots[3] = ItemInstance(Tile::rock.id, 1, 0);
		repeaterRecipe.slots[4] = ItemInstance(Tile::rock.id, 1, 0);
		repeaterRecipe.slots[5] = ItemInstance(Tile::rock.id, 1, 0);
		ItemInstance repeaterOut = Recipes::getInstance().getItemFor(repeaterRecipe);
		ok &= expect(repeaterOut.itemID == Items::redstoneRepeater->getShiftedIndex(), "repeater recipe should match b173");
		GridCraftingContainer stoneButtonRecipe{};
		stoneButtonRecipe.slots[0] = ItemInstance(Tile::rock.id, 1, 0);
		stoneButtonRecipe.slots[3] = ItemInstance(Tile::rock.id, 1, 0);
		ItemInstance stoneButtonOut = Recipes::getInstance().getItemFor(stoneButtonRecipe);
		ok &= expect(stoneButtonOut.itemID == Tile::buttonStone.id, "stone button recipe should use two stone stacked vertically");
		GridCraftingContainer stonePlateRecipe{};
		stonePlateRecipe.slots[0] = ItemInstance(Tile::rock.id, 1, 0);
		stonePlateRecipe.slots[1] = ItemInstance(Tile::rock.id, 1, 0);
		ItemInstance stonePlateOut = Recipes::getInstance().getItemFor(stonePlateRecipe);
		ok &= expect(stonePlateOut.itemID == Tile::pressurePlateStone.id, "stone pressure plate recipe should use stone, not cobblestone");

		ok &= expect(Tile::lightEmission[95] == 15, "locked chest should emit full light");
		ok &= expect(Tile::lockedChest.getTexture(Facing::UP, 0) == 25, "locked chest top should use the beta top texture");
		ok &= expect(Tile::lockedChest.getTexture(Facing::SOUTH, 0) == 27, "locked chest default front should use the beta latch texture");
		ok &= expect(Tile::lockedChest.descriptionId == u"tile.lockedchest", "locked chest should use the lockedchest localization key");
		ok &= expect(Tile::chest.descriptionId == u"tile.chest", "chest should use the chest localization key");
		ok &= expect(Tile::lightBlock[54] == 255, "chest should fully block light as a full cube (b173 parity)");
		ok &= expect(Items::minecartChest->getShiftedIndex() == 342, "chest minecart item should use the beta shifted id 342");
		ok &= expect(ItemInstance(Items::minecartChest->getShiftedIndex(), 1, 0).getIcon() == 151, "chest minecart item should use the beta chest minecart icon");
		GridCraftingContainer chestRecipe{};
		chestRecipe.slots[0] = ItemInstance(Tile::wood.id, 1, -1);
		chestRecipe.slots[1] = ItemInstance(Tile::wood.id, 1, -1);
		chestRecipe.slots[2] = ItemInstance(Tile::wood.id, 1, -1);
		chestRecipe.slots[3] = ItemInstance(Tile::wood.id, 1, -1);
		chestRecipe.slots[5] = ItemInstance(Tile::wood.id, 1, -1);
		chestRecipe.slots[6] = ItemInstance(Tile::wood.id, 1, -1);
		chestRecipe.slots[7] = ItemInstance(Tile::wood.id, 1, -1);
		chestRecipe.slots[8] = ItemInstance(Tile::wood.id, 1, -1);
		ItemInstance chestOut = Recipes::getInstance().getItemFor(chestRecipe);
		ok &= expect(chestOut.itemID == Tile::chest.id, "chest recipe should match beta");
		GridCraftingContainer chestMinecartRecipe{};
		chestMinecartRecipe.slots[0] = ItemInstance(Tile::chest.id, 1, 0);
		chestMinecartRecipe.slots[3] = ItemInstance(Items::minecart->getShiftedIndex(), 1, 0);
		ItemInstance chestMinecartOut = Recipes::getInstance().getItemFor(chestMinecartRecipe);
		ok &= expect(chestMinecartOut.itemID == Items::minecartChest->getShiftedIndex(), "chest minecart recipe should match beta");

		ok &= expect(Items::bread->getMaxStackSize() == 1, "bread should use the beta food stack limit");
		ok &= expect(Items::cookie->getShiftedIndex() == 357, "cookie should use the beta shifted id 357");
		ok &= expect(Items::cookie->getMaxStackSize() == 8, "cookie should stack to eight like beta");
		ok &= expect(ItemInstance(Items::cookie->getShiftedIndex(), 1, 0).getIcon() == 92, "cookie should use the beta cookie icon");
		GridCraftingContainer cookieRecipe{};
		cookieRecipe.slots[0] = ItemInstance(Items::wheat->getShiftedIndex(), 1, 0);
		cookieRecipe.slots[1] = ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 3);
		cookieRecipe.slots[2] = ItemInstance(Items::wheat->getShiftedIndex(), 1, 0);
		ItemInstance cookieOut = Recipes::getInstance().getItemFor(cookieRecipe);
		ok &= expect(cookieOut.itemID == Items::cookie->getShiftedIndex() && cookieOut.stackSize == 8, "cookie recipe should match b173test");
		ok &= expect(Tile::rail.getRenderShape() == Tile::SHAPE_RAIL, "rail should use the custom rail render shape");
		ok &= expect(Tile::railPowered.getRenderShape() == Tile::SHAPE_RAIL, "powered rail should use the custom rail render shape");
		ok &= expect(Tile::railDetector.getRenderShape() == Tile::SHAPE_RAIL, "detector rail should use the custom rail render shape");
		ok &= expect(!TileRenderer::canRender(Tile::SHAPE_RAIL), "rail items should use flat icon rendering");
		ok &= expect(Tile::lightBlock[27] == 0, "powered rail should not block light");
		ok &= expect(Tile::lightBlock[28] == 0, "detector rail should not block light");
		ok &= expect(Tile::lightBlock[66] == 0, "rail should not block light");
		ok &= expect(Tile::railPowered.getTexture(Facing::NORTH, 0) == 163, "unpowered powered rail should use the beta dark texture");
		ok &= expect(Tile::railPowered.getTexture(Facing::NORTH, 9) == 179, "powered rail should use the lit beta texture when powered");
		ok &= expect(Tile::rail.descriptionId == u"tile.rail", "rail should use the rail localization key");
		ok &= expect(Tile::railPowered.descriptionId == u"tile.goldenRail", "powered rail should use the goldenRail localization key");
		ok &= expect(Tile::railDetector.descriptionId == u"tile.detectorRail", "detector rail should use the detectorRail localization key");
		ok &= expect(Items::minecart->getShiftedIndex() == 328, "minecart item should use the beta shifted id 328");
		ok &= expect(ItemInstance(Items::minecart->getShiftedIndex(), 1, 0).getIcon() == 135, "minecart item should use the beta minecart icon");
		ok &= expect(Items::minecartPowered->getShiftedIndex() == 343, "furnace minecart item should use the beta shifted id 343");
		ok &= expect(ItemInstance(Items::minecartPowered->getShiftedIndex(), 1, 0).getIcon() == 167, "furnace minecart item should use the beta powered minecart icon");
		GridCraftingContainer railRecipe{};
		railRecipe.slots[0] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		railRecipe.slots[2] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		railRecipe.slots[3] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		railRecipe.slots[4] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		railRecipe.slots[5] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		railRecipe.slots[6] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		railRecipe.slots[8] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		ItemInstance railOut = Recipes::getInstance().getItemFor(railRecipe);
		ok &= expect(railOut.itemID == Tile::rail.id && railOut.stackSize == 16, "rail recipe should match b173test");
		GridCraftingContainer poweredRailRecipe{};
		poweredRailRecipe.slots[0] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		poweredRailRecipe.slots[2] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		poweredRailRecipe.slots[3] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		poweredRailRecipe.slots[4] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		poweredRailRecipe.slots[5] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		poweredRailRecipe.slots[6] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		poweredRailRecipe.slots[7] = ItemInstance(Items::redstone->getShiftedIndex(), 1, 0);
		poweredRailRecipe.slots[8] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		ItemInstance poweredRailOut = Recipes::getInstance().getItemFor(poweredRailRecipe);
		ok &= expect(poweredRailOut.itemID == Tile::railPowered.id && poweredRailOut.stackSize == 6, "powered rail recipe should match b173test");
		GridCraftingContainer detectorRailRecipe{};
		detectorRailRecipe.slots[0] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		detectorRailRecipe.slots[2] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		detectorRailRecipe.slots[3] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		detectorRailRecipe.slots[4] = ItemInstance(Tile::pressurePlateStone.id, 1, 0);
		detectorRailRecipe.slots[5] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		detectorRailRecipe.slots[6] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		detectorRailRecipe.slots[7] = ItemInstance(Items::redstone->getShiftedIndex(), 1, 0);
		detectorRailRecipe.slots[8] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		ItemInstance detectorRailOut = Recipes::getInstance().getItemFor(detectorRailRecipe);
		ok &= expect(detectorRailOut.itemID == Tile::railDetector.id && detectorRailOut.stackSize == 6, "detector rail recipe should match b173test");
		GridCraftingContainer minecartRecipe{};
		minecartRecipe.slots[0] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		minecartRecipe.slots[2] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		minecartRecipe.slots[3] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		minecartRecipe.slots[4] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		minecartRecipe.slots[5] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		ItemInstance minecartOut = Recipes::getInstance().getItemFor(minecartRecipe);
		ok &= expect(minecartOut.itemID == Items::minecart->getShiftedIndex(), "minecart recipe should match b173test");
		GridCraftingContainer furnaceMinecartRecipe{};
		furnaceMinecartRecipe.slots[0] = ItemInstance(Tile::furnace.id, 1, 0);
		furnaceMinecartRecipe.slots[3] = ItemInstance(Items::minecart->getShiftedIndex(), 1, 0);
		ItemInstance furnaceMinecartOut = Recipes::getInstance().getItemFor(furnaceMinecartRecipe);
		ok &= expect(furnaceMinecartOut.itemID == Items::minecartPowered->getShiftedIndex(), "furnace minecart recipe should match b173test");

		std::cerr << "block-smoke: natural spawning" << std::endl;
		NaturalSpawnerTestLevel naturalSpawnLevel(0);
		naturalSpawnLevel.random.setSeed(12345LL);
		std::shared_ptr<Player> naturalSpawnPlayer = std::make_shared<Player>(naturalSpawnLevel);
		naturalSpawnPlayer->moveTo(0.5, 64.0, 0.5, 0.0f, 0.0f);
		naturalSpawnLevel.players.push_back(naturalSpawnPlayer);
		int_t reportedNaturalSpawns = SpawnerAnimals::performSpawning(naturalSpawnLevel, false, true);
		bool spawnedOnlySquid = !naturalSpawnLevel.spawnedEntities.empty();
		for (const auto &entity : naturalSpawnLevel.spawnedEntities)
			spawnedOnlySquid = spawnedOnlySquid && dynamic_cast<Squid *>(entity.get()) != nullptr;
		ok &= expect(naturalSpawnLevel.spawnedEntities.size() == 4,
			"natural spawner should fill one valid water chunk to the squid pack limit");
		ok &= expect(spawnedOnlySquid && reportedNaturalSpawns > 0,
			"water-creature spawning should use the Beta squid list and report its cumulative pack count");

		NaturalSpawnerTestLevel hellNaturalSpawnLevel(-1);
		hellNaturalSpawnLevel.random.setSeed(12345LL);
		std::shared_ptr<Player> hellNaturalSpawnPlayer = std::make_shared<Player>(hellNaturalSpawnLevel);
		hellNaturalSpawnPlayer->moveTo(0.5, 64.0, 0.5, 0.0f, 0.0f);
		hellNaturalSpawnLevel.players.push_back(hellNaturalSpawnPlayer);
		SpawnerAnimals::performSpawning(hellNaturalSpawnLevel, false, true);
		ok &= expect(hellNaturalSpawnLevel.spawnedEntities.empty(),
			"Hell biome should expose no peaceful or water-creature spawn entries");

		std::cerr << "block-smoke: level setup" << std::endl;
		Level level(File::open(u"build/block-smoke-workdir"), u"block-smoke-world", 12345);
		
		const double pushCases[][5] = {
			{0.0075, 0.0075, 0.0, 0.0, 0.0},
			{0.25, 0.75, 0.3f, 0.010103629870652761, 0.030310889611958287},
			{3.0, -4.0, 0.0, 0.037500000558793545, -0.05000000074505806},
			{static_cast<double>(0.01f), 0.0, 0.0, 0.004999999888241291, 0.0},
			{1.0, std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0}
		};
		for (const auto &values : pushCases)
		{
			Entity first(level);
			Entity second(level);
			second.x = values[0];
			second.z = values[1];
			first.pushthrough = static_cast<float>(values[2]);
			first.push(second);
			ok &= expect(first.xd == -values[3] && first.zd == -values[4] &&
				second.xd == values[3] && second.zd == values[4],
				"entity collision impulses should match the Beta Java oracle");
		}
		CaptureListener listener;
		level.addListener(listener);
		Player player(level);
		player.yRot = 0.0f;
		int_t baseY = 80;
		User overflowUser(u"BlockSmokeOverflow", u"local");
		std::unique_ptr<File> overflowDirectory(File::open(u"build/block-smoke-workdir"));
		StatFileWriter overflowStats(overflowUser, *overflowDirectory);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		auto syncStart = std::chrono::steady_clock::now();
		overflowStats.syncStats();
		auto syncDuration = std::chrono::steady_clock::now() - syncStart;
		ok &= expect(syncDuration < std::chrono::seconds(1),
			"completed Beta stats receive workers should release synchronous saves before the main-thread tick");
		overflowStats.readStat(*StatList::dropStat, std::numeric_limits<int_t>::max());
		overflowStats.readStat(*StatList::dropStat, 1);
		ok &= expect(overflowStats.getStat(*StatList::dropStat) == std::numeric_limits<int_t>::min(),
			"readStat should wrap INT_MAX plus one like a Java int");
		overflowStats.readStat(*StatList::jumpStat, 1);
		StatMap loadedOverflowStats;
		loadedOverflowStats[StatList::jumpStat] = std::numeric_limits<int_t>::max();
		overflowStats.mergeLoadedTotal(loadedOverflowStats);
		ok &= expect(overflowStats.getStat(*StatList::jumpStat) == std::numeric_limits<int_t>::min(),
			"mergeLoadedTotal should wrap INT_MAX plus one like a Java int");
		StatCapturePlayer statsPlayer(level);
		statsPlayer.tick();
		statsPlayer.jumpForSmoke();
		statsPlayer.fallForSmoke(2.25f);
		ok &= expect(statsPlayer.getCapturedStat(StatList::minutesPlayedStat) == 1 &&
			statsPlayer.getCapturedStat(StatList::jumpStat) == 1 &&
			statsPlayer.getCapturedStat(StatList::distanceFallenStat) == 225,
			"player tick, jump, and fall should award the Beta counters");
		int_t preciseFallBefore = statsPlayer.getCapturedStat(StatList::distanceFallenStat);
		statsPlayer.fallForSmoke(2.135f);
		ok &= expect(statsPlayer.getCapturedStat(StatList::distanceFallenStat) == preciseFallBefore + 213,
			"fall distance should promote the Java float to double before rounding centimeters");
		StatCapturePlayer cachedWaterPlayer(level);
		cachedWaterPlayer.setPos(210.5, 126.0, 30.5);
		cachedWaterPlayer.noPhysics = true;
		cachedWaterPlayer.setCachedWaterForSmoke(true);
		ok &= expect(cachedWaterPlayer.isInWater() && !cachedWaterPlayer.handleWaterMovement(),
			"isInWater should return the cached base-tick state instead of rescanning the level");
		cachedWaterPlayer.travel(0.0f, 1.0f);
		ok &= expect(cachedWaterPlayer.getCapturedStat(StatList::distanceSwumStat) > 0 &&
			cachedWaterPlayer.getCapturedStat(StatList::distanceDoveStat) == 0 &&
			cachedWaterPlayer.getCapturedStat(StatList::distanceWalkedStat) == 0 &&
			cachedWaterPlayer.getCapturedStat(StatList::distanceFlownStat) == 0,
			"cached water state should classify player movement as swimming");
		ItemInstance craftedPickaxe(Items::pickaxeWood->getShiftedIndex(), 3, 0);
		craftedPickaxe.onCrafted(level, statsPlayer);
		ItemInstance brokenPickaxe(Items::pickaxeWood->getShiftedIndex(), 1, 0);
		brokenPickaxe.damageItem(brokenPickaxe.getMaxDamage() + 1, statsPlayer);
		ok &= expect(statsPlayer.getCapturedStat(StatList::craftItemStats[Items::pickaxeWood->getShiftedIndex()]) == 3 &&
			statsPlayer.getCapturedStat(StatList::breakItemStats[Items::pickaxeWood->getShiftedIndex()]) == 1,
			"crafting and depleting an item should award the Beta counters by output count and break event");
		Mob statTarget(level);
		ItemInstance statSword(Items::swordWood->getShiftedIndex(), 1, 0);
		statSword.hurtEnemy(statTarget, statsPlayer);
		statsPlayer.awardKillScore(statTarget, 1);
		Player statPlayerTarget(level);
		statsPlayer.awardKillScore(statPlayerTarget, 1);
		auto attackedTarget = std::make_shared<Mob>(level);
		statsPlayer.attack(attackedTarget);
		statsPlayer.yd = -0.1;
		auto fallingAttackTarget = std::make_shared<Mob>(level);
		statsPlayer.attack(fallingAttackTarget);
		statsPlayer.yd = 0.0;
		int_t damageTakenBefore = statsPlayer.getCapturedStat(StatList::damageTakenStat);
		statsPlayer.hurt(nullptr, 2);
		Monster hostileAttacker(level);
		level.difficulty = 1;
		statsPlayer.hurt(&hostileAttacker, 6);
		level.difficulty = 0;
		statsPlayer.hurt(&hostileAttacker, 6);
		level.difficulty = 2;
		ItemInstance droppedStack(Items::apple->getShiftedIndex(), 1, 0);
		statsPlayer.drop(droppedStack);
		Tile::rock.harvestBlock(level, statsPlayer, 230, baseY, 0, 0);
		statsPlayer.die(nullptr);
		ok &= expect(statsPlayer.getCapturedStat(StatList::useItemStats[Items::swordWood->getShiftedIndex()]) == 1 &&
			statsPlayer.getCapturedStat(StatList::mobKillsStat) == 1 && statsPlayer.getCapturedStat(StatList::playerKillsStat) == 1,
			"successful item hits and kill scoring should award the Beta counters");
		ok &= expect(statsPlayer.getCapturedStat(StatList::damageDealtStat) == 3,
			"successful attacks should award damage dealt including the falling-attack bonus");
		ok &= expect(statsPlayer.getCapturedStat(StatList::damageTakenStat) == damageTakenBefore + 5,
			"incoming hostile damage should award the difficulty-scaled amount and skip peaceful damage");
		ok &= expect(statsPlayer.getCapturedStat(StatList::dropStat) == 1,
			"dropping an item should award the drop counter");
		ok &= expect(statsPlayer.getCapturedStat(StatList::mineBlockStats[Tile::rock.id]) == 1,
			"block harvest should award the matching mined-block counter");
		ok &= expect(statsPlayer.getCapturedStat(StatList::deathsStat) == 1,
			"player death should award the death counter");
		auto mountPlayer = std::make_shared<Player>(level);
		mountPlayer->setPos(248.5, baseY + 2.0, 2.5);
		mountPlayer->yRot = 0.0f;
		ok &= expect(level.addEntity(mountPlayer), "minecart rider player should join the level");


		auto advancePendingTicks = [&](int_t ticks) {
			for (int_t i = 0; i < ticks; ++i)
			{
				level.time++;
				for (int_t pass = 0; pass < 32; ++pass)
					level.tickPendingTicks(false);
			}
		};

		std::cerr << "block-smoke: mob base" << std::endl;
		Mob viewer(level);
		Mob target(level);
		viewer.setPos(240.5, baseY + 2.0, 20.5);
		target.setPos(244.5, baseY + 2.0, 20.5);
		ok &= expect(viewer.isPickable(), "mob should be pickable before removal");
		ok &= expect(viewer.isPushable(), "mob should be pushable before removal");
		ok &= expect(viewer.isShootable(), "mob should be shootable like beta living entities");
		ok &= expectNear(viewer.getHeadHeight(), viewer.bbHeight * 0.85, 1.0e-6, "mob eye height should be 85 percent of height");
		ok &= expect(viewer.canSee(target), "mob line of sight should pass through air");
		level.setTile(242, baseY + 3, 20, Tile::rock.id);
		ok &= expect(!viewer.canSee(target), "mob line of sight should be blocked by solid tiles");
		level.setTile(242, baseY + 3, 20, 0);
		Mob dying(level);
		dying.setPos(250.5, baseY + 2.0, 20.5);
		dying.health = 1;
		listener.clear();
		ok &= expect(dying.hurt(nullptr, 1), "mob lethal damage should be accepted");
		for (int_t i = 0; i < 21; ++i)
			dying.baseTick();
		ok &= expect(dying.removed, "dead mob should be removed after beta corpse timer");
		ok &= expect(listener.particles.size() == 20 && containsPrefix(listener.particles, u"explode"), "dead mob should spawn 20 explode particles");

		std::cerr << "block-smoke: armor" << std::endl;
		ok &= expect(Items::helmetLeather != nullptr && Items::helmetLeather->getShiftedIndex() == 298, "leather helmet should use the beta item id");
		ok &= expect(Items::bootsGold != nullptr && Items::bootsGold->getShiftedIndex() == 317, "gold boots should use the beta item id");
		GridCraftingContainer leatherHelmetRecipe{};
		leatherHelmetRecipe.slots[0] = ItemInstance(Items::leather->getShiftedIndex(), 1, 0);
		leatherHelmetRecipe.slots[1] = ItemInstance(Items::leather->getShiftedIndex(), 1, 0);
		leatherHelmetRecipe.slots[2] = ItemInstance(Items::leather->getShiftedIndex(), 1, 0);
		leatherHelmetRecipe.slots[3] = ItemInstance(Items::leather->getShiftedIndex(), 1, 0);
		leatherHelmetRecipe.slots[5] = ItemInstance(Items::leather->getShiftedIndex(), 1, 0);
		ItemInstance leatherHelmetOut = Recipes::getInstance().getItemFor(leatherHelmetRecipe);
		ok &= expect(leatherHelmetOut.itemID == Items::helmetLeather->getShiftedIndex(), "leather helmet recipe should match beta");
		GridCraftingContainer chainHelmetRecipe{};
		chainHelmetRecipe.slots[0] = ItemInstance(Tile::fire.id, 1, 0);
		chainHelmetRecipe.slots[1] = ItemInstance(Tile::fire.id, 1, 0);
		chainHelmetRecipe.slots[2] = ItemInstance(Tile::fire.id, 1, 0);
		chainHelmetRecipe.slots[3] = ItemInstance(Tile::fire.id, 1, 0);
		chainHelmetRecipe.slots[5] = ItemInstance(Tile::fire.id, 1, 0);
		ItemInstance chainHelmetOut = Recipes::getInstance().getItemFor(chainHelmetRecipe);
		ok &= expect(chainHelmetOut.itemID == Items::helmetChain->getShiftedIndex(), "chain helmet recipe should use fire like beta");
		Player armorPlayer(level);
		armorPlayer.inventory.armorInventory[0] = ItemInstance(Items::bootsIron->getShiftedIndex(), 1, 0);
		armorPlayer.inventory.armorInventory[1] = ItemInstance(Items::legsIron->getShiftedIndex(), 1, 0);
		armorPlayer.inventory.armorInventory[2] = ItemInstance(Items::plateIron->getShiftedIndex(), 1, 0);
		armorPlayer.inventory.armorInventory[3] = ItemInstance(Items::helmetIron->getShiftedIndex(), 1, 0);
		ok &= expect(armorPlayer.inventory.getArmorValue() == 20, "full iron armor should report 20 beta armor points");
		ItemInstance &nearBreakBoots = armorPlayer.inventory.armorInventory[0];
		nearBreakBoots.itemDamage = nearBreakBoots.getMaxDamage() - 1;
		armorPlayer.inventory.hurtArmor(1);
		ok &= expect(!armorPlayer.inventory.armorInventory[0].isEmpty(), "armor should survive when damage reaches max exactly");
		ok &= expect(armorPlayer.inventory.armorInventory[0].itemDamage == armorPlayer.inventory.armorInventory[0].getMaxDamage(), "armor damage should stop at max before breaking");
		armorPlayer.inventory.hurtArmor(1);
		ok &= expect(armorPlayer.inventory.armorInventory[0].isEmpty(), "armor should only break after exceeding max damage");
		InspectablePlayer spillPlayer(level);
		spillPlayer.health = 20;
		spillPlayer.dmgSpill = 0;
		spillPlayer.inventory.armorInventory[0] = ItemInstance(Items::bootsDiamond->getShiftedIndex(), 1, 0);
		spillPlayer.inventory.armorInventory[1] = ItemInstance(Items::legsDiamond->getShiftedIndex(), 1, 0);
		spillPlayer.inventory.armorInventory[2] = ItemInstance(Items::plateDiamond->getShiftedIndex(), 1, 0);
		spillPlayer.inventory.armorInventory[3] = ItemInstance(Items::helmetDiamond->getShiftedIndex(), 1, 0);
		spillPlayer.applyArmorDamage(1);
		ok &= expect(spillPlayer.health == 20, "full armor should absorb the first point of damage into spill");
		ok &= expect(spillPlayer.dmgSpill == 5, "armor spill remainder should match beta formula");
		ok &= expect(spillPlayer.inventory.armorInventory[0].itemDamage == 1, "armor durability should consume the raw incoming damage");

		std::cerr << "block-smoke: pathfinding" << std::endl;
		TestLevelSource pathSource(level);
		for (int_t x = 259; x <= 266; ++x)
			pathSource.setTile(x, baseY + 1, 20, Tile::rock.id);
		Mob pathStart(level);
		Mob pathTarget(level);
		pathStart.setPos(260.5, baseY + 2.0, 20.5);
		pathTarget.setPos(265.5, baseY + 2.0, 20.5);
		auto openPath = Pathfinder(pathSource).createEntityPathTo(pathStart, pathTarget, 10.0f);
		ok &= expect(openPath != nullptr && openPath->pathLength > 1, "pathfinding should build a path across open ground");

		std::cerr << "block-smoke: projectiles" << std::endl;
		GridCraftingContainer bowRecipe{};
		bowRecipe.slots[1] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		bowRecipe.slots[2] = ItemInstance(Items::silk->getShiftedIndex(), 1, 0);
		bowRecipe.slots[3] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		bowRecipe.slots[5] = ItemInstance(Items::silk->getShiftedIndex(), 1, 0);
		bowRecipe.slots[7] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		bowRecipe.slots[8] = ItemInstance(Items::silk->getShiftedIndex(), 1, 0);
		ItemInstance bowOut = Recipes::getInstance().getItemFor(bowRecipe);
		ok &= expect(bowOut.itemID == Items::bow->getShiftedIndex(), "bow recipe should match beta");
		GridCraftingContainer arrowRecipe{};
		arrowRecipe.slots[0] = ItemInstance(Items::flint->getShiftedIndex(), 1, 0);
		arrowRecipe.slots[3] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		arrowRecipe.slots[6] = ItemInstance(Items::feather->getShiftedIndex(), 1, 0);
		ItemInstance arrowOut = Recipes::getInstance().getItemFor(arrowRecipe);
		ok &= expect(arrowOut.itemID == Items::arrow->getShiftedIndex() && arrowOut.stackSize == 4, "arrow recipe should match beta");
		GridCraftingContainer fishingRodRecipe{};
		fishingRodRecipe.slots[2] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		fishingRodRecipe.slots[4] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		fishingRodRecipe.slots[5] = ItemInstance(Items::silk->getShiftedIndex(), 1, 0);
		fishingRodRecipe.slots[6] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		fishingRodRecipe.slots[8] = ItemInstance(Items::silk->getShiftedIndex(), 1, 0);
		ItemInstance fishingRodOut = Recipes::getInstance().getItemFor(fishingRodRecipe);
		ok &= expect(fishingRodOut.itemID == Items::fishingRod->getShiftedIndex(), "fishing rod recipe should match beta");
		ok &= expect(Items::fishingRod->getMaxDamage() == 64 && Items::fishingRod->getMaxStackSize() == 1 &&
			Items::fishingRod->isFull3D() && Items::fishingRod->shouldRotateAroundWhenRendering(),
			"fishing rod properties should match beta");
		GridCraftingContainer paintingRecipe{};
		for (int_t slot = 0; slot < 9; ++slot)
			paintingRecipe.slots[slot] = ItemInstance(Items::stick->getShiftedIndex(), 1, 0);
		paintingRecipe.slots[4] = ItemInstance(Tile::wool.id, 1, 7);
		ItemInstance paintingOut = Recipes::getInstance().getItemFor(paintingRecipe);
		ok &= expect(paintingOut.itemID == Items::painting->getShiftedIndex(), "painting recipe should match beta");
		auto bowPlayer = std::make_shared<Player>(level);
		bowPlayer->setPos(222.5, baseY + 2.0, 20.5);
		ok &= expect(level.addEntity(bowPlayer), "bow player should join the level");
		bowPlayer->inventory.setItem(0, ItemInstance(Items::bow->getShiftedIndex(), 1, 0));
		bowPlayer->inventory.setItem(1, ItemInstance(Items::arrow->getShiftedIndex(), 2, 0));
		size_t entityCountBeforeShot = level.getAllEntities().size();
		bowPlayer->inventory.mainInventory[0].use(level, *bowPlayer);
		ok &= expect(bowPlayer->inventory.mainInventory[1].stackSize == 1, "bow use should consume one arrow");
		ok &= expect(level.getAllEntities().size() == entityCountBeforeShot + 1, "bow use should spawn an arrow entity");
		InspectableArrow pickupArrow(level, *bowPlayer);
		pickupArrow.plantForPickup();
		bowPlayer->inventory.setItem(1, ItemInstance());
		pickupArrow.playerTouch(*bowPlayer);
		ok &= expect(!bowPlayer->inventory.mainInventory[1].isEmpty() && bowPlayer->inventory.mainInventory[1].itemID == Items::arrow->getShiftedIndex(), "grounded player arrow should be picked back up");
		ok &= expect(pickupArrow.removed, "picked up arrow should remove itself");

		std::cerr << "block-smoke: fishing and paintings" << std::endl;
		auto fishingPlayer = std::make_shared<Player>(level);
		fishingPlayer->setPos(228.5, baseY + 2.0, 20.5);
		ok &= expect(level.addEntity(fishingPlayer), "fishing player should join the level");
		fishingPlayer->inventory.setItem(0, ItemInstance(Items::fishingRod->getShiftedIndex(), 1, 0));
		size_t entityCountBeforeCast = level.getAllEntities().size();
		fishingPlayer->inventory.mainInventory[0].use(level, *fishingPlayer);
		auto fishHook = fishingPlayer->fishEntity;
		ok &= expect(fishHook != nullptr && fishHook->angler == fishingPlayer.get() && !fishHook->removed,
			"casting should create an owned fishing hook for the player");
		ok &= expect(level.getAllEntities().size() == entityCountBeforeCast + 1,
			"casting should add one fishing hook entity to the level");
		fishHook->bobber = bowPlayer;
		double hookedMotionBefore = bowPlayer->xd * bowPlayer->xd + bowPlayer->yd * bowPlayer->yd + bowPlayer->zd * bowPlayer->zd;
		fishingPlayer->inventory.mainInventory[0].use(level, *fishingPlayer);
		double hookedMotionAfter = bowPlayer->xd * bowPlayer->xd + bowPlayer->yd * bowPlayer->yd + bowPlayer->zd * bowPlayer->zd;
		ok &= expect(fishingPlayer->fishEntity == nullptr && fishHook->removed,
			"retracting should remove the hook and clear player ownership");
		ok &= expect(fishingPlayer->inventory.mainInventory[0].itemDamage == 3 && hookedMotionAfter > hookedMotionBefore,
			"retracting a hooked entity should damage the rod by three and pull the entity");

		for (int_t wallX = 232; wallX <= 239; ++wallX)
			for (int_t wallY = baseY + 1; wallY <= baseY + 8; ++wallY)
				level.setTile(wallX, wallY, 30, Tile::rock.id);
		ItemInstance paintingStack(Items::painting->getShiftedIndex(), 1, 0);
		size_t entityCountBeforePainting = level.getAllEntities().size();
		ok &= expect(paintingStack.useOn(*fishingPlayer, level, 235, baseY + 4, 30, Facing::NORTH),
			"painting should be usable on a wall face");
		std::shared_ptr<EntityPainting> placedPainting;
		for (const auto &entity : level.getAllEntities())
		{
			auto candidate = std::dynamic_pointer_cast<EntityPainting>(entity);
			if (candidate != nullptr && !candidate->removed)
			{
				placedPainting = candidate;
				break;
			}
		}
		ok &= expect(paintingStack.stackSize == 0 && placedPainting != nullptr && placedPainting->direction == 0 && placedPainting->survives(),
			"painting placement should consume the item and create a surviving north-facing painting");
		ok &= expect(level.getAllEntities().size() == entityCountBeforePainting + 1,
			"painting placement should add one entity to the level");
		size_t paintingDropsBefore = 0;
		for (const auto &entity : level.getAllEntities())
		{
			auto item = std::dynamic_pointer_cast<EntityItem>(entity);
			if (item != nullptr && item->item.itemID == Items::painting->getShiftedIndex())
				paintingDropsBefore++;
		}
		if (placedPainting != nullptr)
			placedPainting->hurt(fishingPlayer.get(), 0);
		size_t paintingDropsAfter = 0;
		for (const auto &entity : level.getAllEntities())
		{
			auto item = std::dynamic_pointer_cast<EntityItem>(entity);
			if (item != nullptr && item->item.itemID == Items::painting->getShiftedIndex())
				paintingDropsAfter++;
		}
		ok &= expect(placedPainting != nullptr && placedPainting->removed && paintingDropsAfter == paintingDropsBefore + 1,
			"breaking a painting should remove it and drop exactly one painting item");

		std::cerr << "block-smoke: wolf entity events and shaking" << std::endl;
		Wolf statusWolf(level);
		statusWolf.interpolateOnly = true;
		statusWolf.setPos(244.5, baseY + 2.0, 20.5);
		statusWolf.onGround = true;
		listener.clear();
		statusWolf.handleEntityEvent(7);
		ok &= expect(listener.particles.size() == 7,
			"wolf tame success event should spawn seven heart particles");
		for (const auto &particle : listener.particles)
			ok &= expect(particle == u"heart", "wolf tame success event should only spawn heart particles");
		listener.clear();
		statusWolf.handleEntityEvent(6);
		ok &= expect(listener.particles.size() == 7,
			"wolf tame failure event should spawn seven smoke particles");
		for (const auto &particle : listener.particles)
			ok &= expect(particle == u"smoke", "wolf tame failure event should only spawn smoke particles");
		listener.clear();
		statusWolf.handleEntityEvent(8);
		for (int_t tick = 0; tick < 10; ++tick)
			statusWolf.tick();
		int_t shakeSounds = 0;
		int_t splashParticles = 0;
		for (const auto &sound : listener.sounds)
			if (sound == u"mob.wolf.shake")
				shakeSounds++;
		for (const auto &particle : listener.particles)
			if (particle == u"splash")
				splashParticles++;
		ok &= expect(shakeSounds == 1, "wolf shake event should play its sound once");
		ok &= expect(splashParticles == 3,
			"wolf shake should emit the beta splash counts through its first ten ticks");
		ok &= expect(std::abs(statusWolf.getShakeAngle(1.0f, 0.0f)) > 0.001f,
			"wolf shake event should advance the model shake angle");
		WolfModel statusWolfModel;
		statusWolfModel.prepare(statusWolf, 0.0f, 0.0f, 1.0f);
		ok &= expect(std::abs(statusWolfModel.head.zRot) > 0.001f &&
			std::abs(statusWolfModel.mane.zRot) > 0.001f &&
			std::abs(statusWolfModel.body.zRot) > 0.001f &&
			std::abs(statusWolfModel.tail.zRot) > 0.001f,
			"wolf model should apply the beta shake offsets to head, mane, body, and tail");
		for (int_t tick = 0; tick < 32; ++tick)
			statusWolf.tick();
		ok &= expect(statusWolf.getShakeAngle(1.0f, 0.0f) == 0.0f,
			"wolf shake state should reset after two seconds");
		
		std::cerr << "block-smoke: validation mobs" << std::endl;
		InspectableCow cow(level);
		Player bucketPlayer(level);
		bucketPlayer.inventory.setItem(bucketPlayer.inventory.currentItem, ItemInstance(Items::bucketEmpty->getShiftedIndex(), 1, 0));
		ok &= expect(cow.interact(bucketPlayer), "cow should accept an empty bucket");
		ok &= expect(bucketPlayer.inventory.mainInventory[0].itemID == Items::bucketMilk->getShiftedIndex(), "cow should return a milk bucket");
		ok &= expect(cow.getDeathLoot() == Items::leather->getShiftedIndex(), "cow should drop leather");
		int_t oldDifficulty = level.difficulty;
		level.difficulty = 2;
		auto zombieTarget = std::make_shared<Player>(level);
		zombieTarget->setPos(272.5, baseY + 2.0, 20.5);
		zombieTarget->health = 20;
		ok &= expect(level.addEntity(zombieTarget), "zombie target player should join the level");
		InspectableZombie zombie(level);
		zombie.setPos(271.5, baseY + 2.0, 20.5);
		auto acquiredTarget = zombie.findAttackTarget();
		ok &= expect(acquiredTarget.get() == zombieTarget.get(), "zombie should acquire a nearby visible player target");
		zombie.checkHurtTarget(*zombieTarget, 1.5f);
		ok &= expect(zombie.attackTime == 20, "zombie melee should set a 20 tick cooldown");
		ok &= expect(zombieTarget->health == 15, "zombie melee should deal five damage at normal difficulty");
		level.difficulty = 0;
		zombie.tick();
		ok &= expect(zombie.removed, "zombie should despawn immediately on peaceful difficulty");
		level.difficulty = oldDifficulty;

		auto saddledPig = std::make_shared<InspectablePig>(level);
		saddledPig->setPos(224.5, baseY + 2.0, 20.5);
		ok &= expect(level.addEntity(saddledPig), "pig should join the level for riding tests");
		auto pigPlayer = std::make_shared<Player>(level);
		pigPlayer->setPos(224.5, baseY + 2.0, 21.5);
		pigPlayer->inventory.setItem(pigPlayer->inventory.currentItem, ItemInstance(Items::saddle->getShiftedIndex(), 1, 0));
		ok &= expect(level.addEntity(pigPlayer), "pig rider player should join the level");
		pigPlayer->interact(std::static_pointer_cast<Entity>(saddledPig));
		ok &= expect(saddledPig->isSaddled(), "pig should become saddled from the beta saddle item interaction");
		ok &= expect(pigPlayer->inventory.mainInventory[0].isEmpty(), "saddle item should be consumed when pig becomes saddled");
		pigPlayer->interact(std::static_pointer_cast<Entity>(saddledPig));
		ok &= expect(pigPlayer->riding.get() == saddledPig.get(), "saddled pig should mount the player on interact");
		pigPlayer->ride(nullptr);

		InspectableSheep sheep(level);
		sheep.setPos(226.5, baseY + 2.0, 20.5);
		pigPlayer->inventory.setItem(pigPlayer->inventory.currentItem, ItemInstance(Items::shears->getShiftedIndex(), 1, 0));
		size_t entityCountBeforeShear = level.getAllEntities().size();
		sheep.interact(*pigPlayer);
		ok &= expect(sheep.isSheared(), "sheep should become sheared after using shears");
		ok &= expect(pigPlayer->inventory.mainInventory[0].itemDamage == 1, "shears should lose one durability when shearing a sheep");
		ok &= expect(level.getAllEntities().size() >= entityCountBeforeShear + 2, "shearing should spawn multiple wool item entities");

		InspectableChicken chicken(level);
		chicken.setPos(228.5, baseY + 2.0, 20.5);
		chicken.eggTime = 1;
		size_t entityCountBeforeEgg = level.getAllEntities().size();
		chicken.aiStep();
		ok &= expect(chicken.eggTime > 1, "chicken should reset its egg timer after laying an egg");
		ok &= expect(level.getAllEntities().size() == entityCountBeforeEgg + 1, "chicken egg timer should spawn an egg item entity");

		std::cerr << "block-smoke: multiplayer weather presentation" << std::endl;
		ok &= expect(!level.getBiomeSource().getBiomeInfo(BiomeId::Taiga).canSpawnLightningBolt() &&
			level.getBiomeSource().getBiomeInfo(BiomeId::Plains).canSpawnLightningBolt(),
			"snow biomes must not also render rain or spawn rain particles");
		level.lastLightningBolt = 0;
		level.setRainStrength(0.0f);
		Vec3 *drySky = level.getSkyColor(player, 1.0f);
		double drySkyR = drySky->x;
		double drySkyG = drySky->y;
		double drySkyB = drySky->z;
		Vec3 *dryCloud = level.getCloudColor(1.0f);
		double dryCloudR = dryCloud->x;
		double dryCloudG = dryCloud->y;
		double dryCloudB = dryCloud->z;
		level.setRainStrength(0.625f);
		ok &= expect(level.getRainStrength(0.0f) == 0.625f &&
			level.getRainStrength(0.5f) == 0.625f && level.getRainStrength(1.0f) == 0.625f,
			"multiplayer rain packets should set both interpolation endpoints immediately");
		level.setRainStrength(1.0f);
		ok &= expect(level.isRaining(), "full rain strength should satisfy Beta's 0.2 rain threshold");
		Vec3 *wetSky = level.getSkyColor(player, 1.0f);
		double rainGray = (drySkyR * 0.3 + drySkyG * 0.59 + drySkyB * 0.11) * 0.6;
		ok &= expect(std::abs(wetSky->x - (drySkyR * 0.25 + rainGray * 0.75)) < 1.0e-6 &&
			std::abs(wetSky->y - (drySkyG * 0.25 + rainGray * 0.75)) < 1.0e-6 &&
			std::abs(wetSky->z - (drySkyB * 0.25 + rainGray * 0.75)) < 1.0e-6,
			"rain should apply Beta's exact sky-color grayscale blend");
		Vec3 *wetCloud = level.getCloudColor(1.0f);
		ok &= expect(wetCloud->x != dryCloudR || wetCloud->y != dryCloudG || wetCloud->z != dryCloudB,
			"rain should attenuate cloud color");
		level.setRainStrength(0.0f);
		ok &= expect(!level.isRaining(), "zero rain strength should clear Beta's rain threshold immediately");

		ok &= expect(level.setTile(10, 127, 20, Tile::glass.id), "weather top-solid test block should place");
		int_t weatherTop = level.findTopSolidBlock(10, 20);
		ok &= expect(weatherTop == 128,
			"rain placement should scan solid and liquid materials from y=127 like Beta");
		level.setTile(10, 127, 20, 0);

		InspectableRainParticle rainParticle(level, 341.5, 125.0, 20.5);
		ok &= expect(rainParticle.textureIndex() >= 19 && rainParticle.textureIndex() <= 22,
			"rain particle should select one of Beta's four rain sprites");
		ok &= expect(rainParticle.bbWidth == 0.01f && rainParticle.bbHeight == 0.01f &&
			rainParticle.particleGravity() == 0.06f,
			"rain particle should use Beta's exact size and gravity");
		int_t rainLifetime = rainParticle.remainingLifetime();
		double rainXd = rainParticle.xd;
		double rainYd = rainParticle.yd;
		double rainZd = rainParticle.zd;
		rainParticle.tick();
		ok &= expect(rainParticle.remainingLifetime() == rainLifetime - 1 &&
			std::abs(rainParticle.xd - rainXd * 0.98f) < 1.0e-9 &&
			std::abs(rainParticle.yd - (rainYd - 0.06f) * 0.98f) < 1.0e-9 &&
			std::abs(rainParticle.zd - rainZd * 0.98f) < 1.0e-9,
			"rain particle tick should apply Beta's gravity, friction, and countdown order");

		std::cerr << "block-smoke: lightning and spawn commands" << std::endl;
		InspectablePig lightningPig(level);
		lightningPig.setPos(232.5, baseY + 2.0, 20.5);
		size_t entitiesBeforeLightningPig = level.getAllEntities().size();
		EntityLightningBolt lightning(level, lightningPig.x, lightningPig.y, lightningPig.z);
		level.lastLightningBolt = 0;
		Vec3 *normalSky = level.getSkyColor(lightning, 1.0f);
		double normalSkyR = normalSky->x;
		double normalSkyG = normalSky->y;
		double normalSkyB = normalSky->z;
		lightning.tick();
		ok &= expect(level.lastLightningBolt == 2, "active lightning should set the world's sky-flash counter");
		Vec3 *flashedSky = level.getSkyColor(lightning, 1.0f);
		ok &= expect(std::abs(flashedSky->x - (normalSkyR * 0.55 + 0.8 * 0.45)) < 1.0e-6 &&
			std::abs(flashedSky->y - (normalSkyG * 0.55 + 0.8 * 0.45)) < 1.0e-6 &&
			std::abs(flashedSky->z - (normalSkyB * 0.55 + 0.45)) < 1.0e-6,
			"lightning sky flash should use Beta 1.7.3's exact color blend");
		ok &= expect(EntityIO::newEntity(48, level) == nullptr,
			"abstract numeric Mob ID 48 should fail construction like Beta's reflective EntityList");
		lightningPig.onStruckByLightning(lightning);
		ok &= expect(lightningPig.removed, "pig struck by lightning should be removed");
		ok &= expect(level.getAllEntities().size() == entitiesBeforeLightningPig + 1, "pig lightning should spawn a pig zombie");
		bool sawPigZombie = false;
		for (const auto &entity : level.getAllEntities())
			if (dynamic_cast<PigZombie *>(entity.get()) != nullptr)
				sawPigZombie = true;
		ok &= expect(sawPigZombie, "pig lightning should create a pig zombie entity");
		InspectableCreeper poweredCreeper(level);
		poweredCreeper.onStruckByLightning(lightning);
		ok &= expect(poweredCreeper.isPowered(), "creeper struck by lightning should become powered");
		auto skeletonTarget = std::make_shared<Player>(level);
		skeletonTarget->setPos(236.5, baseY + 2.0, 20.5);
		ok &= expect(level.addEntity(skeletonTarget), "skeleton target should join the level");
		InspectableSkeleton skeleton(level);
		skeleton.setPos(233.5, baseY + 2.0, 20.5);
		size_t entitiesBeforeSkeletonShot = level.getAllEntities().size();
		skeleton.checkHurtTarget(*skeletonTarget, skeleton.distanceTo(*skeletonTarget));
		ok &= expect(level.getAllEntities().size() == entitiesBeforeSkeletonShot + 1, "skeleton should fire an arrow at range");
		ok &= expect(skeleton.getDeathLoot() == Items::arrow->getShiftedIndex(), "skeleton should drop arrows as its base loot");
		ok &= expect(chicken.getDeathLoot() == Items::feather->getShiftedIndex(), "chicken should drop feathers");

		InspectableSpider spider(level);
		spider.setPos(230.5, baseY + 2.0, 20.5);
		spider.horizontalCollision = true;
		ok &= expect(spider.onLadder(), "spider should treat horizontal collision as climbable like beta");
		ok &= expectNear(spider.getRideHeight(), 0.175, 1.0e-6, "spider rider offset should match beta");
		ok &= expect(spider.getDeathLoot() == Items::silk->getShiftedIndex(), "spider should drop string");

		InspectableCreeper creeper(level);
		creeper.setPos(282.5, baseY + 2.0, 20.5);
		Player creeperTarget(level);
		creeperTarget.setPos(283.5, baseY + 2.0, 20.5);
		for (int_t i = 0; i < 30 && !creeper.removed; ++i)
			creeper.checkHurtTarget(creeperTarget, 2.5f);
		ok &= expect(creeper.removed, "creeper should explode and remove itself after a full fuse near a target");
		ok &= expect(creeper.getDeathLoot() == Items::gunpowder->getShiftedIndex(), "creeper should drop gunpowder");
		


		std::cerr << "block-smoke: natural spawning" << std::endl;
		level.setSpawnSettings(true, true);
		for (int_t i = 0; i < 200; ++i)
			level.tick();
		bool foundNaturalAnimal = false;
		bool foundNaturalMonster = false;
		for (const auto &entity : level.getAllEntities())
		{
			if (!foundNaturalAnimal && dynamic_cast<Animal *>(entity.get()) != nullptr)
				foundNaturalAnimal = true;
			if (!foundNaturalMonster && dynamic_cast<Monster *>(entity.get()) != nullptr)
				foundNaturalMonster = true;
		}
		ok &= expect(foundNaturalAnimal, "natural spawning should add passive animals");
		ok &= expect(foundNaturalMonster, "natural spawning should add hostile monsters");


		std::cerr << "block-smoke: food items" << std::endl;
		player.health = 10;
		ItemInstance breadStack(Items::bread->getShiftedIndex(), 1, 0);
		breadStack.use(level, player);
		ok &= expect(player.health == 15, "bread should heal five points on use");
		ok &= expect(breadStack.isEmpty(), "bread should be consumed on use");
		player.health = 10;
		ItemInstance cookieStack(Items::cookie->getShiftedIndex(), 2, 0);
		cookieStack.use(level, player);
		ok &= expect(player.health == 11, "cookie should heal one point on use");
		ok &= expect(cookieStack.stackSize == 1, "cookie should consume one item per use");
		player.health = 20;

		std::cerr << "block-smoke: locked chest" << std::endl;
		ok &= expect(level.setTile(220, baseY + 1, 10, Tile::lockedChest.id), "locked chest should place");
		level.setTile(220, baseY + 1, 9, 1);
		ok &= expect(Tile::lockedChest.getTexture(level, 220, baseY + 1, 10, Facing::SOUTH) == 27, "locked chest front should face away from a north obstruction");
		Tile::lockedChest.tick(level, 220, baseY + 1, 10, random);
		ok &= expect(level.getTile(220, baseY + 1, 10) == 0, "locked chest should disappear when ticked");

		std::cerr << "block-smoke: chest" << std::endl;
		ok &= expect(level.setTile(224, baseY + 1, 10, Tile::chest.id), "chest should place");
		auto chestEntity = std::dynamic_pointer_cast<ChestTileEntity>(level.getTileEntity(224, baseY + 1, 10));
		ok &= expect(chestEntity != nullptr, "chest should create a chest tile entity");
		if (chestEntity != nullptr)
		{
			chestEntity->setItem(0, ItemInstance(Items::stick->getShiftedIndex(), 32, 0));
			ItemInstance removedChestItem = chestEntity->removeItem(0, 12);
			ok &= expect(removedChestItem.stackSize == 12, "chest should remove the requested item count");
			ok &= expect(chestEntity->getItem(0).stackSize == 20, "chest should keep the remaining stack after removal");
		}
		ok &= expect(level.setTile(225, baseY + 1, 10, Tile::chest.id), "double chest partner should place");
		ok &= expect(!Tile::chest.mayPlace(level, 226, baseY + 1, 10), "third chest in a row should be rejected");
		auto chestCart = std::make_shared<EntityMinecart>(level, 226.5, baseY + 1.0, 10.5, EntityMinecart::TYPE_CHEST);
		chestCart->setItem(0, ItemInstance(Items::coal->getShiftedIndex(), 8, 0));
		ItemInstance removedChestCartItem = chestCart->removeItem(0, 3);
		ok &= expect(removedChestCartItem.stackSize == 3, "chest minecart should remove the requested item count");
		ok &= expect(chestCart->getItem(0).stackSize == 5, "chest minecart should retain the remaining stack");

		std::cerr << "block-smoke: fire and portal" << std::endl;
		ok &= expect(Tile::fire.getRenderShape() == Tile::SHAPE_FIRE, "fire should use the beta fire render shape");
		ok &= expect(Tile::lightEmission[51] == 15, "fire should emit full beta light");
		ok &= expect(Tile::portal.getRenderLayer() == 1, "portal should render in the translucent layer");
		ok &= expect(Tile::lightEmission[90] == 11, "portal should use the beta portal light level");
		ok &= expect(Items::flintAndSteel->getShiftedIndex() == 259, "flint and steel should use the beta shifted id 259");
		ItemInstance flintAndSteel(Items::flintAndSteel->getShiftedIndex(), 1, 0);
		level.setTile(214, baseY, 10, 1);
		listener.clear();
		ok &= expect(flintAndSteel.useOn(player, level, 214, baseY, 10, Facing::UP), "flint and steel should be usable on a block top face");
		ok &= expect(level.getTile(214, baseY + 1, 10) == Tile::fire.id, "flint and steel should place fire above a solid block");
		ok &= expect(flintAndSteel.getAuxValue() == 1, "flint and steel should take one point of durability on use");
		AABB fireBox(214.0, static_cast<double>(baseY + 1), 10.0, 215.0, static_cast<double>(baseY + 2), 11.0);
		ok &= expect(level.containsFireTile(fireBox), "level fire query should detect fire blocks");
		listener.clear();
		level.extinguishFire(214, baseY, 10, Facing::UP);
		ok &= expect(level.getTile(214, baseY + 1, 10) == 0, "extinguishFire should remove adjacent fire");
		ok &= expect(!listener.sounds.empty() && listener.sounds.back() == u"random.fizz", "extinguishFire should play the fizz sound");
		level.setTile(215, baseY + 1, 10, Tile::lava.id);
		AABB lavaBox(215.0, static_cast<double>(baseY + 1), 10.0, 216.0, static_cast<double>(baseY + 2), 11.0);
		ok &= expect(level.containsFireTile(lavaBox), "level fire query should treat lava as burning");
		level.setTile(215, baseY + 1, 10, 0);
		for (int_t frameY = baseY; frameY <= baseY + 4; ++frameY)
		{
			level.setTile(216, frameY, 10, Tile::obsidian.id);
			level.setTile(219, frameY, 10, Tile::obsidian.id);
		}
		for (int_t frameX = 216; frameX <= 219; ++frameX)
		{
			level.setTile(frameX, baseY, 10, Tile::obsidian.id);
			level.setTile(frameX, baseY + 4, 10, Tile::obsidian.id);
		}
		for (int_t innerX = 217; innerX <= 218; ++innerX)
			for (int_t innerY = baseY + 1; innerY <= baseY + 3; ++innerY)
				level.setTile(innerX, innerY, 10, 0);
		ItemInstance portalIgniter(Items::flintAndSteel->getShiftedIndex(), 1, 0);
		ok &= expect(portalIgniter.useOn(player, level, 217, baseY, 10, Facing::UP), "flint and steel should ignite a portal frame interior");
		ok &= expect(level.getTile(217, baseY + 1, 10) == Tile::portal.id, "igniting a valid obsidian frame should spawn portal blocks");
		ok &= expect(level.getTile(218, baseY + 3, 10) == Tile::portal.id, "portal creation should fill the full 2x3 interior");
		level.setTile(216, baseY + 2, 10, 0);
		ok &= expect(level.getTile(217, baseY + 2, 10) == 0, "portal should break when its obsidian frame is interrupted");
		TextureFlamesFX flamesLower(0);
		bool flamesVisible = false;
		for (int_t tick = 0; tick < 8 && !flamesVisible; ++tick)
		{
			flamesLower.onTick();
			for (int_t pixel = 0; pixel < 256; ++pixel)
			{
				if (flamesLower.imageData[pixel * 4 + 3] != 0)
				{
					flamesVisible = true;
					break;
				}
			}
		}
		ok &= expect(flamesLower.iconIndex == Tile::fire.tex, "lower flame texture fx should target the base fire icon");
		ok &= expect(flamesVisible, "flame texture fx should generate visible pixels after a few ticks");
		TextureFlamesFX flamesUpper(1);
		ok &= expect(flamesUpper.iconIndex == Tile::fire.tex + 16, "upper flame texture fx should target the second fire icon");
		TexturePortalFX portalFx(14);
		portalFx.onTick();
		ok &= expect(portalFx.iconIndex == 14, "portal texture fx should target the beta portal icon");
		ok &= expect(portalFx.imageData[3] != 0, "portal texture fx should generate portal pixels after a tick");
		std::cerr << "block-smoke: lighting" << std::endl;
		level.setTile(10, baseY, 0, 1);
		ok &= expect(level.setTile(10, baseY + 1, 0, 53), "wood stair should place");
		level.setTile(10, baseY + 2, 0, 1);
		int_t eastBrightness = level.getRawBrightness(11, baseY + 1, 0, false);
		ok &= expect(level.getRawBrightness(10, baseY + 1, 0) == eastBrightness, "stairs should sample neighboring brightness like beta");
		level.setTile(12, baseY + 1, 0, 1);
		ok &= expect(level.setTile(12, baseY + 1, 1, 65), "ladder should place");
		level.setData(12, baseY + 1, 1, 3);
		int_t frontBrightness = level.getRawBrightness(12, baseY + 1, 2, false);
		ok &= expect(level.getRawBrightness(12, baseY + 1, 1) == frontBrightness, "ladder should stay transparent for brightness sampling");


		std::cerr << "block-smoke: rails" << std::endl;
		for (int_t x = 230; x <= 241; ++x)
			level.setTile(x, baseY, 0, 1);
		ok &= expect(level.setTile(230, baseY + 1, 0, Tile::rail.id), "first rail should place");
		ok &= expect(level.setTile(231, baseY + 1, 0, Tile::rail.id), "second rail should place");
		ok &= expect(level.getData(230, baseY + 1, 0) == 1, "first straight rail should orient east-west");
		ok &= expect(level.getData(231, baseY + 1, 0) == 1, "second straight rail should orient east-west");
		level.setTile(240, baseY + 1, 0, 1);
		level.setTile(241, baseY + 1, 0, 1);
		ok &= expect(level.setTile(239, baseY + 1, 0, Tile::lever.id), "powered rail lever should place");
		level.setData(239, baseY + 1, 0, 2);
		ok &= expect(level.setTile(240, baseY + 2, 0, Tile::railPowered.id), "source powered rail should place");
		ok &= expect(level.setTile(241, baseY + 2, 0, Tile::railPowered.id), "neighbor powered rail should place");
		ok &= expect(Tile::lever.use(level, 239, baseY + 1, 0, player), "powered rail lever should toggle on");
		Tile::railPowered.neighborChanged(level, 240, baseY + 2, 0, Tile::lever.id);
		Tile::railPowered.neighborChanged(level, 241, baseY + 2, 0, Tile::lever.id);
		ok &= expect((level.getData(240, baseY + 2, 0) & 8) != 0, "source powered rail should become powered from a lever-fed block");
		ok &= expect((level.getData(241, baseY + 2, 0) & 8) != 0, "powered rail should recursively power from an adjacent externally-powered rail");
		ok &= expect(Tile::lever.use(level, 239, baseY + 1, 0, player), "powered rail lever should toggle off");
		Tile::railPowered.neighborChanged(level, 240, baseY + 2, 0, Tile::lever.id);
		Tile::railPowered.neighborChanged(level, 241, baseY + 2, 0, Tile::lever.id);
		ok &= expect((level.getData(241, baseY + 2, 0) & 8) == 0, "powered rail should depower when the source rail depowers");
		level.setTile(250, baseY, 0, 1);
		ok &= expect(level.setTile(250, baseY + 1, 0, Tile::railDetector.id), "detector rail should place");
		auto detectorCart = std::make_shared<EntityMinecart>(level, 250.5, baseY + 1.0, 0.5, EntityMinecart::TYPE_RIDEABLE);
		ok &= expect(level.addEntity(detectorCart), "detector rail minecart should join the level");
		Tile::railDetector.entityInside(level, 250, baseY + 1, 0, *detectorCart);
		ok &= expect((level.getData(250, baseY + 1, 0) & 8) != 0, "detector rail should power when a minecart sits on it");
		level.removeEntityImmediately(detectorCart);
		Tile::railDetector.tick(level, 250, baseY + 1, 0, random);
		ok &= expect((level.getData(250, baseY + 1, 0) & 8) == 0, "detector rail should depower when rechecked with no minecart");
		for (int_t x = 252; x <= 260; ++x)
			level.setTile(x, baseY, 0, 1);
		
		level.setTile(252, baseY + 1, 0, Tile::torchRedstoneActive.id);
		level.setData(252, baseY + 1, 0, 5);
		for (int_t x = 253; x <= 260; ++x)
			ok &= expect(level.setTileAndData(x, baseY + 1, 0, Tile::railPowered.id, 1), "powered rail launch segment should place");
		for (int_t x = 253; x <= 260; ++x)
			Tile::railPowered.neighborChanged(level, x, baseY + 1, 0, Tile::torchRedstoneActive.id);
		ok &= expect((level.getData(253, baseY + 1, 0) & 8) != 0, "powered rail adjacent to torch should be powered");
		auto launchCart = std::make_shared<EntityMinecart>(level, 253.5, baseY + 1.0, 0.5, EntityMinecart::TYPE_RIDEABLE);
		launchCart->xd = 0.02;
		for (int_t i = 0; i < 60; ++i)
			launchCart->tick();
		ok &= expect(launchCart->x > 254.0, "minecart should accelerate along powered rails");

		std::cerr << "block-smoke: rideable minecart mount toggles" << std::endl;
		auto rideCart = std::make_shared<EntityMinecart>(level, 250.5, baseY + 1.0, 2.5, EntityMinecart::TYPE_RIDEABLE);
		ok &= expect(level.addEntity(rideCart), "rideable minecart should join the level");
		const double beforeMountX = mountPlayer->x;
		const double beforeMountY = mountPlayer->y;
		const double beforeMountZ = mountPlayer->z;
		ok &= expect(rideCart->interact(*mountPlayer), "rideable minecart should mount on first interaction");
		double mountedY = rideCart->y + rideCart->getRideHeight() + mountPlayer->getRidingHeight();
		ok &= expect(mountPlayer->riding == rideCart, "minecart interact should set the player's riding pointer");
		ok &= expect(rideCart->rider == mountPlayer, "minecart interact should set the cart rider pointer");
		ok &= expect(mountPlayer->x == beforeMountX && mountPlayer->y == beforeMountY && mountPlayer->z == beforeMountZ,
			"mounting should leave position unchanged until rider positioning");
		rideCart->positionRider();
		ok &= expect(mountPlayer->x == rideCart->x && mountPlayer->y == mountedY && mountPlayer->z == rideCart->z,
			"rider positioning should place the player at the cart seat");
		rideCart->xd = 0.0;
		rideCart->zd = 0.0;
		mountPlayer->xd = 0.0;
		mountPlayer->zd = 0.0;
		rideCart->push(*mountPlayer);
		mountPlayer->push(*rideCart);
		ok &= expectNear(rideCart->xd, 0.0, 1.0e-9, "mounted rider collisions should not push the minecart");
		ok &= expectNear(rideCart->zd, 0.0, 1.0e-9, "mounted rider collisions should not push the minecart sideways");
		ok &= expectNear(mountPlayer->xd, 0.0, 1.0e-9, "mounted vehicle collisions should not push the rider");
		ok &= expectNear(mountPlayer->zd, 0.0, 1.0e-9, "mounted vehicle collisions should not push the rider sideways");
		ok &= expect(rideCart->interact(*mountPlayer), "rideable minecart should dismount on second interaction");
		double dismountedY = rideCart->bb.y0 + rideCart->bbHeight + mountPlayer->heightOffset;
		ok &= expect(mountPlayer->riding == nullptr, "second minecart interaction should clear the player's riding pointer");
		ok &= expect(rideCart->rider == nullptr, "second minecart interaction should clear the cart rider pointer");
		ok &= expectNear(mountPlayer->x, rideCart->x, 1.0e-6, "dismounted player should stay aligned with the cart x");
		ok &= expectNear(mountPlayer->y, dismountedY, 1.0e-6, "dismounted player should be placed above the cart");
		ok &= expectNear(mountPlayer->bb.y0, rideCart->bb.y0 + rideCart->bbHeight, 1.0e-6, "dismounted player feet should rest on top of the cart");
		ok &= expectNear(mountPlayer->z, rideCart->z, 1.0e-6, "dismounted player should stay aligned with the cart z");
		level.removeEntityImmediately(rideCart);

		level.setTile(20, baseY + 1, 0, 0);
		std::cerr << "block-smoke: lever helper connectivity" << std::endl;
		level.setTile(18, baseY, 0, 1);
		ok &= expect(level.setTile(18, baseY + 1, 0, Tile::lever.id), "lever helper source should place");
		level.setData(18, baseY + 1, 0, 5 | 8);
		ok &= expect(RedStoneDustTile::isPowerProviderOrWire(level, 18, baseY + 1, 0, 5), "dust helper should treat powered lever as a power provider");
		
		std::cerr << "block-smoke: redstone placement" << std::endl;
		level.setTile(20, baseY, 0, 1);
		ItemInstance redstoneDust(Items::redstone->getShiftedIndex(), 1, 0);
		ok &= expect(redstoneDust.useOn(player, level, 20, baseY, 0, Facing::UP), "redstone item should be usable on a block top face");
		ok &= expect(level.getTile(20, baseY + 1, 0) == Tile::redstoneWire.id, "redstone item should place redstone wire");
		ok &= expect(redstoneDust.stackSize == 0, "redstone item should consume one dust on placement");

		std::cerr << "block-smoke: live lever wire growth" << std::endl;
		for (int_t x = 60; x <= 63; ++x)
			level.setTile(x, baseY, 0, 1);
		ok &= expect(level.setTile(60, baseY + 1, 0, Tile::lever.id), "live-growth lever source should place");
		level.setData(60, baseY + 1, 0, 5);
		ok &= expect(Tile::lever.use(level, 60, baseY + 1, 0, player), "live-growth lever source should toggle on");
		for (int_t x = 61; x <= 63; ++x)
		{
			ItemInstance growthDust(Items::redstone->getShiftedIndex(), 1, 0);
			ok &= expect(growthDust.useOn(player, level, x, baseY, 0, Facing::UP), "live-growth dust should place on a powered line");
			ok &= expect(level.getData(61, baseY + 1, 0) == 15, "live-growth first wire should stay powered");
			if (x >= 62)
				ok &= expect(level.getData(62, baseY + 1, 0) == 14, "live-growth second wire should power when added");
			if (x >= 63)
				ok &= expect(level.getData(63, baseY + 1, 0) == 13, "live-growth third wire should power when added");
		}

		std::cerr << "block-smoke: live corner wire growth" << std::endl;
		level.setTile(70, baseY, 0, 1);
		level.setTile(71, baseY, 0, 1);
		level.setTile(72, baseY, 0, 1);
		level.setTile(72, baseY, 1, 1);
		level.setTile(72, baseY, 2, 1);
		ok &= expect(level.setTile(70, baseY + 1, 0, Tile::lever.id), "live-corner lever source should place");
		level.setData(70, baseY + 1, 0, 5);
		ok &= expect(Tile::lever.use(level, 70, baseY + 1, 0, player), "live-corner lever source should toggle on");
		{
			ItemInstance firstDust(Items::redstone->getShiftedIndex(), 1, 0);
			ok &= expect(firstDust.useOn(player, level, 71, baseY, 0, Facing::UP), "live-corner first dust should place");
			ok &= expect(level.getData(71, baseY + 1, 0) == 15, "live-corner first wire should power at strength 15");
		}
		{
			ItemInstance secondDust(Items::redstone->getShiftedIndex(), 1, 0);
			ok &= expect(secondDust.useOn(player, level, 72, baseY, 0, Facing::UP), "live-corner second dust should place");
			ok &= expect(level.getData(71, baseY + 1, 0) == 15, "live-corner first wire should stay powered before the turn");
			ok &= expect(level.getData(72, baseY + 1, 0) == 14, "live-corner second wire should power at strength 14");
		}
		{
			ItemInstance cornerDust(Items::redstone->getShiftedIndex(), 1, 0);
			ok &= expect(cornerDust.useOn(player, level, 72, baseY, 1, Facing::UP), "live-corner turn dust should place");
			ok &= expect(level.getData(71, baseY + 1, 0) == 15, "live-corner first wire should stay powered after the turn");
			ok &= expect(level.getData(72, baseY + 1, 0) == 14, "live-corner second wire should stay powered after the turn");
			ok &= expect(level.getData(72, baseY + 1, 1) == 13, "live-corner turn dust should power at strength 13");
		}
		{
			ItemInstance tailDust(Items::redstone->getShiftedIndex(), 1, 0);
			ok &= expect(tailDust.useOn(player, level, 72, baseY, 2, Facing::UP), "live-corner tail dust should place");
			ok &= expect(level.getData(72, baseY + 1, 1) == 13, "live-corner turn dust should stay powered when extended");
			ok &= expect(level.getData(72, baseY + 1, 2) == 12, "live-corner tail dust should power at strength 12");
		}

		std::cerr << "block-smoke: corner wire propagation" << std::endl;
		level.setTile(80, baseY, 0, 1);
		level.setTile(81, baseY, 0, 1);
		level.setTile(82, baseY, 0, 1);
		level.setTile(82, baseY, 1, 1);
		level.setTile(82, baseY, 2, 1);
		ok &= expect(level.setTile(80, baseY + 1, 0, Tile::lever.id), "corner lever source should place");
		level.setData(80, baseY + 1, 0, 5);
		ok &= expect(level.setTile(81, baseY + 1, 0, Tile::redstoneWire.id), "corner first wire should place");
		ok &= expect(level.setTile(82, baseY + 1, 0, Tile::redstoneWire.id), "corner second wire should place");
		ok &= expect(level.setTile(82, baseY + 1, 1, Tile::redstoneWire.id), "corner turn dust should place");
		ok &= expect(level.setTile(82, baseY + 1, 2, Tile::redstoneWire.id), "corner tail dust should place");
		ok &= expect(Tile::lever.use(level, 80, baseY + 1, 0, player), "corner lever source should toggle on");
		ok &= expect(level.getData(81, baseY + 1, 0) == 15, "corner first wire should power at strength 15");
		ok &= expect(level.getData(82, baseY + 1, 0) == 14, "corner second wire should power at strength 14");
		ok &= expect(level.getData(82, baseY + 1, 1) == 13, "corner turn dust should power at strength 13");
		ok &= expect(level.getData(82, baseY + 1, 2) == 12, "corner tail dust should power at strength 12");

		std::cerr << "block-smoke: long corner wire range" << std::endl;
		for (int_t x = 90; x <= 98; ++x)
			level.setTile(x, baseY, 0, 1);
		for (int_t z = 1; z <= 7; ++z)
			level.setTile(98, baseY, z, 1);
		ok &= expect(level.setTile(90, baseY + 1, 0, Tile::lever.id), "long-corner lever source should place");
		level.setData(90, baseY + 1, 0, 5);
		for (int_t x = 91; x <= 98; ++x)
			ok &= expect(level.setTile(x, baseY + 1, 0, Tile::redstoneWire.id), "long-corner horizontal wire should place");
		for (int_t z = 1; z <= 7; ++z)
			ok &= expect(level.setTile(98, baseY + 1, z, Tile::redstoneWire.id), "long-corner vertical wire should place");
		ok &= expect(Tile::lever.use(level, 90, baseY + 1, 0, player), "long-corner lever source should toggle on");
		ok &= expect(level.getData(91, baseY + 1, 0) == 15, "long-corner first wire should power at strength 15");
		ok &= expect(level.getData(98, baseY + 1, 0) == 8, "long-corner last horizontal wire should power at strength 8");
		ok &= expect(level.getData(98, baseY + 1, 1) == 7, "long-corner first turned wire should power at strength 7");
		ok &= expect(level.getData(98, baseY + 1, 6) == 2, "long-corner near-tail wire should power at strength 2");
		ok &= expect(level.getData(98, baseY + 1, 7) == 1, "long-corner tail wire should power at strength 1");

		std::cerr << "block-smoke: torch long corner wire range" << std::endl;
		for (int_t x = 110; x <= 118; ++x)
			level.setTile(x, baseY, 0, 1);
		for (int_t z = 1; z <= 7; ++z)
			level.setTile(118, baseY, z, 1);
		ok &= expect(level.setTileAndData(110, baseY + 1, 0, Tile::torchRedstoneActive.id, 5), "torch long-corner source should place");
		for (int_t x = 111; x <= 118; ++x)
			ok &= expect(level.setTile(x, baseY + 1, 0, Tile::redstoneWire.id), "torch long-corner horizontal wire should place");
		for (int_t z = 1; z <= 7; ++z)
			ok &= expect(level.setTile(118, baseY + 1, z, Tile::redstoneWire.id), "torch long-corner vertical wire should place");
		ok &= expect(level.getData(111, baseY + 1, 0) == 15, "torch long-corner first wire should power at strength 15");
		ok &= expect(level.getData(118, baseY + 1, 0) == 8, "torch long-corner last horizontal wire should power at strength 8");
		ok &= expect(level.getData(118, baseY + 1, 1) == 7, "torch long-corner first turned wire should power at strength 7");
		ok &= expect(level.getData(118, baseY + 1, 6) == 2, "torch long-corner near-tail wire should power at strength 2");
		ok &= expect(level.getData(118, baseY + 1, 7) == 1, "torch long-corner tail wire should power at strength 1");

		std::cerr << "block-smoke: long straight wire range" << std::endl;
		for (int_t x = 150; x <= 160; ++x)
			level.setTile(x, baseY, 0, 1);
		ok &= expect(level.setTile(150, baseY + 1, 0, Tile::lever.id), "long-straight lever source should place");
		level.setData(150, baseY + 1, 0, 5);
		for (int_t x = 151; x <= 160; ++x)
			ok &= expect(level.setTile(x, baseY + 1, 0, Tile::redstoneWire.id), "long-straight wire should place");
		ok &= expect(Tile::lever.use(level, 150, baseY + 1, 0, player), "long-straight lever source should toggle on");
		ok &= expect(level.getData(151, baseY + 1, 0) == 15, "long-straight first wire should power at strength 15");
		ok &= expect(level.getData(152, baseY + 1, 0) == 14, "long-straight second wire should power at strength 14");
		ok &= expect(level.getData(157, baseY + 1, 0) == 9, "long-straight mid wire should power at strength 9");
		ok &= expect(level.getData(160, baseY + 1, 0) == 6, "long-straight tail wire should power at strength 6");
		ok &= expect(Tile::lever.use(level, 150, baseY + 1, 0, player), "long-straight lever source should toggle off");
		ok &= expect(level.getData(151, baseY + 1, 0) == 0, "long-straight first wire should depower when switched off");
		ok &= expect(level.getData(152, baseY + 1, 0) == 0, "long-straight second wire should depower when switched off");
		ok &= expect(level.getData(157, baseY + 1, 0) == 0, "long-straight mid wire should depower when switched off");
		ok &= expect(level.getData(160, baseY + 1, 0) == 0, "long-straight tail wire should depower when switched off");
		
		std::cerr << "block-smoke: ascending wire step" << std::endl;
		for (int_t x = 130; x <= 133; ++x)
			level.setTile(x, baseY, 0, 1);
		level.setTile(132, baseY + 1, 0, 1);
		level.setTile(133, baseY + 1, 0, 1);
		ok &= expect(level.setTile(130, baseY + 1, 0, Tile::lever.id), "ascending-step lever source should place");
		level.setData(130, baseY + 1, 0, 5);
		ok &= expect(level.setTile(131, baseY + 1, 0, Tile::redstoneWire.id), "ascending-step lower wire should place");
		ok &= expect(level.setTile(132, baseY + 2, 0, Tile::redstoneWire.id), "ascending-step raised wire should place");
		ok &= expect(level.setTile(133, baseY + 2, 0, Tile::redstoneWire.id), "ascending-step raised tail wire should place");
		ok &= expect(Tile::lever.use(level, 130, baseY + 1, 0, player), "ascending-step lever source should toggle on");
		ok &= expect(level.getData(131, baseY + 1, 0) == 15, "ascending-step lower wire should power at strength 15");
		ok &= expect(level.getData(132, baseY + 2, 0) == 14, "ascending-step raised wire should power at strength 14");
		ok &= expect(level.getData(133, baseY + 2, 0) == 13, "ascending-step raised tail wire should power at strength 13");
		
		std::cerr << "block-smoke: descending wire step" << std::endl;
		level.setTile(140, baseY + 1, 0, 1);
		level.setTile(141, baseY + 1, 0, 1);
		level.setTile(142, baseY, 0, 1);
		level.setTile(143, baseY, 0, 1);
		ok &= expect(level.setTileAndData(140, baseY + 2, 0, Tile::torchRedstoneActive.id, 5), "descending-step torch source should place");
		ok &= expect(level.setTile(141, baseY + 2, 0, Tile::redstoneWire.id), "descending-step raised wire should place");
		ok &= expect(level.setTile(142, baseY + 1, 0, Tile::redstoneWire.id), "descending-step lower wire should place");
		ok &= expect(level.setTile(143, baseY + 1, 0, Tile::redstoneWire.id), "descending-step lower tail wire should place");
		ok &= expect(level.getData(141, baseY + 2, 0) == 15, "descending-step raised wire should power at strength 15");
		ok &= expect(level.getData(142, baseY + 1, 0) == 14, "descending-step lower wire should power at strength 14");
		ok &= expect(level.getData(143, baseY + 1, 0) == 13, "descending-step lower tail wire should power at strength 13");
		
		std::cerr << "block-smoke: lever wire propagation" << std::endl;
		for (int_t x = 32; x <= 36; ++x)
			level.setTile(x, baseY, 0, 1);
		ok &= expect(level.setTile(32, baseY + 1, 0, Tile::lever.id), "lever source should place");
		level.setData(32, baseY + 1, 0, 5);
		for (int_t x = 33; x <= 36; ++x)
			ok &= expect(level.setTile(x, baseY + 1, 0, Tile::redstoneWire.id), "lever test wire should place");
		listener.clear();
		ok &= expect(Tile::lever.use(level, 32, baseY + 1, 0, player), "lever source should toggle on");
		ok &= expect(listener.dirtyCalls > 0, "runtime wire power changes should dirty render ranges without re-placement");
		ok &= expect(level.getData(33, baseY + 1, 0) == 15, "lever should power first wire at strength 15");
		ok &= expect(level.getData(34, baseY + 1, 0) == 14, "lever should power second wire at strength 14");
		ok &= expect(level.getData(35, baseY + 1, 0) == 13, "lever should power third wire at strength 13");
		ok &= expect(level.getData(36, baseY + 1, 0) == 12, "lever should power fourth wire at strength 12");
		listener.clear();
		ok &= expect(Tile::lever.use(level, 32, baseY + 1, 0, player), "lever source should toggle off");
		ok &= expect(listener.dirtyCalls > 0, "runtime wire depower should dirty render ranges without re-placement");
		ok &= expect(level.getData(33, baseY + 1, 0) == 0, "lever should depower first wire when switched off");
		ok &= expect(level.getData(34, baseY + 1, 0) == 0, "lever should depower second wire when switched off");
		ok &= expect(level.getData(35, baseY + 1, 0) == 0, "lever should depower third wire when switched off");
		ok &= expect(level.getData(36, baseY + 1, 0) == 0, "lever should depower fourth wire when switched off");

		std::cerr << "block-smoke: wall lever block wire range" << std::endl;
		for (int_t x = 190; x <= 200; ++x)
			level.setTile(x, baseY + 1, 0, 1);
		ok &= expect(level.setTile(189, baseY + 1, 0, Tile::lever.id), "wall-lever source should place");
		level.setData(189, baseY + 1, 0, 2);
		for (int_t x = 190; x <= 200; ++x)
			ok &= expect(level.setTile(x, baseY + 2, 0, Tile::redstoneWire.id), "wall-lever wire should place");
		ok &= expect(Tile::lever.use(level, 189, baseY + 1, 0, player), "wall-lever source should toggle on");
		ok &= expect(level.getData(190, baseY + 2, 0) == 15, "wall-lever first wire should power at strength 15");
		ok &= expect(level.getData(191, baseY + 2, 0) == 14, "wall-lever second wire should power at strength 14");
		ok &= expect(level.getData(196, baseY + 2, 0) == 9, "wall-lever mid wire should power at strength 9");
		ok &= expect(level.getData(200, baseY + 2, 0) == 5, "wall-lever tail wire should power at strength 5");
		ok &= expect(Tile::lever.use(level, 189, baseY + 1, 0, player), "wall-lever source should toggle off");
		ok &= expect(level.getData(190, baseY + 2, 0) == 0, "wall-lever first wire should depower when switched off");
		ok &= expect(level.getData(191, baseY + 2, 0) == 0, "wall-lever second wire should depower when switched off");
		ok &= expect(level.getData(196, baseY + 2, 0) == 0, "wall-lever mid wire should depower when switched off");
		ok &= expect(level.getData(200, baseY + 2, 0) == 0, "wall-lever tail wire should depower when switched off");
		
		std::cerr << "block-smoke: torch wire propagation" << std::endl;
		for (int_t x = 40; x <= 44; ++x)
			level.setTile(x, baseY, 0, 1);
		ok &= expect(level.setTileAndData(40, baseY + 1, 0, Tile::torchRedstoneActive.id, 5), "redstone torch source should place");
		for (int_t x = 41; x <= 44; ++x)
		{
			ItemInstance lineDust(Items::redstone->getShiftedIndex(), 1, 0);
			ok &= expect(lineDust.useOn(player, level, x, baseY, 0, Facing::UP), "torch test wire should place");
		}
		ok &= expect(level.getData(41, baseY + 1, 0) == 15, "torch should power first wire at strength 15");
		ok &= expect(level.getData(42, baseY + 1, 0) == 14, "torch should power second wire at strength 14");
		ok &= expect(level.getData(43, baseY + 1, 0) == 13, "torch should power third wire at strength 13");
		ok &= expect(level.getData(44, baseY + 1, 0) == 12, "torch should power fourth wire at strength 12");
		
		std::cerr << "block-smoke: button block wire propagation" << std::endl;
		for (int_t x = 50; x <= 53; ++x)
			level.setTile(x, baseY + 1, 0, 1);
		ok &= expect(level.setTile(49, baseY + 1, 0, Tile::buttonStone.id), "button source should place");
		level.setData(49, baseY + 1, 0, 2);
		for (int_t x = 50; x <= 53; ++x)
			ok &= expect(level.setTile(x, baseY + 2, 0, Tile::redstoneWire.id), "button test wire should place");
		ok &= expect(Tile::buttonStone.use(level, 49, baseY + 1, 0, player), "button source should toggle on");
		ok &= expect(level.getData(50, baseY + 2, 0) == 15, "button should power first wire through attached block at strength 15");
		ok &= expect(level.getData(51, baseY + 2, 0) == 14, "button should power second wire through attached block at strength 14");
		ok &= expect(level.getData(52, baseY + 2, 0) == 13, "button should power third wire through attached block at strength 13");
		ok &= expect(level.getData(53, baseY + 2, 0) == 12, "button should power fourth wire through attached block at strength 12");
		
		std::cerr << "block-smoke: repeater propagation" << std::endl;
		for (int_t x = 210; x <= 213; ++x)
			level.setTile(x, baseY, 0, 1);
		ok &= expect(level.setTile(210, baseY + 1, 0, Tile::lever.id), "repeater test lever should place");
		level.setData(210, baseY + 1, 0, 5);
		ok &= expect(level.setTile(211, baseY + 1, 0, Tile::redstoneWire.id), "repeater test input wire should place");
		player.yRot = 270.0f;
		ItemInstance repeaterItem(Items::redstoneRepeater->getShiftedIndex(), 1, 0);
		ok &= expect(repeaterItem.useOn(player, level, 212, baseY, 0, Facing::UP), "repeater item should place on top of a solid block");
		ok &= expect(level.getTile(212, baseY + 1, 0) == Tile::repeaterIdle.id, "repeater item should place the idle repeater block");
		ok &= expect((level.getData(212, baseY + 1, 0) & 3) == 1, "repeater placement should face away from the player");
		ok &= expect(repeaterItem.stackSize == 0, "repeater item should be consumed on placement");
		ok &= expect(level.setTile(213, baseY + 1, 0, Tile::redstoneWire.id), "repeater test output wire should place");
		ok &= expect(RedStoneDustTile::isPowerProviderOrWire(level, 212, baseY + 1, 0, 3), "dust helper should connect to the repeater front");
		ok &= expect(Tile::lever.use(level, 210, baseY + 1, 0, player), "repeater test lever should toggle on");
		ok &= expect(level.getData(211, baseY + 1, 0) == 15, "repeater input wire should power at strength 15");
		advancePendingTicks(1);
		ok &= expect(level.getTile(212, baseY + 1, 0) == Tile::repeaterIdle.id, "repeater should stay idle until its default delay expires");
		ok &= expect(level.getData(213, baseY + 1, 0) == 0, "repeater should not power its output before the delay completes");
		advancePendingTicks(1);
		ok &= expect(level.getTile(212, baseY + 1, 0) == Tile::repeaterActive.id, "repeater should switch to the active block after two ticks");
		ok &= expect(level.getData(213, baseY + 1, 0) == 15, "repeater should power the output wire after the delay");
		ok &= expect(Tile::repeaterActive.use(level, 212, baseY + 1, 0, player), "active repeater should cycle its delay on use");
		ok &= expect((level.getData(212, baseY + 1, 0) & 12) == 4, "repeater should advance to the second delay setting");
		ok &= expect(Tile::lever.use(level, 210, baseY + 1, 0, player), "repeater test lever should toggle off");
		for (int_t i = 0; i < 3; ++i)
		{
			advancePendingTicks(1);
			ok &= expect(level.getTile(212, baseY + 1, 0) == Tile::repeaterActive.id, "repeater should stay active until the four-tick delay expires");
		}
		advancePendingTicks(1);
		ok &= expect(level.getTile(212, baseY + 1, 0) == Tile::repeaterIdle.id, "repeater should turn off after four ticks at the second delay setting");
		ok &= expect(level.getData(213, baseY + 1, 0) == 0, "repeater should depower the output wire after turning off");
		player.yRot = 0.0f;
		
		std::cerr << "block-smoke: button bounds" << std::endl;
		level.setTile(148, baseY + 1, 0, 1);
		ok &= expect(level.setTile(149, baseY + 1, 0, Tile::buttonStone.id), "button bounds test button should place");
		level.setData(149, baseY + 1, 0, 1);
		Tile::buttonStone.updateShape(level, 149, baseY + 1, 0);
		ok &= expect(Tile::buttonStone.xx0 == 0.0 && Tile::buttonStone.xx1 == 2.0 / 16.0, "unpowered button should be 2/16 deep in-world");
		ok &= expect(Tile::buttonStone.yy0 == 6.0 / 16.0 && Tile::buttonStone.yy1 == 10.0 / 16.0, "button should match beta in-world height bounds");
		ok &= expect(Tile::buttonStone.zz0 == 5.0 / 16.0 && Tile::buttonStone.zz1 == 11.0 / 16.0, "button should match beta in-world width bounds");
		level.setData(149, baseY + 1, 0, 1 | 8);
		Tile::buttonStone.updateShape(level, 149, baseY + 1, 0);
		ok &= expect(Tile::buttonStone.xx0 == 0.0 && Tile::buttonStone.xx1 == 1.0 / 16.0, "powered button should shrink to 1/16 depth in-world");
		
		std::cerr << "block-smoke: powered door" << std::endl;
		level.setTile(24, baseY, 0, 1);
		level.setTile(24, baseY + 1, 0, 64);
		level.setData(24, baseY + 1, 0, 0);
		level.setTile(24, baseY + 2, 0, 64);
		level.setData(24, baseY + 2, 0, 8);
		level.setTile(25, baseY, 0, 1);
		level.setTile(25, baseY + 1, 0, 76);
		level.setData(25, baseY + 1, 0, 5);
		Tile::doorWood.neighborChanged(level, 24, baseY + 2, 0, 76);
		ok &= expect((level.getData(24, baseY + 1, 0) & 4) != 0, "door should open when indirectly powered");

		std::cerr << "block-smoke: powered dispenser" << std::endl;
		ok &= expect(level.setTile(28, baseY + 1, 0, 23), "powered dispenser should place");
		auto poweredDispenser = std::dynamic_pointer_cast<DispenserTileEntity>(level.getTileEntity(28, baseY + 1, 0));
		ok &= expect(poweredDispenser != nullptr, "powered dispenser should create a tile entity");
		if (poweredDispenser != nullptr)
		{
			poweredDispenser->setItem(0, ItemInstance(Items::coal->getShiftedIndex(), 1, 0));
			level.setTile(29, baseY, 0, 1);
			level.setTile(29, baseY + 1, 0, 76);
			level.setData(29, baseY + 1, 0, 5);
			size_t beforeEntities = level.entities.size();
			Tile::dispenser.neighborChanged(level, 28, baseY + 1, 0, 76);
			Tile::dispenser.tick(level, 28, baseY + 1, 0, random);
			ok &= expect(level.entities.size() > beforeEntities, "dispenser should eject an item when powered");
		}
		std::cerr << "block-smoke: door particles" << std::endl;
		Tile *doorTile = Tile::tiles[64];
		int_t mirroredFace = -1;
		int_t mirroredData = -1;
		int_t mirroredTex = 0;
		for (int_t data = 0; data < 16 && mirroredFace < 0; ++data)
		{
			for (int_t face = 0; face < 6; ++face)
			{
				int_t tex = doorTile->getTexture(static_cast<Facing>(face), data);
				if (tex < 0)
				{
					mirroredFace = face;
					mirroredData = data;
					mirroredTex = tex;
					break;
				}
			}
		}
		ok &= expect(mirroredFace >= 0, "door should still use mirrored textures internally");
		if (mirroredFace >= 0)
		{
			InspectableTerrainParticle particle(level, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, doorTile, mirroredFace, mirroredData);
			ok &= expect(particle.textureIndex() == -mirroredTex, "door crack particles should clamp mirrored textures");
		}
	
		std::cerr << "block-smoke: repeated powered door" << std::endl;
		level.setTile(14, baseY, 0, 5);
		ok &= expect(level.setTile(14, baseY + 1, 0, 64), "repeat-test door bottom should place");
		level.setData(14, baseY + 1, 0, 0);
		ok &= expect(level.setTile(14, baseY + 2, 0, 64), "repeat-test door top should place");
		level.setData(14, baseY + 2, 0, 8);
		level.setTile(15, baseY + 1, 0, 1);
		ok &= expect(level.setTile(16, baseY + 1, 0, Tile::lever.id), "repeat-test door lever should place");
		level.setData(16, baseY + 1, 0, 1);
		listener.clear();
		logRepeatedDoorState(level, listener, "before first toggle", 14, baseY + 1, 0, 15, baseY + 1, 0, 16, baseY + 1, 0);
		ok &= expect(Tile::lever.use(level, 16, baseY + 1, 0, player), "repeat-test door lever should toggle on");
		ok &= expect((level.getData(14, baseY + 1, 0) & 4) != 0, "door should open on first redstone activation");
		logRepeatedDoorState(level, listener, "after first toggle", 14, baseY + 1, 0, 15, baseY + 1, 0, 16, baseY + 1, 0);
		listener.clear();
		ok &= expect(Tile::lever.use(level, 16, baseY + 1, 0, player), "repeat-test door lever should toggle off");
		ok &= expect((level.getData(14, baseY + 1, 0) & 4) == 0, "door should close when redstone power is removed");
		logRepeatedDoorState(level, listener, "after second toggle", 14, baseY + 1, 0, 15, baseY + 1, 0, 16, baseY + 1, 0);
		listener.clear();
		ok &= expect(Tile::lever.use(level, 16, baseY + 1, 0, player), "repeat-test door lever should toggle on again");
		ok &= expect((level.getData(14, baseY + 1, 0) & 4) != 0, "door should reopen on second redstone activation");
		logRepeatedDoorState(level, listener, "after third toggle", 14, baseY + 1, 0, 15, baseY + 1, 0, 16, baseY + 1, 0);
		
		std::cerr << "block-smoke: repeated powered note block" << std::endl;
		level.setTile(8, baseY, 0, 5);
		level.setTile(8, baseY + 2, 0, 0);
		ok &= expect(level.setTile(8, baseY + 1, 0, Tile::noteBlock.id), "repeat-test note block should place");
		ok &= expect(level.setTile(9, baseY + 1, 0, Tile::lever.id), "repeat-test note lever should place");
		level.setData(9, baseY + 1, 0, 1);
		auto repeatNoteEntity = std::dynamic_pointer_cast<NoteTileEntity>(level.getTileEntity(8, baseY + 1, 0));
		ok &= expect(repeatNoteEntity != nullptr, "repeat-test note block should create a tile entity");
		listener.clear();
		logRepeatedNoteState(level, listener, "before first toggle", 8, baseY + 1, 0, 9, baseY + 1, 0, repeatNoteEntity.get());
		ok &= expect(Tile::lever.use(level, 9, baseY + 1, 0, player), "repeat-test note lever should toggle on");
		ok &= expect(containsPrefix(listener.sounds, u"note."), "note block should play on first redstone activation");
		ok &= expect(!listener.particles.empty() && listener.particles.back() == u"note", "note block should spawn a particle on first redstone activation");
		if (repeatNoteEntity != nullptr)
			ok &= expect(repeatNoteEntity->previousRedstoneState, "note block should latch powered state after first activation");
		logRepeatedNoteState(level, listener, "after first toggle", 8, baseY + 1, 0, 9, baseY + 1, 0, repeatNoteEntity.get());
		listener.clear();
		ok &= expect(Tile::lever.use(level, 9, baseY + 1, 0, player), "repeat-test note lever should toggle off");
		ok &= expect(listener.particles.empty(), "note block should not trigger a note on power removal");
		if (repeatNoteEntity != nullptr)
			ok &= expect(!repeatNoteEntity->previousRedstoneState, "note block should clear powered latch after power removal");
		logRepeatedNoteState(level, listener, "after second toggle", 8, baseY + 1, 0, 9, baseY + 1, 0, repeatNoteEntity.get());
		listener.clear();
		ok &= expect(Tile::lever.use(level, 9, baseY + 1, 0, player), "repeat-test note lever should toggle on again");
		ok &= expect(containsPrefix(listener.sounds, u"note."), "note block should play again on second redstone activation");
		ok &= expect(!listener.particles.empty() && listener.particles.back() == u"note", "note block should spawn a particle on second redstone activation");
		logRepeatedNoteState(level, listener, "after third toggle", 8, baseY + 1, 0, 9, baseY + 1, 0, repeatNoteEntity.get());
		
		std::cerr << "block-smoke: door" << std::endl;
		level.setTile(0, baseY, 0, 5);
		level.setTile(0, baseY + 1, 0, 0);
		level.setTile(0, baseY + 2, 0, 0);
		ItemInstance doorStack(Items::doorWood->getShiftedIndex(), 1, 0);
		ok &= expect(doorStack.useOn(player, level, 0, baseY, 0, Facing::UP), "wood door item should place a door");
		ok &= expect(level.getTile(0, baseY + 1, 0) == 64, "wood door bottom half should place");
		ok &= expect(level.getTile(0, baseY + 2, 0) == 64, "wood door top half should place");
		listener.clear();
		ok &= expect(Tile::tiles[64]->use(level, 0, baseY + 1, 0, player), "wood door should toggle on use");
		ok &= expect(!listener.sounds.empty() && listener.sounds.back() == u"random.door_open", "wood door should play open sound");

		std::cerr << "block-smoke: note block" << std::endl;
		level.setTile(2, baseY, 0, 5);
		level.setTile(2, baseY + 1, 0, 0);
		level.setTile(2, baseY + 2, 0, 0);
		ok &= expect(level.setTile(2, baseY + 1, 0, 25), "note block should place");
		listener.clear();
		ok &= expect(Tile::tiles[25]->use(level, 2, baseY + 1, 0, player), "note block should retune and play");
		ok &= expect(!listener.sounds.empty() && listener.sounds.back().find(u"note.") == 0, "note block should play a note sound");
		ok &= expect(!listener.particles.empty() && listener.particles.back() == u"note", "note block should spawn a note particle");
		listener.clear();
		InspectableNoteParticle noteParticle(level, 0.0, 0.0, 0.0, 0.5, 2.0f);
		ok &= expect(noteParticle.currentSize() > 1.4f, "note particle should use the corrected beta scale");

		std::cerr << "block-smoke: jukebox" << std::endl;
		ok &= expect(level.setTile(6, baseY + 1, 0, 84), "jukebox should place");
		listener.clear();
		ItemInstance record13(Items::record13->getShiftedIndex(), 1, 0);
		ok &= expect(record13.useOn(player, level, 6, baseY + 1, 0, Facing::UP), "record should insert into jukebox");
		ok &= expect(level.getData(6, baseY + 1, 0) == 1, "jukebox should mark itself occupied");
		ok &= expect(!listener.streams.empty() && listener.streams.back() == u"13", "jukebox should start record playback");
		listener.clear();
		ok &= expect(Tile::tiles[84]->use(level, 6, baseY + 1, 0, player), "jukebox should eject record on use");
		ok &= expect(level.getData(6, baseY + 1, 0) == 0, "jukebox should clear occupied state after eject");
		ok &= expect(!listener.streams.empty() && listener.streams.back().empty(), "jukebox eject should stop streaming music");

		std::cerr << "block-smoke: dispenser" << std::endl;
		ok &= expect(level.setTile(8, baseY + 1, 0, 23), "dispenser should place");
		auto dispenserEntity = std::dynamic_pointer_cast<DispenserTileEntity>(level.getTileEntity(8, baseY + 1, 0));
		ok &= expect(dispenserEntity != nullptr, "dispenser should create a tile entity");
		if (dispenserEntity != nullptr)
		{
			dispenserEntity->setItem(0, ItemInstance(Items::coal->getShiftedIndex(), 3, 0));
			CompoundTag tag;
			dispenserEntity->save(tag);
			DispenserTileEntity loaded;
			loaded.load(tag);
			ok &= expect(loaded.getItem(0).itemID == Items::coal->getShiftedIndex(), "dispenser should save its inventory");
			ok &= expect(loaded.getItem(0).stackSize == 3, "dispenser should preserve stack counts");
			size_t entityCountBefore = level.entities.size();
			level.setTile(8, baseY + 1, 0, 0);
			ok &= expect(level.entities.size() > entityCountBefore, "dispenser removal should drop stored items");
		}

		std::cerr << "block-smoke: sign" << std::endl;
		level.setTile(14, baseY, 0, 5);
		level.setTile(14, baseY + 1, 0, 0);
		ItemInstance signPostStack(Items::sign->getShiftedIndex(), 1, 0);
		ok &= expect(signPostStack.useOn(player, level, 14, baseY, 0, Facing::UP), "sign item should place a sign post");
		ok &= expect(level.getTile(14, baseY + 1, 0) == 63, "sign post item should place sign post tile");
		auto signEntity = std::dynamic_pointer_cast<SignTileEntity>(level.getTileEntity(14, baseY + 1, 0));
		ok &= expect(signEntity != nullptr, "sign post should create a tile entity");
		if (signEntity != nullptr)
		{
			signEntity->signText[0] = u"hello";
			CompoundTag tag;
			signEntity->save(tag);
			signEntity->signText[0] = u"";
			signEntity->load(tag);
			ok &= expect(signEntity->signText[0] == u"hello", "sign should save/load text");
		}
		level.setTile(18, baseY + 1, 0, 1);
		level.setTile(19, baseY + 1, 0, 0);
		ItemInstance signWallStack(Items::sign->getShiftedIndex(), 1, 0);
		ok &= expect(signWallStack.useOn(player, level, 18, baseY + 1, 0, Facing::EAST), "sign item should place a wall sign");
		ok &= expect(level.getTile(19, baseY + 1, 0) == 68, "wall sign item should place wall sign tile");
		ok &= expect(level.getData(19, baseY + 1, 0) == static_cast<int_t>(Facing::EAST), "wall sign should keep face data");
		ok &= expect(std::dynamic_pointer_cast<SignTileEntity>(level.getTileEntity(19, baseY + 1, 0)) != nullptr, "wall sign should create a tile entity");
		ok &= expect(Tile::tiles[63]->getResource(0, level.random) == Items::sign->getShiftedIndex(), "sign post should drop sign item");

		std::cerr << "block-smoke: cobweb" << std::endl;
		player.setPos(4.0, static_cast<double>(baseY + 5), 0.0);
		double beforeX = player.x;
		double beforeY = player.y;
		Tile::tiles[30]->entityInside(level, 4, baseY + 5, 0, player);
		player.move(1.0, 1.0, 0.0);
		ok &= expect(!player.isInWeb, "web flag should clear after movement");
		ok &= expect((player.x - beforeX) < 0.5, "cobweb should strongly reduce horizontal motion");
		ok &= expect((player.y - beforeY) < 0.2, "cobweb should strongly reduce vertical motion");

		std::cerr << "block-smoke: compass and clock" << std::endl;
		ok &= expect(Items::compass->getShiftedIndex() == 345, "compass should use the beta shifted id 345");
		ok &= expect(Items::clock->getShiftedIndex() == 347, "clock should use the beta shifted id 347");
		ok &= expect(ItemInstance(Items::compass->getShiftedIndex(), 1, 0).getIcon() == 54, "compass should use the beta compass icon");
		ok &= expect(ItemInstance(Items::clock->getShiftedIndex(), 1, 0).getIcon() == 70, "clock should use the beta clock icon");
		GridCraftingContainer compassRecipe{};
		compassRecipe.slots[1] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		compassRecipe.slots[3] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		compassRecipe.slots[4] = ItemInstance(Items::redstone->getShiftedIndex(), 1, 0);
		compassRecipe.slots[5] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		compassRecipe.slots[7] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		ItemInstance compassOut = Recipes::getInstance().getItemFor(compassRecipe);
		ok &= expect(compassOut.itemID == Items::compass->getShiftedIndex(), "compass recipe should match beta");
		GridCraftingContainer clockRecipe{};
		clockRecipe.slots[1] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		clockRecipe.slots[3] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		clockRecipe.slots[4] = ItemInstance(Items::redstone->getShiftedIndex(), 1, 0);
		clockRecipe.slots[5] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		clockRecipe.slots[7] = ItemInstance(Items::ingotGold->getShiftedIndex(), 1, 0);
		ItemInstance clockOut = Recipes::getInstance().getItemFor(clockRecipe);
		ok &= expect(clockOut.itemID == Items::clock->getShiftedIndex(), "clock recipe should match beta");
		Minecraft textureFxMinecraft(320, 240, false);
		TextureCompassFX compassFx(textureFxMinecraft);
		compassFx.onTick();
		bool compassPixels = false;
		for (int_t pixel = 0; pixel < 256; ++pixel)
		{
			if (compassFx.imageData[pixel * 4 + 3] != 0)
			{
				compassPixels = true;
				break;
			}
		}
		ok &= expect(compassFx.tileImage == 1, "compass texture fx should target the items atlas");
		ok &= expect(compassFx.iconIndex == 54, "compass texture fx should target the compass icon slot");
		ok &= expect(compassPixels, "compass texture fx should draw compass pixels");
		TextureWatchFX clockFx(textureFxMinecraft);
		clockFx.onTick();
		bool clockPixels = false;
		for (int_t pixel = 0; pixel < 256; ++pixel)
		{
			if (clockFx.imageData[pixel * 4 + 3] != 0)
			{
				clockPixels = true;
				break;
			}
		}
		ok &= expect(clockFx.tileImage == 1, "clock texture fx should target the items atlas");
		ok &= expect(clockFx.iconIndex == 70, "clock texture fx should target the clock icon slot");
		ok &= expect(clockPixels, "clock texture fx should draw clock pixels");

		std::cerr << "block-smoke: pistons" << std::endl;
		ok &= expect(Tile::pistonBase.id == 33, "normal piston base should use id 33");
		ok &= expect(Tile::pistonStickyBase.id == 29, "sticky piston base should use id 29");
		ok &= expect(Tile::pistonExtension.id == 34, "piston extension should use id 34");
		ok &= expect(Tile::pistonMoving.id == 36, "piston moving should use id 36");
		ok &= expect(Tile::pistonBase.tex == 107, "normal piston should use texture 107");
		ok &= expect(Tile::pistonStickyBase.tex == 106, "sticky piston should use texture 106");
		ok &= expect(Tile::pistonBase.getMobilityFlag() == 2, "piston base material should be immovable");
		ok &= expect(Tile::pistonExtension.getMobilityFlag() == 2, "piston extension material should be immovable");
		ok &= expect(Tile::leaves.getMobilityFlag() == 1, "leaves should be breakable when pushed");
		ok &= expect(Tile::fire.getMobilityFlag() == 1, "fire should be breakable when pushed");
		ok &= expect(Tile::cobweb.getMobilityFlag() == 1, "cobweb should be breakable when pushed");
		ok &= expect(Tile::pistonBase.getRenderShape() == Tile::SHAPE_PISTON_BASE, "piston base should render as piston base shape");
		ok &= expect(Tile::pistonExtension.getRenderShape() == Tile::SHAPE_PISTON_EXTENSION, "piston extension should render as piston extension shape");
		ok &= expect(Tile::pistonMoving.getRenderShape() == Tile::SHAPE_INVISIBLE, "piston moving should be invisible");
		GridCraftingContainer pistonRecipe{};
		pistonRecipe.slots[0] = ItemInstance(Tile::wood.id, 1, -1);
		pistonRecipe.slots[1] = ItemInstance(Tile::wood.id, 1, -1);
		pistonRecipe.slots[2] = ItemInstance(Tile::wood.id, 1, -1);
		pistonRecipe.slots[3] = ItemInstance(Tile::cobblestone.id, 1, -1);
		pistonRecipe.slots[4] = ItemInstance(Items::ingotIron->getShiftedIndex(), 1, 0);
		pistonRecipe.slots[5] = ItemInstance(Tile::cobblestone.id, 1, -1);
		pistonRecipe.slots[6] = ItemInstance(Tile::cobblestone.id, 1, -1);
		pistonRecipe.slots[7] = ItemInstance(Items::redstone->getShiftedIndex(), 1, 0);
		pistonRecipe.slots[8] = ItemInstance(Tile::cobblestone.id, 1, -1);
		ItemInstance pistonOut = Recipes::getInstance().getItemFor(pistonRecipe);
		ok &= expect(pistonOut.itemID == Tile::pistonBase.id, "piston recipe should match b173");
		GridCraftingContainer stickyPistonRecipe{};
		stickyPistonRecipe.slots[0] = ItemInstance(Items::slimeball->getShiftedIndex(), 1, 0);
		stickyPistonRecipe.slots[3] = ItemInstance(Tile::pistonBase.id, 1, 0);
		ItemInstance stickyPistonOut = Recipes::getInstance().getItemFor(stickyPistonRecipe);
		ok &= expect(stickyPistonOut.itemID == Tile::pistonStickyBase.id, "sticky piston recipe should match b173");

		level.setTile(300, baseY, 0, 1);
		ok &= expect(level.setTile(300, baseY + 1, 0, Tile::pistonBase.id), "piston base should place");
		level.setData(300, baseY + 1, 0, 5);
		ok &= expect(PistonBaseTile::getDirection(level.getData(300, baseY + 1, 0)) == 5, "piston data should store direction");
		ok &= expect(!PistonBaseTile::isPowered(level.getData(300, baseY + 1, 0)), "unpowered piston should not have powered bit");
		level.setTile(301, baseY, 0, Tile::rock.id);
		level.setTile(301, baseY + 1, 0, Tile::redstoneWire.id);
		level.setData(301, baseY + 1, 0, 15);
		Tile::pistonBase.neighborChanged(level, 300, baseY + 1, 0, Tile::redstoneWire.id);
		ok &= expect(!PistonBaseTile::isPowered(level.getData(300, baseY + 1, 0)),
			"piston should ignore power supplied through its front face");
		
		
		level.setTile(298, baseY, 0, Tile::rock.id);
		level.setTile(299, baseY, 0, Tile::rock.id);
		level.setTile(299, baseY + 1, 0, Tile::redstoneWire.id);
		level.setTileAndData(298, baseY + 1, 0, Tile::torchRedstoneActive.id, 5);
		ok &= expect(level.getData(299, baseY + 1, 0) == 15, "torch-fed wire should carry full strength");
		ok &= expect(PistonBaseTile::isPowered(level.getData(300, baseY + 1, 0)),
			"piston should accept power supplied through its rear face");

		std::cerr << "block-smoke: TNT drop" << std::endl;
		ok &= expect(Tile::tnt.getResourceCount(random) == 0, "TNT should not drop via getResourceCount");
		ok &= expect(Tile::tnt.descriptionId == u"tile.tnt", "TNT should use the tnt localization key");

		std::cerr << "block-smoke: HD texture effects" << std::endl;
		TileSize::set(32);
		TextureFlamesFX highResolutionFlames(0);
		highResolutionFlames.onTick();
		ok &= expect(highResolutionFlames.imageData.size() == 32 * 32 * 4, "HD flame texture fx should use the selected tile size");
		TexturePortalFX highResolutionPortal(14);
		highResolutionPortal.onTick();
		ok &= expect(highResolutionPortal.imageData.size() == 32 * 32 * 4, "HD portal texture fx should use the selected tile size");
		TileSize::set(16);

		if (ok)
		{
			std::cout << "block-smoke: PASS" << std::endl;
			return 0;
		}
		std::cerr << "block-smoke: FAIL" << std::endl;
		return 1;
	}
	catch (const std::exception &e)
	{
		std::cerr << "block-smoke exception: " << e.what() << std::endl;
		return 2;
	}
	catch (...)
	{
		std::cerr << "block-smoke unknown exception" << std::endl;
		return 3;
	}
}

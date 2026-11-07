#include "tools/StructureSmoke.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "java/File.h"
#include "world/item/Items.h"
#include "world/stats/StatList.h"
#include "world/entity/Entity.h"
#include "world/entity/npc/Villager.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/levelgen/RandomLevelSource.h"
#include "world/level/levelgen/structure/VillageFeature.h"
#include "world/level/levelgen/structure/MineShaftFeature.h"
#include "world/level/levelgen/structure/FortressFeature.h"
#include "world/entity/monster/Blaze.h"
#include "world/entity/monster/LavaSlime.h"
#include "world/level/tile/EndPortalFrameTile.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/FenceTile.h"
#include "world/level/tile/StoneBrickTile.h"
#include "world/level/tile/WoodTile.h"
#include "world/level/tile/NetherWartTile.h"
#include "world/level/tile/MobSpawnerTile.h"
#include "world/level/tile/entity/MobSpawnerTileEntity.h"

namespace
{
	bool expect(bool condition, const char *message)
	{
		if (!condition)
			std::cerr << "FAIL: " << message << std::endl;
		return condition;
	}

	struct MineProbe : public MineShaftFeature
	{
		explicit MineProbe(Level &lvl) { level = &lvl; random.setSeed(lvl.seed); }
		std::unique_ptr<StructureStart> make(int_t x, int_t z) { return createStructureStart(x, z); }
	};

	struct FortressProbe : public FortressFeature
	{
		explicit FortressProbe(Level &lvl) { level = &lvl; }
		bool isFortressChunk(int_t x, int_t z) { return isFeatureChunk(x, z); }
		std::unique_ptr<StructureStart> make(int_t x, int_t z) { return createStructureStart(x, z); }
	};

	struct VillageProbe : public VillageFeature
	{
		explicit VillageProbe(Level &lvl) { level = &lvl; }
		bool isVillageChunk(int_t x, int_t z) { return isFeatureChunk(x, z); }
	};

	// loads chunks around a block position so structure pieces get placed
	void loadAround(Level &level, int_t bx, int_t bz, int_t radius)
	{
		int_t cx = bx >> 4;
		int_t cz = bz >> 4;
		for (int_t x = cx - radius; x <= cx + radius; ++x)
			for (int_t z = cz - radius; z <= cz + radius; ++z)
				level.getChunk(x, z);
	}

	int_t countTiles(Level &level, int_t bx, int_t bz, int_t r, int_t y0, int_t y1, int_t tile)
	{
		int_t n = 0;
		for (int_t x = bx - r; x <= bx + r; ++x)
			for (int_t z = bz - r; z <= bz + r; ++z)
				for (int_t y = y0; y <= y1; ++y)
					if (level.getTile(x, y, z) == tile)
						++n;
		return n;
	}
}

int runStructureSmoke()
{
	Tile::initTiles();
	Items::initItems();
	StatList::init();

	bool ok = true;
	const long_t seed = 1234567890LL;
	// always a fresh world: chunks saved by an earlier run would skip generation
	{
		std::unique_ptr<File> workdir(File::open(u"build/structure-smoke-workdir"));
		Level::deleteLevel(*workdir, u"structure-smoke-world");
	}
	Level level(File::open(u"build/structure-smoke-workdir"), u"structure-smoke-world", seed);

	std::cerr << "structure-smoke: stronghold" << std::endl;
	{
		RandomLevelSource source(level, seed);
		int_t sx = 0, sy = 0, sz = 0;
		bool found = source.findNearestStronghold(0, 64, 0, sx, sy, sz);
		ok &= expect(found, "a stronghold position should be known for the seed");
		if (found)
		{
			std::cerr << "structure-smoke: stronghold at " << sx << " " << sy << " " << sz << std::endl;
			loadAround(level, sx, sz, 7);
			int_t frames = countTiles(level, sx, sz, 64, 1, 80, Tile::endPortalFrame.id);
			std::cerr << "structure-smoke: end portal frames " << frames << std::endl;
			ok &= expect(frames == 12, "the stronghold portal room should hold 12 end portal frames");
			int_t bricks = countTiles(level, sx, sz, 24, 1, 80, Tile::stoneBrick.id);
			ok &= expect(bricks > 200, "the stronghold should be built from stone bricks");
		}
	}

	std::cerr << "structure-smoke: village" << std::endl;
	{
		VillageProbe probe(level);
		int_t vx = 0, vz = 0;
		bool found = false;
		for (int_t r = 0; r < 96 && !found; ++r)
			for (int_t x = -r; x <= r && !found; ++x)
				for (int_t z = -r; z <= r && !found; ++z)
				{
					if (std::max(std::abs(x), std::abs(z)) != r)
						continue;
					if (probe.isVillageChunk(x, z))
					{
						vx = x;
						vz = z;
						found = true;
					}
				}
		ok &= expect(found, "a village chunk should exist within 96 chunks");
		if (found)
		{
			int_t bx = (vx << 4) + 8;
			int_t bz = (vz << 4) + 8;
			std::cerr << "structure-smoke: village at chunk " << vx << " " << vz << std::endl;
			loadAround(level, bx, bz, 7);
			int_t villagers = 0;
			for (auto &entity : level.getAllEntities())
				if (std::dynamic_pointer_cast<Villager>(entity) != nullptr)
					++villagers;
			int_t cobble = countTiles(level, bx, bz, 48, 50, 100, Tile::cobblestone.id);
			int_t planks = countTiles(level, bx, bz, 48, 50, 100, Tile::wood.id);
			std::cerr << "structure-smoke: villagers " << villagers << ", cobblestone " << cobble << ", planks " << planks << std::endl;
			ok &= expect(villagers > 0, "a generated village should have villagers");
			ok &= expect(cobble > 50 && planks > 50, "village houses should be built from cobblestone and planks");
		}
	}

	std::cerr << "structure-smoke: mineshaft" << std::endl;
	{
		MineProbe probe(level);
		std::unique_ptr<StructureStart> start = probe.make(40, 40);
		BoundingBox box = start->getBoundingBox();
		std::cerr << "structure-smoke: mineshaft pieces " << start->getPieces().size() << " box " << box.x0 << "," << box.y0 << "," << box.z0 << " .. " << box.x1 << "," << box.y1 << "," << box.z1 << std::endl;
		ok &= expect(start->getPieces().size() > 10, "a mineshaft should branch into more than 10 pieces");
		ok &= expect(box.y1 < 64, "a mineshaft should be moved below sea level");
		Random random(seed);
		for (int_t cx = box.x0 >> 4; cx <= box.x1 >> 4; ++cx)
			for (int_t cz = box.z0 >> 4; cz <= box.z1 >> 4; ++cz)
			{
				level.getChunk(cx, cz);
				start->postProcess(level, random, BoundingBox(cx << 4, cz << 4, (cx << 4) + 15, (cz << 4) + 15));
			}
		int_t fences = 0, planks = 0;
		for (int_t x = box.x0; x <= box.x1; ++x)
			for (int_t z = box.z0; z <= box.z1; ++z)
				for (int_t y = std::max<int_t>(box.y0, 1); y <= box.y1; ++y)
				{
					int_t t = level.getTile(x, y, z);
					fences += t == Tile::fence.id;
					planks += t == Tile::wood.id;
				}
		std::cerr << "structure-smoke: mineshaft fences " << fences << ", planks " << planks << std::endl;
		ok &= expect(fences > 20 && planks > 20, "mineshaft corridors should have fence and plank supports");
	}

	std::cerr << "structure-smoke: nether fortress" << std::endl;
	{
		FortressProbe probe(level);
		int_t fx = 0, fz = 0;
		bool found = false;
		for (int_t x = -64; x <= 64 && !found; ++x)
			for (int_t z = -64; z <= 64 && !found; ++z)
				if (probe.isFortressChunk(x, z))
				{
					fx = x;
					fz = z;
					found = true;
				}
		ok &= expect(found, "a nether fortress chunk should exist within 64 chunks");
		if (found)
		{
			std::cerr << "structure-smoke: fortress at chunk " << fx << " " << fz << std::endl;
			{
				std::unique_ptr<File> workdir(File::open(u"build/structure-smoke-workdir"));
				Level::deleteLevel(*workdir, u"structure-smoke-nether");
			}
			Level nether(File::open(u"build/structure-smoke-workdir"), u"structure-smoke-nether", seed, -1);
			int_t bx = (fx << 4) + 8;
			int_t bz = (fz << 4) + 8;
			loadAround(nether, bx, bz, 6);
			int_t bricks = 0, spawners = 0, warts = 0;
			for (int_t x = bx - 90; x <= bx + 90; ++x)
				for (int_t z = bz - 90; z <= bz + 90; ++z)
					for (int_t y = 30; y <= 100; ++y)
					{
						int_t t = nether.getTile(x, y, z);
						bricks += t == Tile::netherBrick.id;
						warts += t == Tile::netherWart.id;
						if (t == Tile::mobSpawner.id)
						{
							auto spawner = std::dynamic_pointer_cast<MobSpawnerTileEntity>(nether.getTileEntity(x, y, z));
							if (spawner != nullptr && spawner->getEntityId() == u"Blaze")
								spawners++;
						}
					}
			std::cerr << "structure-smoke: fortress nether bricks " << bricks << ", blaze spawners " << spawners << ", nether wart " << warts << std::endl;
			ok &= expect(bricks > 1000, "the nether should generate a fortress of nether bricks");
			const std::vector<BiomeSpawnEntry> *mobs = nether.getChunkSource()->getStructureMonsters(bx, 64, bz);
			(void)mobs;
		}
	}

	std::cerr << "structure-smoke: blaze and magma cube" << std::endl;
	{
		auto blaze = std::make_shared<Blaze>(level);
		blaze->moveTo(0.5, 120.0, 0.5, 0.0f, 0.0f);
		level.addEntity(blaze);
		auto cube = std::make_shared<LavaSlime>(level);
		cube->setSlimeSize(4);
		cube->moveTo(4.5, 120.0, 0.5, 0.0f, 0.0f);
		level.addEntity(cube);
		for (int_t i = 0; i < 40; ++i)
		{
			blaze->tick();
			cube->tick();
		}
		ok &= expect(blaze->y < 120.0 && blaze->y > 100.0, "blazes fall slowly (60% of the speed each tick)");
		ok &= expect(cube->getSlimeSize() == 4 && cube->bbWidth > 2.3f, "a size 4 magma cube is 2.4 blocks wide");
		ok &= expect(!cube->isOnFire(), "magma cubes are never drawn burning");
	}

	std::cerr << "structure-smoke: 1.0 player abilities" << std::endl;
	{
		struct TestPlayer : public Player
		{
			using Player::Player;
			using Player::causeFallDamage;
			using Player::travel;
		};
		auto player = std::make_shared<TestPlayer>(level);
		player->moveTo(0.5, 110.0, 0.5, 0.0f, 0.0f);
		level.addEntity(player);
		player->abilities.invulnerable = player->abilities.mayfly = player->abilities.instabuild = true;
		int_t healthBefore = player->health;
		ok &= expect(!player->hurt(nullptr, 5) && player->health == healthBefore, "invulnerable players take no damage");
		player->causeFallDamage(30.0f);
		ok &= expect(player->health == healthBefore, "players that may fly take no fall damage");
		player->abilities.flying = true;
		player->yd = 0.0;
		double startY = player->y;
		for (int_t i = 0; i < 20; i++)
			player->travel(0.0f, 0.0f);
		ok &= expect(std::abs(player->y - startY) < 0.01, "a flying player does not fall");
		player->abilities.flying = false;
		player->yd = 0.0;
		for (int_t i = 0; i < 20; i++)
			player->travel(0.0f, 0.0f);
		ok &= expect(player->y < startY - 1.0, "without flight gravity pulls the player down");
		player->foodExhaustionLevel = 0.0f;
		player->addExhaustion(3.0f);
		ok &= expect(player->foodExhaustionLevel == 0.0f, "invulnerable players do not get hungry");
		level.removeEntity(player);
	}

	std::cerr << "structure-smoke: " << (ok ? "PASS" : "FAIL") << std::endl;
	return ok ? 0 : 1;
}

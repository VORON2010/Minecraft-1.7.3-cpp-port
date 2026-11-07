#include "tools/EndSmoke.h"

#include <cmath>
#include <iostream>
#include <memory>

#include "java/File.h"
#include "world/entity/Entity.h"
#include "world/entity/boss/EnderDragon.h"
#include "world/entity/item/EnderCrystal.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/dimension/Dimension.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/EndPortalTile.h"
#include "world/level/tile/DragonEggTile.h"
#include "world/stats/StatList.h"

namespace
{
	bool expect(bool condition, const char *message)
	{
		if (!condition)
			std::cerr << "FAIL: " << message << std::endl;
		return condition;
	}
}

int runEndSmoke()
{
	Tile::initTiles();
	Items::initItems();
	StatList::init();

	bool ok = true;
	const long_t seed = 1234567890LL;
	{
		std::unique_ptr<File> workdir(File::open(u"build/end-smoke-workdir"));
		Level::deleteLevel(*workdir, u"end-smoke-world");
	}
	{
		Level end(File::open(u"build/end-smoke-workdir"), u"end-smoke-world", seed, 1);
		ok &= expect(end.dimension->id == 1, "dimension 1 is the End");
		ok &= expect(end.dimension->hasCeiling && end.dimension->foggy, "the End has no sky light and no day cycle");
		ok &= expect(!end.dimension->mayRespawn(), "players do not respawn in the End");
		int_t fx = 0, fy = 0, fz = 0;
		ok &= expect(end.dimension->getForcedSpawnPoint(fx, fy, fz) && fx == 100 && fy == 50 && fz == 0, "the End entry point is 100,50,0");

		for (int_t x = -6; x <= 6; ++x)
			for (int_t z = -6; z <= 6; ++z)
				end.getChunk(x, z);

		int_t stone = 0, obsidian = 0, bedrock = 0, other = 0;
		for (int_t x = -96; x < 96; ++x)
			for (int_t z = -96; z < 96; ++z)
				for (int_t y = 0; y < 128; ++y)
				{
					int_t t = end.getTile(x, y, z);
					if (t == 0)
						continue;
					if (t == Tile::endStone.id)
						stone++;
					else if (t == Tile::obsidian.id)
						obsidian++;
					else if (t == Tile::bedrock.id)
						bedrock++;
					else
						other++;
				}
		int_t dragons = 0, crystals = 0;
		std::shared_ptr<EnderDragon> dragon;
		for (auto &e : end.entities)
		{
			if (auto d = std::dynamic_pointer_cast<EnderDragon>(e))
			{
				dragons++;
				dragon = d;
			}
			if (std::dynamic_pointer_cast<EnderCrystal>(e) != nullptr)
				crystals++;
		}
		std::cerr << "end-smoke: end stone " << stone << ", obsidian " << obsidian << ", bedrock " << bedrock << ", other " << other
				  << ", dragons " << dragons << ", crystals " << crystals << std::endl;
		ok &= expect(stone > 50000, "the island is made of end stone");
		ok &= expect(obsidian > 200, "obsidian pillars stand on the island");
		ok &= expect(crystals >= 2 && crystals == bedrock, "every pillar has a crystal on a bedrock block");
		ok &= expect(other == 0, "the End has only end stone, obsidian and bedrock");
		ok &= expect(dragons == 1, "the dragon spawns once, in chunk 0,0");

		if (dragon != nullptr)
		{
			ok &= expect(dragon->getSubEntities().size() == 7, "the dragon has seven hit boxes");
			for (int_t i = 0; i < 200; ++i)
				end.tickEntities();
			ok &= expect(!dragon->removed && dragon->health > 0, "the dragon flies around without a player");
			std::cerr << "end-smoke: dragon at " << dragon->x << " " << dragon->y << " " << dragon->z << std::endl;

			dragon->health = 0;
			for (int_t i = 0; i < 260 && !dragon->removed; ++i)
				end.tickEntities();
			ok &= expect(dragon->removed, "the dying dragon is removed after 200 ticks");
			int_t portal = 0, eggs = 0;
			int_t dx = static_cast<int_t>(std::floor(dragon->x)), dz = static_cast<int_t>(std::floor(dragon->z));
			for (int_t x = dx - 10; x <= dx + 10; ++x)
				for (int_t z = dz - 10; z <= dz + 10; ++z)
					for (int_t y = 0; y < 128; ++y)
					{
						int_t t = end.getTile(x, y, z);
						portal += t == Tile::endPortal.id;
						eggs += t == Tile::dragonEgg.id;
					}
			std::cerr << "end-smoke: exit portal blocks " << portal << ", eggs " << eggs << std::endl;
			ok &= expect(portal == 20, "the exit portal is a disc of 20 portal blocks around the bedrock column");
			ok &= expect(eggs == 1, "the dragon egg sits on top of the exit portal");
		}
	}
	{
		std::unique_ptr<File> workdir(File::open(u"build/end-smoke-workdir"));
		Level::deleteLevel(*workdir, u"end-smoke-world");
	}
	std::cerr << "end-smoke: " << (ok ? "PASS" : "FAIL") << std::endl;
	return ok ? 0 : 1;
}

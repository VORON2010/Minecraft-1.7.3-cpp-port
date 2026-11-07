#include "world/level/biome/Biome.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "world/entity/animal/Chicken.h"
#include "world/entity/animal/Cow.h"
#include "world/entity/animal/Pig.h"
#include "world/entity/animal/Sheep.h"
#include "world/entity/animal/Squid.h"
#include "world/entity/animal/Wolf.h"
#include "world/entity/monster/Creeper.h"
#include "world/entity/monster/Enderman.h"
#include "world/entity/monster/Ghast.h"
#include "world/entity/monster/PigZombie.h"
#include "world/entity/monster/Skeleton.h"
#include "world/entity/monster/Slime.h"
#include "world/entity/monster/Spider.h"
#include "world/entity/monster/Zombie.h"
#include "world/entity/boss/EnderDragon.h"
#include "world/level/Level.h"
#include "world/level/levelgen/feature/EndPillarFeature.h"
#include "world/level/FoliageColor.h"
#include "world/level/GrassColor.h"
#include "world/level/LevelSource.h"
#include "world/level/biome/BiomeDecorator.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/levelgen/feature/BigTreeFeature.h"
#include "world/level/levelgen/feature/ForestFeature.h"
#include "world/level/levelgen/feature/SwampTreeFeature.h"
#include "world/level/levelgen/feature/Taiga1Feature.h"
#include "world/level/levelgen/feature/Taiga2Feature.h"
#include "world/level/levelgen/feature/TreeFeature.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/MyceliumTile.h"

std::array<Biome *, 256> Biome::biomes = {};

Biome *Biome::ocean = nullptr;
Biome *Biome::plains = nullptr;
Biome *Biome::desert = nullptr;
Biome *Biome::extremeHills = nullptr;
Biome *Biome::forest = nullptr;
Biome *Biome::taiga = nullptr;
Biome *Biome::swampland = nullptr;
Biome *Biome::river = nullptr;
Biome *Biome::hell = nullptr;
Biome *Biome::sky = nullptr;
Biome *Biome::frozenOcean = nullptr;
Biome *Biome::frozenRiver = nullptr;
Biome *Biome::icePlains = nullptr;
Biome *Biome::iceMountains = nullptr;
Biome *Biome::mushroomIsland = nullptr;
Biome *Biome::mushroomIslandShore = nullptr;

namespace
{
	template <typename T>
	BiomeSpawnEntry spawn(int_t weight, int_t minCount, int_t maxCount)
	{
		BiomeSpawnEntry entry;
		entry.create = [](Level &level) -> std::shared_ptr<Mob> { return std::make_shared<T>(level); };
		entry.weight = weight;
		entry.minCount = minCount;
		entry.maxCount = maxCount;
		return entry;
	}

	// java.awt.Color.HSBtoRGB
	int_t hsbToRgb(float hue, float saturation, float brightness)
	{
		int_t r = 0, g = 0, b = 0;
		if (saturation == 0.0f)
		{
			r = g = b = static_cast<int_t>(brightness * 255.0f + 0.5f);
		}
		else
		{
			float h = (hue - std::floor(hue)) * 6.0f;
			float f = h - std::floor(h);
			float p = brightness * (1.0f - saturation);
			float q = brightness * (1.0f - saturation * f);
			float t = brightness * (1.0f - saturation * (1.0f - f));
			switch (static_cast<int_t>(h))
			{
			case 0: r = static_cast<int_t>(brightness * 255.0f + 0.5f); g = static_cast<int_t>(t * 255.0f + 0.5f); b = static_cast<int_t>(p * 255.0f + 0.5f); break;
			case 1: r = static_cast<int_t>(q * 255.0f + 0.5f); g = static_cast<int_t>(brightness * 255.0f + 0.5f); b = static_cast<int_t>(p * 255.0f + 0.5f); break;
			case 2: r = static_cast<int_t>(p * 255.0f + 0.5f); g = static_cast<int_t>(brightness * 255.0f + 0.5f); b = static_cast<int_t>(t * 255.0f + 0.5f); break;
			case 3: r = static_cast<int_t>(p * 255.0f + 0.5f); g = static_cast<int_t>(q * 255.0f + 0.5f); b = static_cast<int_t>(brightness * 255.0f + 0.5f); break;
			case 4: r = static_cast<int_t>(t * 255.0f + 0.5f); g = static_cast<int_t>(p * 255.0f + 0.5f); b = static_cast<int_t>(brightness * 255.0f + 0.5f); break;
			case 5: r = static_cast<int_t>(brightness * 255.0f + 0.5f); g = static_cast<int_t>(p * 255.0f + 0.5f); b = static_cast<int_t>(q * 255.0f + 0.5f); break;
			default: break;
			}
		}
		return static_cast<int_t>(0xFF000000u | static_cast<uint_t>(r) << 16 | static_cast<uint_t>(g) << 8 | static_cast<uint_t>(b));
	}

	class OceanBiome : public Biome
	{
	public:
		explicit OceanBiome(int_t id) : Biome(id) { friendlies.clear(); }
	};

	class PlainsBiome : public Biome
	{
	public:
		explicit PlainsBiome(int_t id) : Biome(id)
		{
			decorator->treeCount = -999;
			decorator->flowerCount = 4;
			decorator->grassCount = 10;
		}
	};

	class DesertBiome : public Biome
	{
	public:
		explicit DesertBiome(int_t id) : Biome(id)
		{
			friendlies.clear();
			topMaterial = static_cast<ubyte_t>(Tile::sand.id);
			material = static_cast<ubyte_t>(Tile::sand.id);
			decorator->treeCount = -999;
			decorator->deadBushCount = 2;
			decorator->reedsCount = 50;
			decorator->cactusCount = 10;
		}
	};

	class ExtremeHillsBiome : public Biome
	{
	public:
		explicit ExtremeHillsBiome(int_t id) : Biome(id) {}
	};

	class ForestBiome : public Biome
	{
	public:
		explicit ForestBiome(int_t id) : Biome(id)
		{
			friendlies.push_back(spawn<Wolf>(5, 4, 4));
			decorator->treeCount = 10;
			decorator->grassCount = 2;
		}

		Feature &getTreeFeature(Random &random) override
		{
			if (random.nextInt(5) == 0)
				return *birchTree;
			return random.nextInt(10) == 0 ? *fancyTree : *normalTree;
		}
	};

	class TaigaBiome : public Biome
	{
	private:
		Taiga1Feature pine;
		Taiga2Feature spruce;

	public:
		explicit TaigaBiome(int_t id) : Biome(id)
		{
			friendlies.push_back(spawn<Wolf>(8, 4, 4));
			decorator->treeCount = 10;
			decorator->grassCount = 1;
		}

		Feature &getTreeFeature(Random &random) override
		{
			if (random.nextInt(3) == 0)
				return pine;
			return spruce;
		}
	};

	class SwampBiome : public Biome
	{
	public:
		explicit SwampBiome(int_t id) : Biome(id)
		{
			decorator->treeCount = 2;
			decorator->flowerCount = -999;
			decorator->deadBushCount = 1;
			decorator->mushroomCount = 8;
			decorator->reedsCount = 10;
			decorator->clayCount = 1;
			decorator->waterlilyCount = 4;
			waterColor = 14745456;
		}

		Feature &getTreeFeature(Random &random) override
		{
			(void)random;
			return *swampTree;
		}

		int_t getGrassColor(LevelSource &level, int_t x, int_t y, int_t z) override
		{
			double t = level.getBiomeSource().getTemperature(x, y, z);
			double d = level.getBiomeSource().getDownfall(x, z);
			return ((GrassColor::get(t, d) & 0xFEFEFE) + 5115470) / 2;
		}

		int_t getFolageColor(LevelSource &level, int_t x, int_t y, int_t z) override
		{
			double t = level.getBiomeSource().getTemperature(x, y, z);
			double d = level.getBiomeSource().getDownfall(x, z);
			return ((FoliageColor::get(t, d) & 0xFEFEFE) + 5115470) / 2;
		}
	};

	class RiverBiome : public Biome
	{
	public:
		explicit RiverBiome(int_t id) : Biome(id) { friendlies.clear(); }
	};

	class HellBiome : public Biome
	{
	public:
		explicit HellBiome(int_t id) : Biome(id)
		{
			enemies.clear();
			friendlies.clear();
			waterFriendlies.clear();
			enemies.push_back(spawn<Ghast>(50, 4, 4));
			enemies.push_back(spawn<PigZombie>(100, 4, 4));
		}
	};

	// 1.0 EndIslandDecorator: ores, an obsidian pillar in one chunk of five, the dragon at 0,0
	class EndIslandDecorator : public BiomeDecorator
	{
		EndPillarFeature pillar = EndPillarFeature(Tile::endStone.id);

	public:
		explicit EndIslandDecorator(Biome &biome) : BiomeDecorator(biome) {}

	protected:
		void decorate() override
		{
			decorateOres();
			if (random->nextInt(5) == 0)
			{
				int_t x = xo + random->nextInt(16) + 8;
				int_t z = zo + random->nextInt(16) + 8;
				int_t y = level->getHeightmap(x, z);
				pillar.place(*level, *random, x, y, z);
			}
			if (xo == 0 && zo == 0)
			{
				auto dragon = std::make_shared<EnderDragon>(*level);
				dragon->moveTo(0.0, 128.0, 0.0, random->nextFloat() * 360.0f, 0.0f);
				level->addEntity(dragon);
			}
		}
	};

	class SkyBiome : public Biome
	{
	public:
		explicit SkyBiome(int_t id) : Biome(id)
		{
			enemies.clear();
			friendlies.clear();
			waterFriendlies.clear();
			enemies.push_back(spawn<Enderman>(10, 4, 4));
			topMaterial = static_cast<ubyte_t>(Tile::dirt.id);
			material = static_cast<ubyte_t>(Tile::dirt.id);
			decorator = std::make_unique<EndIslandDecorator>(*this);
		}

		int_t getSkyColor(float temperature) override
		{
			(void)temperature;
			return 0;
		}
	};

	class IceBiome : public Biome
	{
	public:
		explicit IceBiome(int_t id) : Biome(id) {}
	};

	class MushroomBiome : public Biome
	{
	public:
		explicit MushroomBiome(int_t id) : Biome(id)
		{
			decorator->treeCount = -100;
			decorator->flowerCount = -100;
			decorator->grassCount = -100;
			decorator->mushroomCount = 1;
			decorator->hugeMushrooms = 1;
			topMaterial = static_cast<ubyte_t>(Tile::mycelium.id);
			enemies.clear();
			friendlies.clear();
			waterFriendlies.clear();
		}
	};
}

Biome::Biome(int_t id) : id(id)
{
	topMaterial = static_cast<ubyte_t>(Tile::grass.id);
	material = static_cast<ubyte_t>(Tile::dirt.id);
	normalTree = std::make_unique<TreeFeature>();
	fancyTree = std::make_unique<BigTreeFeature>();
	birchTree = std::make_unique<ForestFeature>();
	swampTree = std::make_unique<SwampTreeFeature>();
	biomes[id] = this;
	decorator = std::make_unique<BiomeDecorator>(*this);

	friendlies.push_back(spawn<Sheep>(12, 4, 4));
	friendlies.push_back(spawn<Pig>(10, 4, 4));
	friendlies.push_back(spawn<Chicken>(10, 4, 4));
	friendlies.push_back(spawn<Cow>(8, 4, 4));
	enemies.push_back(spawn<Spider>(10, 4, 4));
	enemies.push_back(spawn<Zombie>(10, 4, 4));
	enemies.push_back(spawn<Skeleton>(10, 4, 4));
	enemies.push_back(spawn<Creeper>(10, 4, 4));
	enemies.push_back(spawn<Slime>(10, 4, 4));
	enemies.push_back(spawn<Enderman>(1, 1, 4));
	waterFriendlies.push_back(spawn<Squid>(10, 4, 4));
}

Biome::~Biome() = default;

void Biome::init()
{
	if (ocean != nullptr)
		return;
	ocean = &(new OceanBiome(0))->setColor(112).setName(u"Ocean").setDepthAndScale(-1.0f, 0.4f);
	plains = &(new PlainsBiome(1))->setColor(9286496).setName(u"Plains").setTemperatureAndDownfall(0.8f, 0.4f);
	desert = &(new DesertBiome(2))->setColor(16421912).setName(u"Desert").setNoRain().setTemperatureAndDownfall(2.0f, 0.0f).setDepthAndScale(0.1f, 0.2f);
	extremeHills = &(new ExtremeHillsBiome(3))->setColor(6316128).setName(u"Extreme Hills").setDepthAndScale(0.2f, 1.8f).setTemperatureAndDownfall(0.2f, 0.3f);
	forest = &(new ForestBiome(4))->setColor(353825).setName(u"Forest").setLeafColor(5159473).setTemperatureAndDownfall(0.7f, 0.8f);
	taiga = &(new TaigaBiome(5))->setColor(747097).setName(u"Taiga").setLeafColor(5159473).setTemperatureAndDownfall(0.3f, 0.8f).setDepthAndScale(0.1f, 0.4f);
	swampland = &(new SwampBiome(6))->setColor(522674).setName(u"Swampland").setLeafColor(9154376).setDepthAndScale(-0.2f, 0.1f).setTemperatureAndDownfall(0.8f, 0.9f);
	river = &(new RiverBiome(7))->setColor(255).setName(u"River").setDepthAndScale(-0.5f, 0.0f);
	hell = &(new HellBiome(8))->setColor(16711680).setName(u"Hell").setNoRain().setTemperatureAndDownfall(2.0f, 0.0f);
	sky = &(new SkyBiome(9))->setColor(8421631).setName(u"Sky").setNoRain();
	frozenOcean = &(new OceanBiome(10))->setColor(9474208).setName(u"FrozenOcean").setDepthAndScale(-1.0f, 0.5f).setTemperatureAndDownfall(0.0f, 0.5f);
	frozenRiver = &(new RiverBiome(11))->setColor(10526975).setName(u"FrozenRiver").setDepthAndScale(-0.5f, 0.0f).setTemperatureAndDownfall(0.0f, 0.5f);
	icePlains = &(new IceBiome(12))->setColor(16777215).setName(u"Ice Plains").setTemperatureAndDownfall(0.0f, 0.5f);
	iceMountains = &(new IceBiome(13))->setColor(10526880).setName(u"Ice Mountains").setDepthAndScale(0.2f, 1.8f).setTemperatureAndDownfall(0.0f, 0.5f);
	mushroomIsland = &(new MushroomBiome(14))->setColor(16711935).setName(u"MushroomIsland").setTemperatureAndDownfall(0.9f, 1.0f).setDepthAndScale(0.2f, 1.0f);
	mushroomIslandShore = &(new MushroomBiome(15))->setColor(10486015).setName(u"MushroomIslandShore").setTemperatureAndDownfall(0.9f, 1.0f).setDepthAndScale(-1.0f, 0.1f);
}

Biome &Biome::setTemperatureAndDownfall(float temperature, float downfall)
{
	if (temperature > 0.1f && temperature < 0.2f)
		throw std::invalid_argument("Please avoid temperatures in the range 0.1 - 0.2 because of snow");
	this->temperature = temperature;
	this->downfall = downfall;
	return *this;
}

Biome &Biome::setDepthAndScale(float depth, float scale)
{
	this->depth = depth;
	this->scale = scale;
	return *this;
}

Feature &Biome::getTreeFeature(Random &random)
{
	return random.nextInt(10) == 0 ? *fancyTree : *normalTree;
}

int_t Biome::getSkyColor(float temperature)
{
	temperature /= 3.0f;
	temperature = std::max(-1.0f, std::min(1.0f, temperature));
	return hsbToRgb(0.62222224f - temperature * 0.05f, 0.5f + temperature * 0.1f, 1.0f);
}

const std::vector<BiomeSpawnEntry> &Biome::getMobs(MobCategory category) const
{
	if (category == MobCategory::Monster)
		return enemies;
	if (category == MobCategory::Creature)
		return friendlies;
	return waterFriendlies;
}

void Biome::decorate(Level &level, Random &random, int_t x, int_t z)
{
	decorator->decorate(level, random, x, z);
}

int_t Biome::getGrassColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	double t = level.getBiomeSource().getTemperature(x, y, z);
	double d = level.getBiomeSource().getDownfall(x, z);
	return GrassColor::get(t, d);
}

int_t Biome::getFolageColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	double t = level.getBiomeSource().getTemperature(x, y, z);
	double d = level.getBiomeSource().getDownfall(x, z);
	return FoliageColor::get(t, d);
}

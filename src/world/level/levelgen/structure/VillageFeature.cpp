#include "world/level/levelgen/structure/VillageFeature.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>

#include "java/Random.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/biome/Biome.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/material/Material.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/ChestTileEntity.h"
#include "world/level/tile/entity/MobSpawnerTileEntity.h"
#include "world/entity/npc/Villager.h"

namespace
{
struct MathHelper
{
	static int_t nextInt(Random &random, int_t a, int_t b) { return a >= b ? a : random.nextInt(b - a + 1) + a; }
};

template <typename T>
T jremove(std::vector<T> &v, int_t i)
{
	T x = v[static_cast<size_t>(i)];
	v.erase(v.begin() + i);
	return x;
}
}

#include "world/level/tile/BedTile.h"
#include "world/level/tile/BookshelfTile.h"
#include "world/level/tile/ButtonTile.h"
#include "world/level/tile/CactusTile.h"
#include "world/level/tile/CakeTile.h"
#include "world/level/tile/ChestTile.h"
#include "world/level/tile/ClayTile.h"
#include "world/level/tile/ClothTile.h"
#include "world/level/tile/CropsTile.h"
#include "world/level/tile/DeadBushTile.h"
#include "world/level/tile/DetectorRailTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/DispenserTile.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/EndPortalFrameTile.h"
#include "world/level/tile/FarmlandTile.h"
#include "world/level/tile/FenceGateTile.h"
#include "world/level/tile/FenceTile.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/FlowerTile.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/GlassTile.h"
#include "world/level/tile/GlowStoneTile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/GravelTile.h"
#include "world/level/tile/HugeMushroomTile.h"
#include "world/level/tile/IceTile.h"
#include "world/level/tile/InfestedTile.h"
#include "world/level/tile/JukeboxTile.h"
#include "world/level/tile/LadderTile.h"
#include "world/level/tile/LeafTile.h"
#include "world/level/tile/LeverTile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/LockedChestTile.h"
#include "world/level/tile/MelonTile.h"
#include "world/level/tile/MobSpawnerTile.h"
#include "world/level/tile/MushroomTile.h"
#include "world/level/tile/MyceliumTile.h"
#include "world/level/tile/NetherWartTile.h"
#include "world/level/tile/NotGateTile.h"
#include "world/level/tile/NoteTile.h"
#include "world/level/tile/OreTile.h"
#include "world/level/tile/PaneTile.h"
#include "world/level/tile/PistonBaseTile.h"
#include "world/level/tile/PistonExtensionTile.h"
#include "world/level/tile/PistonMovingTile.h"
#include "world/level/tile/PistonTextures.h"
#include "world/level/tile/PortalTile.h"
#include "world/level/tile/PressurePlateTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/level/tile/RailLogic.h"
#include "world/level/tile/RailTile.h"
#include "world/level/tile/RedStoneDustTile.h"
#include "world/level/tile/RedstoneOreTile.h"
#include "world/level/tile/ReedTile.h"
#include "world/level/tile/RepeaterTile.h"
#include "world/level/tile/SandStoneTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/SaplingTile.h"
#include "world/level/tile/SignTile.h"
#include "world/level/tile/SlabTile.h"
#include "world/level/tile/SnowBlockTile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/SoulSandTile.h"
#include "world/level/tile/SpongeTile.h"
#include "world/level/tile/StairTile.h"
#include "world/level/tile/StemTile.h"
#include "world/level/tile/StepSound.h"
#include "world/level/tile/StoneBrickTile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/TNTTile.h"
#include "world/level/tile/TallGrassTile.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/TorchTile.h"
#include "world/level/tile/TransparentTile.h"
#include "world/level/tile/TrapDoorTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/VineTile.h"
#include "world/level/tile/WaterlilyTile.h"
#include "world/level/tile/WebTile.h"
#include "world/level/tile/WoodTile.h"
#include "world/level/tile/WorkbenchTile.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/ItemArmor.h"

class VillagePieces;
class VillagePieces__VillagePiece;
class VillagePieces__BlacksmithHouse;
class VillagePieces__ButcherHouse;
class VillagePieces__DoubleFarm;
class VillagePieces__Farm;
class VillagePieces__FarmerHouse;
class VillagePieces__LampPost;
class VillagePieces__LargeHouse;
class VillagePieces__LibrarianHouse;
class VillagePieces__PriestHouse;
class VillagePieces__RoadPiece;
class VillagePieces__Road;
class VillagePieces__SmallHouse;
class VillagePieces__Well;
class VillagePieces__Start;
class VillagePieces__VillagePieceWeight;
using WeightList = std::vector<std::shared_ptr<VillagePieces__VillagePieceWeight>>;
std::vector<Biome *> &villageBiomes();

enum : int_t { CLS_VillagePieces = 1, CLS_VillagePieces__VillagePiece = 2, CLS_VillagePieces__BlacksmithHouse = 3, CLS_VillagePieces__ButcherHouse = 4, CLS_VillagePieces__DoubleFarm = 5, CLS_VillagePieces__Farm = 6, CLS_VillagePieces__FarmerHouse = 7, CLS_VillagePieces__LampPost = 8, CLS_VillagePieces__LargeHouse = 9, CLS_VillagePieces__LibrarianHouse = 10, CLS_VillagePieces__PriestHouse = 11, CLS_VillagePieces__RoadPiece = 12, CLS_VillagePieces__Road = 13, CLS_VillagePieces__SmallHouse = 14, CLS_VillagePieces__Well = 15, CLS_VillagePieces__Start = 16, CLS_VillagePieces__VillagePieceWeight = 17 };

class VillagePieces
{
public:
	static WeightList getPieceWeights(Random & random, int_t size);
	static int_t updateTotalWeight(WeightList & weights);
	static std::shared_ptr<VillagePieces__VillagePiece> createBuildingPiece(std::shared_ptr<VillagePieces__VillagePieceWeight> weight, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	static std::shared_ptr<VillagePieces__VillagePiece> generateNextBuildingPiece(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	static std::shared_ptr<StructurePiece> generateBuildingPiece(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	static std::shared_ptr<StructurePiece> generateRoadPiece(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
};

class VillagePieces__VillagePiece : public StructurePiece
{
public:
	using Base = StructurePiece;
	int_t villageCount{};
	VillagePieces__VillagePiece(int_t var1);
	virtual std::shared_ptr<StructurePiece> generatePieceLeft(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset);
	virtual std::shared_ptr<StructurePiece> generatePieceRight(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset);
	virtual int_t updateHeight(Level & level, const BoundingBox & bb);
	static bool isValidBounds(const BoundingBox & bb);
	virtual void spawnVillagers(Level & level, const BoundingBox & bb, int_t x, int_t y, int_t z, int_t max);
	virtual int_t getVillagerProfession(int_t index);
};

class VillagePieces__BlacksmithHouse : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	VillagePieces__BlacksmithHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<VillagePieces__BlacksmithHouse> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
	virtual int_t getVillagerProfession(int_t index);
};

class VillagePieces__ButcherHouse : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	VillagePieces__ButcherHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<VillagePieces__ButcherHouse> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
	virtual int_t getVillagerProfession(int_t index);
};

class VillagePieces__DoubleFarm : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	VillagePieces__DoubleFarm(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<VillagePieces__DoubleFarm> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class VillagePieces__Farm : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	VillagePieces__Farm(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<VillagePieces__Farm> create(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class VillagePieces__FarmerHouse : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	bool lowCeiling{};
	int_t tablePosition{};
	VillagePieces__FarmerHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<VillagePieces__FarmerHouse> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class VillagePieces__LampPost : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	VillagePieces__LampPost(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static BoundingBox findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class VillagePieces__LargeHouse : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	VillagePieces__LargeHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<VillagePieces__LargeHouse> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class VillagePieces__LibrarianHouse : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	VillagePieces__LibrarianHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<VillagePieces__LibrarianHouse> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
	virtual int_t getVillagerProfession(int_t index);
};

class VillagePieces__PriestHouse : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	VillagePieces__PriestHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<VillagePieces__PriestHouse> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
	virtual int_t getVillagerProfession(int_t index);
};

class VillagePieces__RoadPiece : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	VillagePieces__RoadPiece(int_t var1);
};

class VillagePieces__Road : public VillagePieces__RoadPiece
{
public:
	using Base = VillagePieces__RoadPiece;
	int_t length{};
	VillagePieces__Road(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static BoundingBox findSize(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class VillagePieces__SmallHouse : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	int_t hpos = -1;
	bool terrace{};
	VillagePieces__SmallHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<VillagePieces__SmallHouse> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class VillagePieces__Well : public VillagePieces__VillagePiece
{
public:
	using Base = VillagePieces__VillagePiece;
	bool f_1400559{};
	int_t hpos = -1;
	VillagePieces__Well(int_t generationDepth, Random & random, int_t x, int_t z);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class VillagePieces__Start : public VillagePieces__Well
{
public:
	using Base = VillagePieces__Well;
	BiomeSource * biomeSource{};
	int_t size{};
	std::shared_ptr<VillagePieces__VillagePieceWeight> previous{};
	WeightList weights;
	PieceList buildingPieces;
	PieceList roadPieces;
	VillagePieces__Start(BiomeSource * biomeSource, int_t var2, Random & random, int_t x, int_t z, WeightList & weights, int_t size);
	virtual BiomeSource * getBiomeSource();
};

class VillagePieces__VillagePieceWeight
{
public:
	int_t type{};
	int_t weight{};
	int_t amountGenerated{};
	int_t maxAmount{};
	VillagePieces__VillagePieceWeight(int_t type, int_t weight, int_t maxAmount);
	virtual bool isValid(int_t generationDepth);
	virtual bool isValid();
};

WeightList VillagePieces::getPieceWeights(Random & random, int_t size)
{
      WeightList var2;
      var2.push_back(std::make_shared<VillagePieces__VillagePieceWeight>(CLS_VillagePieces__SmallHouse, 4, MathHelper::nextInt(random, 2 + size, 4 + size * 2)));
      var2.push_back(std::make_shared<VillagePieces__VillagePieceWeight>(CLS_VillagePieces__PriestHouse, 20, MathHelper::nextInt(random, 0 + size, 1 + size)));
      var2.push_back(std::make_shared<VillagePieces__VillagePieceWeight>(CLS_VillagePieces__LibrarianHouse, 20, MathHelper::nextInt(random, 0 + size, 2 + size)));
      var2.push_back(std::make_shared<VillagePieces__VillagePieceWeight>(CLS_VillagePieces__FarmerHouse, 3, MathHelper::nextInt(random, 2 + size, 5 + size * 3)));
      var2.push_back(std::make_shared<VillagePieces__VillagePieceWeight>(CLS_VillagePieces__ButcherHouse, 15, MathHelper::nextInt(random, 0 + size, 2 + size)));
      var2.push_back(std::make_shared<VillagePieces__VillagePieceWeight>(CLS_VillagePieces__DoubleFarm, 3, MathHelper::nextInt(random, 1 + size, 4 + size)));
      var2.push_back(std::make_shared<VillagePieces__VillagePieceWeight>(CLS_VillagePieces__Farm, 3, MathHelper::nextInt(random, 2 + size, 4 + size * 2)));
      var2.push_back(std::make_shared<VillagePieces__VillagePieceWeight>(CLS_VillagePieces__BlacksmithHouse, 15, MathHelper::nextInt(random, 0, 1 + size)));
      var2.push_back(std::make_shared<VillagePieces__VillagePieceWeight>(CLS_VillagePieces__LargeHouse, 8, MathHelper::nextInt(random, 0 + size, 3 + size * 2)));
      var2.erase(std::remove_if(var2.begin(), var2.end(), [](const std::shared_ptr<VillagePieces__VillagePieceWeight> &w) { return w->maxAmount == 0; }), var2.end());

      return var2;
   }

int_t VillagePieces::updateTotalWeight(WeightList & weights)
{
      bool var1 = false;
      int_t var2 = 0;

      for (auto var4 : weights) {
         if (var4->maxAmount > 0 && var4->amountGenerated < var4->maxAmount) {
            var1 = true;
         }

         var2 += var4->weight;
      }

      return var1 ? var2 : -1;
   }

std::shared_ptr<VillagePieces__VillagePiece> VillagePieces::createBuildingPiece(std::shared_ptr<VillagePieces__VillagePieceWeight> weight, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      int_t var8 = weight->type;
      std::shared_ptr<StructurePiece> var9 = nullptr;
      if (var8 == CLS_VillagePieces__SmallHouse) {
         var9 = VillagePieces__SmallHouse::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_VillagePieces__PriestHouse) {
         var9 = VillagePieces__PriestHouse::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_VillagePieces__LibrarianHouse) {
         var9 = VillagePieces__LibrarianHouse::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_VillagePieces__FarmerHouse) {
         var9 = VillagePieces__FarmerHouse::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_VillagePieces__ButcherHouse) {
         var9 = VillagePieces__ButcherHouse::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_VillagePieces__DoubleFarm) {
         var9 = VillagePieces__DoubleFarm::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_VillagePieces__Farm) {
         var9 = VillagePieces__Farm::create(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_VillagePieces__BlacksmithHouse) {
         var9 = VillagePieces__BlacksmithHouse::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_VillagePieces__LargeHouse) {
         var9 = VillagePieces__LargeHouse::of(pieces, random, x, y, z, facing, generationDepth);
      }

      return std::dynamic_pointer_cast<VillagePieces__VillagePiece>(var9);
   }

std::shared_ptr<VillagePieces__VillagePiece> VillagePieces::generateNextBuildingPiece(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      int_t var8 = updateTotalWeight(startPiece->weights);
      if (var8 <= 0) {
         return nullptr;
      } else {
         int_t var9 = 0;

         while (var9 < 5) {
            var9++;
            int_t var10 = random.nextInt(var8);

            for (auto var12 : startPiece->weights) {
               var10 -= var12->weight;
               if (var10 < 0) {
                  if (!var12->isValid(generationDepth) || var12 == startPiece->previous && startPiece->weights.size() > 1) {
                     break;
                  }

                  std::shared_ptr<VillagePieces__VillagePiece> var13 = createBuildingPiece(var12, pieces, random, x, y, z, facing, generationDepth);
                  if (var13 != nullptr) {
                     var12->amountGenerated++;
                     startPiece->previous = var12;
                     if (!var12->isValid()) {
                        startPiece->weights.erase(std::find(startPiece->weights.begin(), startPiece->weights.end(), var12));
                     }

                     return var13;
                  }
               }
            }
         }

         BoundingBox var14 = VillagePieces__LampPost::findSize(pieces, random, x, y, z, facing);
         return var14.valid ? std::make_shared<VillagePieces__LampPost>(generationDepth, random, var14, facing) : nullptr;
      }
   }

std::shared_ptr<StructurePiece> VillagePieces::generateBuildingPiece(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      if (generationDepth > 50) {
         return nullptr;
      } else if (std::abs(x - startPiece->getBoundingBox().x0) <= 112 && std::abs(z - startPiece->getBoundingBox().z0) <= 112) {
         std::shared_ptr<VillagePieces__VillagePiece> var8 = generateNextBuildingPiece(startPiece, pieces, random, x, y, z, facing, generationDepth + 1);
         if (var8 != nullptr) {
            int_t var9 = (var8->boundingBox.x0 + var8->boundingBox.x1) / 2;
            int_t var10 = (var8->boundingBox.z0 + var8->boundingBox.z1) / 2;
            int_t var11 = var8->boundingBox.x1 - var8->boundingBox.x0;
            int_t var12 = var8->boundingBox.z1 - var8->boundingBox.z0;
            int_t var13 = var11 > var12 ? var11 : var12;
            if (startPiece->getBiomeSource()->containsOnly(var9, var10, var13 / 2 + 4, villageBiomes())) {
               pieces.push_back(var8);
               startPiece->buildingPieces.push_back(var8);
               return var8;
            }
         }

         return nullptr;
      } else {
         return nullptr;
      }
   }

std::shared_ptr<StructurePiece> VillagePieces::generateRoadPiece(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      if (generationDepth > 3 + startPiece->size) {
         return nullptr;
      } else if (std::abs(x - startPiece->getBoundingBox().x0) <= 112 && std::abs(z - startPiece->getBoundingBox().z0) <= 112) {
         BoundingBox var8 = VillagePieces__Road::findSize(startPiece, pieces, random, x, y, z, facing);
         if (var8.valid && var8.y0 > 10) {
            std::shared_ptr<VillagePieces__Road> var9 = std::make_shared<VillagePieces__Road>(generationDepth, random, var8, facing);
            int_t var10 = (var9->boundingBox.x0 + var9->boundingBox.x1) / 2;
            int_t var11 = (var9->boundingBox.z0 + var9->boundingBox.z1) / 2;
            int_t var12 = var9->boundingBox.x1 - var9->boundingBox.x0;
            int_t var13 = var9->boundingBox.z1 - var9->boundingBox.z0;
            int_t var14 = var12 > var13 ? var12 : var13;
            if (startPiece->getBiomeSource()->containsOnly(var10, var11, var14 / 2 + 4, villageBiomes())) {
               pieces.push_back(var9);
               startPiece->roadPieces.push_back(var9);
               return var9;
            }
         }

         return nullptr;
      } else {
         return nullptr;
      }
   }

VillagePieces__VillagePiece::VillagePieces__VillagePiece(int_t var1) : StructurePiece(var1)
{
   }

std::shared_ptr<StructurePiece> VillagePieces__VillagePiece::generatePieceLeft(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset)
{
      switch (this->orientation) {
         case 0:
            return VillagePieces::generateRoadPiece(
               startPiece, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, 1, this->getGenDepth()
            );
         case 1:
            return VillagePieces::generateRoadPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z0 - 1, 2, this->getGenDepth()
            );
         case 2:
            return VillagePieces::generateRoadPiece(
               startPiece, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, 1, this->getGenDepth()
            );
         case 3:
            return VillagePieces::generateRoadPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z0 - 1, 2, this->getGenDepth()
            );
         default:
            return nullptr;
      }
   }

std::shared_ptr<StructurePiece> VillagePieces__VillagePiece::generatePieceRight(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset)
{
      switch (this->orientation) {
         case 0:
            return VillagePieces::generateRoadPiece(
               startPiece, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, 3, this->getGenDepth()
            );
         case 1:
            return VillagePieces::generateRoadPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z1 + 1, 0, this->getGenDepth()
            );
         case 2:
            return VillagePieces::generateRoadPiece(
               startPiece, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, 3, this->getGenDepth()
            );
         case 3:
            return VillagePieces::generateRoadPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z1 + 1, 0, this->getGenDepth()
            );
         default:
            return nullptr;
      }
   }

int_t VillagePieces__VillagePiece::updateHeight(Level & level, const BoundingBox & bb)
{
      int_t var3 = 0;
      int_t var4 = 0;

      for (int_t var5 = this->boundingBox.z0; var5 <= this->boundingBox.z1; var5++) {
         for (int_t var6 = this->boundingBox.x0; var6 <= this->boundingBox.x1; var6++) {
            if (bb.isInside(var6, 64, var5)) {
               var3 += std::max(level.getTopSolidBlock(var6, var5), 63);
               var4++;
            }
         }
      }

      return var4 == 0 ? -1 : var3 / var4;
   }

bool VillagePieces__VillagePiece::isValidBounds(const BoundingBox & bb)
{
      return bb.valid && bb.y0 > 10;
   }

void VillagePieces__VillagePiece::spawnVillagers(Level & level, const BoundingBox & bb, int_t x, int_t y, int_t z, int_t max)
{
      if (this->villageCount < max) {
         for (int_t var7 = this->villageCount; var7 < max; var7++) {
            int_t var8 = this->getWorldX(x + var7, z);
            int_t var9 = this->getWorldY(y);
            int_t var10 = this->getWorldZ(x + var7, z);
            if (!bb.isInside(var8, var9, var10)) {
               break;
            }

            this->villageCount++;
            std::shared_ptr<Villager> var11 = std::make_shared<Villager>(level, this->getVillagerProfession(var7));
            var11->moveTo((double)var8 + 0.5, (double)var9, (double)var10 + 0.5, 0.0f, 0.0f);
            level.addEntity(var11);
         }
      }
   }

int_t VillagePieces__VillagePiece::getVillagerProfession(int_t index)
{
      return 0;
   }

VillagePieces__BlacksmithHouse::VillagePieces__BlacksmithHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void VillagePieces__BlacksmithHouse::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<VillagePieces__BlacksmithHouse> VillagePieces__BlacksmithHouse::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 10, 6, 7, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<VillagePieces__BlacksmithHouse>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool VillagePieces__BlacksmithHouse::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 6 - 1, 0);
      }

      this->generateBox(level, chunkBB, 0, 1, 0, 9, 4, 6, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 0, 0, 9, 0, 6, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 4, 0, 9, 4, 6, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 5, 0, 9, 5, 6, Tile::slabSingle.id, Tile::slabSingle.id, false);
      this->generateBox(level, chunkBB, 1, 5, 1, 8, 5, 5, 0, 0, false);
      this->generateBox(level, chunkBB, 1, 1, 0, 2, 3, 0, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 1, 0, 0, 4, 0, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 3, 1, 0, 3, 4, 0, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 0, 1, 6, 0, 4, 6, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->placeBlock(level, Tile::wood.id, 0, 3, 3, 1, chunkBB);
      this->generateBox(level, chunkBB, 3, 1, 2, 3, 3, 2, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 4, 1, 3, 5, 3, 3, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 1, 1, 0, 3, 5, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 1, 6, 5, 3, 6, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 5, 1, 0, 5, 3, 0, Tile::fence.id, Tile::fence.id, false);
      this->generateBox(level, chunkBB, 9, 1, 0, 9, 3, 0, Tile::fence.id, Tile::fence.id, false);
      this->generateBox(level, chunkBB, 6, 1, 4, 9, 4, 6, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->placeBlock(level, Tile::lava.id, 0, 7, 1, 5, chunkBB);
      this->placeBlock(level, Tile::lava.id, 0, 8, 1, 5, chunkBB);
      this->placeBlock(level, Tile::ironBars.id, 0, 9, 2, 5, chunkBB);
      this->placeBlock(level, Tile::ironBars.id, 0, 9, 2, 4, chunkBB);
      this->generateBox(level, chunkBB, 7, 2, 4, 8, 2, 5, 0, 0, false);
      this->placeBlock(level, Tile::cobblestone.id, 0, 6, 1, 3, chunkBB);
      this->placeBlock(level, Tile::furnace.id, 0, 6, 2, 3, chunkBB);
      this->placeBlock(level, Tile::furnace.id, 0, 6, 3, 3, chunkBB);
      this->placeBlock(level, Tile::slabDouble.id, 0, 8, 1, 1, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 4, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 2, 6, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 4, 2, 6, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 2, 1, 4, chunkBB);
      this->placeBlock(level, Tile::pressurePlateWood.id, 0, 2, 2, 4, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 1, 1, 5, chunkBB);
      this->placeBlock(level, Tile::stairsWood.id, this->getOrientationData(Tile::stairsWood.id, 3), 2, 1, 5, chunkBB);
      this->placeBlock(level, Tile::stairsWood.id, this->getOrientationData(Tile::stairsWood.id, 1), 1, 1, 4, chunkBB);

      for (int_t var4 = 6; var4 <= 8; var4++) {
         if (this->getBlock(level, var4, 0, -1, chunkBB) == 0 && this->getBlock(level, var4, -1, -1, chunkBB) != 0) {
            this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), var4, 0, -1, chunkBB);
         }
      }

      for (int_t var6 = 0; var6 < 7; var6++) {
         for (int_t var5 = 0; var5 < 10; var5++) {
            this->generateAirColumnUp(level, var5, 6, var6, chunkBB);
            this->fillColumnDown(level, Tile::cobblestone.id, 0, var5, -1, var6, chunkBB);
         }
      }

      this->spawnVillagers(level, chunkBB, 7, 1, 1, 1);
      return true;
   }

int_t VillagePieces__BlacksmithHouse::getVillagerProfession(int_t index)
{
      return 3;
   }

VillagePieces__ButcherHouse::VillagePieces__ButcherHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void VillagePieces__ButcherHouse::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<VillagePieces__ButcherHouse> VillagePieces__ButcherHouse::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 9, 7, 11, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<VillagePieces__ButcherHouse>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool VillagePieces__ButcherHouse::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 7 - 1, 0);
      }

      this->generateBox(level, chunkBB, 1, 1, 1, 7, 4, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 2, 1, 6, 8, 4, 10, 0, 0, false);
      this->generateBox(level, chunkBB, 2, 0, 6, 8, 0, 10, Tile::dirt.id, Tile::dirt.id, false);
      this->placeBlock(level, Tile::cobblestone.id, 0, 6, 0, 6, chunkBB);
      this->generateBox(level, chunkBB, 2, 1, 6, 2, 1, 10, Tile::fence.id, Tile::fence.id, false);
      this->generateBox(level, chunkBB, 8, 1, 6, 8, 1, 10, Tile::fence.id, Tile::fence.id, false);
      this->generateBox(level, chunkBB, 3, 1, 10, 7, 1, 10, Tile::fence.id, Tile::fence.id, false);
      this->generateBox(level, chunkBB, 1, 0, 1, 7, 0, 4, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 0, 0, 0, 3, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 8, 0, 0, 8, 3, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 0, 0, 7, 1, 0, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 0, 5, 7, 1, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 2, 0, 7, 3, 0, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 2, 5, 7, 3, 5, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 4, 1, 8, 4, 1, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 4, 4, 8, 4, 4, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 5, 2, 8, 5, 3, Tile::wood.id, Tile::wood.id, false);
      this->placeBlock(level, Tile::wood.id, 0, 0, 4, 2, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 0, 4, 3, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 8, 4, 2, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 8, 4, 3, chunkBB);
      int_t var4 = this->getOrientationData(Tile::stairsWood.id, 3);
      int_t var5 = this->getOrientationData(Tile::stairsWood.id, 2);

      for (int_t var6 = -1; var6 <= 2; var6++) {
         for (int_t var7 = 0; var7 <= 8; var7++) {
            this->placeBlock(level, Tile::stairsWood.id, var4, var7, 4 + var6, var6, chunkBB);
            this->placeBlock(level, Tile::stairsWood.id, var5, var7, 4 + var6, 5 - var6, chunkBB);
         }
      }

      this->placeBlock(level, Tile::treeTrunk.id, 0, 0, 2, 1, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 0, 2, 4, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 8, 2, 1, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 8, 2, 4, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 3, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 2, 3, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 2, 5, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 3, 2, 5, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 5, 2, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 6, 2, 5, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 2, 1, 3, chunkBB);
      this->placeBlock(level, Tile::pressurePlateWood.id, 0, 2, 2, 3, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 1, 1, 4, chunkBB);
      this->placeBlock(level, Tile::stairsWood.id, this->getOrientationData(Tile::stairsWood.id, 3), 2, 1, 4, chunkBB);
      this->placeBlock(level, Tile::stairsWood.id, this->getOrientationData(Tile::stairsWood.id, 1), 1, 1, 3, chunkBB);
      this->generateBox(level, chunkBB, 5, 0, 1, 7, 0, 3, Tile::slabDouble.id, Tile::slabDouble.id, false);
      this->placeBlock(level, Tile::slabDouble.id, 0, 6, 1, 1, chunkBB);
      this->placeBlock(level, Tile::slabDouble.id, 0, 6, 1, 2, chunkBB);
      this->placeBlock(level, 0, 0, 2, 1, 0, chunkBB);
      this->placeBlock(level, 0, 0, 2, 2, 0, chunkBB);
      this->placeBlock(level, Tile::torch.id, 0, 2, 3, 1, chunkBB);
      this->createDoor(level, chunkBB, random, 2, 1, 0, this->getOrientationData(Tile::doorWood.id, 1));
      if (this->getBlock(level, 2, 0, -1, chunkBB) == 0 && this->getBlock(level, 2, -1, -1, chunkBB) != 0) {
         this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), 2, 0, -1, chunkBB);
      }

      this->placeBlock(level, 0, 0, 6, 1, 5, chunkBB);
      this->placeBlock(level, 0, 0, 6, 2, 5, chunkBB);
      this->placeBlock(level, Tile::torch.id, 0, 6, 3, 4, chunkBB);
      this->createDoor(level, chunkBB, random, 6, 1, 5, this->getOrientationData(Tile::doorWood.id, 1));

      for (int_t var8 = 0; var8 < 5; var8++) {
         for (int_t var9 = 0; var9 < 9; var9++) {
            this->generateAirColumnUp(level, var9, 7, var8, chunkBB);
            this->fillColumnDown(level, Tile::cobblestone.id, 0, var9, -1, var8, chunkBB);
         }
      }

      this->spawnVillagers(level, chunkBB, 4, 1, 2, 2);
      return true;
   }

int_t VillagePieces__ButcherHouse::getVillagerProfession(int_t index)
{
      return index == 0 ? 4 : 0;
   }

VillagePieces__DoubleFarm::VillagePieces__DoubleFarm(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void VillagePieces__DoubleFarm::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<VillagePieces__DoubleFarm> VillagePieces__DoubleFarm::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 13, 4, 9, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<VillagePieces__DoubleFarm>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool VillagePieces__DoubleFarm::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 4 - 1, 0);
      }

      this->generateBox(level, chunkBB, 0, 1, 0, 12, 4, 8, 0, 0, false);
      this->generateBox(level, chunkBB, 1, 0, 1, 2, 0, 7, Tile::farmland.id, Tile::farmland.id, false);
      this->generateBox(level, chunkBB, 4, 0, 1, 5, 0, 7, Tile::farmland.id, Tile::farmland.id, false);
      this->generateBox(level, chunkBB, 7, 0, 1, 8, 0, 7, Tile::farmland.id, Tile::farmland.id, false);
      this->generateBox(level, chunkBB, 10, 0, 1, 11, 0, 7, Tile::farmland.id, Tile::farmland.id, false);
      this->generateBox(level, chunkBB, 0, 0, 0, 0, 0, 8, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 6, 0, 0, 6, 0, 8, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 12, 0, 0, 12, 0, 8, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 1, 0, 0, 11, 0, 0, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 1, 0, 8, 11, 0, 8, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 3, 0, 1, 3, 0, 7, Tile::water.id, Tile::water.id, false);
      this->generateBox(level, chunkBB, 9, 0, 1, 9, 0, 7, Tile::water.id, Tile::water.id, false);

      for (int_t var4 = 1; var4 <= 7; var4++) {
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 1, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 2, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 4, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 5, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 7, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 8, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 10, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 11, 1, var4, chunkBB);
      }

      for (int_t var6 = 0; var6 < 9; var6++) {
         for (int_t var5 = 0; var5 < 13; var5++) {
            this->generateAirColumnUp(level, var5, 4, var6, chunkBB);
            this->fillColumnDown(level, Tile::dirt.id, 0, var5, -1, var6, chunkBB);
         }
      }

      return true;
   }

VillagePieces__Farm::VillagePieces__Farm(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void VillagePieces__Farm::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<VillagePieces__Farm> VillagePieces__Farm::create(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 7, 4, 9, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<VillagePieces__Farm>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool VillagePieces__Farm::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 4 - 1, 0);
      }

      this->generateBox(level, chunkBB, 0, 1, 0, 6, 4, 8, 0, 0, false);
      this->generateBox(level, chunkBB, 1, 0, 1, 2, 0, 7, Tile::farmland.id, Tile::farmland.id, false);
      this->generateBox(level, chunkBB, 4, 0, 1, 5, 0, 7, Tile::farmland.id, Tile::farmland.id, false);
      this->generateBox(level, chunkBB, 0, 0, 0, 0, 0, 8, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 6, 0, 0, 6, 0, 8, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 1, 0, 0, 5, 0, 0, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 1, 0, 8, 5, 0, 8, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 3, 0, 1, 3, 0, 7, Tile::water.id, Tile::water.id, false);

      for (int_t var4 = 1; var4 <= 7; var4++) {
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 1, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 2, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 4, 1, var4, chunkBB);
         this->placeBlock(level, Tile::crops.id, MathHelper::nextInt(random, 2, 7), 5, 1, var4, chunkBB);
      }

      for (int_t var6 = 0; var6 < 9; var6++) {
         for (int_t var5 = 0; var5 < 7; var5++) {
            this->generateAirColumnUp(level, var5, 4, var6, chunkBB);
            this->fillColumnDown(level, Tile::dirt.id, 0, var5, -1, var6, chunkBB);
         }
      }

      return true;
   }

VillagePieces__FarmerHouse::VillagePieces__FarmerHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
      this->lowCeiling = random.nextBoolean();
      this->tablePosition = random.nextInt(3);
   }

void VillagePieces__FarmerHouse::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<VillagePieces__FarmerHouse> VillagePieces__FarmerHouse::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 4, 6, 5, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<VillagePieces__FarmerHouse>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool VillagePieces__FarmerHouse::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 6 - 1, 0);
      }

      this->generateBox(level, chunkBB, 1, 1, 1, 3, 5, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 0, 0, 3, 0, 4, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 0, 1, 2, 0, 3, Tile::dirt.id, Tile::dirt.id, false);
      if (this->lowCeiling) {
         this->generateBox(level, chunkBB, 1, 4, 1, 2, 4, 3, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      } else {
         this->generateBox(level, chunkBB, 1, 5, 1, 2, 5, 3, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      }

      this->placeBlock(level, Tile::treeTrunk.id, 0, 1, 4, 0, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 2, 4, 0, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 1, 4, 4, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 2, 4, 4, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 0, 4, 1, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 0, 4, 2, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 0, 4, 3, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 3, 4, 1, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 3, 4, 2, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 3, 4, 3, chunkBB);
      this->generateBox(level, chunkBB, 0, 1, 0, 0, 3, 0, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 3, 1, 0, 3, 3, 0, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 0, 1, 4, 0, 3, 4, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 3, 1, 4, 3, 3, 4, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 0, 1, 1, 0, 3, 3, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 3, 1, 1, 3, 3, 3, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 1, 0, 2, 3, 0, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 1, 4, 2, 3, 4, Tile::wood.id, Tile::wood.id, false);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 3, 2, 2, chunkBB);
      if (this->tablePosition > 0) {
         this->placeBlock(level, Tile::fence.id, 0, this->tablePosition, 1, 3, chunkBB);
         this->placeBlock(level, Tile::pressurePlateWood.id, 0, this->tablePosition, 2, 3, chunkBB);
      }

      this->placeBlock(level, 0, 0, 1, 1, 0, chunkBB);
      this->placeBlock(level, 0, 0, 1, 2, 0, chunkBB);
      this->createDoor(level, chunkBB, random, 1, 1, 0, this->getOrientationData(Tile::doorWood.id, 1));
      if (this->getBlock(level, 1, 0, -1, chunkBB) == 0 && this->getBlock(level, 1, -1, -1, chunkBB) != 0) {
         this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), 1, 0, -1, chunkBB);
      }

      for (int_t var4 = 0; var4 < 5; var4++) {
         for (int_t var5 = 0; var5 < 4; var5++) {
            this->generateAirColumnUp(level, var5, 6, var4, chunkBB);
            this->fillColumnDown(level, Tile::cobblestone.id, 0, var5, -1, var4, chunkBB);
         }
      }

      this->spawnVillagers(level, chunkBB, 1, 1, 2, 1);
      return true;
   }

VillagePieces__LampPost::VillagePieces__LampPost(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void VillagePieces__LampPost::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

BoundingBox VillagePieces__LampPost::findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing)
{
      BoundingBox var6 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 3, 4, 2, facing);
      return StructurePiece::findCollisionPiece(pieces, var6) != nullptr ? BoundingBox::none() : var6;
   }

bool VillagePieces__LampPost::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 4 - 1, 0);
      }

      this->generateBox(level, chunkBB, 0, 0, 0, 2, 3, 1, 0, 0, false);
      this->placeBlock(level, Tile::fence.id, 0, 1, 0, 0, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 1, 1, 0, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 1, 2, 0, chunkBB);
      this->placeBlock(level, Tile::wool.id, 15, 1, 3, 0, chunkBB);
      this->placeBlock(level, Tile::torch.id, 15, 0, 3, 0, chunkBB);
      this->placeBlock(level, Tile::torch.id, 15, 1, 3, 1, chunkBB);
      this->placeBlock(level, Tile::torch.id, 15, 2, 3, 0, chunkBB);
      this->placeBlock(level, Tile::torch.id, 15, 1, 3, -1, chunkBB);
      return true;
   }

VillagePieces__LargeHouse::VillagePieces__LargeHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void VillagePieces__LargeHouse::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<VillagePieces__LargeHouse> VillagePieces__LargeHouse::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 9, 7, 12, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<VillagePieces__LargeHouse>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool VillagePieces__LargeHouse::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 7 - 1, 0);
      }

      this->generateBox(level, chunkBB, 1, 1, 1, 7, 4, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 2, 1, 6, 8, 4, 10, 0, 0, false);
      this->generateBox(level, chunkBB, 2, 0, 5, 8, 0, 10, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 0, 1, 7, 0, 4, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 0, 0, 0, 3, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 8, 0, 0, 8, 3, 10, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 0, 0, 7, 2, 0, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 0, 5, 2, 1, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 2, 0, 6, 2, 3, 10, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 3, 0, 10, 7, 3, 10, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 2, 0, 7, 3, 0, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 2, 5, 2, 3, 5, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 4, 1, 8, 4, 1, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 4, 4, 3, 4, 4, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 0, 5, 2, 8, 5, 3, Tile::wood.id, Tile::wood.id, false);
      this->placeBlock(level, Tile::wood.id, 0, 0, 4, 2, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 0, 4, 3, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 8, 4, 2, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 8, 4, 3, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 8, 4, 4, chunkBB);
      int_t var4 = this->getOrientationData(Tile::stairsWood.id, 3);
      int_t var5 = this->getOrientationData(Tile::stairsWood.id, 2);

      for (int_t var6 = -1; var6 <= 2; var6++) {
         for (int_t var7 = 0; var7 <= 8; var7++) {
            this->placeBlock(level, Tile::stairsWood.id, var4, var7, 4 + var6, var6, chunkBB);
            if ((var6 > -1 || var7 <= 1) && (var6 > 0 || var7 <= 3) && (var6 > 1 || var7 <= 4 || var7 >= 6)) {
               this->placeBlock(level, Tile::stairsWood.id, var5, var7, 4 + var6, 5 - var6, chunkBB);
            }
         }
      }

      this->generateBox(level, chunkBB, 3, 4, 5, 3, 4, 10, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 7, 4, 2, 7, 4, 10, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 4, 5, 4, 4, 5, 10, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 6, 5, 4, 6, 5, 10, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 5, 6, 3, 5, 6, 10, Tile::wood.id, Tile::wood.id, false);
      int_t var10 = this->getOrientationData(Tile::stairsWood.id, 0);

      for (int_t var11 = 4; var11 >= 1; var11--) {
         this->placeBlock(level, Tile::wood.id, 0, var11, 2 + var11, 7 - var11, chunkBB);

         for (int_t var8 = 8 - var11; var8 <= 10; var8++) {
            this->placeBlock(level, Tile::stairsWood.id, var10, var11, 2 + var11, var8, chunkBB);
         }
      }

      int_t var12 = this->getOrientationData(Tile::stairsWood.id, 1);
      this->placeBlock(level, Tile::wood.id, 0, 6, 6, 3, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 7, 5, 4, chunkBB);
      this->placeBlock(level, Tile::stairsWood.id, var12, 6, 6, 4, chunkBB);

      for (int_t var13 = 6; var13 <= 8; var13++) {
         for (int_t var9 = 5; var9 <= 10; var9++) {
            this->placeBlock(level, Tile::stairsWood.id, var12, var13, 12 - var13, var9, chunkBB);
         }
      }

      this->placeBlock(level, Tile::treeTrunk.id, 0, 0, 2, 1, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 0, 2, 4, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 3, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 4, 2, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 5, 2, 0, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 6, 2, 0, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 8, 2, 1, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 2, 3, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 8, 2, 4, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 8, 2, 5, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 8, 2, 6, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 2, 7, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 2, 8, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 8, 2, 9, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 2, 2, 6, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 2, 7, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 2, 8, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 2, 2, 9, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 4, 4, 10, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 5, 4, 10, chunkBB);
      this->placeBlock(level, Tile::treeTrunk.id, 0, 6, 4, 10, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 5, 5, 10, chunkBB);
      this->placeBlock(level, 0, 0, 2, 1, 0, chunkBB);
      this->placeBlock(level, 0, 0, 2, 2, 0, chunkBB);
      this->placeBlock(level, Tile::torch.id, 0, 2, 3, 1, chunkBB);
      this->createDoor(level, chunkBB, random, 2, 1, 0, this->getOrientationData(Tile::doorWood.id, 1));
      this->generateBox(level, chunkBB, 1, 0, -1, 3, 2, -1, 0, 0, false);
      if (this->getBlock(level, 2, 0, -1, chunkBB) == 0 && this->getBlock(level, 2, -1, -1, chunkBB) != 0) {
         this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), 2, 0, -1, chunkBB);
      }

      for (int_t var14 = 0; var14 < 5; var14++) {
         for (int_t var16 = 0; var16 < 9; var16++) {
            this->generateAirColumnUp(level, var16, 7, var14, chunkBB);
            this->fillColumnDown(level, Tile::cobblestone.id, 0, var16, -1, var14, chunkBB);
         }
      }

      for (int_t var15 = 5; var15 < 11; var15++) {
         for (int_t var17 = 2; var17 < 9; var17++) {
            this->generateAirColumnUp(level, var17, 7, var15, chunkBB);
            this->fillColumnDown(level, Tile::cobblestone.id, 0, var17, -1, var15, chunkBB);
         }
      }

      this->spawnVillagers(level, chunkBB, 4, 1, 2, 2);
      return true;
   }

VillagePieces__LibrarianHouse::VillagePieces__LibrarianHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void VillagePieces__LibrarianHouse::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<VillagePieces__LibrarianHouse> VillagePieces__LibrarianHouse::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 9, 9, 6, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<VillagePieces__LibrarianHouse>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool VillagePieces__LibrarianHouse::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 9 - 1, 0);
      }

      this->generateBox(level, chunkBB, 1, 1, 1, 7, 5, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 0, 0, 8, 0, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 5, 0, 8, 5, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 6, 1, 8, 6, 4, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 7, 2, 8, 7, 3, Tile::cobblestone.id, Tile::cobblestone.id, false);
      int_t var4 = this->getOrientationData(Tile::stairsWood.id, 3);
      int_t var5 = this->getOrientationData(Tile::stairsWood.id, 2);

      for (int_t var6 = -1; var6 <= 2; var6++) {
         for (int_t var7 = 0; var7 <= 8; var7++) {
            this->placeBlock(level, Tile::stairsWood.id, var4, var7, 6 + var6, var6, chunkBB);
            this->placeBlock(level, Tile::stairsWood.id, var5, var7, 6 + var6, 5 - var6, chunkBB);
         }
      }

      this->generateBox(level, chunkBB, 0, 1, 0, 0, 1, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 1, 5, 8, 1, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 8, 1, 0, 8, 1, 4, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 2, 1, 0, 7, 1, 0, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 0, 4, 0, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 2, 5, 0, 4, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 8, 2, 5, 8, 4, 5, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 8, 2, 0, 8, 4, 0, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 2, 1, 0, 4, 4, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 2, 5, 7, 4, 5, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 8, 2, 1, 8, 4, 4, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 2, 0, 7, 4, 0, Tile::wood.id, Tile::wood.id, false);
      this->placeBlock(level, Tile::glassPane.id, 0, 4, 2, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 5, 2, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 6, 2, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 4, 3, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 5, 3, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 6, 3, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 3, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 3, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 3, 3, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 2, 3, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 3, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 8, 3, 3, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 2, 5, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 3, 2, 5, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 5, 2, 5, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 6, 2, 5, chunkBB);
      this->generateBox(level, chunkBB, 1, 4, 1, 7, 4, 1, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 4, 4, 7, 4, 4, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 3, 4, 7, 3, 4, Tile::bookshelf.id, Tile::bookshelf.id, false);
      this->placeBlock(level, Tile::wood.id, 0, 7, 1, 4, chunkBB);
      this->placeBlock(level, Tile::stairsWood.id, this->getOrientationData(Tile::stairsWood.id, 0), 7, 1, 3, chunkBB);
      int_t var9 = this->getOrientationData(Tile::stairsWood.id, 3);
      this->placeBlock(level, Tile::stairsWood.id, var9, 6, 1, 4, chunkBB);
      this->placeBlock(level, Tile::stairsWood.id, var9, 5, 1, 4, chunkBB);
      this->placeBlock(level, Tile::stairsWood.id, var9, 4, 1, 4, chunkBB);
      this->placeBlock(level, Tile::stairsWood.id, var9, 3, 1, 4, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 6, 1, 3, chunkBB);
      this->placeBlock(level, Tile::pressurePlateWood.id, 0, 6, 2, 3, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 4, 1, 3, chunkBB);
      this->placeBlock(level, Tile::pressurePlateWood.id, 0, 4, 2, 3, chunkBB);
      this->placeBlock(level, Tile::workBench.id, 0, 7, 1, 1, chunkBB);
      this->placeBlock(level, 0, 0, 1, 1, 0, chunkBB);
      this->placeBlock(level, 0, 0, 1, 2, 0, chunkBB);
      this->createDoor(level, chunkBB, random, 1, 1, 0, this->getOrientationData(Tile::doorWood.id, 1));
      if (this->getBlock(level, 1, 0, -1, chunkBB) == 0 && this->getBlock(level, 1, -1, -1, chunkBB) != 0) {
         this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), 1, 0, -1, chunkBB);
      }

      for (int_t var10 = 0; var10 < 6; var10++) {
         for (int_t var8 = 0; var8 < 9; var8++) {
            this->generateAirColumnUp(level, var8, 9, var10, chunkBB);
            this->fillColumnDown(level, Tile::cobblestone.id, 0, var8, -1, var10, chunkBB);
         }
      }

      this->spawnVillagers(level, chunkBB, 2, 1, 2, 1);
      return true;
   }

int_t VillagePieces__LibrarianHouse::getVillagerProfession(int_t index)
{
      return 1;
   }

VillagePieces__PriestHouse::VillagePieces__PriestHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void VillagePieces__PriestHouse::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<VillagePieces__PriestHouse> VillagePieces__PriestHouse::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 5, 12, 9, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<VillagePieces__PriestHouse>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool VillagePieces__PriestHouse::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 12 - 1, 0);
      }

      this->generateBox(level, chunkBB, 1, 1, 1, 3, 3, 7, 0, 0, false);
      this->generateBox(level, chunkBB, 1, 5, 1, 3, 9, 3, 0, 0, false);
      this->generateBox(level, chunkBB, 1, 0, 0, 3, 0, 8, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 1, 0, 3, 10, 0, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 1, 1, 0, 10, 3, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 4, 1, 1, 4, 10, 3, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 0, 4, 0, 4, 7, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 4, 0, 4, 4, 4, 7, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 1, 8, 3, 4, 8, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 5, 4, 3, 10, 4, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 1, 5, 5, 3, 5, 7, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 9, 0, 4, 9, 4, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 4, 0, 4, 4, 4, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->placeBlock(level, Tile::cobblestone.id, 0, 0, 11, 2, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 4, 11, 2, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 2, 11, 0, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 2, 11, 4, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 1, 1, 6, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 1, 1, 7, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 2, 1, 7, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 3, 1, 6, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 3, 1, 7, chunkBB);
      this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), 1, 1, 5, chunkBB);
      this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), 2, 1, 6, chunkBB);
      this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), 3, 1, 5, chunkBB);
      this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 1), 1, 2, 7, chunkBB);
      this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 0), 3, 2, 7, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 3, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 4, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 4, 3, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 6, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 7, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 4, 6, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 4, 7, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 6, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 7, 0, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 6, 4, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 7, 4, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 3, 6, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 4, 3, 6, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 3, 8, chunkBB);
      this->placeBlock(level, Tile::torch.id, 0, 2, 4, 7, chunkBB);
      this->placeBlock(level, Tile::torch.id, 0, 1, 4, 6, chunkBB);
      this->placeBlock(level, Tile::torch.id, 0, 3, 4, 6, chunkBB);
      this->placeBlock(level, Tile::torch.id, 0, 2, 4, 5, chunkBB);
      int_t var4 = this->getOrientationData(Tile::ladder.id, 4);

      for (int_t var5 = 1; var5 <= 9; var5++) {
         this->placeBlock(level, Tile::ladder.id, var4, 3, var5, 3, chunkBB);
      }

      this->placeBlock(level, 0, 0, 2, 1, 0, chunkBB);
      this->placeBlock(level, 0, 0, 2, 2, 0, chunkBB);
      this->createDoor(level, chunkBB, random, 2, 1, 0, this->getOrientationData(Tile::doorWood.id, 1));
      if (this->getBlock(level, 2, 0, -1, chunkBB) == 0 && this->getBlock(level, 2, -1, -1, chunkBB) != 0) {
         this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), 2, 0, -1, chunkBB);
      }

      for (int_t var7 = 0; var7 < 9; var7++) {
         for (int_t var6 = 0; var6 < 5; var6++) {
            this->generateAirColumnUp(level, var6, 12, var7, chunkBB);
            this->fillColumnDown(level, Tile::cobblestone.id, 0, var6, -1, var7, chunkBB);
         }
      }

      this->spawnVillagers(level, chunkBB, 2, 1, 2, 1);
      return true;
   }

int_t VillagePieces__PriestHouse::getVillagerProfession(int_t index)
{
      return 2;
   }

VillagePieces__RoadPiece::VillagePieces__RoadPiece(int_t var1) : VillagePieces__VillagePiece(var1)
{
   }

VillagePieces__Road::VillagePieces__Road(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__RoadPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
      this->length = std::max(bb.getXSpan(), bb.getZSpan());
   }

void VillagePieces__Road::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      bool var4 = false;

      for (int_t var5 = random.nextInt(5); var5 < this->length - 8; var5 += 2 + random.nextInt(5)) {
         std::shared_ptr<StructurePiece> var6 = this->generatePieceLeft(std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, 0, var5);
         if (var6 != nullptr) {
            var5 += std::max(var6->boundingBox.getXSpan(), var6->boundingBox.getZSpan());
            var4 = true;
         }
      }

      for (int_t var7 = random.nextInt(5); var7 < this->length - 8; var7 += 2 + random.nextInt(5)) {
         std::shared_ptr<StructurePiece> var8 = this->generatePieceRight(std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, 0, var7);
         if (var8 != nullptr) {
            var7 += std::max(var8->boundingBox.getXSpan(), var8->boundingBox.getZSpan());
            var4 = true;
         }
      }

      if (var4 && random.nextInt(3) > 0) {
         switch (this->orientation) {
            case 0:
               VillagePieces::generateBuildingPiece(
                  std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0, this->boundingBox.z1 - 2, 1, this->getGenDepth()
               );
               break;
            case 1:
               VillagePieces::generateBuildingPiece(
                  std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x0, this->boundingBox.y0, this->boundingBox.z0 - 1, 2, this->getGenDepth()
               );
               break;
            case 2:
               VillagePieces::generateBuildingPiece(
                  std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0, this->boundingBox.z0, 1, this->getGenDepth()
               );
               break;
            case 3:
               VillagePieces::generateBuildingPiece(
                  std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x1 - 2, this->boundingBox.y0, this->boundingBox.z0 - 1, 2, this->getGenDepth()
               );
         }
      }

      if (var4 && random.nextInt(3) > 0) {
         switch (this->orientation) {
            case 0:
               VillagePieces::generateBuildingPiece(
                  std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0, this->boundingBox.z1 - 2, 3, this->getGenDepth()
               );
               break;
            case 1:
               VillagePieces::generateBuildingPiece(
                  std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x0, this->boundingBox.y0, this->boundingBox.z1 + 1, 0, this->getGenDepth()
               );
               break;
            case 2:
               VillagePieces::generateBuildingPiece(
                  std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0, this->boundingBox.z0, 3, this->getGenDepth()
               );
               break;
            case 3:
               VillagePieces::generateBuildingPiece(
                  std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x1 - 2, this->boundingBox.y0, this->boundingBox.z1 + 1, 0, this->getGenDepth()
               );
         }
      }
   }

BoundingBox VillagePieces__Road::findSize(std::shared_ptr<VillagePieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing)
{
      for (int_t var7 = 7 * MathHelper::nextInt(random, 3, 5); var7 >= 7; var7 -= 7) {
         BoundingBox var8 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 3, 3, var7, facing);
         if (StructurePiece::findCollisionPiece(pieces, var8) == nullptr) {
            return var8;
         }
      }

      return BoundingBox::none();
   }

bool VillagePieces__Road::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      for (int_t var4 = this->boundingBox.x0; var4 <= this->boundingBox.x1; var4++) {
         for (int_t var5 = this->boundingBox.z0; var5 <= this->boundingBox.z1; var5++) {
            if (chunkBB.isInside(var4, 64, var5)) {
               int_t var6 = level.getTopSolidBlock(var4, var5) - 1;
               level.setTileNoUpdate(var4, var6, var5, Tile::gravel.id);
            }
         }
      }

      return true;
   }

VillagePieces__SmallHouse::VillagePieces__SmallHouse(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : VillagePieces__VillagePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
      this->terrace = random.nextBoolean();
   }

void VillagePieces__SmallHouse::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<VillagePieces__SmallHouse> VillagePieces__SmallHouse::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, 0, 0, 0, 5, 6, 5, facing);
      return StructurePiece::findCollisionPiece(pieces, var7) != nullptr ? nullptr : std::make_shared<VillagePieces__SmallHouse>(generationDepth, random, var7, facing);
   }

bool VillagePieces__SmallHouse::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 6 - 1, 0);
      }

      this->generateBox(level, chunkBB, 0, 0, 0, 4, 0, 4, Tile::cobblestone.id, Tile::cobblestone.id, false);
      this->generateBox(level, chunkBB, 0, 4, 0, 4, 4, 4, Tile::treeTrunk.id, Tile::treeTrunk.id, false);
      this->generateBox(level, chunkBB, 1, 4, 1, 3, 4, 3, Tile::wood.id, Tile::wood.id, false);
      this->placeBlock(level, Tile::cobblestone.id, 0, 0, 1, 0, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 0, 2, 0, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 0, 3, 0, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 4, 1, 0, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 4, 2, 0, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 4, 3, 0, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 0, 1, 4, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 0, 2, 4, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 0, 3, 4, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 4, 1, 4, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 4, 2, 4, chunkBB);
      this->placeBlock(level, Tile::cobblestone.id, 0, 4, 3, 4, chunkBB);
      this->generateBox(level, chunkBB, 0, 1, 1, 0, 3, 3, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 4, 1, 1, 4, 3, 3, Tile::wood.id, Tile::wood.id, false);
      this->generateBox(level, chunkBB, 1, 1, 4, 3, 3, 4, Tile::wood.id, Tile::wood.id, false);
      this->placeBlock(level, Tile::glassPane.id, 0, 0, 2, 2, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 2, 2, 4, chunkBB);
      this->placeBlock(level, Tile::glassPane.id, 0, 4, 2, 2, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 1, 1, 0, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 1, 2, 0, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 1, 3, 0, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 2, 3, 0, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 3, 3, 0, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 3, 2, 0, chunkBB);
      this->placeBlock(level, Tile::wood.id, 0, 3, 1, 0, chunkBB);
      if (this->getBlock(level, 2, 0, -1, chunkBB) == 0 && this->getBlock(level, 2, -1, -1, chunkBB) != 0) {
         this->placeBlock(level, Tile::stairsStone.id, this->getOrientationData(Tile::stairsStone.id, 3), 2, 0, -1, chunkBB);
      }

      this->generateBox(level, chunkBB, 1, 1, 1, 3, 3, 3, 0, 0, false);
      if (this->terrace) {
         this->placeBlock(level, Tile::fence.id, 0, 0, 5, 0, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 1, 5, 0, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 2, 5, 0, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 3, 5, 0, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 4, 5, 0, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 0, 5, 4, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 1, 5, 4, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 2, 5, 4, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 3, 5, 4, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 4, 5, 4, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 4, 5, 1, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 4, 5, 2, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 4, 5, 3, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 0, 5, 1, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 0, 5, 2, chunkBB);
         this->placeBlock(level, Tile::fence.id, 0, 0, 5, 3, chunkBB);
      }

      if (this->terrace) {
         int_t var4 = this->getOrientationData(Tile::ladder.id, 3);
         this->placeBlock(level, Tile::ladder.id, var4, 3, 1, 3, chunkBB);
         this->placeBlock(level, Tile::ladder.id, var4, 3, 2, 3, chunkBB);
         this->placeBlock(level, Tile::ladder.id, var4, 3, 3, 3, chunkBB);
         this->placeBlock(level, Tile::ladder.id, var4, 3, 4, 3, chunkBB);
      }

      this->placeBlock(level, Tile::torch.id, 0, 2, 3, 1, chunkBB);

      for (int_t var6 = 0; var6 < 5; var6++) {
         for (int_t var5 = 0; var5 < 5; var5++) {
            this->generateAirColumnUp(level, var5, 6, var6, chunkBB);
            this->fillColumnDown(level, Tile::cobblestone.id, 0, var5, -1, var6, chunkBB);
         }
      }

      this->spawnVillagers(level, chunkBB, 1, 1, 2, 1);
      return true;
   }

VillagePieces__Well::VillagePieces__Well(int_t generationDepth, Random & random, int_t x, int_t z) : VillagePieces__VillagePiece(generationDepth)
{
      this->f_1400559 = true;
      this->orientation = random.nextInt(4);
      switch (this->orientation) {
         case 0:
         case 2:
            this->boundingBox = BoundingBox(x, 64, z, x + 6 - 1, 78, z + 6 - 1);
            break;
         default:
            this->boundingBox = BoundingBox(x, 64, z, x + 6 - 1, 78, z + 6 - 1);
      }
   }

void VillagePieces__Well::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      VillagePieces::generateBuildingPiece(
         std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y1 - 4, this->boundingBox.z0 + 1, 1, this->getGenDepth()
      );
      VillagePieces::generateBuildingPiece(
         std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y1 - 4, this->boundingBox.z0 + 1, 3, this->getGenDepth()
      );
      VillagePieces::generateBuildingPiece(
         std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y1 - 4, this->boundingBox.z0 - 1, 2, this->getGenDepth()
      );
      VillagePieces::generateBuildingPiece(
         std::dynamic_pointer_cast<VillagePieces__Start>(start), pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y1 - 4, this->boundingBox.z1 + 1, 0, this->getGenDepth()
      );
   }

bool VillagePieces__Well::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->hpos < 0) {
         this->hpos = this->updateHeight(level, chunkBB);
         if (this->hpos < 0) {
            return true;
         }

         this->boundingBox.move(0, this->hpos - this->boundingBox.y1 + 3, 0);
      }

      if (this->f_1400559) {
      }

      this->generateBox(level, chunkBB, 1, 0, 1, 4, 12, 4, Tile::cobblestone.id, Tile::water.id, false);
      this->placeBlock(level, 0, 0, 2, 12, 2, chunkBB);
      this->placeBlock(level, 0, 0, 3, 12, 2, chunkBB);
      this->placeBlock(level, 0, 0, 2, 12, 3, chunkBB);
      this->placeBlock(level, 0, 0, 3, 12, 3, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 1, 13, 1, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 1, 14, 1, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 4, 13, 1, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 4, 14, 1, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 1, 13, 4, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 1, 14, 4, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 4, 13, 4, chunkBB);
      this->placeBlock(level, Tile::fence.id, 0, 4, 14, 4, chunkBB);
      this->generateBox(level, chunkBB, 1, 15, 1, 4, 15, 4, Tile::cobblestone.id, Tile::cobblestone.id, false);

      for (int_t var4 = 0; var4 <= 5; var4++) {
         for (int_t var5 = 0; var5 <= 5; var5++) {
            if (var5 == 0 || var5 == 5 || var4 == 0 || var4 == 5) {
               this->placeBlock(level, Tile::gravel.id, 0, var5, 11, var4, chunkBB);
               this->generateAirColumnUp(level, var5, 12, var4, chunkBB);
            }
         }
      }

      return true;
   }

VillagePieces__Start::VillagePieces__Start(BiomeSource * biomeSource, int_t var2, Random & random, int_t x, int_t z, WeightList & weights, int_t size) : VillagePieces__Well(0, random, x, z)
{
      this->biomeSource = biomeSource;
      this->weights = weights;
      this->size = size;
   }

BiomeSource * VillagePieces__Start::getBiomeSource()
{
      return this->biomeSource;
   }

VillagePieces__VillagePieceWeight::VillagePieces__VillagePieceWeight(int_t type, int_t weight, int_t maxAmount)
{
      this->type = type;
      this->weight = weight;
      this->maxAmount = maxAmount;
   }

bool VillagePieces__VillagePieceWeight::isValid(int_t generationDepth)
{
      return this->maxAmount == 0 || this->amountGenerated < this->maxAmount;
   }

bool VillagePieces__VillagePieceWeight::isValid()
{
      return this->maxAmount == 0 || this->amountGenerated < this->maxAmount;
   }

std::vector<Biome *> &villageBiomes()
{
	static std::vector<Biome *> biomes = {Biome::plains, Biome::desert};
	return biomes;
}

class VillageStart : public StructureStart
{
	bool valid = false;

public:
	VillageStart(Level &level, Random &random, int_t chunkX, int_t chunkZ)
	{
		int_t size = 0;
		WeightList weights = VillagePieces::getPieceWeights(random, size);
		auto start = std::make_shared<VillagePieces__Start>(&level.getBiomeSource(), 0, random, (chunkX << 4) + 2, (chunkZ << 4) + 2, weights, size);
		pieces.push_back(start);
		start->addChildren(start, pieces, random);
		PieceList &roads = start->roadPieces;
		PieceList &buildings = start->buildingPieces;
		while (!roads.empty() || !buildings.empty())
		{
			if (!roads.empty())
			{
				int_t i = random.nextInt(static_cast<int_t>(roads.size()));
				std::shared_ptr<StructurePiece> piece = jremove(roads, i);
				piece->addChildren(start, pieces, random);
			}
			else
			{
				int_t i = random.nextInt(static_cast<int_t>(buildings.size()));
				std::shared_ptr<StructurePiece> piece = jremove(buildings, i);
				piece->addChildren(start, pieces, random);
			}
		}

		calculateBoundingBox();
		int_t count = 0;
		for (auto &piece : pieces)
		{
			if (std::dynamic_pointer_cast<VillagePieces__RoadPiece>(piece) == nullptr)
				count++;
		}
		valid = count > 2;
	}

	bool isValid() override { return valid; }
};

bool VillageFeature::isFeatureChunk(int_t x, int_t z)
{
	int_t spacing = 32;
	int_t separation = 8;
	int_t origX = x;
	int_t origZ = z;
	if (x < 0)
		x -= spacing - 1;
	if (z < 0)
		z -= spacing - 1;
	int_t cx = x / spacing;
	int_t cz = z / spacing;
	Random &r = level->setRandomSeed(cx, cz, 10387312);
	cx *= spacing;
	cz *= spacing;
	cx += r.nextInt(spacing - separation);
	cz += r.nextInt(spacing - separation);
	if (origX == cx && origZ == cz)
		return level->getBiomeSource().containsOnly(origX * 16 + 8, origZ * 16 + 8, 0, villageBiomes());
	return false;
}

std::unique_ptr<StructureStart> VillageFeature::createStructureStart(int_t x, int_t z)
{
	return std::make_unique<VillageStart>(*level, random, x, z);
}

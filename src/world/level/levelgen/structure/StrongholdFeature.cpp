#include "world/level/levelgen/structure/StrongholdFeature.h"
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

class StrongholdPieces;
class StrongholdPieces__StrongholdPieceWeight;
class StrongholdPieces__8455444;
class StrongholdPieces__9971267;
class StrongholdPieces__StrongholdPiece;
class StrongholdPieces__ChestCorridor;
class StrongholdPieces__EndPortalRoom;
class StrongholdPieces__FiveWayCrossing;
class StrongholdPieces__LeftTurn;
class StrongholdPieces__Library;
class StrongholdPieces__PlainCorridor;
class StrongholdPieces__Prison;
class StrongholdPieces__RightTurn;
class StrongholdPieces__RoomCrossing;
class StrongholdPieces__SpiralStaircase;
class StrongholdPieces__Start;
class StrongholdPieces__StoneBrickPicker;
class StrongholdPieces__StraightCorridor;
class StrongholdPieces__StraightStairs;
enum class StrongholdPieces__StrongholdPiece__EntranceType : int_t { OPENING, WOOD_DOOR, GATES, IRON_DOOR };

enum : int_t { CLS_StrongholdPieces = 1, CLS_StrongholdPieces__StrongholdPieceWeight = 2, CLS_StrongholdPieces__8455444 = 3, CLS_StrongholdPieces__9971267 = 4, CLS_StrongholdPieces__StrongholdPiece = 5, CLS_StrongholdPieces__ChestCorridor = 6, CLS_StrongholdPieces__EndPortalRoom = 7, CLS_StrongholdPieces__FiveWayCrossing = 8, CLS_StrongholdPieces__LeftTurn = 9, CLS_StrongholdPieces__Library = 10, CLS_StrongholdPieces__PlainCorridor = 11, CLS_StrongholdPieces__Prison = 12, CLS_StrongholdPieces__RightTurn = 13, CLS_StrongholdPieces__RoomCrossing = 14, CLS_StrongholdPieces__SpiralStaircase = 15, CLS_StrongholdPieces__Start = 16, CLS_StrongholdPieces__StoneBrickPicker = 17, CLS_StrongholdPieces__StraightCorridor = 18, CLS_StrongholdPieces__StraightStairs = 19 };

class StrongholdPieces
{
public:
	static std::vector<std::shared_ptr<StrongholdPieces__StrongholdPieceWeight>> &PIECE_WEIGHTS();
	static std::vector<std::shared_ptr<StrongholdPieces__StrongholdPieceWeight>> currentWeights;
	static int_t forcedPiece;
	static int_t totalWeight;
	static std::shared_ptr<StrongholdPieces__StoneBrickPicker> &STONE_BRICK_PICKER();
	static void resetWeights();
	static bool updateTotalWeight();
	static std::shared_ptr<StrongholdPieces__StrongholdPiece> createPiece(int_t type, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	static std::shared_ptr<StrongholdPieces__StrongholdPiece> generateNextPiece(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	static std::shared_ptr<StructurePiece> tryGenerateNextPiece(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
};

class StrongholdPieces__StrongholdPieceWeight
{
public:
	int_t type{};
	int_t weight{};
	int_t amountGenerated{};
	int_t maxAmount{};
	StrongholdPieces__StrongholdPieceWeight(int_t type, int_t weight, int_t maxAmount);
	virtual bool isValid(int_t generationDepth);
	virtual bool isValid();
};

class StrongholdPieces__8455444 : public StrongholdPieces__StrongholdPieceWeight
{
public:
	using Base = StrongholdPieces__StrongholdPieceWeight;
	StrongholdPieces__8455444(int_t var1, int_t var2, int_t var3);
	virtual bool isValid(int_t generationDepth);
};

class StrongholdPieces__9971267 : public StrongholdPieces__StrongholdPieceWeight
{
public:
	using Base = StrongholdPieces__StrongholdPieceWeight;
	StrongholdPieces__9971267(int_t var1, int_t var2, int_t var3);
	virtual bool isValid(int_t generationDepth);
};

class StrongholdPieces__StrongholdPiece : public StructurePiece
{
public:
	using Base = StructurePiece;
	StrongholdPieces__StrongholdPiece(int_t var1);
	virtual void generateEntrance(Level & level, Random & random, const BoundingBox & bb, StrongholdPieces__StrongholdPiece__EntranceType type, int_t x, int_t y, int_t z);
	virtual StrongholdPieces__StrongholdPiece__EntranceType pickEntranceType(Random & random);
	virtual std::shared_ptr<StructurePiece> generatePieceForward(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t offset, int_t yOffset);
	virtual std::shared_ptr<StructurePiece> generatePieceLeft(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset);
	virtual std::shared_ptr<StructurePiece> generatePieceRight(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset);
	static bool isValidBounds(const BoundingBox & bb);
};

class StrongholdPieces__ChestCorridor : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	static std::vector<WeighedTreasureItem> &LOOT_ENTRIES();
	StrongholdPieces__StrongholdPiece__EntranceType entranceType{};
	bool hasChest{};
	StrongholdPieces__ChestCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__ChestCorridor> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__EndPortalRoom : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	bool hasSpawner{};
	StrongholdPieces__EndPortalRoom(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__EndPortalRoom> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__FiveWayCrossing : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	StrongholdPieces__StrongholdPiece__EntranceType entranceType{};
	bool leftLow{};
	bool leftHigh{};
	bool rightLow{};
	bool rightHigh{};
	StrongholdPieces__FiveWayCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__FiveWayCrossing> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__LeftTurn : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	StrongholdPieces__StrongholdPiece__EntranceType entranceType{};
	StrongholdPieces__LeftTurn(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__LeftTurn> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__Library : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	static std::vector<WeighedTreasureItem> &LOOT_ENTRIES();
	StrongholdPieces__StrongholdPiece__EntranceType entranceType{};
	bool tall{};
	StrongholdPieces__Library(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__Library> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__PlainCorridor : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	int_t steps{};
	StrongholdPieces__PlainCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static BoundingBox findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__Prison : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	StrongholdPieces__StrongholdPiece__EntranceType entranceType{};
	StrongholdPieces__Prison(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__Prison> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__RightTurn : public StrongholdPieces__LeftTurn
{
public:
	using Base = StrongholdPieces__LeftTurn;
	StrongholdPieces__RightTurn(int_t var1, Random & var2, const BoundingBox & var3, int_t var4);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__RoomCrossing : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	static std::vector<WeighedTreasureItem> &LOOT_ENTRIES();
	StrongholdPieces__StrongholdPiece__EntranceType entranceType{};
	int_t type{};
	StrongholdPieces__RoomCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__RoomCrossing> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__SpiralStaircase : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	bool isSource{};
	StrongholdPieces__StrongholdPiece__EntranceType entranceType{};
	StrongholdPieces__SpiralStaircase(int_t generationDepth, Random & random, int_t x, int_t z);
	StrongholdPieces__SpiralStaircase(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__SpiralStaircase> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__Start : public StrongholdPieces__SpiralStaircase
{
public:
	using Base = StrongholdPieces__SpiralStaircase;
	std::shared_ptr<StrongholdPieces__StrongholdPieceWeight> previous{};
	std::shared_ptr<StrongholdPieces__EndPortalRoom> endPortalRoom{};
	PieceList children;
	StrongholdPieces__Start(int_t var1, Random & var2, int_t var3, int_t var4);
	TilePos3 getLocatorPosition() const override;
};

class StrongholdPieces__StoneBrickPicker : public BlockSelector
{
public:
	using Base = BlockSelector;
	StrongholdPieces__StoneBrickPicker();
	void next(Random & randomm, int_t x, int_t y, int_t z, bool isNonAir) override;
};

class StrongholdPieces__StraightCorridor : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	StrongholdPieces__StrongholdPiece__EntranceType entranceType{};
	bool left{};
	bool right{};
	StrongholdPieces__StraightCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__StraightCorridor> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class StrongholdPieces__StraightStairs : public StrongholdPieces__StrongholdPiece
{
public:
	using Base = StrongholdPieces__StrongholdPiece;
	StrongholdPieces__StrongholdPiece__EntranceType entranceType{};
	StrongholdPieces__StraightStairs(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<StrongholdPieces__StraightStairs> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

std::vector<std::shared_ptr<StrongholdPieces__StrongholdPieceWeight>> &StrongholdPieces::PIECE_WEIGHTS()
{
	static std::vector<std::shared_ptr<StrongholdPieces__StrongholdPieceWeight>> value = {
      std::make_shared<StrongholdPieces__StrongholdPieceWeight>(CLS_StrongholdPieces__StraightCorridor, 40, 0),
      std::make_shared<StrongholdPieces__StrongholdPieceWeight>(CLS_StrongholdPieces__Prison, 5, 5),
      std::make_shared<StrongholdPieces__StrongholdPieceWeight>(CLS_StrongholdPieces__LeftTurn, 20, 0),
      std::make_shared<StrongholdPieces__StrongholdPieceWeight>(CLS_StrongholdPieces__RightTurn, 20, 0),
      std::make_shared<StrongholdPieces__StrongholdPieceWeight>(CLS_StrongholdPieces__RoomCrossing, 10, 6),
      std::make_shared<StrongholdPieces__StrongholdPieceWeight>(CLS_StrongholdPieces__StraightStairs, 5, 5),
      std::make_shared<StrongholdPieces__StrongholdPieceWeight>(CLS_StrongholdPieces__SpiralStaircase, 5, 5),
      std::make_shared<StrongholdPieces__StrongholdPieceWeight>(CLS_StrongholdPieces__FiveWayCrossing, 5, 4),
      std::make_shared<StrongholdPieces__StrongholdPieceWeight>(CLS_StrongholdPieces__ChestCorridor, 5, 4),
      std::make_shared<StrongholdPieces__8455444>(CLS_StrongholdPieces__Library, 10, 2),
      std::make_shared<StrongholdPieces__9971267>(CLS_StrongholdPieces__EndPortalRoom, 20, 1)
   };
	return value;
}

std::vector<std::shared_ptr<StrongholdPieces__StrongholdPieceWeight>> StrongholdPieces::currentWeights;

int_t StrongholdPieces::forcedPiece{};

int_t StrongholdPieces::totalWeight = 0;

std::shared_ptr<StrongholdPieces__StoneBrickPicker> &StrongholdPieces::STONE_BRICK_PICKER()
{
	static std::shared_ptr<StrongholdPieces__StoneBrickPicker> value = std::make_shared<StrongholdPieces__StoneBrickPicker>();
	return value;
}

void StrongholdPieces::resetWeights()
{
      currentWeights.clear();

      for (auto var3 : PIECE_WEIGHTS()) {
         var3->amountGenerated = 0;
         currentWeights.push_back(var3);
      }

      forcedPiece = 0;
   }

bool StrongholdPieces::updateTotalWeight()
{
      bool var0 = false;
      totalWeight = 0;

      for (auto var2 : currentWeights) {
         if (var2->maxAmount > 0 && var2->amountGenerated < var2->maxAmount) {
            var0 = true;
         }

         totalWeight = totalWeight + var2->weight;
      }

      return var0;
   }

std::shared_ptr<StrongholdPieces__StrongholdPiece> StrongholdPieces::createPiece(int_t type, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      std::shared_ptr<StructurePiece> var8 = nullptr;
      if (type == CLS_StrongholdPieces__StraightCorridor) {
         var8 = StrongholdPieces__StraightCorridor::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__Prison) {
         var8 = StrongholdPieces__Prison::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__LeftTurn) {
         var8 = StrongholdPieces__LeftTurn::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__RightTurn) {
         var8 = StrongholdPieces__RightTurn::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__RoomCrossing) {
         var8 = StrongholdPieces__RoomCrossing::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__StraightStairs) {
         var8 = StrongholdPieces__StraightStairs::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__SpiralStaircase) {
         var8 = StrongholdPieces__SpiralStaircase::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__FiveWayCrossing) {
         var8 = StrongholdPieces__FiveWayCrossing::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__ChestCorridor) {
         var8 = StrongholdPieces__ChestCorridor::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__Library) {
         var8 = StrongholdPieces__Library::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (type == CLS_StrongholdPieces__EndPortalRoom) {
         var8 = StrongholdPieces__EndPortalRoom::of(pieces, random, x, y, z, facing, generationDepth);
      }

      return std::dynamic_pointer_cast<StrongholdPieces__StrongholdPiece>(var8);
   }

std::shared_ptr<StrongholdPieces__StrongholdPiece> StrongholdPieces::generateNextPiece(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      if (!updateTotalWeight()) {
         return nullptr;
      } else {
         if (forcedPiece != 0) {
            std::shared_ptr<StrongholdPieces__StrongholdPiece> var8 = createPiece(forcedPiece, pieces, random, x, y, z, facing, generationDepth);
            forcedPiece = 0;
            if (var8 != nullptr) {
               return var8;
            }
         }

         int_t var13 = 0;

         while (var13 < 5) {
            var13++;
            int_t var9 = random.nextInt(totalWeight);

            for (auto var11 : currentWeights) {
               var9 -= var11->weight;
               if (var9 < 0) {
                  if (!var11->isValid(generationDepth) || var11 == startPiece->previous) {
                     break;
                  }

                  std::shared_ptr<StrongholdPieces__StrongholdPiece> var12 = createPiece(var11->type, pieces, random, x, y, z, facing, generationDepth);
                  if (var12 != nullptr) {
                     var11->amountGenerated++;
                     startPiece->previous = var11;
                     if (!var11->isValid()) {
                        currentWeights.erase(std::find(currentWeights.begin(), currentWeights.end(), var11));
                     }

                     return var12;
                  }
               }
            }
         }

         BoundingBox var14 = StrongholdPieces__PlainCorridor::findSize(pieces, random, x, y, z, facing);
         return var14.valid && var14.y0 > 1 ? std::make_shared<StrongholdPieces__PlainCorridor>(generationDepth, random, var14, facing) : nullptr;
      }
   }

std::shared_ptr<StructurePiece> StrongholdPieces::tryGenerateNextPiece(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      if (generationDepth > 50) {
         return nullptr;
      } else if (std::abs(x - startPiece->getBoundingBox().x0) <= 112 && std::abs(z - startPiece->getBoundingBox().z0) <= 112) {
         std::shared_ptr<StrongholdPieces__StrongholdPiece> var8 = generateNextPiece(startPiece, pieces, random, x, y, z, facing, generationDepth + 1);
         if (var8 != nullptr) {
            pieces.push_back(var8);
            startPiece->children.push_back(var8);
         }

         return var8;
      } else {
         return nullptr;
      }
   }

StrongholdPieces__StrongholdPieceWeight::StrongholdPieces__StrongholdPieceWeight(int_t type, int_t weight, int_t maxAmount)
{
      this->type = type;
      this->weight = weight;
      this->maxAmount = maxAmount;
   }

bool StrongholdPieces__StrongholdPieceWeight::isValid(int_t generationDepth)
{
      return this->maxAmount == 0 || this->amountGenerated < this->maxAmount;
   }

bool StrongholdPieces__StrongholdPieceWeight::isValid()
{
      return this->maxAmount == 0 || this->amountGenerated < this->maxAmount;
   }

StrongholdPieces__8455444::StrongholdPieces__8455444(int_t var1, int_t var2, int_t var3) : StrongholdPieces__StrongholdPieceWeight(var1, var2, var3)
{
   }

bool StrongholdPieces__8455444::isValid(int_t generationDepth)
{
      return Base::isValid(generationDepth) && generationDepth > 4;
   }

StrongholdPieces__9971267::StrongholdPieces__9971267(int_t var1, int_t var2, int_t var3) : StrongholdPieces__StrongholdPieceWeight(var1, var2, var3)
{
   }

bool StrongholdPieces__9971267::isValid(int_t generationDepth)
{
      return Base::isValid(generationDepth) && generationDepth > 5;
   }

StrongholdPieces__StrongholdPiece::StrongholdPieces__StrongholdPiece(int_t var1) : StructurePiece(var1)
{
   }

void StrongholdPieces__StrongholdPiece::generateEntrance(Level & level, Random & random, const BoundingBox & bb, StrongholdPieces__StrongholdPiece__EntranceType type, int_t x, int_t y, int_t z)
{
      switch ((static_cast<int_t>(type) + 1)) {
         case 1:
         default:
            this->generateBox(level, bb, x, y, z, x + 3 - 1, y + 3 - 1, z, 0, 0, false);
            break;
         case 2:
            this->placeBlock(level, Tile::stoneBrick.id, 0, x, y, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x, y + 1, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x, y + 2, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x + 1, y + 2, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x + 2, y + 2, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x + 2, y + 1, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x + 2, y, z, bb);
            this->placeBlock(level, Tile::doorWood.id, 0, x + 1, y, z, bb);
            this->placeBlock(level, Tile::doorWood.id, 8, x + 1, y + 1, z, bb);
            break;
         case 3:
            this->placeBlock(level, 0, 0, x + 1, y, z, bb);
            this->placeBlock(level, 0, 0, x + 1, y + 1, z, bb);
            this->placeBlock(level, Tile::ironBars.id, 0, x, y, z, bb);
            this->placeBlock(level, Tile::ironBars.id, 0, x, y + 1, z, bb);
            this->placeBlock(level, Tile::ironBars.id, 0, x, y + 2, z, bb);
            this->placeBlock(level, Tile::ironBars.id, 0, x + 1, y + 2, z, bb);
            this->placeBlock(level, Tile::ironBars.id, 0, x + 2, y + 2, z, bb);
            this->placeBlock(level, Tile::ironBars.id, 0, x + 2, y + 1, z, bb);
            this->placeBlock(level, Tile::ironBars.id, 0, x + 2, y, z, bb);
            break;
         case 4:
            this->placeBlock(level, Tile::stoneBrick.id, 0, x, y, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x, y + 1, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x, y + 2, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x + 1, y + 2, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x + 2, y + 2, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x + 2, y + 1, z, bb);
            this->placeBlock(level, Tile::stoneBrick.id, 0, x + 2, y, z, bb);
            this->placeBlock(level, Tile::doorIron.id, 0, x + 1, y, z, bb);
            this->placeBlock(level, Tile::doorIron.id, 8, x + 1, y + 1, z, bb);
            this->placeBlock(level, Tile::buttonStone.id, this->getOrientationData(Tile::buttonStone.id, 4), x + 2, y + 1, z + 1, bb);
            this->placeBlock(level, Tile::buttonStone.id, this->getOrientationData(Tile::buttonStone.id, 3), x + 2, y + 1, z - 1, bb);
      }
   }

StrongholdPieces__StrongholdPiece__EntranceType StrongholdPieces__StrongholdPiece::pickEntranceType(Random & random)
{
      int_t var2 = random.nextInt(5);
      switch (var2) {
         case 0:
         case 1:
         default:
            return StrongholdPieces__StrongholdPiece__EntranceType::OPENING;
         case 2:
            return StrongholdPieces__StrongholdPiece__EntranceType::WOOD_DOOR;
         case 3:
            return StrongholdPieces__StrongholdPiece__EntranceType::GATES;
         case 4:
            return StrongholdPieces__StrongholdPiece__EntranceType::IRON_DOOR;
      }
   }

std::shared_ptr<StructurePiece> StrongholdPieces__StrongholdPiece::generatePieceForward(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t offset, int_t yOffset)
{
      switch (this->orientation) {
         case 0:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z1 + 1, this->orientation, this->getGenDepth()
            );
         case 1:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, this->orientation, this->getGenDepth()
            );
         case 2:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z0 - 1, this->orientation, this->getGenDepth()
            );
         case 3:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, this->orientation, this->getGenDepth()
            );
         default:
            return nullptr;
      }
   }

std::shared_ptr<StructurePiece> StrongholdPieces__StrongholdPiece::generatePieceLeft(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset)
{
      switch (this->orientation) {
         case 0:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, 1, this->getGenDepth()
            );
         case 1:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z0 - 1, 2, this->getGenDepth()
            );
         case 2:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, 1, this->getGenDepth()
            );
         case 3:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z0 - 1, 2, this->getGenDepth()
            );
         default:
            return nullptr;
      }
   }

std::shared_ptr<StructurePiece> StrongholdPieces__StrongholdPiece::generatePieceRight(std::shared_ptr<StrongholdPieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset)
{
      switch (this->orientation) {
         case 0:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, 3, this->getGenDepth()
            );
         case 1:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z1 + 1, 0, this->getGenDepth()
            );
         case 2:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 + yOffset, this->boundingBox.z0 + offset, 3, this->getGenDepth()
            );
         case 3:
            return StrongholdPieces::tryGenerateNextPiece(
               startPiece, pieces, random, this->boundingBox.x0 + offset, this->boundingBox.y0 + yOffset, this->boundingBox.z1 + 1, 0, this->getGenDepth()
            );
         default:
            return nullptr;
      }
   }

bool StrongholdPieces__StrongholdPiece::isValidBounds(const BoundingBox & bb)
{
      return bb.valid && bb.y0 > 10;
   }

std::vector<WeighedTreasureItem> &StrongholdPieces__ChestCorridor::LOOT_ENTRIES()
{
	static std::vector<WeighedTreasureItem> value = std::vector<WeighedTreasureItem>{
      WeighedTreasureItem{Items::enderPearl->getShiftedIndex(), 0, 1, 1, 10},
      WeighedTreasureItem{Items::diamond->getShiftedIndex(), 0, 1, 3, 3},
      WeighedTreasureItem{Items::ingotIron->getShiftedIndex(), 0, 1, 5, 10},
      WeighedTreasureItem{Items::ingotGold->getShiftedIndex(), 0, 1, 3, 5},
      WeighedTreasureItem{Items::redstone->getShiftedIndex(), 0, 4, 9, 5},
      WeighedTreasureItem{Items::bread->getShiftedIndex(), 0, 1, 3, 15},
      WeighedTreasureItem{Items::apple->getShiftedIndex(), 0, 1, 3, 15},
      WeighedTreasureItem{Items::pickaxeIron->getShiftedIndex(), 0, 1, 1, 5},
      WeighedTreasureItem{Items::swordIron->getShiftedIndex(), 0, 1, 1, 5},
      WeighedTreasureItem{Items::plateIron->getShiftedIndex(), 0, 1, 1, 5},
      WeighedTreasureItem{Items::helmetIron->getShiftedIndex(), 0, 1, 1, 5},
      WeighedTreasureItem{Items::legsIron->getShiftedIndex(), 0, 1, 1, 5},
      WeighedTreasureItem{Items::bootsIron->getShiftedIndex(), 0, 1, 1, 5},
      WeighedTreasureItem{Items::appleGold->getShiftedIndex(), 0, 1, 1, 1}
   };
	return value;
}

StrongholdPieces__ChestCorridor::StrongholdPieces__ChestCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->entranceType = this->pickEntranceType(random);
      this->boundingBox = bb;
   }

void StrongholdPieces__ChestCorridor::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 1);
   }

std::shared_ptr<StrongholdPieces__ChestCorridor> StrongholdPieces__ChestCorridor::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 7, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<StrongholdPieces__ChestCorridor>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool StrongholdPieces__ChestCorridor::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(level, chunkBB, 0, 0, 0, 4, 4, 6, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 1, 1, 0);
         this->generateEntrance(level, random, chunkBB, StrongholdPieces__StrongholdPiece__EntranceType::OPENING, 1, 1, 6);
         this->generateBox(level, chunkBB, 3, 1, 2, 3, 1, 4, Tile::stoneBrick.id, Tile::stoneBrick.id, false);
         this->placeBlock(level, Tile::slabSingle.id, 5, 3, 1, 1, chunkBB);
         this->placeBlock(level, Tile::slabSingle.id, 5, 3, 1, 5, chunkBB);
         this->placeBlock(level, Tile::slabSingle.id, 5, 3, 2, 2, chunkBB);
         this->placeBlock(level, Tile::slabSingle.id, 5, 3, 2, 4, chunkBB);

         for (int_t var4 = 2; var4 <= 4; var4++) {
            this->placeBlock(level, Tile::slabSingle.id, 5, 2, 1, var4, chunkBB);
         }

         if (!this->hasChest) {
            int_t var7 = this->getWorldY(2);
            int_t var5 = this->getWorldX(3, 3);
            int_t var6 = this->getWorldZ(3, 3);
            if (chunkBB.isInside(var5, var7, var6)) {
               this->hasChest = true;
               this->createChest(level, chunkBB, random, 3, 2, 3, LOOT_ENTRIES(), 2 + random.nextInt(2));
            }
         }

         return true;
      }
   }

StrongholdPieces__EndPortalRoom::StrongholdPieces__EndPortalRoom(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void StrongholdPieces__EndPortalRoom::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      if (start != nullptr) {
         std::dynamic_pointer_cast<StrongholdPieces__Start>(start)->endPortalRoom = std::static_pointer_cast<StrongholdPieces__EndPortalRoom>(shared_from_this());
      }
   }

std::shared_ptr<StrongholdPieces__EndPortalRoom> StrongholdPieces__EndPortalRoom::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -4, -1, 0, 11, 8, 16, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<StrongholdPieces__EndPortalRoom>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool StrongholdPieces__EndPortalRoom::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 0, 0, 10, 7, 15, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateEntrance(level, random, chunkBB, StrongholdPieces__StrongholdPiece__EntranceType::GATES, 4, 1, 0);
      int_t var4 = 6;
      this->generateBox(level, chunkBB, 1, var4, 1, 1, var4, 14, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateBox(level, chunkBB, 9, var4, 1, 9, var4, 14, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateBox(level, chunkBB, 2, var4, 1, 8, var4, 2, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateBox(level, chunkBB, 2, var4, 14, 8, var4, 14, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateBox(level, chunkBB, 1, 1, 1, 2, 1, 4, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateBox(level, chunkBB, 8, 1, 1, 9, 1, 4, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateBox(level, chunkBB, 1, 1, 1, 1, 1, 3, Tile::lava.id, Tile::lava.id, false);
      this->generateBox(level, chunkBB, 9, 1, 1, 9, 1, 3, Tile::lava.id, Tile::lava.id, false);
      this->generateBox(level, chunkBB, 3, 1, 8, 7, 1, 12, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateBox(level, chunkBB, 4, 1, 9, 6, 1, 11, Tile::lava.id, Tile::lava.id, false);

      for (int_t var5 = 3; var5 < 14; var5 += 2) {
         this->generateBox(level, chunkBB, 0, 3, var5, 0, 4, var5, Tile::ironBars.id, Tile::ironBars.id, false);
         this->generateBox(level, chunkBB, 10, 3, var5, 10, 4, var5, Tile::ironBars.id, Tile::ironBars.id, false);
      }

      for (int_t var14 = 2; var14 < 9; var14 += 2) {
         this->generateBox(level, chunkBB, var14, 3, 15, var14, 4, 15, Tile::ironBars.id, Tile::ironBars.id, false);
      }

      int_t var15 = this->getOrientationData(Tile::stairsStoneBrick.id, 3);
      this->generateBox(level, chunkBB, 4, 1, 5, 6, 1, 7, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateBox(level, chunkBB, 4, 2, 6, 6, 2, 7, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
      this->generateBox(level, chunkBB, 4, 3, 7, 6, 3, 7, false, random, *StrongholdPieces::STONE_BRICK_PICKER());

      for (int_t var6 = 4; var6 <= 6; var6++) {
         this->placeBlock(level, Tile::stairsStoneBrick.id, var15, var6, 1, 4, chunkBB);
         this->placeBlock(level, Tile::stairsStoneBrick.id, var15, var6, 2, 5, chunkBB);
         this->placeBlock(level, Tile::stairsStoneBrick.id, var15, var6, 3, 6, chunkBB);
      }

      int_t var16 = 2;
      int_t var7 = 0;
      int_t var8 = 3;
      int_t var9 = 1;
      switch (this->orientation) {
         case 0:
            var16 = 0;
            var7 = 2;
            break;
         case 1:
            var16 = 1;
            var7 = 3;
            var8 = 0;
            var9 = 2;
         case 2:
         default:
            break;
         case 3:
            var16 = 3;
            var7 = 1;
            var8 = 0;
            var9 = 2;
      }

      this->placeBlock(level, Tile::endPortalFrame.id, var16 + (random.nextFloat() > 0.9f ? 4 : 0), 4, 3, 8, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var16 + (random.nextFloat() > 0.9f ? 4 : 0), 5, 3, 8, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var16 + (random.nextFloat() > 0.9f ? 4 : 0), 6, 3, 8, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var7 + (random.nextFloat() > 0.9f ? 4 : 0), 4, 3, 12, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var7 + (random.nextFloat() > 0.9f ? 4 : 0), 5, 3, 12, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var7 + (random.nextFloat() > 0.9f ? 4 : 0), 6, 3, 12, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var8 + (random.nextFloat() > 0.9f ? 4 : 0), 3, 3, 9, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var8 + (random.nextFloat() > 0.9f ? 4 : 0), 3, 3, 10, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var8 + (random.nextFloat() > 0.9f ? 4 : 0), 3, 3, 11, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var9 + (random.nextFloat() > 0.9f ? 4 : 0), 7, 3, 9, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var9 + (random.nextFloat() > 0.9f ? 4 : 0), 7, 3, 10, chunkBB);
      this->placeBlock(level, Tile::endPortalFrame.id, var9 + (random.nextFloat() > 0.9f ? 4 : 0), 7, 3, 11, chunkBB);
      if (!this->hasSpawner) {
         var4 = this->getWorldY(3);
         int_t var10 = this->getWorldX(5, 6);
         int_t var11 = this->getWorldZ(5, 6);
         if (chunkBB.isInside(var10, var4, var11)) {
            this->hasSpawner = true;
            level.setTile(var10, var4, var11, Tile::mobSpawner.id);
            std::shared_ptr<MobSpawnerTileEntity> var12 = std::dynamic_pointer_cast<MobSpawnerTileEntity>(level.getTileEntity(var10, var4, var11));
            if (var12 != nullptr) {
               var12->setEntityId(u"Silverfish");
            }
         }
      }

      return true;
   }

StrongholdPieces__FiveWayCrossing::StrongholdPieces__FiveWayCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->entranceType = this->pickEntranceType(random);
      this->boundingBox = bb;
      this->leftLow = random.nextBoolean();
      this->leftHigh = random.nextBoolean();
      this->rightLow = random.nextBoolean();
      this->rightHigh = random.nextInt(3) > 0;
   }

void StrongholdPieces__FiveWayCrossing::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      int_t var4 = 3;
      int_t var5 = 5;
      if (this->orientation == 1 || this->orientation == 2) {
         var4 = 8 - var4;
         var5 = 8 - var5;
      }

      this->generatePieceForward(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 5, 1);
      if (this->leftLow) {
         this->generatePieceLeft(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, var4, 1);
      }

      if (this->leftHigh) {
         this->generatePieceLeft(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, var5, 7);
      }

      if (this->rightLow) {
         this->generatePieceRight(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, var4, 1);
      }

      if (this->rightHigh) {
         this->generatePieceRight(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, var5, 7);
      }
   }

std::shared_ptr<StrongholdPieces__FiveWayCrossing> StrongholdPieces__FiveWayCrossing::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -4, -3, 0, 10, 9, 11, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<StrongholdPieces__FiveWayCrossing>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool StrongholdPieces__FiveWayCrossing::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(level, chunkBB, 0, 0, 0, 9, 8, 10, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 4, 3, 0);
         if (this->leftLow) {
            this->generateBox(level, chunkBB, 0, 3, 1, 0, 5, 3, 0, 0, false);
         }

         if (this->rightLow) {
            this->generateBox(level, chunkBB, 9, 3, 1, 9, 5, 3, 0, 0, false);
         }

         if (this->leftHigh) {
            this->generateBox(level, chunkBB, 0, 5, 7, 0, 7, 9, 0, 0, false);
         }

         if (this->rightHigh) {
            this->generateBox(level, chunkBB, 9, 5, 7, 9, 7, 9, 0, 0, false);
         }

         this->generateBox(level, chunkBB, 5, 1, 10, 7, 3, 10, 0, 0, false);
         this->generateBox(level, chunkBB, 1, 2, 1, 8, 2, 6, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 4, 1, 5, 4, 4, 9, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 8, 1, 5, 8, 4, 9, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 1, 4, 7, 3, 4, 9, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 1, 3, 5, 3, 3, 6, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 1, 3, 4, 3, 3, 4, Tile::slabSingle.id, Tile::slabSingle.id, false);
         this->generateBox(level, chunkBB, 1, 4, 6, 3, 4, 6, Tile::slabSingle.id, Tile::slabSingle.id, false);
         this->generateBox(level, chunkBB, 5, 1, 7, 7, 1, 8, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 5, 1, 9, 7, 1, 9, Tile::slabSingle.id, Tile::slabSingle.id, false);
         this->generateBox(level, chunkBB, 5, 2, 7, 7, 2, 7, Tile::slabSingle.id, Tile::slabSingle.id, false);
         this->generateBox(level, chunkBB, 4, 5, 7, 4, 5, 9, Tile::slabSingle.id, Tile::slabSingle.id, false);
         this->generateBox(level, chunkBB, 8, 5, 7, 8, 5, 9, Tile::slabSingle.id, Tile::slabSingle.id, false);
         this->generateBox(level, chunkBB, 5, 5, 7, 7, 5, 9, Tile::slabDouble.id, Tile::slabDouble.id, false);
         this->placeBlock(level, Tile::torch.id, 0, 6, 5, 6, chunkBB);
         return true;
      }
   }

StrongholdPieces__LeftTurn::StrongholdPieces__LeftTurn(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->entranceType = this->pickEntranceType(random);
      this->boundingBox = bb;
   }

void StrongholdPieces__LeftTurn::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      if (this->orientation != 2 && this->orientation != 3) {
         this->generatePieceRight(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 1);
      } else {
         this->generatePieceLeft(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 1);
      }
   }

std::shared_ptr<StrongholdPieces__LeftTurn> StrongholdPieces__LeftTurn::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 5, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<StrongholdPieces__LeftTurn>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool StrongholdPieces__LeftTurn::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(level, chunkBB, 0, 0, 0, 4, 4, 4, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 1, 1, 0);
         if (this->orientation != 2 && this->orientation != 3) {
            this->generateBox(level, chunkBB, 4, 1, 1, 4, 3, 3, 0, 0, false);
         } else {
            this->generateBox(level, chunkBB, 0, 1, 1, 0, 3, 3, 0, 0, false);
         }

         return true;
      }
   }

std::vector<WeighedTreasureItem> &StrongholdPieces__Library::LOOT_ENTRIES()
{
	static std::vector<WeighedTreasureItem> value = std::vector<WeighedTreasureItem>{
      WeighedTreasureItem{Items::book->getShiftedIndex(), 0, 1, 3, 20},
      WeighedTreasureItem{Items::paper->getShiftedIndex(), 0, 2, 7, 20},
      WeighedTreasureItem{Items::map->getShiftedIndex(), 0, 1, 1, 1},
      WeighedTreasureItem{Items::compass->getShiftedIndex(), 0, 1, 1, 1}
   };
	return value;
}

StrongholdPieces__Library::StrongholdPieces__Library(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->entranceType = this->pickEntranceType(random);
      this->boundingBox = bb;
      this->tall = bb.getYSpan() > 6;
   }

void StrongholdPieces__Library::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<StrongholdPieces__Library> StrongholdPieces__Library::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -4, -1, 0, 14, 11, 15, facing);
      if (!isValidBounds(var7) || StructurePiece::findCollisionPiece(pieces, var7) != nullptr) {
         var7 = BoundingBox::orientBox(x, y, z, -4, -1, 0, 14, 6, 15, facing);
         if (!isValidBounds(var7) || StructurePiece::findCollisionPiece(pieces, var7) != nullptr) {
            return nullptr;
         }
      }

      return std::make_shared<StrongholdPieces__Library>(generationDepth, random, var7, facing);
   }

bool StrongholdPieces__Library::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         int_t var4 = 11;
         if (!this->tall) {
            var4 = 6;
         }

         this->generateBox(level, chunkBB, 0, 0, 0, 13, var4 - 1, 14, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 4, 1, 0);
         this->generateMaybeBox(level, chunkBB, random, 0.07f, 2, 1, 1, 11, 4, 13, Tile::cobweb.id, Tile::cobweb.id, false);

         for (int_t var7 = 1; var7 <= 13; var7++) {
            if ((var7 - 1) % 4 == 0) {
               this->generateBox(level, chunkBB, 1, 1, var7, 1, 4, var7, Tile::wood.id, Tile::wood.id, false);
               this->generateBox(level, chunkBB, 12, 1, var7, 12, 4, var7, Tile::wood.id, Tile::wood.id, false);
               this->placeBlock(level, Tile::torch.id, 0, 2, 3, var7, chunkBB);
               this->placeBlock(level, Tile::torch.id, 0, 11, 3, var7, chunkBB);
               if (this->tall) {
                  this->generateBox(level, chunkBB, 1, 6, var7, 1, 9, var7, Tile::wood.id, Tile::wood.id, false);
                  this->generateBox(level, chunkBB, 12, 6, var7, 12, 9, var7, Tile::wood.id, Tile::wood.id, false);
               }
            } else {
               this->generateBox(level, chunkBB, 1, 1, var7, 1, 4, var7, Tile::bookshelf.id, Tile::bookshelf.id, false);
               this->generateBox(level, chunkBB, 12, 1, var7, 12, 4, var7, Tile::bookshelf.id, Tile::bookshelf.id, false);
               if (this->tall) {
                  this->generateBox(level, chunkBB, 1, 6, var7, 1, 9, var7, Tile::bookshelf.id, Tile::bookshelf.id, false);
                  this->generateBox(level, chunkBB, 12, 6, var7, 12, 9, var7, Tile::bookshelf.id, Tile::bookshelf.id, false);
               }
            }
         }

         for (int_t var10 = 3; var10 < 12; var10 += 2) {
            this->generateBox(level, chunkBB, 3, 1, var10, 4, 3, var10, Tile::bookshelf.id, Tile::bookshelf.id, false);
            this->generateBox(level, chunkBB, 6, 1, var10, 7, 3, var10, Tile::bookshelf.id, Tile::bookshelf.id, false);
            this->generateBox(level, chunkBB, 9, 1, var10, 10, 3, var10, Tile::bookshelf.id, Tile::bookshelf.id, false);
         }

         if (this->tall) {
            this->generateBox(level, chunkBB, 1, 5, 1, 3, 5, 13, Tile::wood.id, Tile::wood.id, false);
            this->generateBox(level, chunkBB, 10, 5, 1, 12, 5, 13, Tile::wood.id, Tile::wood.id, false);
            this->generateBox(level, chunkBB, 4, 5, 1, 9, 5, 2, Tile::wood.id, Tile::wood.id, false);
            this->generateBox(level, chunkBB, 4, 5, 12, 9, 5, 13, Tile::wood.id, Tile::wood.id, false);
            this->placeBlock(level, Tile::wood.id, 0, 9, 5, 11, chunkBB);
            this->placeBlock(level, Tile::wood.id, 0, 8, 5, 11, chunkBB);
            this->placeBlock(level, Tile::wood.id, 0, 9, 5, 10, chunkBB);
            this->generateBox(level, chunkBB, 3, 6, 2, 3, 6, 12, Tile::fence.id, Tile::fence.id, false);
            this->generateBox(level, chunkBB, 10, 6, 2, 10, 6, 10, Tile::fence.id, Tile::fence.id, false);
            this->generateBox(level, chunkBB, 4, 6, 2, 9, 6, 2, Tile::fence.id, Tile::fence.id, false);
            this->generateBox(level, chunkBB, 4, 6, 12, 8, 6, 12, Tile::fence.id, Tile::fence.id, false);
            this->placeBlock(level, Tile::fence.id, 0, 9, 6, 11, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, 8, 6, 11, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, 9, 6, 10, chunkBB);
            int_t var11 = this->getOrientationData(Tile::ladder.id, 3);
            this->placeBlock(level, Tile::ladder.id, var11, 10, 1, 13, chunkBB);
            this->placeBlock(level, Tile::ladder.id, var11, 10, 2, 13, chunkBB);
            this->placeBlock(level, Tile::ladder.id, var11, 10, 3, 13, chunkBB);
            this->placeBlock(level, Tile::ladder.id, var11, 10, 4, 13, chunkBB);
            this->placeBlock(level, Tile::ladder.id, var11, 10, 5, 13, chunkBB);
            this->placeBlock(level, Tile::ladder.id, var11, 10, 6, 13, chunkBB);
            this->placeBlock(level, Tile::ladder.id, var11, 10, 7, 13, chunkBB);
            int_t var8 = 7;
            int_t var9 = 7;
            this->placeBlock(level, Tile::fence.id, 0, var8 - 1, 9, var9, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8, 9, var9, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8 - 1, 8, var9, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8, 8, var9, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8 - 1, 7, var9, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8, 7, var9, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8 - 2, 7, var9, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8 + 1, 7, var9, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8 - 1, 7, var9 - 1, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8 - 1, 7, var9 + 1, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8, 7, var9 - 1, chunkBB);
            this->placeBlock(level, Tile::fence.id, 0, var8, 7, var9 + 1, chunkBB);
            this->placeBlock(level, Tile::torch.id, 0, var8 - 2, 8, var9, chunkBB);
            this->placeBlock(level, Tile::torch.id, 0, var8 + 1, 8, var9, chunkBB);
            this->placeBlock(level, Tile::torch.id, 0, var8 - 1, 8, var9 - 1, chunkBB);
            this->placeBlock(level, Tile::torch.id, 0, var8 - 1, 8, var9 + 1, chunkBB);
            this->placeBlock(level, Tile::torch.id, 0, var8, 8, var9 - 1, chunkBB);
            this->placeBlock(level, Tile::torch.id, 0, var8, 8, var9 + 1, chunkBB);
         }

         this->createChest(level, chunkBB, random, 3, 3, 5, LOOT_ENTRIES(), 1 + random.nextInt(4));
         if (this->tall) {
            this->placeBlock(level, 0, 0, 12, 9, 1, chunkBB);
            this->createChest(level, chunkBB, random, 12, 8, 1, LOOT_ENTRIES(), 1 + random.nextInt(4));
         }

         return true;
      }
   }

StrongholdPieces__PlainCorridor::StrongholdPieces__PlainCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
      this->steps = facing != 2 && facing != 0 ? bb.getXSpan() : bb.getZSpan();
   }

void StrongholdPieces__PlainCorridor::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

BoundingBox StrongholdPieces__PlainCorridor::findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 4, facing);
      std::shared_ptr<StructurePiece> var8 = StructurePiece::findCollisionPiece(pieces, var7);
      if (var8 == nullptr) {
         return BoundingBox::none();
      } else {
         if (var8->getBoundingBox().y0 == var7.y0) {
            for (int_t var9 = 3; var9 >= 1; var9--) {
               var7 = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, var9 - 1, facing);
               if (!var8->getBoundingBox().intersects(var7)) {
                  return BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, var9, facing);
               }
            }
         }

         return BoundingBox::none();
      }
   }

bool StrongholdPieces__PlainCorridor::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         for (int_t var4 = 0; var4 < this->steps; var4++) {
            this->placeBlock(level, Tile::stoneBrick.id, 0, 0, 0, var4, chunkBB);
            this->placeBlock(level, Tile::stoneBrick.id, 0, 1, 0, var4, chunkBB);
            this->placeBlock(level, Tile::stoneBrick.id, 0, 2, 0, var4, chunkBB);
            this->placeBlock(level, Tile::stoneBrick.id, 0, 3, 0, var4, chunkBB);
            this->placeBlock(level, Tile::stoneBrick.id, 0, 4, 0, var4, chunkBB);

            for (int_t var5 = 1; var5 <= 3; var5++) {
               this->placeBlock(level, Tile::stoneBrick.id, 0, 0, var5, var4, chunkBB);
               this->placeBlock(level, 0, 0, 1, var5, var4, chunkBB);
               this->placeBlock(level, 0, 0, 2, var5, var4, chunkBB);
               this->placeBlock(level, 0, 0, 3, var5, var4, chunkBB);
               this->placeBlock(level, Tile::stoneBrick.id, 0, 4, var5, var4, chunkBB);
            }

            this->placeBlock(level, Tile::stoneBrick.id, 0, 0, 4, var4, chunkBB);
            this->placeBlock(level, Tile::stoneBrick.id, 0, 1, 4, var4, chunkBB);
            this->placeBlock(level, Tile::stoneBrick.id, 0, 2, 4, var4, chunkBB);
            this->placeBlock(level, Tile::stoneBrick.id, 0, 3, 4, var4, chunkBB);
            this->placeBlock(level, Tile::stoneBrick.id, 0, 4, 4, var4, chunkBB);
         }

         return true;
      }
   }

StrongholdPieces__Prison::StrongholdPieces__Prison(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->entranceType = this->pickEntranceType(random);
      this->boundingBox = bb;
   }

void StrongholdPieces__Prison::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 1);
   }

std::shared_ptr<StrongholdPieces__Prison> StrongholdPieces__Prison::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -1, 0, 9, 5, 11, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<StrongholdPieces__Prison>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool StrongholdPieces__Prison::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(level, chunkBB, 0, 0, 0, 8, 4, 10, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 1, 1, 0);
         this->generateBox(level, chunkBB, 1, 1, 10, 3, 3, 10, 0, 0, false);
         this->generateBox(level, chunkBB, 4, 1, 1, 4, 3, 1, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 4, 1, 3, 4, 3, 3, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 4, 1, 7, 4, 3, 7, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 4, 1, 9, 4, 3, 9, false, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateBox(level, chunkBB, 4, 1, 4, 4, 3, 6, Tile::ironBars.id, Tile::ironBars.id, false);
         this->generateBox(level, chunkBB, 5, 1, 5, 7, 3, 5, Tile::ironBars.id, Tile::ironBars.id, false);
         this->placeBlock(level, Tile::ironBars.id, 0, 4, 3, 2, chunkBB);
         this->placeBlock(level, Tile::ironBars.id, 0, 4, 3, 8, chunkBB);
         this->placeBlock(level, Tile::doorIron.id, this->getOrientationData(Tile::doorIron.id, 3), 4, 1, 2, chunkBB);
         this->placeBlock(level, Tile::doorIron.id, this->getOrientationData(Tile::doorIron.id, 3) + 8, 4, 2, 2, chunkBB);
         this->placeBlock(level, Tile::doorIron.id, this->getOrientationData(Tile::doorIron.id, 3), 4, 1, 8, chunkBB);
         this->placeBlock(level, Tile::doorIron.id, this->getOrientationData(Tile::doorIron.id, 3) + 8, 4, 2, 8, chunkBB);
         return true;
      }
   }

StrongholdPieces__RightTurn::StrongholdPieces__RightTurn(int_t var1, Random & var2, const BoundingBox & var3, int_t var4) : StrongholdPieces__LeftTurn(var1, var2, var3, var4)
{
   }

void StrongholdPieces__RightTurn::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      if (this->orientation != 2 && this->orientation != 3) {
         this->generatePieceLeft(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 1);
      } else {
         this->generatePieceRight(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 1);
      }
   }

bool StrongholdPieces__RightTurn::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(level, chunkBB, 0, 0, 0, 4, 4, 4, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 1, 1, 0);
         if (this->orientation != 2 && this->orientation != 3) {
            this->generateBox(level, chunkBB, 0, 1, 1, 0, 3, 3, 0, 0, false);
         } else {
            this->generateBox(level, chunkBB, 4, 1, 1, 4, 3, 3, 0, 0, false);
         }

         return true;
      }
   }

std::vector<WeighedTreasureItem> &StrongholdPieces__RoomCrossing::LOOT_ENTRIES()
{
	static std::vector<WeighedTreasureItem> value = std::vector<WeighedTreasureItem>{
      WeighedTreasureItem{Items::ingotIron->getShiftedIndex(), 0, 1, 5, 10},
      WeighedTreasureItem{Items::ingotGold->getShiftedIndex(), 0, 1, 3, 5},
      WeighedTreasureItem{Items::redstone->getShiftedIndex(), 0, 4, 9, 5},
      WeighedTreasureItem{Items::coal->getShiftedIndex(), 0, 3, 8, 10},
      WeighedTreasureItem{Items::bread->getShiftedIndex(), 0, 1, 3, 15},
      WeighedTreasureItem{Items::apple->getShiftedIndex(), 0, 1, 3, 15},
      WeighedTreasureItem{Items::pickaxeIron->getShiftedIndex(), 0, 1, 1, 1}
   };
	return value;
}

StrongholdPieces__RoomCrossing::StrongholdPieces__RoomCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->entranceType = this->pickEntranceType(random);
      this->boundingBox = bb;
      this->type = random.nextInt(5);
   }

void StrongholdPieces__RoomCrossing::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 4, 1);
      this->generatePieceLeft(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 4);
      this->generatePieceRight(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 4);
   }

std::shared_ptr<StrongholdPieces__RoomCrossing> StrongholdPieces__RoomCrossing::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -4, -1, 0, 11, 7, 11, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<StrongholdPieces__RoomCrossing>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool StrongholdPieces__RoomCrossing::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(level, chunkBB, 0, 0, 0, 10, 6, 10, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 4, 1, 0);
         this->generateBox(level, chunkBB, 4, 1, 10, 6, 3, 10, 0, 0, false);
         this->generateBox(level, chunkBB, 0, 1, 4, 0, 3, 6, 0, 0, false);
         this->generateBox(level, chunkBB, 10, 1, 4, 10, 3, 6, 0, 0, false);
         switch (this->type) {
            case 0:
               this->placeBlock(level, Tile::stoneBrick.id, 0, 5, 1, 5, chunkBB);
               this->placeBlock(level, Tile::stoneBrick.id, 0, 5, 2, 5, chunkBB);
               this->placeBlock(level, Tile::stoneBrick.id, 0, 5, 3, 5, chunkBB);
               this->placeBlock(level, Tile::torch.id, 0, 4, 3, 5, chunkBB);
               this->placeBlock(level, Tile::torch.id, 0, 6, 3, 5, chunkBB);
               this->placeBlock(level, Tile::torch.id, 0, 5, 3, 4, chunkBB);
               this->placeBlock(level, Tile::torch.id, 0, 5, 3, 6, chunkBB);
               this->placeBlock(level, Tile::slabSingle.id, 0, 4, 1, 4, chunkBB);
               this->placeBlock(level, Tile::slabSingle.id, 0, 4, 1, 5, chunkBB);
               this->placeBlock(level, Tile::slabSingle.id, 0, 4, 1, 6, chunkBB);
               this->placeBlock(level, Tile::slabSingle.id, 0, 6, 1, 4, chunkBB);
               this->placeBlock(level, Tile::slabSingle.id, 0, 6, 1, 5, chunkBB);
               this->placeBlock(level, Tile::slabSingle.id, 0, 6, 1, 6, chunkBB);
               this->placeBlock(level, Tile::slabSingle.id, 0, 5, 1, 4, chunkBB);
               this->placeBlock(level, Tile::slabSingle.id, 0, 5, 1, 6, chunkBB);
               break;
            case 1:
               for (int_t var8 = 0; var8 < 5; var8++) {
                  this->placeBlock(level, Tile::stoneBrick.id, 0, 3, 1, 3 + var8, chunkBB);
                  this->placeBlock(level, Tile::stoneBrick.id, 0, 7, 1, 3 + var8, chunkBB);
                  this->placeBlock(level, Tile::stoneBrick.id, 0, 3 + var8, 1, 3, chunkBB);
                  this->placeBlock(level, Tile::stoneBrick.id, 0, 3 + var8, 1, 7, chunkBB);
               }

               this->placeBlock(level, Tile::stoneBrick.id, 0, 5, 1, 5, chunkBB);
               this->placeBlock(level, Tile::stoneBrick.id, 0, 5, 2, 5, chunkBB);
               this->placeBlock(level, Tile::stoneBrick.id, 0, 5, 3, 5, chunkBB);
               this->placeBlock(level, Tile::water.id, 0, 5, 4, 5, chunkBB);
               break;
            case 2:
               for (int_t var4 = 1; var4 <= 9; var4++) {
                  this->placeBlock(level, Tile::cobblestone.id, 0, 1, 3, var4, chunkBB);
                  this->placeBlock(level, Tile::cobblestone.id, 0, 9, 3, var4, chunkBB);
               }

               for (int_t var5 = 1; var5 <= 9; var5++) {
                  this->placeBlock(level, Tile::cobblestone.id, 0, var5, 3, 1, chunkBB);
                  this->placeBlock(level, Tile::cobblestone.id, 0, var5, 3, 9, chunkBB);
               }

               this->placeBlock(level, Tile::cobblestone.id, 0, 5, 1, 4, chunkBB);
               this->placeBlock(level, Tile::cobblestone.id, 0, 5, 1, 6, chunkBB);
               this->placeBlock(level, Tile::cobblestone.id, 0, 5, 3, 4, chunkBB);
               this->placeBlock(level, Tile::cobblestone.id, 0, 5, 3, 6, chunkBB);
               this->placeBlock(level, Tile::cobblestone.id, 0, 4, 1, 5, chunkBB);
               this->placeBlock(level, Tile::cobblestone.id, 0, 6, 1, 5, chunkBB);
               this->placeBlock(level, Tile::cobblestone.id, 0, 4, 3, 5, chunkBB);
               this->placeBlock(level, Tile::cobblestone.id, 0, 6, 3, 5, chunkBB);

               for (int_t var6 = 1; var6 <= 3; var6++) {
                  this->placeBlock(level, Tile::cobblestone.id, 0, 4, var6, 4, chunkBB);
                  this->placeBlock(level, Tile::cobblestone.id, 0, 6, var6, 4, chunkBB);
                  this->placeBlock(level, Tile::cobblestone.id, 0, 4, var6, 6, chunkBB);
                  this->placeBlock(level, Tile::cobblestone.id, 0, 6, var6, 6, chunkBB);
               }

               this->placeBlock(level, Tile::torch.id, 0, 5, 3, 5, chunkBB);

               for (int_t var7 = 2; var7 <= 8; var7++) {
                  this->placeBlock(level, Tile::wood.id, 0, 2, 3, var7, chunkBB);
                  this->placeBlock(level, Tile::wood.id, 0, 3, 3, var7, chunkBB);
                  if (var7 <= 3 || var7 >= 7) {
                     this->placeBlock(level, Tile::wood.id, 0, 4, 3, var7, chunkBB);
                     this->placeBlock(level, Tile::wood.id, 0, 5, 3, var7, chunkBB);
                     this->placeBlock(level, Tile::wood.id, 0, 6, 3, var7, chunkBB);
                  }

                  this->placeBlock(level, Tile::wood.id, 0, 7, 3, var7, chunkBB);
                  this->placeBlock(level, Tile::wood.id, 0, 8, 3, var7, chunkBB);
               }

               this->placeBlock(level, Tile::ladder.id, this->getOrientationData(Tile::ladder.id, 4), 9, 1, 3, chunkBB);
               this->placeBlock(level, Tile::ladder.id, this->getOrientationData(Tile::ladder.id, 4), 9, 2, 3, chunkBB);
               this->placeBlock(level, Tile::ladder.id, this->getOrientationData(Tile::ladder.id, 4), 9, 3, 3, chunkBB);
               this->createChest(level, chunkBB, random, 3, 4, 8, LOOT_ENTRIES(), 1 + random.nextInt(4));
         }

         return true;
      }
   }

StrongholdPieces__SpiralStaircase::StrongholdPieces__SpiralStaircase(int_t generationDepth, Random & random, int_t x, int_t z) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->isSource = true;
      this->orientation = random.nextInt(4);
      this->entranceType = StrongholdPieces__StrongholdPiece__EntranceType::OPENING;
      switch (this->orientation) {
         case 0:
         case 2:
            this->boundingBox = BoundingBox(x, 64, z, x + 5 - 1, 74, z + 5 - 1);
            break;
         default:
            this->boundingBox = BoundingBox(x, 64, z, x + 5 - 1, 74, z + 5 - 1);
      }
   }

StrongholdPieces__SpiralStaircase::StrongholdPieces__SpiralStaircase(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->isSource = false;
      this->orientation = facing;
      this->entranceType = this->pickEntranceType(random);
      this->boundingBox = bb;
   }

void StrongholdPieces__SpiralStaircase::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      if (this->isSource) {
         StrongholdPieces::forcedPiece = CLS_StrongholdPieces__FiveWayCrossing;
      }

      this->generatePieceForward(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 1);
   }

std::shared_ptr<StrongholdPieces__SpiralStaircase> StrongholdPieces__SpiralStaircase::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -7, 0, 5, 11, 5, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<StrongholdPieces__SpiralStaircase>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool StrongholdPieces__SpiralStaircase::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         if (this->isSource) {
         }

         this->generateBox(level, chunkBB, 0, 0, 0, 4, 10, 4, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 1, 7, 0);
         this->generateEntrance(level, random, chunkBB, StrongholdPieces__StrongholdPiece__EntranceType::OPENING, 1, 1, 4);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 2, 6, 1, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 1, 5, 1, chunkBB);
         this->placeBlock(level, Tile::slabSingle.id, 0, 1, 6, 1, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 1, 5, 2, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 1, 4, 3, chunkBB);
         this->placeBlock(level, Tile::slabSingle.id, 0, 1, 5, 3, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 2, 4, 3, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 3, 3, 3, chunkBB);
         this->placeBlock(level, Tile::slabSingle.id, 0, 3, 4, 3, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 3, 3, 2, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 3, 2, 1, chunkBB);
         this->placeBlock(level, Tile::slabSingle.id, 0, 3, 3, 1, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 2, 2, 1, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 1, 1, 1, chunkBB);
         this->placeBlock(level, Tile::slabSingle.id, 0, 1, 2, 1, chunkBB);
         this->placeBlock(level, Tile::stoneBrick.id, 0, 1, 1, 2, chunkBB);
         this->placeBlock(level, Tile::slabSingle.id, 0, 1, 1, 3, chunkBB);
         return true;
      }
   }

StrongholdPieces__Start::StrongholdPieces__Start(int_t var1, Random & var2, int_t var3, int_t var4) : StrongholdPieces__SpiralStaircase(0, var2, var3, var4)
{
   }

TilePos3 StrongholdPieces__Start::getLocatorPosition() const
{
      return this->endPortalRoom != nullptr ? this->endPortalRoom->getLocatorPosition() : Base::getLocatorPosition();
   }

StrongholdPieces__StoneBrickPicker::StrongholdPieces__StoneBrickPicker()
{
   }

void StrongholdPieces__StoneBrickPicker::next(Random & randomm, int_t x, int_t y, int_t z, bool isNonAir)
{
      if (!isNonAir) {
         this->nextId = 0;
         this->nextData = 0;
      } else {
         this->nextId = Tile::stoneBrick.id;
         float var6 = randomm.nextFloat();
         if (var6 < 0.2f) {
            this->nextData = 2;
         } else if (var6 < 0.5f) {
            this->nextData = 1;
         } else if (var6 < 0.55f) {
            this->nextId = Tile::monsterEgg.id;
            this->nextData = 2;
         } else {
            this->nextData = 0;
         }
      }
   }

StrongholdPieces__StraightCorridor::StrongholdPieces__StraightCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->entranceType = this->pickEntranceType(random);
      this->boundingBox = bb;
      this->left = random.nextInt(2) == 0;
      this->right = random.nextInt(2) == 0;
   }

void StrongholdPieces__StraightCorridor::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 1);
      if (this->left) {
         this->generatePieceLeft(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 2);
      }

      if (this->right) {
         this->generatePieceRight(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 2);
      }
   }

std::shared_ptr<StrongholdPieces__StraightCorridor> StrongholdPieces__StraightCorridor::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 7, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<StrongholdPieces__StraightCorridor>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool StrongholdPieces__StraightCorridor::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(level, chunkBB, 0, 0, 0, 4, 4, 6, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 1, 1, 0);
         this->generateEntrance(level, random, chunkBB, StrongholdPieces__StrongholdPiece__EntranceType::OPENING, 1, 1, 6);
         this->maybeGenerateBlock(level, chunkBB, random, 0.1f, 1, 2, 1, Tile::torch.id, 0);
         this->maybeGenerateBlock(level, chunkBB, random, 0.1f, 3, 2, 1, Tile::torch.id, 0);
         this->maybeGenerateBlock(level, chunkBB, random, 0.1f, 1, 2, 5, Tile::torch.id, 0);
         this->maybeGenerateBlock(level, chunkBB, random, 0.1f, 3, 2, 5, Tile::torch.id, 0);
         if (this->left) {
            this->generateBox(level, chunkBB, 0, 1, 2, 0, 3, 4, 0, 0, false);
         }

         if (this->right) {
            this->generateBox(level, chunkBB, 4, 1, 2, 4, 3, 4, 0, 0, false);
         }

         return true;
      }
   }

StrongholdPieces__StraightStairs::StrongholdPieces__StraightStairs(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StrongholdPieces__StrongholdPiece(generationDepth)
{
      this->orientation = facing;
      this->entranceType = this->pickEntranceType(random);
      this->boundingBox = bb;
   }

void StrongholdPieces__StraightStairs::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<StrongholdPieces__Start>(start), pieces, random, 1, 1);
   }

std::shared_ptr<StrongholdPieces__StraightStairs> StrongholdPieces__StraightStairs::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -7, 0, 5, 11, 8, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<StrongholdPieces__StraightStairs>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool StrongholdPieces__StraightStairs::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(level, chunkBB, 0, 0, 0, 4, 10, 7, true, random, *StrongholdPieces::STONE_BRICK_PICKER());
         this->generateEntrance(level, random, chunkBB, this->entranceType, 1, 7, 0);
         this->generateEntrance(level, random, chunkBB, StrongholdPieces__StrongholdPiece__EntranceType::OPENING, 1, 1, 7);
         int_t var4 = this->getOrientationData(Tile::stairsStone.id, 2);

         for (int_t var5 = 0; var5 < 6; var5++) {
            this->placeBlock(level, Tile::stairsStone.id, var4, 1, 6 - var5, 1 + var5, chunkBB);
            this->placeBlock(level, Tile::stairsStone.id, var4, 2, 6 - var5, 1 + var5, chunkBB);
            this->placeBlock(level, Tile::stairsStone.id, var4, 3, 6 - var5, 1 + var5, chunkBB);
            if (var5 < 5) {
               this->placeBlock(level, Tile::stoneBrick.id, 0, 1, 5 - var5, 1 + var5, chunkBB);
               this->placeBlock(level, Tile::stoneBrick.id, 0, 2, 5 - var5, 1 + var5, chunkBB);
               this->placeBlock(level, Tile::stoneBrick.id, 0, 3, 5 - var5, 1 + var5, chunkBB);
            }
         }

         return true;
      }
   }

class StrongholdStart : public StructureStart
{
public:
	StrongholdStart(Level &level, Random &random, int_t chunkX, int_t chunkZ)
	{
		StrongholdPieces::resetWeights();
		auto start = std::make_shared<StrongholdPieces__Start>(0, random, (chunkX << 4) + 2, (chunkZ << 4) + 2);
		pieces.push_back(start);
		start->addChildren(start, pieces, random);
		PieceList &children = start->children;
		while (!children.empty())
		{
			int_t i = random.nextInt(static_cast<int_t>(children.size()));
			std::shared_ptr<StructurePiece> piece = jremove(children, i);
			piece->addChildren(start, pieces, random);
		}
		calculateBoundingBox();
		moveBelowSeaLevel(level, random, 10);
	}

	bool hasPortalRoom()
	{
		if (pieces.empty())
			return false;
		auto start = std::dynamic_pointer_cast<StrongholdPieces__Start>(pieces.front());
		return start != nullptr && start->endPortalRoom != nullptr;
	}
};

bool StrongholdFeature::isFeatureChunk(int_t x, int_t z)
{
	if (!positionsFound)
	{
		static std::vector<Biome *> validBiomes = {Biome::desert, Biome::forest, Biome::extremeHills, Biome::swampland, Biome::taiga, Biome::icePlains, Biome::iceMountains};
		random.setSeed(level->seed);
		double angle = random.nextDouble() * 3.141592653589793 * 2.0;
		for (int_t i = 0; i < 3; i++)
		{
			double dist = (1.25 + random.nextDouble()) * 32.0;
			int_t cx = static_cast<int_t>(std::floor(std::cos(angle) * dist + 0.5));
			int_t cz = static_cast<int_t>(std::floor(std::sin(angle) * dist + 0.5));
			BiomeBlockPos pos = level->getBiomeSource().findBiome((cx << 4) + 8, (cz << 4) + 8, 112, validBiomes, random);
			if (pos.found)
			{
				cx = pos.x >> 4;
				cz = pos.z >> 4;
			}
			posX[i] = cx;
			posZ[i] = cz;
			hasPosition[i] = true;
			angle += (3.141592653589793 * 2) / 3.0;
		}
		positionsFound = true;
	}

	for (int_t i = 0; i < 3; i++)
	{
		if (hasPosition[i] && x == posX[i] && z == posZ[i])
			return true;
	}
	return false;
}

std::vector<TilePos3> StrongholdFeature::getGuessedPositions()
{
	std::vector<TilePos3> result;
	for (int_t i = 0; i < 3; i++)
	{
		if (hasPosition[i])
			result.push_back({(posX[i] << 4) + 8, 64, (posZ[i] << 4) + 8});
	}
	return result;
}

std::unique_ptr<StructureStart> StrongholdFeature::createStructureStart(int_t x, int_t z)
{
	auto start = std::make_unique<StrongholdStart>(*level, random, x, z);
	while (!start->hasPortalRoom())
		start = std::make_unique<StrongholdStart>(*level, random, x, z);
	return start;
}

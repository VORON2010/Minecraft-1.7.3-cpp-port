#include "world/level/levelgen/structure/FortressFeature.h"
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

class FortressPieces;
class FortressPieces__FortressPiece;
class FortressPieces__BlazeSpawner;
class FortressPieces__BridgeCrossing;
class FortressPieces__BridgeEnd;
class FortressPieces__BridgeStairs;
class FortressPieces__CastleEntrance;
class FortressPieces__CorridorBalcony;
class FortressPieces__FortressPieceWeight;
class FortressPieces__LargeCrossing;
class FortressPieces__NetherwartFarm;
class FortressPieces__SmallCorridor;
class FortressPieces__SmallCorridorCrossing;
class FortressPieces__SmallCorridorLeftTurn;
class FortressPieces__SmallCorridorRightTurn;
class FortressPieces__StairsCorridor;
class FortressPieces__Start;
class FortressPieces__StraightBridge;
using FortressWeightList = std::vector<std::shared_ptr<FortressPieces__FortressPieceWeight>>;

enum : int_t { CLS_FortressPieces = 1, CLS_FortressPieces__FortressPiece = 2, CLS_FortressPieces__BlazeSpawner = 3, CLS_FortressPieces__BridgeCrossing = 4, CLS_FortressPieces__BridgeEnd = 5, CLS_FortressPieces__BridgeStairs = 6, CLS_FortressPieces__CastleEntrance = 7, CLS_FortressPieces__CorridorBalcony = 8, CLS_FortressPieces__FortressPieceWeight = 9, CLS_FortressPieces__LargeCrossing = 10, CLS_FortressPieces__NetherwartFarm = 11, CLS_FortressPieces__SmallCorridor = 12, CLS_FortressPieces__SmallCorridorCrossing = 13, CLS_FortressPieces__SmallCorridorLeftTurn = 14, CLS_FortressPieces__SmallCorridorRightTurn = 15, CLS_FortressPieces__StairsCorridor = 16, CLS_FortressPieces__Start = 17, CLS_FortressPieces__StraightBridge = 18 };

class FortressPieces
{
public:
	static std::vector<std::shared_ptr<FortressPieces__FortressPieceWeight>> &BRIDGE_PIECE_WEIGHTS();
	static std::vector<std::shared_ptr<FortressPieces__FortressPieceWeight>> &CASTLE_PIECE_WEIGHTS();
	static std::shared_ptr<FortressPieces__FortressPiece> createPiece(std::shared_ptr<FortressPieces__FortressPieceWeight> weight, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
};

class FortressPieces__FortressPiece : public StructurePiece
{
public:
	using Base = StructurePiece;
	FortressPieces__FortressPiece(int_t var1);
	virtual int_t getTotalWeight(FortressWeightList & pieceWeights);
	virtual std::shared_ptr<FortressPieces__FortressPiece> generateNextPiece(std::shared_ptr<FortressPieces__Start> startPiece, FortressWeightList & weights, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual std::shared_ptr<StructurePiece> generateNextPiece(std::shared_ptr<FortressPieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth, bool pickCastlePiece);
	virtual std::shared_ptr<StructurePiece> generatePieceForward(std::shared_ptr<FortressPieces__Start> startPiece, PieceList & pieces, Random & random, int_t offset, int_t yOffset, bool pickCastlePiece);
	virtual std::shared_ptr<StructurePiece> generatePieceLeft(std::shared_ptr<FortressPieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset, bool pickCastlePiece);
	virtual std::shared_ptr<StructurePiece> generatePieceRight(std::shared_ptr<FortressPieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset, bool pickCastlePiece);
	static bool isValidBounds(const BoundingBox & bb);
};

class FortressPieces__BlazeSpawner : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	bool shouldGenerateMobSpawner{};
	FortressPieces__BlazeSpawner(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__BlazeSpawner> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__BridgeCrossing : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__BridgeCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	FortressPieces__BridgeCrossing(Random & random, int_t chunkX, int_t chunkZ);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__BridgeCrossing> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__BridgeEnd : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	int_t seed{};
	FortressPieces__BridgeEnd(int_t generationDepth, Random & seed, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__BridgeEnd> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__BridgeStairs : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__BridgeStairs(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__BridgeStairs> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t type);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__CastleEntrance : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__CastleEntrance(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__CastleEntrance> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__CorridorBalcony : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__CorridorBalcony(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__CorridorBalcony> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__FortressPieceWeight
{
public:
	int_t type{};
	int_t weight{};
	int_t amountGenerated{};
	int_t maxAmount{};
	bool isConnectorPiece{};
	FortressPieces__FortressPieceWeight(int_t type, int_t weight, int_t maxAmount, bool isConnectorPiece);
	FortressPieces__FortressPieceWeight(int_t type, int_t weight, int_t maxAmount);
	virtual bool isValid(int_t generationDepth);
	virtual bool isValid();
};

class FortressPieces__LargeCrossing : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__LargeCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__LargeCrossing> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__NetherwartFarm : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__NetherwartFarm(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__NetherwartFarm> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__SmallCorridor : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__SmallCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__SmallCorridor> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__SmallCorridorCrossing : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__SmallCorridorCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__SmallCorridorCrossing> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__SmallCorridorLeftTurn : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__SmallCorridorLeftTurn(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__SmallCorridorLeftTurn> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__SmallCorridorRightTurn : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__SmallCorridorRightTurn(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__SmallCorridorRightTurn> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__StairsCorridor : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__StairsCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__StairsCorridor> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class FortressPieces__Start : public FortressPieces__BridgeCrossing
{
public:
	using Base = FortressPieces__BridgeCrossing;
	std::shared_ptr<FortressPieces__FortressPieceWeight> previous{};
	FortressWeightList bridgePieces;
	FortressWeightList castlePieces;
	PieceList children;
	FortressPieces__Start(Random & var1, int_t var2, int_t var3);
};

class FortressPieces__StraightBridge : public FortressPieces__FortressPiece
{
public:
	using Base = FortressPieces__FortressPiece;
	FortressPieces__StraightBridge(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	static std::shared_ptr<FortressPieces__StraightBridge> of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

std::vector<std::shared_ptr<FortressPieces__FortressPieceWeight>> &FortressPieces::BRIDGE_PIECE_WEIGHTS()
{
	static std::vector<std::shared_ptr<FortressPieces__FortressPieceWeight>> value = {
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__StraightBridge, 30, 0, true),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__BridgeCrossing, 10, 4),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__LargeCrossing, 10, 4),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__BridgeStairs, 10, 3),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__BlazeSpawner, 5, 2),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__CastleEntrance, 5, 1)
   };
	return value;
}

std::vector<std::shared_ptr<FortressPieces__FortressPieceWeight>> &FortressPieces::CASTLE_PIECE_WEIGHTS()
{
	static std::vector<std::shared_ptr<FortressPieces__FortressPieceWeight>> value = {
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__SmallCorridor, 25, 0, true),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__SmallCorridorCrossing, 15, 5),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__SmallCorridorRightTurn, 5, 10),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__SmallCorridorLeftTurn, 5, 10),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__StairsCorridor, 10, 3, true),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__CorridorBalcony, 7, 2),
      std::make_shared<FortressPieces__FortressPieceWeight>(CLS_FortressPieces__NetherwartFarm, 5, 2)
   };
	return value;
}

std::shared_ptr<FortressPieces__FortressPiece> FortressPieces::createPiece(std::shared_ptr<FortressPieces__FortressPieceWeight> weight, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      int_t var8 = weight->type;
      std::shared_ptr<StructurePiece> var9 = nullptr;
      if (var8 == CLS_FortressPieces__StraightBridge) {
         var9 = FortressPieces__StraightBridge::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__BridgeCrossing) {
         var9 = FortressPieces__BridgeCrossing::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__LargeCrossing) {
         var9 = FortressPieces__LargeCrossing::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__BridgeStairs) {
         var9 = FortressPieces__BridgeStairs::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__BlazeSpawner) {
         var9 = FortressPieces__BlazeSpawner::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__CastleEntrance) {
         var9 = FortressPieces__CastleEntrance::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__SmallCorridor) {
         var9 = FortressPieces__SmallCorridor::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__SmallCorridorRightTurn) {
         var9 = FortressPieces__SmallCorridorRightTurn::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__SmallCorridorLeftTurn) {
         var9 = FortressPieces__SmallCorridorLeftTurn::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__StairsCorridor) {
         var9 = FortressPieces__StairsCorridor::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__CorridorBalcony) {
         var9 = FortressPieces__CorridorBalcony::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__SmallCorridorCrossing) {
         var9 = FortressPieces__SmallCorridorCrossing::of(pieces, random, x, y, z, facing, generationDepth);
      } else if (var8 == CLS_FortressPieces__NetherwartFarm) {
         var9 = FortressPieces__NetherwartFarm::of(pieces, random, x, y, z, facing, generationDepth);
      }

      return std::dynamic_pointer_cast<FortressPieces__FortressPiece>(var9);
   }

FortressPieces__FortressPiece::FortressPieces__FortressPiece(int_t var1) : StructurePiece(var1)
{
   }

int_t FortressPieces__FortressPiece::getTotalWeight(FortressWeightList & pieceWeights)
{
      bool var2 = false;
      int_t var3 = 0;

      for (auto var5 : pieceWeights) {
         if (var5->maxAmount > 0 && var5->amountGenerated < var5->maxAmount) {
            var2 = true;
         }

         var3 += var5->weight;
      }

      return var2 ? var3 : -1;
   }

std::shared_ptr<FortressPieces__FortressPiece> FortressPieces__FortressPiece::generateNextPiece(std::shared_ptr<FortressPieces__Start> startPiece, FortressWeightList & weights, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      int_t var10 = this->getTotalWeight(weights);
      bool var11 = var10 > 0 && generationDepth <= 30;
      int_t var12 = 0;

      while (var12 < 5 && var11) {
         var12++;
         int_t var13 = random.nextInt(var10);

         for (auto var15 : weights) {
            var13 -= var15->weight;
            if (var13 < 0) {
               if (!var15->isValid(generationDepth) || var15 == startPiece->previous && !var15->isConnectorPiece) {
                  break;
               }

               std::shared_ptr<FortressPieces__FortressPiece> var16 = FortressPieces::createPiece(var15, pieces, random, x, y, z, facing, generationDepth);
               if (var16 != nullptr) {
                  var15->amountGenerated++;
                  startPiece->previous = var15;
                  if (!var15->isValid()) {
                     weights.erase(std::find(weights.begin(), weights.end(), var15));
                  }

                  return var16;
               }
            }
         }
      }

      return FortressPieces__BridgeEnd::of(pieces, random, x, y, z, facing, generationDepth);
   }

std::shared_ptr<StructurePiece> FortressPieces__FortressPiece::generateNextPiece(std::shared_ptr<FortressPieces__Start> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth, bool pickCastlePiece)
{
      if (std::abs(x - startPiece->getBoundingBox().x0) <= 112 && std::abs(z - startPiece->getBoundingBox().z0) <= 112) {
         FortressWeightList &var10 = pickCastlePiece ? startPiece->castlePieces : startPiece->bridgePieces;

         std::shared_ptr<FortressPieces__FortressPiece> var11 = this->generateNextPiece(startPiece, var10, pieces, random, x, y, z, facing, generationDepth + 1);
         if (var11 != nullptr) {
            pieces.push_back(var11);
            startPiece->children.push_back(var11);
         }

         return var11;
      } else {
         return FortressPieces__BridgeEnd::of(pieces, random, x, y, z, facing, generationDepth);
      }
   }

std::shared_ptr<StructurePiece> FortressPieces__FortressPiece::generatePieceForward(std::shared_ptr<FortressPieces__Start> startPiece, PieceList & pieces, Random & random, int_t offset, int_t yOffset, bool pickCastlePiece)
{
      switch (this->orientation) {
         case 0:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x0 + offset,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z1 + 1,
               this->orientation,
               this->getGenDepth(),
               pickCastlePiece
            );
         case 1:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x0 - 1,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z0 + offset,
               this->orientation,
               this->getGenDepth(),
               pickCastlePiece
            );
         case 2:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x0 + offset,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z0 - 1,
               this->orientation,
               this->getGenDepth(),
               pickCastlePiece
            );
         case 3:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x1 + 1,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z0 + offset,
               this->orientation,
               this->getGenDepth(),
               pickCastlePiece
            );
         default:
            return nullptr;
      }
   }

std::shared_ptr<StructurePiece> FortressPieces__FortressPiece::generatePieceLeft(std::shared_ptr<FortressPieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset, bool pickCastlePiece)
{
      switch (this->orientation) {
         case 0:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x0 - 1,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z0 + offset,
               1,
               this->getGenDepth(),
               pickCastlePiece
            );
         case 1:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x0 + offset,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z0 - 1,
               2,
               this->getGenDepth(),
               pickCastlePiece
            );
         case 2:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x0 - 1,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z0 + offset,
               1,
               this->getGenDepth(),
               pickCastlePiece
            );
         case 3:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x0 + offset,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z0 - 1,
               2,
               this->getGenDepth(),
               pickCastlePiece
            );
         default:
            return nullptr;
      }
   }

std::shared_ptr<StructurePiece> FortressPieces__FortressPiece::generatePieceRight(std::shared_ptr<FortressPieces__Start> startPiece, PieceList & pieces, Random & random, int_t yOffset, int_t offset, bool pickCastlePiece)
{
      switch (this->orientation) {
         case 0:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x1 + 1,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z0 + offset,
               3,
               this->getGenDepth(),
               pickCastlePiece
            );
         case 1:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x0 + offset,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z1 + 1,
               0,
               this->getGenDepth(),
               pickCastlePiece
            );
         case 2:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x1 + 1,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z0 + offset,
               3,
               this->getGenDepth(),
               pickCastlePiece
            );
         case 3:
            return this->generateNextPiece(
               startPiece,
               pieces,
               random,
               this->boundingBox.x0 + offset,
               this->boundingBox.y0 + yOffset,
               this->boundingBox.z1 + 1,
               0,
               this->getGenDepth(),
               pickCastlePiece
            );
         default:
            return nullptr;
      }
   }

bool FortressPieces__FortressPiece::isValidBounds(const BoundingBox & bb)
{
      return bb.valid && bb.y0 > 10;
   }

FortressPieces__BlazeSpawner::FortressPieces__BlazeSpawner(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__BlazeSpawner::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<FortressPieces__BlazeSpawner> FortressPieces__BlazeSpawner::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -2, 0, 0, 7, 8, 9, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__BlazeSpawner>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__BlazeSpawner::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 2, 0, 6, 7, 7, 0, 0, false);
      this->generateBox(level, chunkBB, 1, 0, 0, 5, 1, 7, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 2, 1, 5, 2, 7, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 3, 2, 5, 3, 7, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 4, 3, 5, 4, 7, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 2, 0, 1, 4, 2, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 2, 0, 5, 4, 2, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 5, 2, 1, 5, 3, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 5, 2, 5, 5, 3, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 5, 3, 0, 5, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 6, 5, 3, 6, 5, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 5, 8, 5, 5, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->placeBlock(level, Tile::netherFence.id, 0, 1, 6, 3, chunkBB);
      this->placeBlock(level, Tile::netherFence.id, 0, 5, 6, 3, chunkBB);
      this->generateBox(level, chunkBB, 0, 6, 3, 0, 6, 8, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 6, 6, 3, 6, 6, 8, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 1, 6, 8, 5, 7, 8, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 2, 8, 8, 4, 8, 8, Tile::netherFence.id, Tile::netherFence.id, false);
      if (!this->shouldGenerateMobSpawner) {
         int_t var4 = this->getWorldY(5);
         int_t var5 = this->getWorldX(3, 5);
         int_t var6 = this->getWorldZ(3, 5);
         if (chunkBB.isInside(var5, var4, var6)) {
            this->shouldGenerateMobSpawner = true;
            level.setTile(var5, var4, var6, Tile::mobSpawner.id);
            std::shared_ptr<MobSpawnerTileEntity> var7 = std::dynamic_pointer_cast<MobSpawnerTileEntity>(level.getTileEntity(var5, var4, var6));
            if (var7 != nullptr) {
               var7->setEntityId(u"Blaze");
            }
         }
      }

      for (int_t var8 = 0; var8 <= 6; var8++) {
         for (int_t var9 = 0; var9 <= 6; var9++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var8, -1, var9, chunkBB);
         }
      }

      return true;
   }

FortressPieces__BridgeCrossing::FortressPieces__BridgeCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

FortressPieces__BridgeCrossing::FortressPieces__BridgeCrossing(Random & random, int_t chunkX, int_t chunkZ) : FortressPieces__FortressPiece(0)
{
      this->orientation = random.nextInt(4);
      switch (this->orientation) {
         case 0:
         case 2:
            this->boundingBox = BoundingBox(chunkX, 64, chunkZ, chunkX + 19 - 1, 73, chunkZ + 19 - 1);
            break;
         default:
            this->boundingBox = BoundingBox(chunkX, 64, chunkZ, chunkX + 19 - 1, 73, chunkZ + 19 - 1);
      }
   }

void FortressPieces__BridgeCrossing::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 8, 3, false);
      this->generatePieceLeft(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 3, 8, false);
      this->generatePieceRight(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 3, 8, false);
   }

std::shared_ptr<FortressPieces__BridgeCrossing> FortressPieces__BridgeCrossing::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -8, -3, 0, 19, 10, 19, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__BridgeCrossing>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__BridgeCrossing::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 7, 3, 0, 11, 4, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 3, 7, 18, 4, 11, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 8, 5, 0, 10, 7, 18, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 5, 8, 18, 7, 10, 0, 0, false);
      this->generateBox(level, chunkBB, 7, 5, 0, 7, 5, 7, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 7, 5, 11, 7, 5, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 11, 5, 0, 11, 5, 7, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 11, 5, 11, 11, 5, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 5, 7, 7, 5, 7, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 11, 5, 7, 18, 5, 7, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 5, 11, 7, 5, 11, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 11, 5, 11, 18, 5, 11, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 7, 2, 0, 11, 2, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 7, 2, 13, 11, 2, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 7, 0, 0, 11, 1, 3, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 7, 0, 15, 11, 1, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var4 = 7; var4 <= 11; var4++) {
         for (int_t var5 = 0; var5 <= 2; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, var5, chunkBB);
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, 18 - var5, chunkBB);
         }
      }

      this->generateBox(level, chunkBB, 0, 2, 7, 5, 2, 11, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 13, 2, 7, 18, 2, 11, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 0, 7, 3, 1, 11, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 15, 0, 7, 18, 1, 11, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var6 = 0; var6 <= 2; var6++) {
         for (int_t var7 = 7; var7 <= 11; var7++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var6, -1, var7, chunkBB);
            this->fillColumnDown(level, Tile::netherBrick.id, 0, 18 - var6, -1, var7, chunkBB);
         }
      }

      return true;
   }

FortressPieces__BridgeEnd::FortressPieces__BridgeEnd(int_t generationDepth, Random & seed, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
      this->seed = seed.nextInt();
   }

void FortressPieces__BridgeEnd::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
   }

std::shared_ptr<FortressPieces__BridgeEnd> FortressPieces__BridgeEnd::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -3, 0, 5, 10, 8, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__BridgeEnd>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__BridgeEnd::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      Random var4((long_t)this->seed);

      for (int_t var5 = 0; var5 <= 4; var5++) {
         for (int_t var6 = 3; var6 <= 4; var6++) {
            int_t var7 = var4.nextInt(8);
            this->generateBox(level, chunkBB, var5, var6, 0, var5, var6, var7, Tile::netherBrick.id, Tile::netherBrick.id, false);
         }
      }

      int_t var8 = var4.nextInt(8);
      this->generateBox(level, chunkBB, 0, 5, 0, 0, 5, var8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      var8 = var4.nextInt(8);
      this->generateBox(level, chunkBB, 4, 5, 0, 4, 5, var8, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var10 = 0; var10 <= 4; var10++) {
         int_t var12 = var4.nextInt(5);
         this->generateBox(level, chunkBB, var10, 2, 0, var10, 2, var12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      }

      for (int_t var11 = 0; var11 <= 4; var11++) {
         for (int_t var13 = 0; var13 <= 1; var13++) {
            int_t var14 = var4.nextInt(3);
            this->generateBox(level, chunkBB, var11, var13, 0, var11, var13, var14, Tile::netherBrick.id, Tile::netherBrick.id, false);
         }
      }

      return true;
   }

FortressPieces__BridgeStairs::FortressPieces__BridgeStairs(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__BridgeStairs::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceRight(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 6, 2, false);
   }

std::shared_ptr<FortressPieces__BridgeStairs> FortressPieces__BridgeStairs::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t type)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -2, 0, 0, 7, 11, 7, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__BridgeStairs>(type, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__BridgeStairs::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 0, 0, 6, 1, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 6, 10, 6, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 1, 8, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 2, 0, 6, 8, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 1, 0, 8, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 6, 2, 1, 6, 8, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 2, 6, 5, 8, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 3, 2, 0, 5, 4, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 6, 3, 2, 6, 5, 2, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 6, 3, 4, 6, 5, 4, Tile::netherFence.id, Tile::netherFence.id, false);
      this->placeBlock(level, Tile::netherBrick.id, 0, 5, 2, 5, chunkBB);
      this->generateBox(level, chunkBB, 4, 2, 5, 4, 3, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 3, 2, 5, 3, 4, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 2, 5, 2, 5, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 2, 5, 1, 6, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 7, 1, 5, 7, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 6, 8, 2, 6, 8, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 2, 6, 0, 4, 8, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 5, 0, 4, 5, 0, Tile::netherFence.id, Tile::netherFence.id, false);

      for (int_t var4 = 0; var4 <= 6; var4++) {
         for (int_t var5 = 0; var5 <= 6; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, var5, chunkBB);
         }
      }

      return true;
   }

FortressPieces__CastleEntrance::FortressPieces__CastleEntrance(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__CastleEntrance::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 5, 3, true);
   }

std::shared_ptr<FortressPieces__CastleEntrance> FortressPieces__CastleEntrance::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -5, -3, 0, 13, 14, 13, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__CastleEntrance>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__CastleEntrance::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 3, 0, 12, 4, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 5, 0, 12, 13, 12, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 5, 0, 1, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 11, 5, 0, 12, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 5, 11, 4, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 8, 5, 11, 10, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 9, 11, 7, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 5, 0, 4, 12, 1, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 8, 5, 0, 10, 12, 1, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 9, 0, 7, 12, 1, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 11, 2, 10, 12, 10, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 8, 0, 7, 8, 0, Tile::netherFence.id, Tile::netherFence.id, false);

      for (int_t var4 = 1; var4 <= 11; var4 += 2) {
         this->generateBox(level, chunkBB, var4, 10, 0, var4, 11, 0, Tile::netherFence.id, Tile::netherFence.id, false);
         this->generateBox(level, chunkBB, var4, 10, 12, var4, 11, 12, Tile::netherFence.id, Tile::netherFence.id, false);
         this->generateBox(level, chunkBB, 0, 10, var4, 0, 11, var4, Tile::netherFence.id, Tile::netherFence.id, false);
         this->generateBox(level, chunkBB, 12, 10, var4, 12, 11, var4, Tile::netherFence.id, Tile::netherFence.id, false);
         this->placeBlock(level, Tile::netherBrick.id, 0, var4, 13, 0, chunkBB);
         this->placeBlock(level, Tile::netherBrick.id, 0, var4, 13, 12, chunkBB);
         this->placeBlock(level, Tile::netherBrick.id, 0, 0, 13, var4, chunkBB);
         this->placeBlock(level, Tile::netherBrick.id, 0, 12, 13, var4, chunkBB);
         this->placeBlock(level, Tile::netherFence.id, 0, var4 + 1, 13, 0, chunkBB);
         this->placeBlock(level, Tile::netherFence.id, 0, var4 + 1, 13, 12, chunkBB);
         this->placeBlock(level, Tile::netherFence.id, 0, 0, 13, var4 + 1, chunkBB);
         this->placeBlock(level, Tile::netherFence.id, 0, 12, 13, var4 + 1, chunkBB);
      }

      this->placeBlock(level, Tile::netherFence.id, 0, 0, 13, 0, chunkBB);
      this->placeBlock(level, Tile::netherFence.id, 0, 0, 13, 12, chunkBB);
      this->placeBlock(level, Tile::netherFence.id, 0, 0, 13, 0, chunkBB);
      this->placeBlock(level, Tile::netherFence.id, 0, 12, 13, 0, chunkBB);

      for (int_t var7 = 3; var7 <= 9; var7 += 2) {
         this->generateBox(level, chunkBB, 1, 7, var7, 1, 8, var7, Tile::netherFence.id, Tile::netherFence.id, false);
         this->generateBox(level, chunkBB, 11, 7, var7, 11, 8, var7, Tile::netherFence.id, Tile::netherFence.id, false);
      }

      this->generateBox(level, chunkBB, 4, 2, 0, 8, 2, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 4, 12, 2, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 4, 0, 0, 8, 1, 3, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 4, 0, 9, 8, 1, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 0, 4, 3, 1, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 9, 0, 4, 12, 1, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var8 = 4; var8 <= 8; var8++) {
         for (int_t var5 = 0; var5 <= 2; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var8, -1, var5, chunkBB);
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var8, -1, 12 - var5, chunkBB);
         }
      }

      for (int_t var9 = 0; var9 <= 2; var9++) {
         for (int_t var11 = 4; var11 <= 8; var11++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var9, -1, var11, chunkBB);
            this->fillColumnDown(level, Tile::netherBrick.id, 0, 12 - var9, -1, var11, chunkBB);
         }
      }

      this->generateBox(level, chunkBB, 5, 5, 5, 7, 5, 7, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 6, 1, 6, 6, 4, 6, 0, 0, false);
      this->placeBlock(level, Tile::netherBrick.id, 0, 6, 0, 6, chunkBB);
      this->placeBlock(level, Tile::lava.id, 0, 6, 5, 6, chunkBB);
      int_t var10 = this->getWorldX(6, 6);
      int_t var12 = this->getWorldY(5);
      int_t var6 = this->getWorldZ(6, 6);
      if (chunkBB.isInside(var10, var12, var6)) {
         level.instaTick = true;
         Tile::tiles[Tile::lava.id]->tick(level, var10, var12, var6, random);
         level.instaTick = false;
      }

      return true;
   }

FortressPieces__CorridorBalcony::FortressPieces__CorridorBalcony(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__CorridorBalcony::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      int_t var4 = 1;
      if (this->orientation == 1 || this->orientation == 2) {
         var4 = 5;
      }

      this->generatePieceLeft(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 0, var4, random.nextInt(8) > 0);
      this->generatePieceRight(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 0, var4, random.nextInt(8) > 0);
   }

std::shared_ptr<FortressPieces__CorridorBalcony> FortressPieces__CorridorBalcony::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -3, 0, 0, 9, 7, 9, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__CorridorBalcony>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__CorridorBalcony::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 0, 0, 8, 1, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 8, 5, 8, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 6, 0, 8, 6, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 2, 5, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 6, 2, 0, 8, 5, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 3, 0, 1, 4, 0, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 7, 3, 0, 7, 4, 0, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 2, 4, 8, 2, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 1, 4, 2, 2, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 6, 1, 4, 7, 2, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 3, 8, 8, 3, 8, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 3, 6, 0, 3, 7, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 8, 3, 6, 8, 3, 7, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 3, 4, 0, 5, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 8, 3, 4, 8, 5, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 3, 5, 2, 5, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 6, 3, 5, 7, 5, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 4, 5, 1, 5, 5, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 7, 4, 5, 7, 5, 5, Tile::netherFence.id, Tile::netherFence.id, false);

      for (int_t var4 = 0; var4 <= 5; var4++) {
         for (int_t var5 = 0; var5 <= 8; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var5, -1, var4, chunkBB);
         }
      }

      return true;
   }

FortressPieces__FortressPieceWeight::FortressPieces__FortressPieceWeight(int_t type, int_t weight, int_t maxAmount, bool isConnectorPiece)
{
      this->type = type;
      this->weight = weight;
      this->maxAmount = maxAmount;
      this->isConnectorPiece = isConnectorPiece;
   }

FortressPieces__FortressPieceWeight::FortressPieces__FortressPieceWeight(int_t type, int_t weight, int_t maxAmount)
{
      this->type = type;
      this->weight = weight;
      this->maxAmount = maxAmount;
      this->isConnectorPiece = false;
   }

bool FortressPieces__FortressPieceWeight::isValid(int_t generationDepth)
{
      return this->maxAmount == 0 || this->amountGenerated < this->maxAmount;
   }

bool FortressPieces__FortressPieceWeight::isValid()
{
      return this->maxAmount == 0 || this->amountGenerated < this->maxAmount;
   }

FortressPieces__LargeCrossing::FortressPieces__LargeCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__LargeCrossing::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 2, 0, false);
      this->generatePieceLeft(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 0, 2, false);
      this->generatePieceRight(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 0, 2, false);
   }

std::shared_ptr<FortressPieces__LargeCrossing> FortressPieces__LargeCrossing::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -2, 0, 0, 7, 9, 7, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__LargeCrossing>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__LargeCrossing::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 0, 0, 6, 1, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 6, 7, 6, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 1, 6, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 6, 1, 6, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 2, 0, 6, 6, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 2, 6, 6, 6, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 0, 6, 1, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 5, 0, 6, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 6, 2, 0, 6, 6, 1, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 6, 2, 5, 6, 6, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 6, 0, 4, 6, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 5, 0, 4, 5, 0, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 2, 6, 6, 4, 6, 6, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 5, 6, 4, 5, 6, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 6, 2, 0, 6, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 5, 2, 0, 5, 4, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 6, 6, 2, 6, 6, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 6, 5, 2, 6, 5, 4, Tile::netherFence.id, Tile::netherFence.id, false);

      for (int_t var4 = 0; var4 <= 6; var4++) {
         for (int_t var5 = 0; var5 <= 6; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, var5, chunkBB);
         }
      }

      return true;
   }

FortressPieces__NetherwartFarm::FortressPieces__NetherwartFarm(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__NetherwartFarm::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 5, 3, true);
      this->generatePieceForward(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 5, 11, true);
   }

std::shared_ptr<FortressPieces__NetherwartFarm> FortressPieces__NetherwartFarm::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -5, -3, 0, 13, 14, 13, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__NetherwartFarm>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__NetherwartFarm::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 3, 0, 12, 4, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 5, 0, 12, 13, 12, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 5, 0, 1, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 11, 5, 0, 12, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 5, 11, 4, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 8, 5, 11, 10, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 9, 11, 7, 12, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 5, 0, 4, 12, 1, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 8, 5, 0, 10, 12, 1, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 5, 9, 0, 7, 12, 1, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 11, 2, 10, 12, 10, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var4 = 1; var4 <= 11; var4 += 2) {
         this->generateBox(level, chunkBB, var4, 10, 0, var4, 11, 0, Tile::netherFence.id, Tile::netherFence.id, false);
         this->generateBox(level, chunkBB, var4, 10, 12, var4, 11, 12, Tile::netherFence.id, Tile::netherFence.id, false);
         this->generateBox(level, chunkBB, 0, 10, var4, 0, 11, var4, Tile::netherFence.id, Tile::netherFence.id, false);
         this->generateBox(level, chunkBB, 12, 10, var4, 12, 11, var4, Tile::netherFence.id, Tile::netherFence.id, false);
         this->placeBlock(level, Tile::netherBrick.id, 0, var4, 13, 0, chunkBB);
         this->placeBlock(level, Tile::netherBrick.id, 0, var4, 13, 12, chunkBB);
         this->placeBlock(level, Tile::netherBrick.id, 0, 0, 13, var4, chunkBB);
         this->placeBlock(level, Tile::netherBrick.id, 0, 12, 13, var4, chunkBB);
         this->placeBlock(level, Tile::netherFence.id, 0, var4 + 1, 13, 0, chunkBB);
         this->placeBlock(level, Tile::netherFence.id, 0, var4 + 1, 13, 12, chunkBB);
         this->placeBlock(level, Tile::netherFence.id, 0, 0, 13, var4 + 1, chunkBB);
         this->placeBlock(level, Tile::netherFence.id, 0, 12, 13, var4 + 1, chunkBB);
      }

      this->placeBlock(level, Tile::netherFence.id, 0, 0, 13, 0, chunkBB);
      this->placeBlock(level, Tile::netherFence.id, 0, 0, 13, 12, chunkBB);
      this->placeBlock(level, Tile::netherFence.id, 0, 0, 13, 0, chunkBB);
      this->placeBlock(level, Tile::netherFence.id, 0, 12, 13, 0, chunkBB);

      for (int_t var9 = 3; var9 <= 9; var9 += 2) {
         this->generateBox(level, chunkBB, 1, 7, var9, 1, 8, var9, Tile::netherFence.id, Tile::netherFence.id, false);
         this->generateBox(level, chunkBB, 11, 7, var9, 11, 8, var9, Tile::netherFence.id, Tile::netherFence.id, false);
      }

      int_t var10 = this->getOrientationData(Tile::stairsNetherBrick.id, 3);

      for (int_t var5 = 0; var5 <= 6; var5++) {
         int_t var6 = var5 + 4;

         for (int_t var7 = 5; var7 <= 7; var7++) {
            this->placeBlock(level, Tile::stairsNetherBrick.id, var10, var7, 5 + var5, var6, chunkBB);
         }

         if (var6 >= 5 && var6 <= 8) {
            this->generateBox(level, chunkBB, 5, 5, var6, 7, var5 + 4, var6, Tile::netherBrick.id, Tile::netherBrick.id, false);
         } else if (var6 >= 9 && var6 <= 10) {
            this->generateBox(level, chunkBB, 5, 8, var6, 7, var5 + 4, var6, Tile::netherBrick.id, Tile::netherBrick.id, false);
         }

         if (var5 >= 1) {
            this->generateBox(level, chunkBB, 5, 6 + var5, var6, 7, 9 + var5, var6, 0, 0, false);
         }
      }

      for (int_t var11 = 5; var11 <= 7; var11++) {
         this->placeBlock(level, Tile::stairsNetherBrick.id, var10, var11, 12, 11, chunkBB);
      }

      this->generateBox(level, chunkBB, 5, 6, 7, 5, 7, 7, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 7, 6, 7, 7, 7, 7, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 5, 13, 12, 7, 13, 12, 0, 0, false);
      this->generateBox(level, chunkBB, 2, 5, 2, 3, 5, 3, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 5, 9, 3, 5, 10, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 2, 5, 4, 2, 5, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 9, 5, 2, 10, 5, 3, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 9, 5, 9, 10, 5, 10, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 10, 5, 4, 10, 5, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      int_t var12 = this->getOrientationData(Tile::stairsNetherBrick.id, 0);
      int_t var13 = this->getOrientationData(Tile::stairsNetherBrick.id, 1);
      this->placeBlock(level, Tile::stairsNetherBrick.id, var13, 4, 5, 2, chunkBB);
      this->placeBlock(level, Tile::stairsNetherBrick.id, var13, 4, 5, 3, chunkBB);
      this->placeBlock(level, Tile::stairsNetherBrick.id, var13, 4, 5, 9, chunkBB);
      this->placeBlock(level, Tile::stairsNetherBrick.id, var13, 4, 5, 10, chunkBB);
      this->placeBlock(level, Tile::stairsNetherBrick.id, var12, 8, 5, 2, chunkBB);
      this->placeBlock(level, Tile::stairsNetherBrick.id, var12, 8, 5, 3, chunkBB);
      this->placeBlock(level, Tile::stairsNetherBrick.id, var12, 8, 5, 9, chunkBB);
      this->placeBlock(level, Tile::stairsNetherBrick.id, var12, 8, 5, 10, chunkBB);
      this->generateBox(level, chunkBB, 3, 4, 4, 4, 4, 8, Tile::soulSand.id, Tile::soulSand.id, false);
      this->generateBox(level, chunkBB, 8, 4, 4, 9, 4, 8, Tile::soulSand.id, Tile::soulSand.id, false);
      this->generateBox(level, chunkBB, 3, 5, 4, 4, 5, 8, Tile::netherWart.id, Tile::netherWart.id, false);
      this->generateBox(level, chunkBB, 8, 5, 4, 9, 5, 8, Tile::netherWart.id, Tile::netherWart.id, false);
      this->generateBox(level, chunkBB, 4, 2, 0, 8, 2, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 4, 12, 2, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 4, 0, 0, 8, 1, 3, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 4, 0, 9, 8, 1, 12, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 0, 4, 3, 1, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 9, 0, 4, 12, 1, 8, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var14 = 4; var14 <= 8; var14++) {
         for (int_t var8 = 0; var8 <= 2; var8++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var14, -1, var8, chunkBB);
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var14, -1, 12 - var8, chunkBB);
         }
      }

      for (int_t var15 = 0; var15 <= 2; var15++) {
         for (int_t var16 = 4; var16 <= 8; var16++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var15, -1, var16, chunkBB);
            this->fillColumnDown(level, Tile::netherBrick.id, 0, 12 - var15, -1, var16, chunkBB);
         }
      }

      return true;
   }

FortressPieces__SmallCorridor::FortressPieces__SmallCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__SmallCorridor::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 1, 0, true);
   }

std::shared_ptr<FortressPieces__SmallCorridor> FortressPieces__SmallCorridor::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, 0, 0, 5, 7, 5, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__SmallCorridor>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__SmallCorridor::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 0, 0, 4, 1, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 4, 5, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 0, 5, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 4, 2, 0, 4, 5, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 3, 1, 0, 4, 1, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 3, 3, 0, 4, 3, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 4, 3, 1, 4, 4, 1, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 4, 3, 3, 4, 4, 3, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 6, 0, 4, 6, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var4 = 0; var4 <= 4; var4++) {
         for (int_t var5 = 0; var5 <= 4; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, var5, chunkBB);
         }
      }

      return true;
   }

FortressPieces__SmallCorridorCrossing::FortressPieces__SmallCorridorCrossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__SmallCorridorCrossing::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 1, 0, true);
      this->generatePieceLeft(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 0, 1, true);
      this->generatePieceRight(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 0, 1, true);
   }

std::shared_ptr<FortressPieces__SmallCorridorCrossing> FortressPieces__SmallCorridorCrossing::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, 0, 0, 5, 7, 5, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__SmallCorridorCrossing>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__SmallCorridorCrossing::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 0, 0, 4, 1, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 4, 5, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 0, 5, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 4, 2, 0, 4, 5, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 4, 0, 5, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 4, 2, 4, 4, 5, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 6, 0, 4, 6, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var4 = 0; var4 <= 4; var4++) {
         for (int_t var5 = 0; var5 <= 4; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, var5, chunkBB);
         }
      }

      return true;
   }

FortressPieces__SmallCorridorLeftTurn::FortressPieces__SmallCorridorLeftTurn(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__SmallCorridorLeftTurn::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceLeft(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 0, 1, true);
   }

std::shared_ptr<FortressPieces__SmallCorridorLeftTurn> FortressPieces__SmallCorridorLeftTurn::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, 0, 0, 5, 7, 5, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__SmallCorridorLeftTurn>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__SmallCorridorLeftTurn::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 0, 0, 4, 1, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 4, 5, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 4, 2, 0, 4, 5, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 4, 3, 1, 4, 4, 1, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 4, 3, 3, 4, 4, 3, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 0, 5, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 4, 3, 5, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 3, 4, 1, 4, 4, Tile::netherFence.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 3, 3, 4, 3, 4, 4, Tile::netherFence.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 6, 0, 4, 6, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var4 = 0; var4 <= 4; var4++) {
         for (int_t var5 = 0; var5 <= 4; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, var5, chunkBB);
         }
      }

      return true;
   }

FortressPieces__SmallCorridorRightTurn::FortressPieces__SmallCorridorRightTurn(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__SmallCorridorRightTurn::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceRight(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 0, 1, true);
   }

std::shared_ptr<FortressPieces__SmallCorridorRightTurn> FortressPieces__SmallCorridorRightTurn::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, 0, 0, 5, 7, 5, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__SmallCorridorRightTurn>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__SmallCorridorRightTurn::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 0, 0, 4, 1, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 4, 5, 4, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 0, 5, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 3, 1, 0, 4, 1, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 3, 3, 0, 4, 3, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 4, 2, 0, 4, 5, 0, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 2, 4, 4, 5, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 3, 4, 1, 4, 4, Tile::netherFence.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 3, 3, 4, 3, 4, 4, Tile::netherFence.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 6, 0, 4, 6, 4, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var4 = 0; var4 <= 4; var4++) {
         for (int_t var5 = 0; var5 <= 4; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, var5, chunkBB);
         }
      }

      return true;
   }

FortressPieces__StairsCorridor::FortressPieces__StairsCorridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__StairsCorridor::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 1, 0, true);
   }

std::shared_ptr<FortressPieces__StairsCorridor> FortressPieces__StairsCorridor::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -7, 0, 5, 14, 10, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__StairsCorridor>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__StairsCorridor::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      int_t var4 = this->getOrientationData(Tile::stairsNetherBrick.id, 2);

      for (int_t var5 = 0; var5 <= 9; var5++) {
         int_t var6 = std::max(1, 7 - var5);
         int_t var7 = std::min(std::max(var6 + 5, 14 - var5), 13);
         int_t var8 = var5;
         this->generateBox(level, chunkBB, 0, 0, var5, 4, var6, var5, Tile::netherBrick.id, Tile::netherBrick.id, false);
         this->generateBox(level, chunkBB, 1, var6 + 1, var5, 3, var7 - 1, var5, 0, 0, false);
         if (var5 <= 6) {
            this->placeBlock(level, Tile::stairsNetherBrick.id, var4, 1, var6 + 1, var5, chunkBB);
            this->placeBlock(level, Tile::stairsNetherBrick.id, var4, 2, var6 + 1, var5, chunkBB);
            this->placeBlock(level, Tile::stairsNetherBrick.id, var4, 3, var6 + 1, var5, chunkBB);
         }

         this->generateBox(level, chunkBB, 0, var7, var5, 4, var7, var5, Tile::netherBrick.id, Tile::netherBrick.id, false);
         this->generateBox(level, chunkBB, 0, var6 + 1, var5, 0, var7 - 1, var5, Tile::netherBrick.id, Tile::netherBrick.id, false);
         this->generateBox(level, chunkBB, 4, var6 + 1, var5, 4, var7 - 1, var5, Tile::netherBrick.id, Tile::netherBrick.id, false);
         if ((var5 & 1) == 0) {
            this->generateBox(level, chunkBB, 0, var6 + 2, var5, 0, var6 + 3, var5, Tile::netherFence.id, Tile::netherFence.id, false);
            this->generateBox(level, chunkBB, 4, var6 + 2, var5, 4, var6 + 3, var5, Tile::netherFence.id, Tile::netherFence.id, false);
         }

         for (int_t var9 = 0; var9 <= 4; var9++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var9, -1, var8, chunkBB);
         }
      }

      return true;
   }

FortressPieces__Start::FortressPieces__Start(Random & var1, int_t var2, int_t var3) : FortressPieces__BridgeCrossing(var1, var2, var3)
{
      this->bridgePieces.clear();

      for (auto var7 : FortressPieces::BRIDGE_PIECE_WEIGHTS()) {
         var7->amountGenerated = 0;
         this->bridgePieces.push_back(var7);
      }

      this->castlePieces.clear();

      for (auto var11 : FortressPieces::CASTLE_PIECE_WEIGHTS()) {
         var11->amountGenerated = 0;
         this->castlePieces.push_back(var11);
      }
   }

FortressPieces__StraightBridge::FortressPieces__StraightBridge(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : FortressPieces__FortressPiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

void FortressPieces__StraightBridge::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      this->generatePieceForward(std::dynamic_pointer_cast<FortressPieces__Start>(start), pieces, random, 1, 3, false);
   }

std::shared_ptr<FortressPieces__StraightBridge> FortressPieces__StraightBridge::of(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      BoundingBox var7 = BoundingBox::orientBox(x, y, z, -1, -3, 0, 5, 10, 19, facing);
      return isValidBounds(var7) && StructurePiece::findCollisionPiece(pieces, var7) == nullptr
         ? std::make_shared<FortressPieces__StraightBridge>(generationDepth, random, var7, facing)
         : nullptr;
   }

bool FortressPieces__StraightBridge::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      this->generateBox(level, chunkBB, 0, 3, 0, 4, 4, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 1, 5, 0, 3, 7, 18, 0, 0, false);
      this->generateBox(level, chunkBB, 0, 5, 0, 0, 5, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 4, 5, 0, 4, 5, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 0, 4, 2, 5, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 2, 13, 4, 2, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 0, 0, 4, 1, 3, Tile::netherBrick.id, Tile::netherBrick.id, false);
      this->generateBox(level, chunkBB, 0, 0, 15, 4, 1, 18, Tile::netherBrick.id, Tile::netherBrick.id, false);

      for (int_t var4 = 0; var4 <= 4; var4++) {
         for (int_t var5 = 0; var5 <= 2; var5++) {
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, var5, chunkBB);
            this->fillColumnDown(level, Tile::netherBrick.id, 0, var4, -1, 18 - var5, chunkBB);
         }
      }

      this->generateBox(level, chunkBB, 0, 1, 1, 0, 4, 1, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 3, 4, 0, 4, 4, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 3, 14, 0, 4, 14, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 0, 1, 17, 0, 4, 17, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 4, 1, 1, 4, 4, 1, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 4, 3, 4, 4, 4, 4, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 4, 3, 14, 4, 4, 14, Tile::netherFence.id, Tile::netherFence.id, false);
      this->generateBox(level, chunkBB, 4, 1, 17, 4, 4, 17, Tile::netherFence.id, Tile::netherFence.id, false);
      return true;
   }

#include "world/entity/monster/Blaze.h"
#include "world/entity/monster/LavaSlime.h"
#include "world/entity/monster/PigZombie.h"

class FortressStart : public StructureStart
{
public:
	FortressStart(Level &level, Random &random, int_t chunkX, int_t chunkZ)
	{
		auto start = std::make_shared<FortressPieces__Start>(random, (chunkX << 4) + 2, (chunkZ << 4) + 2);
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
		moveInsideHeights(level, random, 48, 70);
	}
};

FortressFeature::FortressFeature()
{
	monsters.push_back({[](Level &level) -> std::shared_ptr<Mob> { return std::make_shared<Blaze>(level); }, 10, 2, 3});
	monsters.push_back({[](Level &level) -> std::shared_ptr<Mob> { return std::make_shared<PigZombie>(level); }, 10, 4, 4});
	monsters.push_back({[](Level &level) -> std::shared_ptr<Mob> { return std::make_shared<LavaSlime>(level); }, 3, 4, 4});
}

bool FortressFeature::isFeatureChunk(int_t x, int_t z)
{
	int_t rx = x >> 4;
	int_t rz = z >> 4;
	random.setSeed(static_cast<long_t>(rx ^ (rz << 4)) ^ level->seed);
	random.nextInt();
	if (random.nextInt(3) != 0)
		return false;
	if (x != (rx << 4) + 4 + random.nextInt(8))
		return false;
	return z == (rz << 4) + 4 + random.nextInt(8);
}

std::unique_ptr<StructureStart> FortressFeature::createStructureStart(int_t x, int_t z)
{
	return std::make_unique<FortressStart>(*level, random, x, z);
}

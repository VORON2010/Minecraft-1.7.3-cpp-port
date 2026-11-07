#pragma once

#include <array>
#include <climits>
#include <list>
#include <memory>
#include <vector>

#include "java/Random.h"
#include "java/Type.h"

class Level;
class Tile;

// Structure framework from b1.8 / 1.0 (StructureBoundingBox, StructureComponent, StructureStart, MapGenStructure).

struct BoundingBox
{
	int_t x0 = 0, y0 = 0, z0 = 0;
	int_t x1 = 0, y1 = 0, z1 = 0;
	bool valid = true; // false stands in for Java's null box

	BoundingBox() = default;
	BoundingBox(int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1) : x0(x0), y0(y0), z0(z0), x1(x1), y1(y1), z1(z1) {}
	BoundingBox(int_t x0, int_t z0, int_t x1, int_t z1) : x0(x0), y0(0), z0(z0), x1(x1), y1(65536), z1(z1) {}

	static BoundingBox none() { BoundingBox b; b.valid = false; return b; }
	static BoundingBox getUnknownBox() { return BoundingBox(INT_MAX, INT_MAX, INT_MAX, INT_MIN, INT_MIN, INT_MIN); }
	static BoundingBox orientBox(int_t x, int_t y, int_t z, int_t offX, int_t offY, int_t offZ, int_t width, int_t height, int_t depth, int_t orientation);

	explicit operator bool() const { return valid; }

	bool intersects(const BoundingBox &b) const
	{
		return x1 >= b.x0 && x0 <= b.x1 && z1 >= b.z0 && z0 <= b.z1 && y1 >= b.y0 && y0 <= b.y1;
	}
	bool intersects(int_t minX, int_t minZ, int_t maxX, int_t maxZ) const
	{
		return x1 >= minX && x0 <= maxX && z1 >= minZ && z0 <= maxZ;
	}
	void expand(const BoundingBox &b);
	void move(int_t dx, int_t dy, int_t dz) { x0 += dx; y0 += dy; z0 += dz; x1 += dx; y1 += dy; z1 += dz; }
	bool isInside(int_t x, int_t y, int_t z) const { return x >= x0 && x <= x1 && z >= z0 && z <= z1 && y >= y0 && y <= y1; }
	int_t getXSpan() const { return x1 - x0 + 1; }
	int_t getYSpan() const { return y1 - y0 + 1; }
	int_t getZSpan() const { return z1 - z0 + 1; }
	int_t getXCenter() const { return x0 + (x1 - x0 + 1) / 2; }
	int_t getYCenter() const { return y0 + (y1 - y0 + 1) / 2; }
	int_t getZCenter() const { return z0 + (z1 - z0 + 1) / 2; }
};

struct TilePos3
{
	int_t x = 0, y = 0, z = 0;
};

// weighted chest loot
struct WeighedTreasureItem
{
	int_t itemId;
	int_t auxValue;
	int_t minCount;
	int_t maxCount;
	int_t weight;
};

class StructurePiece;
using PieceList = std::vector<std::shared_ptr<StructurePiece>>;

class BlockSelector
{
protected:
	int_t nextId = 0;
	int_t nextData = 0;

public:
	virtual ~BlockSelector() {}
	virtual void next(Random &random, int_t x, int_t y, int_t z, bool wall) = 0;
	int_t getNextId() const { return nextId; }
	int_t getNextData() const { return nextData; }
};

class StructurePiece : public std::enable_shared_from_this<StructurePiece>
{
public:
	BoundingBox boundingBox;
	int_t orientation = -1;
	int_t genDepth = 0;

protected:
	explicit StructurePiece(int_t genDepth) : genDepth(genDepth) {}

public:
	virtual ~StructurePiece() {}

	virtual void addChildren(std::shared_ptr<StructurePiece> startPiece, PieceList &pieces, Random &random) { (void)startPiece; (void)pieces; (void)random; }
	virtual bool postProcess(Level &level, Random &random, const BoundingBox &chunkBB) = 0;

	BoundingBox &getBoundingBox() { return boundingBox; }
	const BoundingBox &getBoundingBox() const { return boundingBox; }
	int_t getGenDepth() const { return genDepth; }

	static std::shared_ptr<StructurePiece> findCollisionPiece(PieceList &pieces, const BoundingBox &box);
	virtual TilePos3 getLocatorPosition() const { return {boundingBox.getXCenter(), boundingBox.getYCenter(), boundingBox.getZCenter()}; }

protected:
	bool edgesLiquid(Level &level, const BoundingBox &chunkBB);
	int_t getWorldX(int_t x, int_t z) const;
	int_t getWorldY(int_t y) const;
	int_t getWorldZ(int_t x, int_t z) const;
	int_t getOrientationData(int_t tile, int_t data) const;

	void placeBlock(Level &level, int_t block, int_t data, int_t x, int_t y, int_t z, const BoundingBox &chunkBB);
	int_t getBlock(Level &level, int_t x, int_t y, int_t z, const BoundingBox &chunkBB);
	void generateBox(Level &level, const BoundingBox &chunkBB, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, int_t edgeTile, int_t fillTile, bool skipAir);
	void generateBox(Level &level, const BoundingBox &chunkBB, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, bool skipAir, Random &random, BlockSelector &selector);
	void generateMaybeBox(Level &level, const BoundingBox &chunkBB, Random &random, float probability, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, int_t edgeTile, int_t fillTile, bool skipAir);
	void maybeGenerateBlock(Level &level, const BoundingBox &chunkBB, Random &random, float probability, int_t x, int_t y, int_t z, int_t tile, int_t data);
	void generateUpperHalfSphere(Level &level, const BoundingBox &chunkBB, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, int_t fillTile, bool skipAir);
	void generateAirColumnUp(Level &level, int_t x, int_t y, int_t z, const BoundingBox &chunkBB);
	void fillColumnDown(Level &level, int_t tile, int_t data, int_t x, int_t y, int_t z, const BoundingBox &chunkBB);
	void createChest(Level &level, const BoundingBox &chunkBB, Random &random, int_t x, int_t y, int_t z, const std::vector<WeighedTreasureItem> &treasure, int_t numRolls);
	void createDoor(Level &level, const BoundingBox &chunkBB, Random &random, int_t x, int_t y, int_t z, int_t orientation);

public:
	static void addChestLoot(Random &random, const std::vector<WeighedTreasureItem> &treasure, class ChestTileEntity &chest, int_t numRolls);
};

class StructureStart
{
protected:
	PieceList pieces;
	BoundingBox boundingBox;

public:
	virtual ~StructureStart() {}

	BoundingBox &getBoundingBox() { return boundingBox; }
	PieceList &getPieces() { return pieces; }
	void postProcess(Level &level, Random &random, const BoundingBox &chunkBB);
	virtual bool isValid() { return true; }

protected:
	void calculateBoundingBox();
	void moveBelowSeaLevel(Level &level, Random &random, int_t offset);
	void moveInsideHeights(Level &level, Random &random, int_t lowestAllowed, int_t highestAllowed);
};

// MapGenStructure: decides where structures start and builds them chunk by chunk
class StructureFeature
{
protected:
	int_t radius = 8;
	Random random;
	Level *level = nullptr;
	std::vector<std::pair<long_t, std::unique_ptr<StructureStart>>> cachedStructures;

	bool hasStructureAt(long_t key) const;
	virtual bool isFeatureChunk(int_t x, int_t z) = 0;
	virtual std::unique_ptr<StructureStart> createStructureStart(int_t x, int_t z) = 0;
	virtual std::vector<TilePos3> getGuessedPositions() { return {}; }
	void addFeature(Level &level, int_t startX, int_t startZ);

public:
	virtual ~StructureFeature() {}

	void apply(class ChunkSource &source, Level &level, int_t x, int_t z, std::array<ubyte_t, 16 * 16 * 128> &blocks);
	bool postProcess(Level &level, Random &random, int_t chunkX, int_t chunkZ);
	bool isIntersection(int_t x, int_t y, int_t z);
	bool findNearest(Level &level, int_t x, int_t y, int_t z, int_t &outX, int_t &outY, int_t &outZ);
};

// WeightedRandom.getRandomItem for treasure tables
const WeighedTreasureItem &pickWeighted(Random &random, const std::vector<WeighedTreasureItem> &items);

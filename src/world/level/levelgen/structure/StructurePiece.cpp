#include "world/level/levelgen/structure/StructurePiece.h"

#include <algorithm>
#include <limits>

#include "java/Number.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/chunk/ChunkSource.h"
#include "world/level/material/Material.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/ChestTile.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/RailTile.h"
#include "world/level/tile/StairTile.h"
#include "world/level/tile/LadderTile.h"
#include "world/level/tile/ButtonTile.h"
#include "world/level/tile/entity/ChestTileEntity.h"

BoundingBox BoundingBox::orientBox(int_t x, int_t y, int_t z, int_t offX, int_t offY, int_t offZ, int_t width, int_t height, int_t depth, int_t orientation)
{
	switch (orientation)
	{
	case 0:
		return BoundingBox(x + offX, y + offY, z + offZ, x + width - 1 + offX, y + height - 1 + offY, z + depth - 1 + offZ);
	case 1:
		return BoundingBox(x - depth + 1 + offZ, y + offY, z + offX, x + offZ, y + height - 1 + offY, z + width - 1 + offX);
	case 2:
		return BoundingBox(x + offX, y + offY, z - depth + 1 + offZ, x + width - 1 + offX, y + height - 1 + offY, z + offZ);
	case 3:
		return BoundingBox(x + offZ, y + offY, z + offX, x + depth - 1 + offZ, y + height - 1 + offY, z + width - 1 + offX);
	default:
		return BoundingBox(x + offX, y + offY, z + offZ, x + width - 1 + offX, y + height - 1 + offY, z + depth - 1 + offZ);
	}
}

void BoundingBox::expand(const BoundingBox &b)
{
	x0 = std::min(x0, b.x0);
	y0 = std::min(y0, b.y0);
	z0 = std::min(z0, b.z0);
	x1 = std::max(x1, b.x1);
	y1 = std::max(y1, b.y1);
	z1 = std::max(z1, b.z1);
}

const WeighedTreasureItem &pickWeighted(Random &random, const std::vector<WeighedTreasureItem> &items)
{
	int_t total = 0;
	for (const WeighedTreasureItem &item : items)
		total += item.weight;
	int_t pick = random.nextInt(total);
	for (const WeighedTreasureItem &item : items)
	{
		pick -= item.weight;
		if (pick < 0)
			return item;
	}
	return items.back();
}

// ---------------------------------------------------------------- StructurePiece

std::shared_ptr<StructurePiece> StructurePiece::findCollisionPiece(PieceList &pieces, const BoundingBox &box)
{
	for (auto &piece : pieces)
	{
		if (piece->getBoundingBox().valid && piece->getBoundingBox().intersects(box))
			return piece;
	}
	return nullptr;
}

bool StructurePiece::edgesLiquid(Level &level, const BoundingBox &chunkBB)
{
	int_t x0 = std::max(boundingBox.x0 - 1, chunkBB.x0);
	int_t y0 = std::max(boundingBox.y0 - 1, chunkBB.y0);
	int_t z0 = std::max(boundingBox.z0 - 1, chunkBB.z0);
	int_t x1 = std::min(boundingBox.x1 + 1, chunkBB.x1);
	int_t y1 = std::min(boundingBox.y1 + 1, chunkBB.y1);
	int_t z1 = std::min(boundingBox.z1 + 1, chunkBB.z1);

	auto liquid = [&](int_t x, int_t y, int_t z) {
		int_t t = level.getTile(x, y, z);
		return t > 0 && Tile::tiles[t] != nullptr && Tile::tiles[t]->material.isLiquid();
	};

	for (int_t x = x0; x <= x1; x++)
		for (int_t z = z0; z <= z1; z++)
			if (liquid(x, y0, z) || liquid(x, y1, z))
				return true;
	for (int_t x = x0; x <= x1; x++)
		for (int_t y = y0; y <= y1; y++)
			if (liquid(x, y, z0) || liquid(x, y, z1))
				return true;
	for (int_t z = z0; z <= z1; z++)
		for (int_t y = y0; y <= y1; y++)
			if (liquid(x0, y, z) || liquid(x1, y, z))
				return true;
	return false;
}

int_t StructurePiece::getWorldX(int_t x, int_t z) const
{
	switch (orientation)
	{
	case 0:
	case 2:
		return boundingBox.x0 + x;
	case 1:
		return boundingBox.x1 - z;
	case 3:
		return boundingBox.x0 + z;
	default:
		return x;
	}
}

int_t StructurePiece::getWorldY(int_t y) const
{
	return orientation == -1 ? y : y + boundingBox.y0;
}

int_t StructurePiece::getWorldZ(int_t x, int_t z) const
{
	switch (orientation)
	{
	case 0:
		return boundingBox.z0 + z;
	case 1:
	case 3:
		return boundingBox.z0 + x;
	case 2:
		return boundingBox.z1 - z;
	default:
		return z;
	}
}

int_t StructurePiece::getOrientationData(int_t tile, int_t data) const
{
	if (tile == Tile::rail.id)
	{
		if (orientation == 1 || orientation == 3)
			return data == 1 ? 0 : 1;
	}
	else if (tile == Tile::doorWood.id || tile == Tile::doorIron.id)
	{
		if (orientation == 0)
		{
			if (data == 0) return 2;
			if (data == 2) return 0;
		}
		else
		{
			if (orientation == 1) return (data + 1) & 3;
			if (orientation == 3) return (data + 3) & 3;
		}
	}
	else if (tile == Tile::stairsStone.id || tile == Tile::stairsWood.id || tile == Tile::stairsNetherBrick.id || tile == Tile::stairsStoneBrick.id)
	{
		if (orientation == 0)
		{
			if (data == 2) return 3;
			if (data == 3) return 2;
		}
		else if (orientation == 1)
		{
			if (data == 0) return 2;
			if (data == 1) return 3;
			if (data == 2) return 0;
			if (data == 3) return 1;
		}
		else if (orientation == 3)
		{
			if (data == 0) return 2;
			if (data == 1) return 3;
			if (data == 2) return 1;
			if (data == 3) return 0;
		}
	}
	else if (tile == Tile::ladder.id)
	{
		if (orientation == 0)
		{
			if (data == 2) return 3;
			if (data == 3) return 2;
		}
		else if (orientation == 1)
		{
			if (data == 2) return 4;
			if (data == 3) return 5;
			if (data == 4) return 2;
			if (data == 5) return 3;
		}
		else if (orientation == 3)
		{
			if (data == 2) return 5;
			if (data == 3) return 4;
			if (data == 4) return 2;
			if (data == 5) return 3;
		}
	}
	else if (tile == Tile::buttonStone.id)
	{
		if (orientation == 0)
		{
			if (data == 3) return 4;
			if (data == 4) return 3;
		}
		else if (orientation == 1)
		{
			if (data == 3) return 1;
			if (data == 4) return 2;
			if (data == 2) return 3;
			if (data == 1) return 4;
		}
		else if (orientation == 3)
		{
			if (data == 3) return 2;
			if (data == 4) return 1;
			if (data == 2) return 3;
			if (data == 1) return 4;
		}
	}
	return data;
}

void StructurePiece::placeBlock(Level &level, int_t block, int_t data, int_t x, int_t y, int_t z, const BoundingBox &chunkBB)
{
	int_t wx = getWorldX(x, z);
	int_t wy = getWorldY(y);
	int_t wz = getWorldZ(x, z);
	if (chunkBB.isInside(wx, wy, wz))
		level.setTileAndDataNoUpdate(wx, wy, wz, block, data);
}

int_t StructurePiece::getBlock(Level &level, int_t x, int_t y, int_t z, const BoundingBox &chunkBB)
{
	int_t wx = getWorldX(x, z);
	int_t wy = getWorldY(y);
	int_t wz = getWorldZ(x, z);
	return !chunkBB.isInside(wx, wy, wz) ? 0 : level.getTile(wx, wy, wz);
}

void StructurePiece::generateBox(Level &level, const BoundingBox &chunkBB, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, int_t edgeTile, int_t fillTile, bool skipAir)
{
	for (int_t y = y0; y <= y1; y++)
	{
		for (int_t x = x0; x <= x1; x++)
		{
			for (int_t z = z0; z <= z1; z++)
			{
				if (skipAir && getBlock(level, x, y, z, chunkBB) == 0)
					continue;
				if (y != y0 && y != y1 && x != x0 && x != x1 && z != z0 && z != z1)
					placeBlock(level, fillTile, 0, x, y, z, chunkBB);
				else
					placeBlock(level, edgeTile, 0, x, y, z, chunkBB);
			}
		}
	}
}

void StructurePiece::generateBox(Level &level, const BoundingBox &chunkBB, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, bool skipAir, Random &random, BlockSelector &selector)
{
	for (int_t y = y0; y <= y1; y++)
	{
		for (int_t x = x0; x <= x1; x++)
		{
			for (int_t z = z0; z <= z1; z++)
			{
				if (skipAir && getBlock(level, x, y, z, chunkBB) == 0)
					continue;
				selector.next(random, x, y, z, y == y0 || y == y1 || x == x0 || x == x1 || z == z0 || z == z1);
				placeBlock(level, selector.getNextId(), selector.getNextData(), x, y, z, chunkBB);
			}
		}
	}
}

void StructurePiece::generateMaybeBox(Level &level, const BoundingBox &chunkBB, Random &random, float probability, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, int_t edgeTile, int_t fillTile, bool skipAir)
{
	for (int_t y = y0; y <= y1; y++)
	{
		for (int_t x = x0; x <= x1; x++)
		{
			for (int_t z = z0; z <= z1; z++)
			{
				if (random.nextFloat() > probability)
					continue;
				if (skipAir && getBlock(level, x, y, z, chunkBB) == 0)
					continue;
				if (y != y0 && y != y1 && x != x0 && x != x1 && z != z0 && z != z1)
					placeBlock(level, fillTile, 0, x, y, z, chunkBB);
				else
					placeBlock(level, edgeTile, 0, x, y, z, chunkBB);
			}
		}
	}
}

void StructurePiece::maybeGenerateBlock(Level &level, const BoundingBox &chunkBB, Random &random, float probability, int_t x, int_t y, int_t z, int_t tile, int_t data)
{
	if (random.nextFloat() < probability)
		placeBlock(level, tile, data, x, y, z, chunkBB);
}

void StructurePiece::generateUpperHalfSphere(Level &level, const BoundingBox &chunkBB, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, int_t fillTile, bool skipAir)
{
	float diagX = static_cast<float>(x1 - x0 + 1);
	float diagY = static_cast<float>(y1 - y0 + 1);
	float diagZ = static_cast<float>(z1 - z0 + 1);
	float cx = static_cast<float>(x0) + diagX / 2.0f;
	float cz = static_cast<float>(z0) + diagZ / 2.0f;

	for (int_t y = y0; y <= y1; y++)
	{
		float normalizedYDistance = static_cast<float>(y - y0) / diagY;
		for (int_t x = x0; x <= x1; x++)
		{
			float normalizedXDistance = (static_cast<float>(x) - cx) / (diagX * 0.5f);
			for (int_t z = z0; z <= z1; z++)
			{
				float normalizedZDistance = (static_cast<float>(z) - cz) / (diagZ * 0.5f);
				if (skipAir && getBlock(level, x, y, z, chunkBB) == 0)
					continue;
				float dist = normalizedXDistance * normalizedXDistance + normalizedYDistance * normalizedYDistance + normalizedZDistance * normalizedZDistance;
				if (dist <= 1.05f)
					placeBlock(level, fillTile, 0, x, y, z, chunkBB);
			}
		}
	}
}

void StructurePiece::generateAirColumnUp(Level &level, int_t x, int_t y, int_t z, const BoundingBox &chunkBB)
{
	int_t wx = getWorldX(x, z);
	int_t wy = getWorldY(y);
	int_t wz = getWorldZ(x, z);
	if (!chunkBB.isInside(wx, wy, wz))
		return;
	while (!level.isEmptyTile(wx, wy, wz) && wy < Level::DEPTH - 1)
	{
		level.setTileAndDataNoUpdate(wx, wy, wz, 0, 0);
		wy++;
	}
}

void StructurePiece::fillColumnDown(Level &level, int_t tile, int_t data, int_t x, int_t y, int_t z, const BoundingBox &chunkBB)
{
	int_t wx = getWorldX(x, z);
	int_t wy = getWorldY(y);
	int_t wz = getWorldZ(x, z);
	if (!chunkBB.isInside(wx, wy, wz))
		return;
	while ((level.isEmptyTile(wx, wy, wz) || level.getMaterial(wx, wy, wz).isLiquid()) && wy > 1)
	{
		level.setTileAndDataNoUpdate(wx, wy, wz, tile, data);
		wy--;
	}
}

void StructurePiece::addChestLoot(Random &random, const std::vector<WeighedTreasureItem> &treasure, ChestTileEntity &chest, int_t numRolls)
{
	for (int_t r = 0; r < numRolls; r++)
	{
		const WeighedTreasureItem &item = pickWeighted(random, treasure);
		int_t count = item.minCount + random.nextInt(item.maxCount - item.minCount + 1);
		int_t maxStack = (item.itemId >= 0 && item.itemId < static_cast<int_t>(Item::items.size()) && Item::items[item.itemId] != nullptr) ? Item::items[item.itemId]->getMaxStackSize() : 64;
		if (maxStack >= count)
		{
			chest.setItem(random.nextInt(chest.getContainerSize()), ItemInstance(item.itemId, count, item.auxValue));
		}
		else
		{
			for (int_t i = 0; i < count; i++)
				chest.setItem(random.nextInt(chest.getContainerSize()), ItemInstance(item.itemId, 1, item.auxValue));
		}
	}
}

void StructurePiece::createChest(Level &level, const BoundingBox &chunkBB, Random &random, int_t x, int_t y, int_t z, const std::vector<WeighedTreasureItem> &treasure, int_t numRolls)
{
	int_t wx = getWorldX(x, z);
	int_t wy = getWorldY(y);
	int_t wz = getWorldZ(x, z);
	if (!chunkBB.isInside(wx, wy, wz) || level.getTile(wx, wy, wz) == Tile::chest.id)
		return;
	level.setTile(wx, wy, wz, Tile::chest.id);
	auto chest = std::dynamic_pointer_cast<ChestTileEntity>(level.getTileEntity(wx, wy, wz));
	if (chest != nullptr)
		addChestLoot(random, treasure, *chest, numRolls);
}

void StructurePiece::createDoor(Level &level, const BoundingBox &chunkBB, Random &random, int_t x, int_t y, int_t z, int_t orientation)
{
	(void)random;
	int_t wx = getWorldX(x, z);
	int_t wy = getWorldY(y);
	int_t wz = getWorldZ(x, z);
	if (!chunkBB.isInside(wx, wy, wz))
		return;

	// ItemDoor.place
	int_t data = orientation;
	int_t xd = 0;
	int_t zd = 0;
	if (data == 0) zd = 1;
	if (data == 1) xd = -1;
	if (data == 2) zd = -1;
	if (data == 3) xd = 1;
	int_t solidA = (level.isSolidTile(wx - xd, wy, wz - zd) ? 1 : 0) + (level.isSolidTile(wx - xd, wy + 1, wz - zd) ? 1 : 0);
	int_t solidB = (level.isSolidTile(wx + xd, wy, wz + zd) ? 1 : 0) + (level.isSolidTile(wx + xd, wy + 1, wz + zd) ? 1 : 0);
	int_t door = Tile::doorWood.id;
	bool doorA = level.getTile(wx - xd, wy, wz - zd) == door || level.getTile(wx - xd, wy + 1, wz - zd) == door;
	bool doorB = level.getTile(wx + xd, wy, wz + zd) == door || level.getTile(wx + xd, wy + 1, wz + zd) == door;
	bool flip = false;
	if (doorA && !doorB)
		flip = true;
	else if (solidB > solidA)
		flip = true;
	if (flip)
	{
		data = (data - 1) & 3;
		data += 4;
	}
	level.setTileAndDataNoUpdate(wx, wy, wz, door, data);
	level.setTileAndDataNoUpdate(wx, wy + 1, wz, door, data + 8);
}

// ---------------------------------------------------------------- StructureStart

void StructureStart::postProcess(Level &level, Random &random, const BoundingBox &chunkBB)
{
	for (auto it = pieces.begin(); it != pieces.end();)
	{
		if ((*it)->getBoundingBox().intersects(chunkBB) && !(*it)->postProcess(level, random, chunkBB))
			it = pieces.erase(it);
		else
			++it;
	}
}

void StructureStart::calculateBoundingBox()
{
	boundingBox = BoundingBox::getUnknownBox();
	for (auto &piece : pieces)
		boundingBox.expand(piece->getBoundingBox());
}

void StructureStart::moveBelowSeaLevel(Level &level, Random &random, int_t offset)
{
	(void)level;
	int_t maxY = 63 - offset;
	int_t y = boundingBox.getYSpan() + 1;
	if (y < maxY)
		y += random.nextInt(maxY - y);
	int_t dy = y - boundingBox.y1;
	boundingBox.move(0, dy, 0);
	for (auto &piece : pieces)
		piece->getBoundingBox().move(0, dy, 0);
}

void StructureStart::moveInsideHeights(Level &level, Random &random, int_t lowestAllowed, int_t highestAllowed)
{
	(void)level;
	int_t heightSpan = highestAllowed - lowestAllowed + 1 - boundingBox.getYSpan();
	int_t y;
	if (heightSpan > 1)
		y = lowestAllowed + random.nextInt(heightSpan);
	else
		y = lowestAllowed;
	int_t dy = y - boundingBox.y0;
	boundingBox.move(0, dy, 0);
	for (auto &piece : pieces)
		piece->getBoundingBox().move(0, dy, 0);
}

// ---------------------------------------------------------------- StructureFeature

namespace
{
	long_t chunkKey(int_t x, int_t z)
	{
		return static_cast<long_t>(static_cast<ulong_t>(static_cast<uint_t>(x)) | static_cast<ulong_t>(static_cast<uint_t>(z)) << 32);
	}
}

bool StructureFeature::hasStructureAt(long_t key) const
{
	for (const auto &entry : cachedStructures)
		if (entry.first == key)
			return true;
	return false;
}

void StructureFeature::addFeature(Level &level, int_t startX, int_t startZ)
{
	(void)level;
	long_t key = chunkKey(startX, startZ);
	if (hasStructureAt(key))
		return;
	random.nextInt();
	if (isFeatureChunk(startX, startZ))
		cachedStructures.emplace_back(key, createStructureStart(startX, startZ));
}

void StructureFeature::apply(ChunkSource &source, Level &level, int_t x, int_t z, std::array<ubyte_t, 16 * 16 * 128> &blocks)
{
	(void)source;
	(void)blocks;
	this->level = &level;
	random.setSeed(level.seed);
	long_t xScale = random.nextLong();
	long_t zScale = random.nextLong();

	for (int_t xx = x - radius; xx <= x + radius; xx++)
	{
		for (int_t zz = z - radius; zz <= z + radius; zz++)
		{
			ulong_t a = static_cast<ulong_t>(static_cast<long_t>(xx)) * static_cast<ulong_t>(xScale);
			ulong_t b = static_cast<ulong_t>(static_cast<long_t>(zz)) * static_cast<ulong_t>(zScale);
			random.setSeed(Java::longFromBits(a ^ b ^ static_cast<ulong_t>(level.seed)));
			addFeature(level, xx, zz);
		}
	}
}

bool StructureFeature::postProcess(Level &level, Random &random, int_t chunkX, int_t chunkZ)
{
	int_t cx = (chunkX << 4) + 8;
	int_t cz = (chunkZ << 4) + 8;
	bool intersection = false;
	for (auto &entry : cachedStructures)
	{
		StructureStart &start = *entry.second;
		if (start.isValid() && start.getBoundingBox().intersects(cx, cz, cx + 15, cz + 15))
		{
			start.postProcess(level, random, BoundingBox(cx, cz, cx + 15, cz + 15));
			intersection = true;
		}
	}
	return intersection;
}

bool StructureFeature::isIntersection(int_t x, int_t y, int_t z)
{
	for (auto &entry : cachedStructures)
	{
		StructureStart &start = *entry.second;
		if (!start.isValid() || !start.getBoundingBox().intersects(x, z, x, z))
			continue;
		for (auto &piece : start.getPieces())
			if (piece->getBoundingBox().isInside(x, y, z))
				return true;
	}
	return false;
}

bool StructureFeature::findNearest(Level &level, int_t x, int_t y, int_t z, int_t &outX, int_t &outY, int_t &outZ)
{
	this->level = &level;
	random.setSeed(level.seed);
	long_t xScale = random.nextLong();
	long_t zScale = random.nextLong();
	ulong_t a = static_cast<ulong_t>(static_cast<long_t>(x >> 4)) * static_cast<ulong_t>(xScale);
	ulong_t b = static_cast<ulong_t>(static_cast<long_t>(z >> 4)) * static_cast<ulong_t>(zScale);
	random.setSeed(Java::longFromBits(a ^ b ^ static_cast<ulong_t>(level.seed)));
	addFeature(level, x >> 4, z >> 4);

	double best = std::numeric_limits<double>::max();
	bool found = false;
	for (auto &entry : cachedStructures)
	{
		StructureStart &start = *entry.second;
		if (!start.isValid() || start.getPieces().empty())
			continue;
		TilePos3 pos = start.getPieces().front()->getLocatorPosition();
		int_t dx = pos.x - x;
		int_t dy = pos.y - y;
		int_t dz = pos.z - z;
		double dist = static_cast<double>(dx + dx * dy * dy + dz * dz);
		if (dist < best)
		{
			best = dist;
			outX = pos.x;
			outY = pos.y;
			outZ = pos.z;
			found = true;
		}
	}
	if (found)
		return true;

	for (const TilePos3 &pos : getGuessedPositions())
	{
		int_t dx = pos.x - x;
		int_t dy = pos.y - y;
		int_t dz = pos.z - z;
		double dist = static_cast<double>(dx + dx * dy * dy + dz * dz);
		if (dist < best)
		{
			best = dist;
			outX = pos.x;
			outY = pos.y;
			outZ = pos.z;
			found = true;
		}
	}
	return found;
}

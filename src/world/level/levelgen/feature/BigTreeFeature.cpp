#include "world/level/levelgen/feature/BigTreeFeature.h"

#include "world/level/Level.h"

#include "world/level/tile/Tile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/LeafTile.h"


#include "util/Mth.h"

#include <cmath>

namespace
{
	static constexpr byte_t AXIS_LOOKUP[] = { 2, 0, 0, 1, 2, 1 };
}

void BigTreeFeature::prepareLeafNodes()
{
	this->height = static_cast<int_t>(static_cast<double>(this->heightLimit) * this->heightAttenuation);
	if (this->height >= this->heightLimit)
		this->height = this->heightLimit - 1;

	int_t nodesPerLayer = static_cast<int_t>(1.382 + std::pow(this->leafDensity * static_cast<double>(this->heightLimit) / 13.0, 2.0));
	if (nodesPerLayer < 1)
		nodesPerLayer = 1;

	std::vector<std::array<int_t, 4>> nodes(nodesPerLayer * this->heightLimit);
	int_t nodeY = this->basePos[1] + this->heightLimit - this->leafDistanceLimit;
	int_t nodeCount = 1;
	int_t trunkTopY = this->basePos[1] + this->height;
	int_t layer = nodeY - this->basePos[1];
	nodes[0][0] = this->basePos[0];
	nodes[0][1] = nodeY;
	nodes[0][2] = this->basePos[2];
	nodes[0][3] = trunkTopY;
	--nodeY;

	while (layer >= 0)
	{
		float layerSize = this->calcLayerSize(layer);
		if (layerSize < 0.0f)
		{
			--nodeY;
			--layer;
			continue;
		}

		for (int_t i = 0; i < nodesPerLayer; ++i)
		{
			double distance = this->scaleWidth * (static_cast<double>(layerSize) * (static_cast<double>(this->treeRandom.nextFloat()) + 0.328));
			double angle = static_cast<double>(this->treeRandom.nextFloat()) * 2.0 * 3.14159;
			int_t leafX = Mth::floor(distance * std::sin(angle) + static_cast<double>(this->basePos[0]) + 0.5);
			int_t leafZ = Mth::floor(distance * std::cos(angle) + static_cast<double>(this->basePos[2]) + 0.5);
			std::array<int_t, 3> leafPos = { leafX, nodeY, leafZ };
			std::array<int_t, 3> leafTop = { leafX, nodeY + this->leafDistanceLimit, leafZ };
			if (this->checkLine(leafPos, leafTop) == -1)
			{
				std::array<int_t, 3> branchBase = { this->basePos[0], this->basePos[1], this->basePos[2] };
				double horizontalDistance = std::sqrt(
					std::pow(static_cast<double>(std::abs(this->basePos[0] - leafPos[0])), 2.0) +
					std::pow(static_cast<double>(std::abs(this->basePos[2] - leafPos[2])), 2.0));
				double branchDrop = horizontalDistance * this->branchSlope;
				if (static_cast<double>(leafPos[1]) - branchDrop > static_cast<double>(trunkTopY))
					branchBase[1] = trunkTopY;
				else
					branchBase[1] = static_cast<int_t>(static_cast<double>(leafPos[1]) - branchDrop);

				if (this->checkLine(branchBase, leafPos) == -1)
				{
					nodes[nodeCount][0] = leafX;
					nodes[nodeCount][1] = nodeY;
					nodes[nodeCount][2] = leafZ;
					nodes[nodeCount][3] = branchBase[1];
					++nodeCount;
				}
			}
		}

		--nodeY;
		--layer;
	}

	this->leafNodes.assign(nodes.begin(), nodes.begin() + nodeCount);
}

void BigTreeFeature::placeLeafDisc(int_t x, int_t y, int_t z, float radius, byte_t axis, int_t tile)
{
	int_t radiusCeil = static_cast<int_t>(static_cast<double>(radius) + 0.618);
	byte_t axis0 = AXIS_LOOKUP[axis];
	byte_t axis1 = AXIS_LOOKUP[axis + 3];
	std::array<int_t, 3> center = { x, y, z };
	std::array<int_t, 3> pos = { 0, 0, 0 };
	pos[axis] = center[axis];

	for (int_t offset0 = -radiusCeil; offset0 <= radiusCeil; ++offset0)
	{
		pos[axis0] = center[axis0] + offset0;
		for (int_t offset1 = -radiusCeil; offset1 <= radiusCeil; ++offset1)
		{
			double distance = std::sqrt(
				std::pow(static_cast<double>(std::abs(offset0)) + 0.5, 2.0) +
				std::pow(static_cast<double>(std::abs(offset1)) + 0.5, 2.0));
			if (distance > static_cast<double>(radius))
				continue;

			pos[axis1] = center[axis1] + offset1;
			int_t existingTile = this->level->getTile(pos[0], pos[1], pos[2]);
			if (existingTile != 0 && existingTile != Tile::leaves.id)
				continue;

			this->level->setTileNoUpdate(pos[0], pos[1], pos[2], tile);
		}
	}
}

float BigTreeFeature::calcLayerSize(int_t layer) const
{
	if (static_cast<double>(layer) < static_cast<double>(this->heightLimit) * 0.3)
		return -1.618f;

	float midpoint = static_cast<float>(this->heightLimit) / 2.0f;
	float distance = midpoint - static_cast<float>(layer);
	float layerSize = 0.0f;
	if (distance == 0.0f)
	{
		layerSize = midpoint;
	}
	else if (std::abs(distance) < midpoint)
	{
		layerSize = static_cast<float>(std::sqrt(
			std::pow(static_cast<double>(std::abs(midpoint)), 2.0) -
			std::pow(static_cast<double>(std::abs(distance)), 2.0)));
	}

	return layerSize * 0.5f;
}

float BigTreeFeature::calcLeafSize(int_t layer) const
{
	if (layer < 0 || layer >= this->leafDistanceLimit)
		return -1.0f;
	if (layer == 0 || layer == this->leafDistanceLimit - 1)
		return 2.0f;
	return 3.0f;
}

void BigTreeFeature::placeLeafNode(int_t x, int_t y, int_t z)
{
	for (int_t yy = y; yy < y + this->leafDistanceLimit; ++yy)
	{
		float leafSize = this->calcLeafSize(yy - y);
		this->placeLeafDisc(x, yy, z, leafSize, 1, Tile::leaves.id);
	}
}

void BigTreeFeature::placeLine(const std::array<int_t, 3> &from, const std::array<int_t, 3> &to, int_t tile)
{
	std::array<int_t, 3> delta = { 0, 0, 0 };
	byte_t dominantAxis = 0;
	for (byte_t axis = 0; axis < 3; ++axis)
	{
		delta[axis] = to[axis] - from[axis];
		if (std::abs(delta[axis]) > std::abs(delta[dominantAxis]))
			dominantAxis = axis;
	}

	if (delta[dominantAxis] == 0)
		return;

	byte_t axis0 = AXIS_LOOKUP[dominantAxis];
	byte_t axis1 = AXIS_LOOKUP[dominantAxis + 3];
	byte_t direction = delta[dominantAxis] > 0 ? 1 : -1;
	double slope0 = static_cast<double>(delta[axis0]) / static_cast<double>(delta[dominantAxis]);
	double slope1 = static_cast<double>(delta[axis1]) / static_cast<double>(delta[dominantAxis]);
	std::array<int_t, 3> pos = { 0, 0, 0 };

	for (int_t offset = 0, end = delta[dominantAxis] + direction; offset != end; offset += direction)
	{
		pos[dominantAxis] = Mth::floor(static_cast<double>(from[dominantAxis] + offset) + 0.5);
		pos[axis0] = Mth::floor(static_cast<double>(from[axis0]) + static_cast<double>(offset) * slope0 + 0.5);
		pos[axis1] = Mth::floor(static_cast<double>(from[axis1]) + static_cast<double>(offset) * slope1 + 0.5);
		this->level->setTileNoUpdate(pos[0], pos[1], pos[2], tile);
	}
}

void BigTreeFeature::placeLeafNodes()
{
	for (const std::array<int_t, 4> &leafNode : this->leafNodes)
		this->placeLeafNode(leafNode[0], leafNode[1], leafNode[2]);
}

bool BigTreeFeature::leafNodeNeedsBase(int_t height) const
{
	return static_cast<double>(height) >= static_cast<double>(this->heightLimit) * 0.2;
}

void BigTreeFeature::placeTrunk()
{
	int_t x = this->basePos[0];
	int_t y0 = this->basePos[1];
	int_t y1 = this->basePos[1] + this->height;
	int_t z = this->basePos[2];
	std::array<int_t, 3> from = { x, y0, z };
	std::array<int_t, 3> to = { x, y1, z };
	this->placeLine(from, to, Tile::treeTrunk.id);
	if (this->trunkSize == 2)
	{
		++from[0];
		++to[0];
		this->placeLine(from, to, Tile::treeTrunk.id);
		++from[2];
		++to[2];
		this->placeLine(from, to, Tile::treeTrunk.id);
		--from[0];
		--to[0];
		this->placeLine(from, to, Tile::treeTrunk.id);
	}
}

void BigTreeFeature::placeLeafNodeBases()
{
	std::array<int_t, 3> branchBase = { this->basePos[0], this->basePos[1], this->basePos[2] };
	for (const std::array<int_t, 4> &leafNode : this->leafNodes)
	{
		std::array<int_t, 3> nodePos = { leafNode[0], leafNode[1], leafNode[2] };
		branchBase[1] = leafNode[3];
		if (this->leafNodeNeedsBase(branchBase[1] - this->basePos[1]))
			this->placeLine(branchBase, nodePos, Tile::treeTrunk.id);
	}
}

int_t BigTreeFeature::checkLine(const std::array<int_t, 3> &from, const std::array<int_t, 3> &to) const
{
	std::array<int_t, 3> delta = { 0, 0, 0 };
	byte_t dominantAxis = 0;
	for (byte_t axis = 0; axis < 3; ++axis)
	{
		delta[axis] = to[axis] - from[axis];
		if (std::abs(delta[axis]) > std::abs(delta[dominantAxis]))
			dominantAxis = axis;
	}

	if (delta[dominantAxis] == 0)
		return -1;

	byte_t axis0 = AXIS_LOOKUP[dominantAxis];
	byte_t axis1 = AXIS_LOOKUP[dominantAxis + 3];
	byte_t direction = delta[dominantAxis] > 0 ? 1 : -1;
	double slope0 = static_cast<double>(delta[axis0]) / static_cast<double>(delta[dominantAxis]);
	double slope1 = static_cast<double>(delta[axis1]) / static_cast<double>(delta[dominantAxis]);
	std::array<int_t, 3> pos = { 0, 0, 0 };
	int_t offset = 0;
	int_t end = delta[dominantAxis] + direction;
	for (; offset != end; offset += direction)
	{
		pos[dominantAxis] = from[dominantAxis] + offset;
		pos[axis0] = Mth::floor(static_cast<double>(from[axis0]) + static_cast<double>(offset) * slope0);
		pos[axis1] = Mth::floor(static_cast<double>(from[axis1]) + static_cast<double>(offset) * slope1);
		int_t tile = this->level->getTile(pos[0], pos[1], pos[2]);
		if (tile != 0 && tile != Tile::leaves.id)
			break;
	}

	return offset == end ? -1 : std::abs(offset);
}

bool BigTreeFeature::validTreeLocation()
{
	std::array<int_t, 3> base = { this->basePos[0], this->basePos[1], this->basePos[2] };
	std::array<int_t, 3> top = { this->basePos[0], this->basePos[1] + this->heightLimit - 1, this->basePos[2] };
	int_t soil = this->level->getTile(this->basePos[0], this->basePos[1] - 1, this->basePos[2]);
	if (soil != Tile::grass.id && soil != Tile::dirt.id)
		return false;

	int_t obstruction = this->checkLine(base, top);
	if (obstruction == -1)
		return true;
	if (obstruction < 6)
		return false;

	this->heightLimit = obstruction;
	return true;
}

void BigTreeFeature::init(double v0, double v1, double v2)
{
	this->heightLimitLimit = static_cast<int_t>(v0 * 12.0);
	if (v0 > 0.5)
		this->leafDistanceLimit = 5;
	this->scaleWidth = v1;
	this->leafDensity = v2;
}

bool BigTreeFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	this->level = &level;
	this->treeRandom.setSeed(random.nextLong());
	this->basePos[0] = x;
	this->basePos[1] = y;
	this->basePos[2] = z;

	if (this->heightLimit == 0)
		this->heightLimit = 5 + this->treeRandom.nextInt(this->heightLimitLimit);

	if (!this->validTreeLocation())
	{
		return false;
	}

	this->prepareLeafNodes();
	this->placeLeafNodes();
	this->placeTrunk();
	this->placeLeafNodeBases();
	return true;
}

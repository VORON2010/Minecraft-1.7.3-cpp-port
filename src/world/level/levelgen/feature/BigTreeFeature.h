#pragma once

#include "world/level/levelgen/feature/Feature.h"

#include <array>
#include <vector>

class BigTreeFeature : public Feature
{
private:
	Level *level = nullptr;
	Random treeRandom;
	std::array<int_t, 3> basePos = { 0, 0, 0 };
	int_t heightLimit = 0;
	int_t height = 0;
	double heightAttenuation = 0.618;
	double branchSlope = 0.381;
	double scaleWidth = 1.0;
	double leafDensity = 1.0;
	int_t trunkSize = 1;
	int_t heightLimitLimit = 12;
	int_t leafDistanceLimit = 4;
	std::vector<std::array<int_t, 4>> leafNodes;

	void prepareLeafNodes();
	void placeLeafDisc(int_t x, int_t y, int_t z, float radius, byte_t axis, int_t tile);
	float calcLayerSize(int_t layer) const;
	float calcLeafSize(int_t layer) const;
	void placeLeafNode(int_t x, int_t y, int_t z);
	void placeLine(const std::array<int_t, 3> &from, const std::array<int_t, 3> &to, int_t tile);
	void placeLeafNodes();
	bool leafNodeNeedsBase(int_t height) const;
	void placeTrunk();
	void placeLeafNodeBases();
	int_t checkLine(const std::array<int_t, 3> &from, const std::array<int_t, 3> &to) const;
	bool validTreeLocation();

public:
	void init(double v0, double v1, double v2) override;
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};

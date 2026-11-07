#pragma once

#include "world/level/levelgen/structure/StructurePiece.h"

class StrongholdFeature : public StructureFeature
{
	bool positionsFound = false;
	bool hasPosition[3] = {false, false, false};
	int_t posX[3] = {0, 0, 0};
	int_t posZ[3] = {0, 0, 0};

protected:
	bool isFeatureChunk(int_t x, int_t z) override;
	std::unique_ptr<StructureStart> createStructureStart(int_t x, int_t z) override;
	std::vector<TilePos3> getGuessedPositions() override;
};

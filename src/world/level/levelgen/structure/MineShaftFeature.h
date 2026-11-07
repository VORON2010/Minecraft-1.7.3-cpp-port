#pragma once

#include "world/level/levelgen/structure/StructurePiece.h"

class MineShaftFeature : public StructureFeature
{
protected:
	bool isFeatureChunk(int_t x, int_t z) override;
	std::unique_ptr<StructureStart> createStructureStart(int_t x, int_t z) override;
};

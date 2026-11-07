#pragma once

#include <vector>

#include "world/level/biome/Biome.h"
#include "world/level/levelgen/structure/StructurePiece.h"

// 1.0 nether fortress; blazes, pigmen and magma cubes spawn inside its pieces
class FortressFeature : public StructureFeature
{
	std::vector<BiomeSpawnEntry> monsters;

public:
	FortressFeature();
	const std::vector<BiomeSpawnEntry> &getMonsters() const { return monsters; }

protected:
	bool isFeatureChunk(int_t x, int_t z) override;
	std::unique_ptr<StructureStart> createStructureStart(int_t x, int_t z) override;
};

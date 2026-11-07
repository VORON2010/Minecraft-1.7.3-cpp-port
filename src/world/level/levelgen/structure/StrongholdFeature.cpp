#include "world/level/levelgen/structure/StrongholdFeature.h"

bool StrongholdFeature::isFeatureChunk(int_t, int_t) { return false; }
std::unique_ptr<StructureStart> StrongholdFeature::createStructureStart(int_t, int_t) { return nullptr; }
std::vector<TilePos3> StrongholdFeature::getGuessedPositions() { return {}; }

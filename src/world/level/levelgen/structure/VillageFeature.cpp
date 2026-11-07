#include "world/level/levelgen/structure/VillageFeature.h"

bool VillageFeature::isFeatureChunk(int_t, int_t) { return false; }
std::unique_ptr<StructureStart> VillageFeature::createStructureStart(int_t, int_t) { return nullptr; }

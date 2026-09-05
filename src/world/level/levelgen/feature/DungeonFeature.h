#pragma once

#include "world/level/levelgen/feature/Feature.h"
#include "java/String.h"

class ItemInstance;

class DungeonFeature : public Feature {
public:
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;

private:
	ItemInstance pickLootItem(Random &random);
	jstring pickMob(Random &random);
};

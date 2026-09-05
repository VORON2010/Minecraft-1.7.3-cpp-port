#pragma once

#include <memory>
#include <vector>

#include "java/Type.h"

class Achievement;

class AchievementList
{
private:
	static std::vector<std::unique_ptr<Achievement>> ownedAchievements;

public:
	static int_t minDisplayColumn;
	static int_t minDisplayRow;
	static int_t maxDisplayColumn;
	static int_t maxDisplayRow;
	static std::vector<Achievement *> achievements;

	static Achievement *openInventory;
	static Achievement *mineWood;
	static Achievement *buildWorkBench;
	static Achievement *buildPickaxe;
	static Achievement *buildFurnace;
	static Achievement *acquireIron;
	static Achievement *buildHoe;
	static Achievement *makeBread;
	static Achievement *bakeCake;
	static Achievement *buildBetterPickaxe;
	static Achievement *cookFish;
	static Achievement *onARail;
	static Achievement *buildSword;
	static Achievement *killEnemy;
	static Achievement *killCow;
	static Achievement *flyPig;

	static void init();
};

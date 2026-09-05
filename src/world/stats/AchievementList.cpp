#include "world/stats/AchievementList.h"

#include <iostream>

#include "world/item/Item.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/Items.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/RailTile.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/WorkbenchTile.h"
#include "world/stats/Achievement.h"

std::vector<std::unique_ptr<Achievement>> AchievementList::ownedAchievements;
int_t AchievementList::minDisplayColumn = 0;
int_t AchievementList::minDisplayRow = 0;
int_t AchievementList::maxDisplayColumn = 0;
int_t AchievementList::maxDisplayRow = 0;
std::vector<Achievement *> AchievementList::achievements;

Achievement *AchievementList::openInventory = nullptr;
Achievement *AchievementList::mineWood = nullptr;
Achievement *AchievementList::buildWorkBench = nullptr;
Achievement *AchievementList::buildPickaxe = nullptr;
Achievement *AchievementList::buildFurnace = nullptr;
Achievement *AchievementList::acquireIron = nullptr;
Achievement *AchievementList::buildHoe = nullptr;
Achievement *AchievementList::makeBread = nullptr;
Achievement *AchievementList::bakeCake = nullptr;
Achievement *AchievementList::buildBetterPickaxe = nullptr;
Achievement *AchievementList::cookFish = nullptr;
Achievement *AchievementList::onARail = nullptr;
Achievement *AchievementList::buildSword = nullptr;
Achievement *AchievementList::killEnemy = nullptr;
Achievement *AchievementList::killCow = nullptr;
Achievement *AchievementList::flyPig = nullptr;

void AchievementList::init()
{
	if (openInventory != nullptr)
		return;

	auto add = [](int_t id, const jstring &name, int_t column, int_t row, int_t itemId, Achievement *parent)
	{
		auto achievement = std::make_unique<Achievement>(id, name, column, row, ItemInstance(itemId), parent);
		Achievement *result = achievement.get();
		ownedAchievements.push_back(std::move(achievement));
		return result;
	};

	openInventory = &add(0, u"openInventory", 0, 0, Items::book->getShiftedIndex(), nullptr)->markIndependentAchievement().registerAchievement();
	mineWood = &add(1, u"mineWood", 2, 1, Tile::treeTrunk.id, openInventory)->registerAchievement();
	buildWorkBench = &add(2, u"buildWorkBench", 4, -1, Tile::workBench.id, mineWood)->registerAchievement();
	buildPickaxe = &add(3, u"buildPickaxe", 4, 2, Items::pickaxeWood->getShiftedIndex(), buildWorkBench)->registerAchievement();
	buildFurnace = &add(4, u"buildFurnace", 3, 4, Tile::furnaceLit.id, buildPickaxe)->registerAchievement();
	acquireIron = &add(5, u"acquireIron", 1, 4, Items::ingotIron->getShiftedIndex(), buildFurnace)->registerAchievement();
	buildHoe = &add(6, u"buildHoe", 2, -3, Items::hoeWood->getShiftedIndex(), buildWorkBench)->registerAchievement();
	makeBread = &add(7, u"makeBread", -1, -3, Items::bread->getShiftedIndex(), buildHoe)->registerAchievement();
	bakeCake = &add(8, u"bakeCake", 0, -5, Items::cake->getShiftedIndex(), buildHoe)->registerAchievement();
	buildBetterPickaxe = &add(9, u"buildBetterPickaxe", 6, 2, Items::pickaxeStone->getShiftedIndex(), buildPickaxe)->registerAchievement();
	cookFish = &add(10, u"cookFish", 2, 6, Items::fishCooked->getShiftedIndex(), buildFurnace)->registerAchievement();
	onARail = &add(11, u"onARail", 2, 3, Tile::rail.id, acquireIron)->setSpecial().registerAchievement();
	buildSword = &add(12, u"buildSword", 6, -1, Items::swordWood->getShiftedIndex(), buildWorkBench)->registerAchievement();
	killEnemy = &add(13, u"killEnemy", 8, -1, Items::bone->getShiftedIndex(), buildSword)->registerAchievement();
	killCow = &add(14, u"killCow", 7, -3, Items::leather->getShiftedIndex(), buildSword)->registerAchievement();
	flyPig = &add(15, u"flyPig", 8, -4, Items::saddle->getShiftedIndex(), killCow)->setSpecial().registerAchievement();

	std::cout << achievements.size() << " achievements" << std::endl;
}

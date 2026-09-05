#include "world/stats/StatList.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

#include "java/String.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/crafting/FurnaceRecipes.h"
#include "world/item/crafting/Recipes.h"
#include "world/level/tile/Tile.h"
#include "world/stats/AchievementList.h"
#include "world/stats/StatBase.h"
#include "world/stats/StatBasic.h"
#include "world/stats/StatCollector.h"
#include "world/stats/StatCrafting.h"

std::unordered_map<int_t, StatBase *> StatList::statsById;
std::vector<std::unique_ptr<StatBase>> StatList::ownedStats;
bool StatList::initialized = false;
std::vector<StatBase *> StatList::allStats;
std::vector<StatBase *> StatList::generalStats;
std::vector<StatCrafting *> StatList::itemStats;
std::vector<StatCrafting *> StatList::blockStats;

StatBase *StatList::startGameStat = nullptr;
StatBase *StatList::createWorldStat = nullptr;
StatBase *StatList::loadWorldStat = nullptr;
StatBase *StatList::joinMultiplayerStat = nullptr;
StatBase *StatList::leaveGameStat = nullptr;
StatBase *StatList::minutesPlayedStat = nullptr;
StatBase *StatList::distanceWalkedStat = nullptr;
StatBase *StatList::distanceSwumStat = nullptr;
StatBase *StatList::distanceFallenStat = nullptr;
StatBase *StatList::distanceClimbedStat = nullptr;
StatBase *StatList::distanceFlownStat = nullptr;
StatBase *StatList::distanceDoveStat = nullptr;
StatBase *StatList::distanceByMinecartStat = nullptr;
StatBase *StatList::distanceByBoatStat = nullptr;
StatBase *StatList::distanceByPigStat = nullptr;
StatBase *StatList::jumpStat = nullptr;
StatBase *StatList::dropStat = nullptr;
StatBase *StatList::damageDealtStat = nullptr;
StatBase *StatList::damageTakenStat = nullptr;
StatBase *StatList::deathsStat = nullptr;
StatBase *StatList::mobKillsStat = nullptr;
StatBase *StatList::playerKillsStat = nullptr;
StatBase *StatList::fishCaughtStat = nullptr;

std::array<StatBase *, 256> StatList::mineBlockStats = {};
std::array<StatBase *, 32000> StatList::craftItemStats = {};
std::array<StatBase *, 32000> StatList::useItemStats = {};
std::array<StatBase *, 32000> StatList::breakItemStats = {};

StatBasic::StatBasic(int_t id, const jstring &name) : StatBase(id, name)
{
}

StatBasic::StatBasic(int_t id, const jstring &name, const StatFormatter &formatter)
	: StatBase(id, name, formatter)
{
}

StatBase &StatBasic::registerStat()
{
	StatBase::registerStat();
	StatList::generalStats.push_back(this);
	return *this;
}

StatCrafting::StatCrafting(int_t id, const jstring &name, int_t itemId)
	: StatBase(id, name), itemId(itemId)
{
}

int_t StatCrafting::getItemId() const
{
	return itemId;
}

namespace
{
	jstring getStatName(int_t id)
	{
		jstring key;
		if (id >= 0 && id < static_cast<int_t>(Tile::tiles.size()) && Tile::tiles[id] != nullptr)
			key = Tile::tiles[id]->descriptionId;
		else if (id >= 0 && id < static_cast<int_t>(Item::items.size()) && Item::items[id] != nullptr)
			key = Item::items[id]->getDescriptionId();
		return StatCollector::translate(key + u".name");
	}

	bool hasItem(int_t id)
	{
		if (id >= 0 && id < static_cast<int_t>(Tile::tiles.size()) && Tile::tiles[id] != nullptr)
			return true;
		return id >= 0 && id < static_cast<int_t>(Item::items.size()) && Item::items[id] != nullptr;
	}

	bool blockEnablesStats(int_t id)
	{
		switch (id)
		{
			case 7: case 8: case 9: case 10: case 11: case 18: case 26: case 51: case 52:
			case 55: case 59: case 63: case 64: case 68: case 71: case 83: case 92:
			case 93: case 94: case 96:
				return false;
			default:
				return true;
		}
	}

	template<typename T>
	void removeValue(std::vector<T *> &values, T *value)
	{
		values.erase(std::remove(values.begin(), values.end(), value), values.end());
	}
}

StatBase *StatList::addBasic(int_t id, const jstring &name, const StatFormatter *formatter, bool independent)
{
	std::unique_ptr<StatBasic> stat;
	if (formatter == nullptr)
		stat = std::make_unique<StatBasic>(id, name);
	else
		stat = std::make_unique<StatBasic>(id, name, *formatter);
	if (independent)
		stat->markIndependent();
	StatBase *result = &stat->registerStat();
	ownedStats.push_back(std::move(stat));
	return result;
}

StatCrafting *StatList::addCrafting(int_t id, const jstring &name, int_t itemId)
{
	auto stat = std::make_unique<StatCrafting>(id, name, itemId);
	auto *result = static_cast<StatCrafting *>(&stat->registerStat());
	ownedStats.push_back(std::move(stat));
	return result;
}

void StatList::init()
{
	if (initialized)
		return;
	initialized = true;

	startGameStat = addBasic(1000, StatCollector::translate(u"stat.startGame"), nullptr, true);
	createWorldStat = addBasic(1001, StatCollector::translate(u"stat.createWorld"), nullptr, true);
	loadWorldStat = addBasic(1002, StatCollector::translate(u"stat.loadWorld"), nullptr, true);
	joinMultiplayerStat = addBasic(1003, StatCollector::translate(u"stat.joinMultiplayer"), nullptr, true);
	leaveGameStat = addBasic(1004, StatCollector::translate(u"stat.leaveGame"), nullptr, true);
	minutesPlayedStat = addBasic(1100, StatCollector::translate(u"stat.playOneMinute"), &StatBase::timeFormatter(), true);
	distanceWalkedStat = addBasic(2000, StatCollector::translate(u"stat.walkOneCm"), &StatBase::distanceFormatter(), true);
	distanceSwumStat = addBasic(2001, StatCollector::translate(u"stat.swimOneCm"), &StatBase::distanceFormatter(), true);
	distanceFallenStat = addBasic(2002, StatCollector::translate(u"stat.fallOneCm"), &StatBase::distanceFormatter(), true);
	distanceClimbedStat = addBasic(2003, StatCollector::translate(u"stat.climbOneCm"), &StatBase::distanceFormatter(), true);
	distanceFlownStat = addBasic(2004, StatCollector::translate(u"stat.flyOneCm"), &StatBase::distanceFormatter(), true);
	distanceDoveStat = addBasic(2005, StatCollector::translate(u"stat.diveOneCm"), &StatBase::distanceFormatter(), true);
	distanceByMinecartStat = addBasic(2006, StatCollector::translate(u"stat.minecartOneCm"), &StatBase::distanceFormatter(), true);
	distanceByBoatStat = addBasic(2007, StatCollector::translate(u"stat.boatOneCm"), &StatBase::distanceFormatter(), true);
	distanceByPigStat = addBasic(2008, StatCollector::translate(u"stat.pigOneCm"), &StatBase::distanceFormatter(), true);
	jumpStat = addBasic(2010, StatCollector::translate(u"stat.jump"), nullptr, true);
	dropStat = addBasic(2011, StatCollector::translate(u"stat.drop"), nullptr, true);
	damageDealtStat = addBasic(2020, StatCollector::translate(u"stat.damageDealt"), nullptr, false);
	damageTakenStat = addBasic(2021, StatCollector::translate(u"stat.damageTaken"), nullptr, false);
	deathsStat = addBasic(2022, StatCollector::translate(u"stat.deaths"), nullptr, false);
	mobKillsStat = addBasic(2023, StatCollector::translate(u"stat.mobKills"), nullptr, false);
	playerKillsStat = addBasic(2024, StatCollector::translate(u"stat.playerKills"), nullptr, false);
	fishCaughtStat = addBasic(2025, StatCollector::translate(u"stat.fishCaught"), nullptr, false);

	initializeMineBlockStats();
	AchievementList::init();
	initializeUseStats(0, static_cast<int_t>(Tile::tiles.size()));
	initializeBreakStats(0, static_cast<int_t>(Tile::tiles.size()));
	initializeUseStats(static_cast<int_t>(Tile::tiles.size()), static_cast<int_t>(Item::items.size()));
	initializeBreakStats(static_cast<int_t>(Tile::tiles.size()), static_cast<int_t>(Item::items.size()));
	initializeCraftStats();
}

void StatList::initializeMineBlockStats()
{
	for (int_t id = 0; id < static_cast<int_t>(mineBlockStats.size()); ++id)
	{
		if (Tile::tiles[id] == nullptr || !blockEnablesStats(id))
			continue;
		jstring name = StatCollector::translate(u"stat.mineBlock", getStatName(id));
		auto *stat = addCrafting(16777216 + id, name, id);
		mineBlockStats[id] = stat;
		blockStats.push_back(stat);
	}
	replaceAllSimilarBlocks(mineBlockStats.data());
}

void StatList::initializeUseStats(int_t first, int_t last)
{
	for (int_t id = first; id < last; ++id)
	{
		if (!hasItem(id))
			continue;
		jstring name = StatCollector::translate(u"stat.useItem", getStatName(id));
		auto *stat = addCrafting(16908288 + id, name, id);
		useItemStats[id] = stat;
		if (id >= static_cast<int_t>(Tile::tiles.size()))
			itemStats.push_back(stat);
	}
	replaceAllSimilarBlocks(useItemStats.data());
}

void StatList::initializeBreakStats(int_t first, int_t last)
{
	for (int_t id = first; id < last; ++id)
	{
		if (id < 0 || id >= static_cast<int_t>(Item::items.size()) || Item::items[id] == nullptr || Item::items[id]->getMaxDamage() <= 0)
			continue;
		jstring name = StatCollector::translate(u"stat.breakItem", getStatName(id));
		breakItemStats[id] = addCrafting(16973824 + id, name, id);
	}
	replaceAllSimilarBlocks(breakItemStats.data());
}

void StatList::initializeCraftStats()
{
	std::unordered_set<int_t> craftedIds;
	for (const auto &recipe : Recipes::getInstance().getRecipes())
		craftedIds.insert(recipe.result.itemID);
	for (const auto &entry : FurnaceRecipes::getInstance().getRecipes())
		craftedIds.insert(entry.second.itemID);

	for (int_t id : craftedIds)
	{
		if (!hasItem(id))
			continue;
		jstring name = StatCollector::translate(u"stat.craftItem", getStatName(id));
		craftItemStats[id] = addCrafting(16842752 + id, name, id);
	}
	replaceAllSimilarBlocks(craftItemStats.data());
}

void StatList::replaceAllSimilarBlocks(StatBase **stats)
{
	replaceSimilarBlocks(stats, 9, 8);
	replaceSimilarBlocks(stats, 11, 11);
	replaceSimilarBlocks(stats, 91, 86);
	replaceSimilarBlocks(stats, 62, 61);
	replaceSimilarBlocks(stats, 74, 73);
	replaceSimilarBlocks(stats, 94, 93);
	replaceSimilarBlocks(stats, 76, 75);
	replaceSimilarBlocks(stats, 40, 39);
	replaceSimilarBlocks(stats, 43, 44);
	replaceSimilarBlocks(stats, 2, 3);
	replaceSimilarBlocks(stats, 60, 3);
}

void StatList::replaceSimilarBlocks(StatBase **stats, int_t first, int_t second)
{
	if (stats[first] != nullptr && stats[second] == nullptr)
	{
		stats[second] = stats[first];
	}
	else
	{
		removeValue(allStats, stats[first]);
		removeValue(blockStats, static_cast<StatCrafting *>(stats[first]));
		removeValue(generalStats, stats[first]);
		stats[first] = stats[second];
	}
}

void StatList::registerStat(StatBase &stat)
{
	auto it = statsById.find(stat.statId);
	if (it != statsById.end())
	{
		throw std::runtime_error(
			"Duplicate stat id: \"" + String::toUTF8(it->second->statName) + "\" and \"" +
			String::toUTF8(stat.statName) + "\" at id " + std::to_string(stat.statId));
	}

	allStats.push_back(&stat);
	statsById.emplace(stat.statId, &stat);
}

StatBase *StatList::getStat(int_t id)
{
	auto it = statsById.find(id);
	return it == statsById.end() ? nullptr : it->second;
}

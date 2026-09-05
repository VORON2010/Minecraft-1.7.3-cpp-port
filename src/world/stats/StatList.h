#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include "java/String.h"
#include "java/Type.h"

class StatBase;
class StatCrafting;

class StatList
{
private:
	static std::unordered_map<int_t, StatBase *> statsById;
	static std::vector<std::unique_ptr<StatBase>> ownedStats;
	static bool initialized;

	static StatBase *addBasic(int_t id, const jstring &name, const class StatFormatter *formatter, bool independent);
	static StatCrafting *addCrafting(int_t id, const jstring &name, int_t itemId);
	static void initializeMineBlockStats();
	static void initializeUseStats(int_t first, int_t last);
	static void initializeBreakStats(int_t first, int_t last);
	static void initializeCraftStats();
	static void replaceAllSimilarBlocks(StatBase **stats);
	static void replaceSimilarBlocks(StatBase **stats, int_t first, int_t second);

public:
	static std::vector<StatBase *> allStats;
	static std::vector<StatBase *> generalStats;
	static std::vector<StatCrafting *> itemStats;
	static std::vector<StatCrafting *> blockStats;

	static StatBase *startGameStat;
	static StatBase *createWorldStat;
	static StatBase *loadWorldStat;
	static StatBase *joinMultiplayerStat;
	static StatBase *leaveGameStat;
	static StatBase *minutesPlayedStat;
	static StatBase *distanceWalkedStat;
	static StatBase *distanceSwumStat;
	static StatBase *distanceFallenStat;
	static StatBase *distanceClimbedStat;
	static StatBase *distanceFlownStat;
	static StatBase *distanceDoveStat;
	static StatBase *distanceByMinecartStat;
	static StatBase *distanceByBoatStat;
	static StatBase *distanceByPigStat;
	static StatBase *jumpStat;
	static StatBase *dropStat;
	static StatBase *damageDealtStat;
	static StatBase *damageTakenStat;
	static StatBase *deathsStat;
	static StatBase *mobKillsStat;
	static StatBase *playerKillsStat;
	static StatBase *fishCaughtStat;

	static std::array<StatBase *, 256> mineBlockStats;
	static std::array<StatBase *, 32000> craftItemStats;
	static std::array<StatBase *, 32000> useItemStats;
	static std::array<StatBase *, 32000> breakItemStats;

	static void init();
	static void registerStat(StatBase &stat);
	static StatBase *getStat(int_t id);
};

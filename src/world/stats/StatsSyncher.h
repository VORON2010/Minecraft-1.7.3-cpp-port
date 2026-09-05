#pragma once

#include <atomic>
#include <future>
#include <memory>

#include "world/stats/StatFileWriter.h"

class File;
class User;

class StatsSyncher
{
private:
	StatFileWriter &writer;
	jstring username;

	std::unique_ptr<File> unsentFile;
	std::unique_ptr<File> statsFile;
	std::unique_ptr<File> unsentOldFile;
	std::unique_ptr<File> statsOldFile;
	std::unique_ptr<File> unsentTempFile;
	std::unique_ptr<File> statsTempFile;

	std::atomic<bool> busy{false};
	int_t saveDelay = 0;
	int_t unusedDelay = 0;
	std::future<std::unique_ptr<StatMap>> receiveFuture;
	std::future<void> sendFuture;
	bool receiving = false;
	bool sending = false;

	static void migrateCaseVariant(const File &directory, const jstring &oldName, const File &destination);
	static std::unique_ptr<StatMap> readMap(const File &primary, const File &temporary, const File &backup);
	void writeMap(const StatMap &stats, const File &primary, const File &temporary, const File &backup) const;
	void startReceive();

public:
	StatsSyncher(const User &user, StatFileWriter &writer, const File &directory);
	~StatsSyncher();

	bool canSave() const;
	void startSave(const StatMap &stats);
	void syncStatsFileWithMap(const StatMap &stats);
	void tick();
};

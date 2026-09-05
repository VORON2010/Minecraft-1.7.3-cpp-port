#include "world/stats/StatsSyncher.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "client/User.h"
#include "java/File.h"

namespace
{
	jstring lowerCase(const jstring &value)
	{
		jstring result = value;
		std::transform(result.begin(), result.end(), result.begin(), [](char_t c)
		{
			return c >= u'A' && c <= u'Z' ? static_cast<char_t>(c + (u'a' - u'A')) : c;
		});
		return result;
	}
}

StatsSyncher::StatsSyncher(const User &user, StatFileWriter &writer, const File &directory)
	: writer(writer), username(user.name)
{
	jstring lowerName = lowerCase(username);
	unsentFile.reset(File::open(directory, u"stats_" + lowerName + u"_unsent.dat"));
	statsFile.reset(File::open(directory, u"stats_" + lowerName + u".dat"));
	unsentOldFile.reset(File::open(directory, u"stats_" + lowerName + u"_unsent.old"));
	statsOldFile.reset(File::open(directory, u"stats_" + lowerName + u".old"));
	unsentTempFile.reset(File::open(directory, u"stats_" + lowerName + u"_unsent.tmp"));
	statsTempFile.reset(File::open(directory, u"stats_" + lowerName + u".tmp"));

	if (lowerName != username)
	{
		migrateCaseVariant(directory, u"stats_" + username + u"_unsent.dat", *unsentFile);
		migrateCaseVariant(directory, u"stats_" + username + u".dat", *statsFile);
		migrateCaseVariant(directory, u"stats_" + username + u"_unsent.old", *unsentOldFile);
		migrateCaseVariant(directory, u"stats_" + username + u".old", *statsOldFile);
		migrateCaseVariant(directory, u"stats_" + username + u"_unsent.tmp", *unsentTempFile);
		migrateCaseVariant(directory, u"stats_" + username + u".tmp", *statsTempFile);
	}

	if (unsentFile->exists())
	{
		auto stats = readMap(*unsentFile, *unsentTempFile, *unsentOldFile);
		if (stats)
			writer.mergeUnsent(*stats);
	}
	startReceive();
}

StatsSyncher::~StatsSyncher()
{
	if (receiving)
		receiveFuture.wait();
	if (sending)
		sendFuture.wait();
}

void StatsSyncher::migrateCaseVariant(const File &directory, const jstring &oldName, const File &destination)
{
	std::unique_ptr<File> oldFile(File::open(directory, oldName));
	if (oldFile->exists() && !oldFile->isDirectory() && !destination.exists())
		oldFile->renameTo(destination);
}

std::unique_ptr<StatMap> StatsSyncher::readMap(const File &primary, const File &temporary, const File &backup)
{
	const File *source = primary.exists() ? &primary : (backup.exists() ? &backup : (temporary.exists() ? &temporary : nullptr));
	if (source == nullptr)
		return nullptr;

	try
	{
		std::unique_ptr<std::istream> stream(source->toStreamIn());
		if (stream == nullptr)
			throw std::runtime_error("Unable to open stats file for reading");
		std::string json;
		for (char c; stream->get(c);)
			if (c != '\r' && c != '\n')
				json.push_back(c);
		return StatFileWriter::parse(json);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return nullptr;
	}
}

void StatsSyncher::writeMap(const StatMap &stats, const File &primary, const File &temporary, const File &backup) const
{
	std::unique_ptr<std::ostream> stream(temporary.toStreamOut());
	if (stream == nullptr)
		throw std::runtime_error("Unable to open temporary stats file for writing");
	jstring local = u"local";
	*stream << StatFileWriter::serialize(&username, &local, stats);
	stream.reset();

	if (backup.exists())
		backup.remove();
	if (primary.exists())
		primary.renameTo(backup);
	temporary.renameTo(primary);
}

void StatsSyncher::startReceive()
{
	if (busy)
		throw std::runtime_error("Can't get stats from server while StatsSyncher is busy!");
	saveDelay = 100;
	busy = true;
	receiving = true;
	receiveFuture = std::async(std::launch::async, [this]
	{
		std::unique_ptr<StatMap> stats;
		try
		{
			if (statsFile->exists())
				stats = readMap(*statsFile, *statsTempFile, *statsOldFile);
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
		busy = false;
		return stats;
	});
}

bool StatsSyncher::canSave() const
{
	return saveDelay <= 0 && !busy;
}

void StatsSyncher::startSave(const StatMap &stats)
{
	if (busy)
		throw std::runtime_error("Can't save stats while StatsSyncher is busy!");
	saveDelay = 100;
	busy = true;
	sending = true;
	sendFuture = std::async(std::launch::async, [this, stats]
	{
		try
		{
			writeMap(stats, *unsentFile, *unsentTempFile, *unsentOldFile);
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
		busy = false;
	});
}

void StatsSyncher::syncStatsFileWithMap(const StatMap &stats)
{
	int_t attempts = 30;
	while (busy && --attempts > 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	busy = true;
	try
	{
		writeMap(stats, *unsentFile, *unsentTempFile, *unsentOldFile);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	busy = false;
}

void StatsSyncher::tick()
{
	if (saveDelay > 0)
		--saveDelay;
	if (unusedDelay > 0)
		--unusedDelay;

	if (receiving && receiveFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		auto stats = receiveFuture.get();
		if (stats)
			writer.mergeLoadedTotal(*stats);
		receiving = false;
	}
	if (sending && sendFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		sendFuture.get();
		sending = false;
	}
}

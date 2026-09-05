#include "world/level/chunk/storage/McRegionChunkStorage.h"

#include "world/level/chunk/storage/OldChunkStorage.h"
#include "world/level/chunk/storage/RegionFileCache.h"
#include "world/level/chunk/storage/RegionFile.h"
#include "world/level/chunk/LevelChunk.h"
#include "world/level/Level.h"

#include "nbt/NbtIo.h"
#include "nbt/CompoundTag.h"

#include "java/File.h"
#include "java/String.h"

#include "util/Memory.h"

#include <sstream>
#include <iostream>

#include "zlib.h"


namespace {

class MemBuf : public std::basic_streambuf<char>
{
public:
	MemBuf(const char *p, size_t l)
	{
		setg(const_cast<char *>(p), const_cast<char *>(p), const_cast<char *>(p) + l);
	}
};

class MemStream : public std::istream
{
public:
	MemStream(const char *p, size_t l) : std::istream(&buffer), buffer(p, l)
	{
		rdbuf(&buffer);
	}
private:
	MemBuf buffer;
};

} 

long_t McRegionChunkStorage::chunkKey(int_t x, int_t z)
{
	return (static_cast<long_t>(x) << 32) ^ (static_cast<long_t>(z) & 0xFFFFFFFFLL);
}

McRegionChunkStorage::McRegionChunkStorage(std::shared_ptr<File> dir, bool create)
{
	baseDir = String::toUTF8(dir->toString());

	if (create)
	{
		if (!dir->exists())
			dir->mkdirs();
	}

	worker = std::thread(&McRegionChunkStorage::writeLoop, this);
}

McRegionChunkStorage::~McRegionChunkStorage()
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		stopping = true;
	}
	wake.notify_one();
	worker.join();
}

std::shared_ptr<LevelChunk> McRegionChunkStorage::load(Level &level, int_t x, int_t z)
{
	
	std::shared_ptr<const std::string> queuedNbt;
	{
		std::lock_guard<std::mutex> lock(mutex);
		auto it = pending.find(chunkKey(x, z));
		if (it != pending.end())
			queuedNbt = it->second.nbt;
	}

	std::vector<byte_t> data;
	if (queuedNbt != nullptr)
		data.assign(reinterpret_cast<const byte_t *>(queuedNbt->data()), reinterpret_cast<const byte_t *>(queuedNbt->data()) + queuedNbt->size());
	else
		data = RegionFileCache::getRegionFile(baseDir, x, z)->getChunkData(x & 31, z & 31);
	if (data.empty())
		return nullptr;

	
	MemStream ms(reinterpret_cast<const char *>(data.data()), data.size());
	std::unique_ptr<CompoundTag> rootTag(NbtIo::read(ms));

	if (!rootTag->contains(u"Level"))
	{
		std::cout << "Chunk file at " << x << "," << z << " is missing level data, skipping\n";
		return nullptr;
	}
	if (!rootTag->getCompound(u"Level")->contains(u"Blocks"))
	{
		std::cout << "Chunk file at " << x << "," << z << " is missing block data, skipping\n";
		return nullptr;
	}

	
	std::shared_ptr<LevelChunk> chunk = OldChunkStorage::load(level, *rootTag->getCompound(u"Level"));
	if (!chunk->isAt(x, z))
	{
		std::cout << "Chunk file at " << x << "," << z << " is in the wrong location; relocating. (Expected "
		          << x << ", " << z << ", got " << chunk->x << ", " << chunk->z << ")\n";
		rootTag->getCompound(u"Level")->putInt(u"xPos", x);
		rootTag->getCompound(u"Level")->putInt(u"zPos", z);
		chunk = OldChunkStorage::load(level, *rootTag->getCompound(u"Level"));
	}

	return chunk;
}

void McRegionChunkStorage::save(Level &level, LevelChunk &chunk)
{
	level.checkSession();

	
	std::unique_ptr<CompoundTag> rootTag = std::make_unique<CompoundTag>();
	std::shared_ptr<CompoundTag> levelTag = Util::make_shared<CompoundTag>();
	rootTag->put(u"Level", levelTag);

	
	OldChunkStorage::save(chunk, level, *levelTag);

	
	std::stringstream ss;
	NbtIo::write(*rootTag, ss);
	auto nbt = std::make_shared<const std::string>(ss.str());

	{
		std::lock_guard<std::mutex> lock(mutex);
		long_t key = chunkKey(chunk.x, chunk.z);
		PendingWrite &entry = pending[key];
		entry.nbt = nbt;
		entry.sequence = ++nextSequence;
		if (!entry.queued)
		{
			entry.queued = true;
			queue.push_back(key);
		}
		
		level.sizeOnDisk += sizeDelta;
		sizeDelta = 0;
		sizeOwner = &level;
	}
	wake.notify_one();
}

void McRegionChunkStorage::writeChunk(int_t x, int_t z, const std::string &nbt)
{
	
	uLongf compBound = compressBound(static_cast<uLong>(nbt.size()));
	std::vector<byte_t> compressed(compBound);
	uLongf compSize = compBound;

	int ret = compress2(
		reinterpret_cast<Bytef *>(compressed.data()), &compSize,
		reinterpret_cast<const Bytef *>(nbt.data()), static_cast<uLong>(nbt.size()),
		Z_DEFAULT_COMPRESSION
	);

	if (ret != Z_OK)
	{
		std::cerr << "Failed to compress chunk at " << x << "," << z << std::endl;
		return;
	}

	
	auto regionFile = RegionFileCache::getRegionFile(baseDir, x, z);
	regionFile->writeChunkData(x & 31, z & 31, compressed.data(), static_cast<int_t>(compSize));

	int_t delta = regionFile->getSizeDelta();
	std::lock_guard<std::mutex> lock(mutex);
	sizeDelta += delta;
}

void McRegionChunkStorage::writeLoop()
{
	std::unique_lock<std::mutex> lock(mutex);
	for (;;)
	{
		wake.wait(lock, [this] { return stopping || !queue.empty(); });
		if (queue.empty())
			return;

		long_t key = queue.front();
		queue.pop_front();
		PendingWrite &entry = pending[key];
		entry.queued = false;
		std::shared_ptr<const std::string> nbt = entry.nbt;
		std::uint64_t sequence = entry.sequence;
		writing = true;
		lock.unlock();

		int_t x = static_cast<int_t>(key >> 32);
		int_t z = static_cast<int_t>(key & 0xFFFFFFFFLL);
		try
		{
			writeChunk(x, z, *nbt);
		}
		catch (const std::exception &error)
		{
			std::cerr << "Failed to write chunk at " << x << "," << z << ": " << error.what() << std::endl;
		}

		lock.lock();
		writing = false;
		auto it = pending.find(key);
		if (it != pending.end() && it->second.sequence == sequence && !it->second.queued)
			pending.erase(it);
		if (queue.empty())
			drained.notify_all();
	}
}

void McRegionChunkStorage::saveEntities(Level &level, LevelChunk &chunk)
{
	
}

void McRegionChunkStorage::tick()
{
	
}

void McRegionChunkStorage::flush()
{
	{
		std::unique_lock<std::mutex> lock(mutex);
		drained.wait(lock, [this] { return queue.empty() && !writing; });
		if (sizeOwner != nullptr)
		{
			sizeOwner->sizeOnDisk += sizeDelta;
			sizeDelta = 0;
		}
	}
	RegionFileCache::clearCache();
}

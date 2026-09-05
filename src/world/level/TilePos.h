#pragma once

#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

#include "java/Type.h"

struct TilePos
{
	int_t x, y, z;

	TilePos(int_t x, int_t y, int_t z) : x(x), y(y), z(z) {}

	bool operator==(const TilePos &other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}
};

class JavaTilePosSet
{
private:
	struct Entry
	{
		uint_t hash;
		TilePos value;
		int_t next;
	};

	std::vector<Entry> entries;
	std::vector<int_t> bucketHeads = std::vector<int_t>(16, -1);
	int_t resizeThreshold = 12;
	int_t entryCount = 0;
	int_t freeEntry = -1;

	static uint_t hash(const TilePos &position)
	{
		uint_t value = static_cast<uint_t>(position.x) * 8976890U +
			static_cast<uint_t>(position.y) * 981131U + static_cast<uint_t>(position.z);
		value ^= (value >> 20) ^ (value >> 12);
		return value ^ (value >> 7) ^ (value >> 4);
	}

	int_t findEntry(const TilePos &position, uint_t positionHash) const
	{
		int_t entryIndex = bucketHeads[positionHash & (bucketHeads.size() - 1)];
		while (entryIndex >= 0)
		{
			const Entry &entry = entries[static_cast<size_t>(entryIndex)];
			if (entry.hash == positionHash && entry.value == position)
				return entryIndex;
			entryIndex = entry.next;
		}
		return -1;
	}

	void resize(int_t capacity)
	{
		std::vector<int_t> newBucketHeads(static_cast<size_t>(capacity), -1);
		for (int_t bucketHead : bucketHeads)
		{
			int_t entryIndex = bucketHead;
			while (entryIndex >= 0)
			{
				Entry &entry = entries[static_cast<size_t>(entryIndex)];
				int_t next = entry.next;
				int_t bucket = static_cast<int_t>(entry.hash & (newBucketHeads.size() - 1));
				entry.next = newBucketHeads[static_cast<size_t>(bucket)];
				newBucketHeads[static_cast<size_t>(bucket)] = entryIndex;
				entryIndex = next;
			}
		}
		bucketHeads = std::move(newBucketHeads);
		resizeThreshold = capacity * 3 / 4;
	}

public:
	class const_iterator
	{
		friend class JavaTilePosSet;

	private:
		const JavaTilePosSet *set = nullptr;
		size_t bucket = 0;
		int_t entryIndex = -1;

		const_iterator(const JavaTilePosSet *set, size_t bucket, int_t entryIndex)
			: set(set), bucket(bucket), entryIndex(entryIndex) {}

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = TilePos;
		using difference_type = std::ptrdiff_t;
		using pointer = const TilePos *;
		using reference = const TilePos &;

		reference operator*() const
		{
			return set->entries[static_cast<size_t>(entryIndex)].value;
		}

		pointer operator->() const
		{
			return &set->entries[static_cast<size_t>(entryIndex)].value;
		}

		const_iterator &operator++()
		{
			int_t next = set->entries[static_cast<size_t>(entryIndex)].next;
			if (next >= 0)
			{
				entryIndex = next;
				return *this;
			}
			while (++bucket < set->bucketHeads.size())
			{
				if (set->bucketHeads[bucket] >= 0)
				{
					entryIndex = set->bucketHeads[bucket];
					return *this;
				}
			}
			entryIndex = -1;
			return *this;
		}

		const_iterator operator++(int)
		{
			const_iterator previous = *this;
			++*this;
			return previous;
		}

		bool operator==(const const_iterator &other) const
		{
			return set == other.set && bucket == other.bucket && entryIndex == other.entryIndex;
		}

		bool operator!=(const const_iterator &other) const
		{
			return !(*this == other);
		}
	};

	void clear()
	{
		entries.clear();
		bucketHeads.assign(bucketHeads.size(), -1);
		entryCount = 0;
		freeEntry = -1;
	}

	bool emplace(int_t x, int_t y, int_t z)
	{
		return insert(TilePos(x, y, z), hash(TilePos(x, y, z)));
	}

	bool emplaceChunk(int_t x, int_t z)
	{
		uint_t value = (x < 0 ? 0x80000000U : 0U) | ((static_cast<uint_t>(x) & 32767U) << 16) |
			(z < 0 ? 32768U : 0U) | (static_cast<uint_t>(z) & 32767U);
		value ^= (value >> 20) ^ (value >> 12);
		return insert(TilePos(x, 0, z), value ^ (value >> 7) ^ (value >> 4));
	}

private:
	bool insert(const TilePos &position, uint_t positionHash)
	{
		if (findEntry(position, positionHash) >= 0)
			return false;
		int_t bucket = static_cast<int_t>(positionHash & (bucketHeads.size() - 1));
		int_t sizeBefore = entryCount++;
		int_t index = freeEntry;
		if (index >= 0)
		{
			freeEntry = entries[static_cast<size_t>(index)].next;
			entries[static_cast<size_t>(index)] = {positionHash, position, bucketHeads[static_cast<size_t>(bucket)]};
		}
		else
		{
			index = static_cast<int_t>(entries.size());
			entries.push_back({positionHash, position, bucketHeads[static_cast<size_t>(bucket)]});
		}
		bucketHeads[static_cast<size_t>(bucket)] = index;
		if (sizeBefore >= resizeThreshold)
			resize(static_cast<int_t>(bucketHeads.size() * 2));
		return true;
	}

public:
	bool erase(const TilePos &position)
	{
		uint_t positionHash = hash(position);
		int_t *link = &bucketHeads[positionHash & (bucketHeads.size() - 1)];
		while (*link >= 0)
		{
			int_t index = *link;
			Entry &entry = entries[static_cast<size_t>(index)];
			if (entry.hash == positionHash && entry.value == position)
			{
				*link = entry.next;
				entry.next = freeEntry;
				freeEntry = index;
				--entryCount;
				return true;
			}
			link = &entry.next;
		}
		return false;
	}

	size_t size() const
	{
		return static_cast<size_t>(entryCount);
	}

	const_iterator begin() const
	{
		for (size_t bucket = 0; bucket < bucketHeads.size(); ++bucket)
		{
			if (bucketHeads[bucket] >= 0)
				return const_iterator(this, bucket, bucketHeads[bucket]);
		}
		return end();
	}

	const_iterator end() const
	{
		return const_iterator(this, bucketHeads.size(), -1);
	}
};

template <>
struct std::hash<TilePos>
{
	size_t operator()(const TilePos &k) const
	{
		uint_t hash = static_cast<uint_t>(k.x) * 8976890U +
			static_cast<uint_t>(k.y) * 981131U + static_cast<uint_t>(k.z);
		return static_cast<size_t>(hash);
	}
};

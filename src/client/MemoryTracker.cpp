#include "MemoryTracker.h"


std::vector<int_t> MemoryTracker::lists;
std::vector<int_t> MemoryTracker::textures;

int_t MemoryTracker::genLists(int_t count)
{
	static int_t list_counter = 1;
	int_t id = list_counter;
	list_counter += count;
	lists.push_back(id);
	lists.push_back(count);
	return id;
}

void MemoryTracker::genTextures(std::vector<int_t> &ib)
{
	static int_t tex_counter = 1;
	for (size_t i = 0; i < ib.size(); i++) {
		ib[i] = tex_counter++;
		textures.push_back(ib[i]);
	}
}

void MemoryTracker::release()
{
	lists.clear();
	textures.clear();
}

std::vector<byte_t> MemoryTracker::createByteBuffer(int_t size)
{
	std::vector<byte_t> buffer(size);
	return buffer;
}
std::vector<short_t> MemoryTracker::createShortBuffer(int_t size)
{
	std::vector<short_t> buffer(size);
	return buffer;
}
std::vector<char_t> MemoryTracker::createCharBuffer(int_t size)
{
	std::vector<char_t> buffer(size);
	return buffer;
}
std::vector<int_t> MemoryTracker::createIntBuffer(int_t size)
{
	std::vector<int_t> buffer(size);
	return buffer;
}
std::vector<long_t> MemoryTracker::createLongBuffer(int_t size)
{
	std::vector<long_t> buffer(size);
	return buffer;
}
std::vector<float> MemoryTracker::createFloatBuffer(int_t size)
{
	std::vector<float> buffer(size);
	return buffer;
}
std::vector<double> MemoryTracker::createDoubleBuffer(int_t size)
{
	std::vector<double> buffer(size);
	return buffer;
}

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iosfwd>

namespace Profiler
{
enum class Section
{
	Frame, Tick, EntityTick, Lighting, ChunkGeneration, ChunkRebuildCapture,
	ChunkTessellation, ChunkPublication, TerrainRender, EntityRender, Particles,
	Gui, Sound, IO, Count
};

void enable(bool enabled);
bool isEnabled();

void reset(std::size_t frames);
void beginFrame(std::size_t frame);
void report(std::ostream &raw, std::ostream &summary);

class Scope
{
	Section section;
	bool active;
	std::size_t frame = 0;
	std::uint64_t generation = 0;
	std::chrono::steady_clock::time_point start;

public:
	explicit Scope(Section section);
	~Scope();
	void finish();
	Scope(const Scope &) = delete;
	Scope &operator=(const Scope &) = delete;
};
}

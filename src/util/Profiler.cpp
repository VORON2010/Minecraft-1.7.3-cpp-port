#include "util/Profiler.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <iomanip>
#include <mutex>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace Profiler
{
struct Measurement
{
	std::uint64_t calls = 0;
	std::uint64_t nanos = 0;
};
using Frame = std::array<Measurement, static_cast<std::size_t>(Section::Count)>;
static std::atomic<bool> enabled{false};
static std::atomic<std::size_t> currentFrame{0};
static std::atomic<std::uint64_t> generation{0};
static std::mutex mutex;
static std::vector<Frame> frames;
static const char *names[] = {
	"frame", "tick", "entity_tick", "lighting", "chunk_generation", "chunk_rebuild_capture",
	"chunk_tessellation", "chunk_publication", "terrain_render", "entity_render", "particles",
	"gui", "sound", "io"
};

void enable(bool value)
{
	enabled.store(value, std::memory_order_release);
}

bool isEnabled()
{
	return enabled.load(std::memory_order_acquire);
}

void reset(std::size_t count)
{
	std::lock_guard<std::mutex> lock(mutex);
	enabled.store(false, std::memory_order_release);
	generation.fetch_add(1, std::memory_order_relaxed);
	frames.assign(count, Frame{});
	currentFrame.store(0, std::memory_order_relaxed);
}

void beginFrame(std::size_t frame)
{
	currentFrame.store(frame, std::memory_order_relaxed);
}

Scope::Scope(Section value) : section(value), active(isEnabled())
{
	if (active)
	{
		frame = currentFrame.load(std::memory_order_relaxed);
		generation = Profiler::generation.load(std::memory_order_relaxed);
		start = std::chrono::steady_clock::now();
	}
}

Scope::~Scope()
{
	finish();
}

void Scope::finish()
{
	if (!active)
		return;
	active = false;
	const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::steady_clock::now() - start).count();
	std::lock_guard<std::mutex> lock(mutex);
	if (generation != Profiler::generation.load(std::memory_order_relaxed) || frame >= frames.size())
		return;
	auto &measurement = frames[frame][static_cast<std::size_t>(section)];
	++measurement.calls;
	measurement.nanos += static_cast<std::uint64_t>(elapsed);
}

void report(std::ostream &raw, std::ostream &summary)
{
	std::lock_guard<std::mutex> lock(mutex);
	raw << "frame,section,status,calls,total_ms\n" << std::setprecision(9);
	summary << "section,status,measured_frames,total_frames,calls,median_ms,p95_ms,total_ms\n" << std::setprecision(9);
	for (std::size_t section = 0; section < static_cast<std::size_t>(Section::Count); ++section)
	{
		std::vector<double> samples;
		samples.reserve(frames.size());
		std::uint64_t calls = 0;
		double total = 0;
		for (std::size_t frame = 0; frame < frames.size(); ++frame)
		{
			const auto &m = frames[frame][section];
			raw << frame << ',' << names[section] << ',';
			if (m.calls == 0)
			{
				raw << "NOT_MEASURED,0,\n";
				continue;
			}
			const double ms = static_cast<double>(m.nanos) / 1000000.0;
			raw << "MEASURED," << m.calls << ',' << ms << '\n';
			samples.push_back(ms);
			calls += m.calls;
			total += ms;
		}
		summary << names[section] << ',';
		if (samples.empty())
		{
			summary << "NOT_MEASURED,0," << frames.size() << ",0,,,\n";
			continue;
		}
		std::sort(samples.begin(), samples.end());
		const std::size_t middle = samples.size() / 2;
		const double median = samples.size() % 2 ? samples[middle] : (samples[middle - 1] + samples[middle]) / 2.0;
		const std::size_t p95 = samples.size() - samples.size() / 20 - 1;
		summary << "MEASURED," << samples.size() << ',' << frames.size() << ',' << calls << ','
			<< median << ',' << samples[p95] << ',' << total << '\n';
	}
	if (!raw || !summary)
		throw std::runtime_error("Failed to write stress profiling CSV");
}
}

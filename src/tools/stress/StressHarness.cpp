
#if defined(_WIN32) && !defined(NOMINMAX)
#define NOMINMAX
#endif

#include "tools/stress/StressHarness.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <stdexcept>

#include "client/Minecraft.h"
#include "client/User.h"
#include "client/gamemode/SurvivalMode.h"
#include "client/gui/AchievementToast.h"
#include "client/renderer/Chunk.h"
#include "java/File.h"
#include "java/String.h"
#include "lwjgl/Display.h"
#include "lwjgl/GLContext.h"
#include "util/Profiler.h"
#include "world/phys/AABB.h"
#include "world/phys/Vec3.h"
#include "OpenGL.h"
#include "stb_image_write.h"

namespace stress
{
long_t parseInteger(const std::string &text)
{
	const std::size_t first = !text.empty() && (text[0] == '-' || text[0] == '+') ? 1 : 0;
	if (first == text.size() || text.find_first_not_of("0123456789", first) != std::string::npos)
		throw std::invalid_argument("Invalid integer: " + text);
	return std::stoll(text);
}

bool Params::has(const std::string &key) const
{
	return values.find(key) != values.end();
}

long_t Params::longOr(const std::string &key, long_t fallback) const
{
	const auto it = values.find(key);
	return it == values.end() ? fallback : parseInteger(it->second);
}

int Params::intOr(const std::string &key, int fallback) const
{
	const long_t value = longOr(key, fallback);
	if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
		throw std::invalid_argument("Integer out of range: --" + key);
	return static_cast<int>(value);
}

double Params::doubleOr(const std::string &key, double fallback) const
{
	const auto it = values.find(key);
	if (it == values.end())
		return fallback;
	std::size_t used = 0;
	const double value = std::stod(it->second, &used);
	if (used != it->second.size() || !std::isfinite(value))
		throw std::invalid_argument("Invalid finite number: --" + key);
	return value;
}

std::string Params::stringOr(const std::string &key, const std::string &fallback) const
{
	const auto it = values.find(key);
	return it == values.end() ? fallback : it->second;
}

static void checkFilename(const std::string &name)
{
	if (!name.empty() && (name == "." || name == ".." || name.back() == '.' ||
		name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-") != std::string::npos))
		throw std::invalid_argument("Output filenames must be plain basenames: " + name);
}

void validateOptions(const Options &options)
{
	if (options.scenario != "all" && !makeScenario(options.scenario))
		throw std::invalid_argument("Unknown renderer scenario: " + options.scenario);
	if (options.outputDirectory.empty())
		throw std::invalid_argument("Empty output directory");
	checkFilename(options.logPath);
	checkFilename(options.capturePath);
	if (!options.capturePath.empty() && (options.capturePath.size() < 5 ||
		options.capturePath.substr(options.capturePath.size() - 4) != ".png"))
		throw std::invalid_argument("--capture must end in .png");
	if (options.frames < 0 || options.frames > 1000000 || options.warmupFrames < 0 ||
		options.warmupFrames > 1000000 || options.tickInterval < 1 || options.tickInterval > 1000000 ||
		options.sampleEvery < 1 || options.sampleEvery > 1000000 ||
		options.viewDistance < 0 || options.viewDistance > 3 || options.fancyGraphics < 0 || options.fancyGraphics > 1)
		throw std::invalid_argument("Out-of-range runner option");
	const std::map<std::string, std::vector<std::string>> keys = {
		{ "idle", {} }, { "spin", { "rate" } }, { "walk", { "radius" } },
		{ "daycycle", { "step" } }, { "travel", { "speed", "distance" } },
		{ "farlands", { "axis", "sign", "shift", "speed", "distance" } },
		{ "building", { "size", "floors", "torch_spacing" } },
		{ "lighting", { "count", "period", "width", "depth" } },
		{ "fluids", { "size", "spacing" } }, { "tnt", { "count", "period" } },
		{ "mobs", { "count" } }, { "entities", { "count" } },
		{ "cave", { "width", "depth" } }, { "all", {} }
	};
	const auto &allowed = keys.at(options.scenario);
	for (const auto &entry : options.params.values)
	{
		const std::string &key = entry.first;
		if (key == "seed")
		{
			options.params.longOr(key, 0);
			continue;
		}
		if (std::find(allowed.begin(), allowed.end(), key) == allowed.end())
			throw std::invalid_argument("Unknown parameter for " + options.scenario + ": --" + key);
		if (key == "axis" || key == "sign")
		{
			if ((key == "axis" && entry.second != "x" && entry.second != "z") ||
				(key == "sign" && entry.second != "+" && entry.second != "-"))
				throw std::invalid_argument("Invalid value: --" + key);
		}
		else if (key == "rate" || key == "speed" || key == "distance" || key == "radius" || key == "shift")
		{
			const double value = options.params.doubleOr(key, 0);
			const double lower = key == "rate" ? -360.0 : key == "shift" ? -4096.0 : 0.01;
			const double upper = key == "rate" ? 360.0 : key == "speed" ? 1024.0 :
				key == "radius" || key == "shift" ? 4096.0 : 1000000.0;
			if (value < lower || value > upper)
				throw std::invalid_argument("Out of range: --" + key);
		}
		else
		{
			const int value = options.params.intOr(key, 0);
			const int lower = key == "step" ? -24000 : 1;
			const int upper = key == "step" ? 24000 : key == "floors" ? 16 :
				key == "count" ? 10000 : key == "period" ? 1000000 : 256;
			if (value < lower || value > upper)
				throw std::invalid_argument("Out of range: --" + key);
		}
	}
	if (options.scenario == "travel" || options.scenario == "all")
	{
		const double duration = std::ceil(options.params.doubleOr("distance", 10000.0) /
			options.params.doubleOr("speed", 8.0)) * options.tickInterval;
		if (duration > 1000000.0)
			throw std::invalid_argument("Travel distance/speed/tick-interval imply more than 1000000 frames");
	}
}

void pinPlayer(LocalPlayer &player, double x, double y, double z, float yRot, float xRot)
{
	
	player.moveTo(x, y, z, yRot, xRot);
	player.yRotO = yRot;
	player.xRotO = xRot;
	player.xd = player.yd = player.zd = 0.0;
	player.health = 20;
}

static std::unique_ptr<std::ostream> outputFile(File &directory, const std::string &name)
{
	std::unique_ptr<File> file(File::open(directory, String::fromUTF8(name)));
	if (!file->createNewFile())
		throw std::runtime_error("Cannot create new stress output: " + name);
	std::unique_ptr<std::ostream> stream(file->toStreamOut());
	if (!stream || !*stream)
		throw std::runtime_error("Cannot open stress output: " + name);
	stream->imbue(std::locale::classic());
	stream->exceptions(std::ios::badbit | std::ios::failbit);
	return stream;
}

static const char *glString(GLenum name)
{
	const auto *value = glGetString(name);
	return value ? reinterpret_cast<const char *>(value) : "UNAVAILABLE";
}

static void checkGl(int frame)
{
	const GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		throw std::runtime_error("OpenGL error " + std::to_string(error) + " at frame " + std::to_string(frame));
}

static void captureFrame(File &directory, const std::string &name)
{
	int width = 0, height = 0;
	SDL_GL_GetDrawableSize(lwjgl::GLContext::detail::getWindow(), &width, &height);
	if (width <= 0 || height <= 0)
		throw std::runtime_error("Empty drawable for framebuffer capture");
	std::unique_ptr<File> file(File::open(directory, String::fromUTF8(name)));
	if (file->exists())
		throw std::runtime_error("Capture already exists: " + name);
	std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
	GLint alignment = 4, readBuffer = GL_BACK;
	glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
	glGetIntegerv(GL_READ_BUFFER, &readBuffer);
	glReadBuffer(GL_BACK);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glFinish();
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	glPixelStorei(GL_PACK_ALIGNMENT, alignment);
	glReadBuffer(static_cast<GLenum>(readBuffer));
	checkGl(-1);
	stbi_flip_vertically_on_write(1);
	const int written = stbi_write_png(String::toUTF8(file->toString()).c_str(), width, height, 3,
		pixels.data(), width * 3);
	stbi_flip_vertically_on_write(0);
	if (!written)
		throw std::runtime_error("Cannot write framebuffer capture: " + name);
}

using Clock = std::chrono::steady_clock;
static double elapsedMs(Clock::time_point start)
{
	return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

int run(const Options &options)
try
{
	std::unique_ptr<File> directory(File::open(String::fromUTF8(options.outputDirectory)));
	if (directory->exists())
		throw std::runtime_error("Output directory already exists; choose a fresh --output directory");
	std::shared_ptr<File> gameDirectory(File::open(*directory, u"game"));
	if (!gameDirectory->mkdirs())
		throw std::runtime_error("Cannot create isolated stress directory");

	lwjgl::GLContext::instantiate();
	if (!GLAD_GL_VERSION_2_1)
		throw std::runtime_error("OpenGL 2.1 required; no null rendering fallback exists");
	std::unique_ptr<Minecraft> owner = std::make_unique<Minecraft>(854, 480, false);
	Minecraft &minecraft = *owner;
	minecraft.unattended = true;
	minecraft.user = std::make_unique<User>(u"StressPlayer", u"0");
	minecraft.options.viewDistance = options.viewDistance;
	minecraft.options.fancyGraphics = options.fancyGraphics != 0;
	minecraft.options.ambientOcclusion = options.fancyGraphics != 0;
	minecraft.options.limitFramerate = 0;
	minecraft.options.difficulty = 2;
	minecraft.options.showDebugInfo = false;
	minecraft.options.hideGui = false;
	minecraft.options.anaglyph3d = false;
	minecraft.init(gameDirectory);
	if (lwjgl::Display::isVisible())
		throw std::runtime_error("Stress window unexpectedly became visible");
	if (minecraft.width != lwjgl::Display::getWidth() || minecraft.height != lwjgl::Display::getHeight())
		minecraft.resize(lwjgl::Display::getWidth(), lwjgl::Display::getHeight());
	const std::vector<std::string> runs = options.scenario == "all" ? scenarioNames() :
		std::vector<std::string>{ options.scenario };
	for (const std::string &name : runs)
	{
		std::unique_ptr<Scenario> scenario = makeScenario(name);
		const std::string prefix = name + "-";
		const std::string logName = options.logPath.empty() ? prefix + "run.log" :
			runs.size() > 1 ? prefix + options.logPath : options.logPath;
		auto log = outputFile(*directory, logName);
		auto raw = outputFile(*directory, prefix + "raw.csv");
		auto summary = outputFile(*directory, prefix + "summary.csv");
		auto samples = outputFile(*directory, prefix + "samples.csv");
		auto emit = [&](const std::string &line) { std::cout << line << '\n'; *log << line << '\n'; };
		emit("status RUNNING");
		emit("scenario " + name);
		emit("renderer real-hidden-opengl21");
		emit("gl_vendor " + std::string(glString(GL_VENDOR)));
		emit("gl_renderer " + std::string(glString(GL_RENDERER)));
		emit("gl_version " + std::string(glString(GL_VERSION)));
		emit("width " + std::to_string(minecraft.width));
		emit("height " + std::to_string(minecraft.height));
		emit("view_distance " + std::to_string(options.viewDistance));
		emit("fancy_graphics " + std::to_string(options.fancyGraphics));
		emit("tick_interval_frames " + std::to_string(options.tickInterval));
		emit("warmup_frames " + std::to_string(options.warmupFrames));
		emit("sample_every_frames " + std::to_string(options.sampleEvery));
		emit("finish_each_frame " + std::to_string(options.finishEachFrame));
		for (const auto &entry : options.params.values)
			emit("param_" + entry.first + " " + entry.second);
		log->flush();
		const long_t seed = options.params.longOr("seed", 1234567);
		emit("seed " + std::to_string(seed));
		const auto levelStart = Clock::now();
		minecraft.gameMode = std::make_shared<SurvivalMode>(minecraft);
		auto level = Level::createSimulationLevel(File::open(*gameDirectory, u"saves"),
			String::fromUTF8("stress-" + name), seed);
		level->xSpawn = level->zSpawn = 0;
		level->ySpawn = 96;
		level->time = 6000;
		minecraft.setLevel(level, u"Stress");
		minecraft.setScreen(nullptr);
		minecraft.pause = false;
		if (!minecraft.player)
			throw std::runtime_error("Stress level produced no player");
		emit("level_ms " + std::to_string(elapsedMs(levelStart)));
		World world{ minecraft, *level, *minecraft.player, seed, options.tickInterval };
		const auto setupStart = Clock::now();
		scenario->setup(world, options.params);
		emit("setup_ms " + std::to_string(elapsedMs(setupStart)));
		emit("setup_light_queue " + std::to_string(level->lightUpdates.size()));
		if (scenario->settleBeforeMeasure())
		{
			const auto settleStart = Clock::now();
			while (level->updateLights()) {}
			minecraft.gameRenderer.updateAllChunks();
			emit("settle_ms " + std::to_string(elapsedMs(settleStart)));
		}
		const int measuredFrames = options.frames ? options.frames : scenario->defaultFrames();
		emit("requested_measured_frames " + std::to_string(measuredFrames));
		emit("timing CPU inclusive per-frame section totals from Profiler; Frame includes swap and optional glFinish");
		emit("timing_exclusions initialization,setup,settling,warmup,CSV,capture; no GPU timer queries");
		emit("unavailable_metrics process_memory,heap_histogram,live_chunk_count,scheduled_tick_count,backend_residency,legacygl_retention,backend_phase_profiles");
		emit("natural_spawning disabled by unattended client; explicit scenario entities tick and render normally");
		*samples << "frame,tick,chunk_updates,entities,light_queue,x,y,z\n" << std::setprecision(17);
		Profiler::reset(static_cast<std::size_t>(measuredFrames));
		long_t gameTicks = 0;
		long_t measuredChunkUpdates = 0, warmupChunkUpdates = 0;
		std::size_t peakLightQueue = 0;
		for (int frame = 0; frame < options.warmupFrames + measuredFrames; ++frame)
		{
			AABB::resetPool();
			Vec3::resetPool();
			lwjgl::Display::processMessages();
			if (lwjgl::Display::isCloseRequested())
				throw std::runtime_error("Window closed before finite run completed");
			const bool measured = frame >= options.warmupFrames;
			if (measured)
				Profiler::beginFrame(static_cast<std::size_t>(frame - options.warmupFrames));
			Profiler::enable(measured);
			const int beforeRebuilds = Chunk::updates;
			{
				Profiler::Scope frameProfile(Profiler::Section::Frame);
				if (frame % options.tickInterval == 0)
				{
					minecraft.stressTick();
					scenario->onTick(world, gameTicks++);
				}
				const float partialTick = static_cast<float>(frame % options.tickInterval) / options.tickInterval;
				{
					Profiler::Scope soundProfile(Profiler::Section::Sound);
					minecraft.soundEngine.update(minecraft.player.get(), partialTick);
				}
				glEnable(GL_TEXTURE_2D);
				glEnable(GL_ALPHA_TEST);
				peakLightQueue = std::max(peakLightQueue, level->lightUpdates.size());
				level->updateLights();
				lwjgl::Display::swapBuffers();
				minecraft.gameMode->render(partialTick);
				minecraft.gameRenderer.render(partialTick);
				if (minecraft.achievementToast)
				{
					Profiler::Scope guiProfile(Profiler::Section::Gui);
					minecraft.achievementToast->render();
				}
				if (options.finishEachFrame)
					glFinish();
			}
			Profiler::enable(false);
			checkGl(frame);
			const int rebuilt = Chunk::updates - beforeRebuilds;
			if (measured) measuredChunkUpdates += rebuilt;
			else warmupChunkUpdates += rebuilt;
			const int measuredIndex = frame - options.warmupFrames + 1;
			if (measured && (measuredIndex % options.sampleEvery == 0 || measuredIndex == measuredFrames))
			{
				*samples << measuredIndex << ',' << gameTicks << ',' << measuredChunkUpdates << ',' <<
					level->entities.size() << ',' << level->lightUpdates.size() << ',' <<
					world.player.x << ',' << world.player.y << ',' << world.player.z << '\n';
				emit("render_stats " + std::to_string(measuredIndex) + " " +
					String::toUTF8(minecraft.levelRenderer.gatherStats1()) + " | " +
					String::toUTF8(minecraft.levelRenderer.gatherStats2()));
			}
		}
		Profiler::report(*raw, *summary);
		if (!options.capturePath.empty())
		{
			const std::string captureName = runs.size() > 1 ? prefix + options.capturePath : options.capturePath;
			captureFrame(*directory, captureName);
			emit("capture " + captureName);
		}
		emit("measured_frames " + std::to_string(measuredFrames));
		emit("game_ticks " + std::to_string(gameTicks));
		emit("warmup_chunk_updates " + std::to_string(warmupChunkUpdates));
		emit("chunk_updates " + std::to_string(measuredChunkUpdates));
		emit("peak_light_queue " + std::to_string(peakLightQueue));
		emit("end_light_queue " + std::to_string(level->lightUpdates.size()));
		emit("end_entities " + std::to_string(level->entities.size()));
		std::vector<std::string> extra;
		scenario->report(world, extra);
		for (const std::string &line : extra) emit(line);
		emit("status COMPLETE");
		log->flush(); raw->flush(); summary->flush(); samples->flush();
	}
	minecraft.running = false;
	std::cout << "stress: complete; results in " << options.outputDirectory << '\n';
	return 0;
}
catch (const std::exception &error)
{
	Profiler::enable(false);
	std::cerr << "stress: " << error.what() << '\n';
	return 1;
}
}

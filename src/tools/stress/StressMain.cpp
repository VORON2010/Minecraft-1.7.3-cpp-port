
#include "tools/stress/StressHarness.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

static void stressUsage()
{
	std::cout <<
		"Usage: McBetaCppStress [options] <scenario|all> [--parameter value]...\n"
		"  --frames N          Measured frames, 0 uses scenario default\n"
		"  --warmup N          Warm-up frames, default 60\n"
		"  --tick-interval N   Frames per fixed game tick, default 3\n"
		"  --sample-every N    State sample interval in frames, default 100\n"
		"  --view-distance N   0=far, 1=normal, 2=short, 3=tiny, default 0\n"
		"  --fancy 0|1         Fancy graphics and ambient occlusion, default 1\n"
		"  --no-finish         Do not glFinish each measured/warm-up frame\n"
		"  --output directory Fresh isolated output/game directory, default stress-results\n"
		"  --log filename     Report basename inside output directory\n"
		"  --capture filename Final framebuffer PNG basename inside output directory\n"
		"  --seed N           Signed 64-bit world seed, default 1234567\n"
		"Scenarios and parameters:\n"
		"  idle, spin [--rate degrees-per-tick], daycycle [--step world-ticks]\n"
		"  walk [--radius blocks], travel [--speed blocks-per-tick --distance blocks]\n"
		"  farlands [--axis x|z --sign +|- --shift blocks --speed blocks-per-tick --distance blocks]\n"
		"  building [--size blocks --floors N --torch_spacing blocks]\n"
		"  lighting [--count N --period ticks --width blocks --depth blocks]\n"
		"  fluids [--size blocks --spacing blocks], tnt [--count N --period ticks]\n"
		"  mobs [--count N], entities [--count N], cave [--width blocks --depth blocks]\n"
		"all runs each offline renderer scenario once with normal lighting and a fresh level.\n"
		"Only --seed is accepted as a scenario parameter for all. Options may be mixed.\n"
		"Real hidden OpenGL 2.1, no pacing. No backend/null sink, fullbright, server,\n"
		"authentication, saved-world loading, or audio-only scenarios.\n"
		"Existing output directories are rejected. Exit: 0 complete, 1 runtime failure, 2 usage error.\n";
}

static void stressTerminate()
{
	const std::exception_ptr current = std::current_exception();
	if (current)
	{
		try { std::rethrow_exception(current); }
		catch (const std::exception &error) { std::cerr << "stress: terminate: " << error.what() << '\n'; }
		catch (...) { std::cerr << "stress: terminate: non-standard exception\n"; }
	}
	else
		std::cerr << "stress: terminate without an active exception\n";
	std::abort();
}

int main(int argc, char *argv[])
{
	std::set_terminate(&stressTerminate);
	stress::Options options;
	try
	{
		for (int i = 1; i < argc; ++i)
		{
			const std::string arg = argv[i];
			if (arg == "--help" || arg == "-h")
			{
				stressUsage();
				return 0;
			}
			if (arg == "--no-finish")
			{
				options.finishEachFrame = false;
				continue;
			}
			if (arg == "--backend" || arg == "--null-sink" || arg == "--no-lighting" ||
				arg == "--no-occlusion" || arg == "--server" || arg == "--user" || arg == "--world")
				throw std::invalid_argument("Unsupported option in the Beta renderer tool: " + arg);
			if (arg.compare(0, 2, "--") != 0)
			{
				if (!options.scenario.empty())
					throw std::invalid_argument("Unexpected argument: " + arg);
				options.scenario = arg;
				continue;
			}
			if (++i >= argc)
				throw std::invalid_argument("Missing value for " + arg);
			const std::string value = argv[i];
			if (arg == "--output") options.outputDirectory = value;
			else if (arg == "--log") options.logPath = value;
			else if (arg == "--capture") options.capturePath = value;
			else if (arg == "--frames" || arg == "--warmup" || arg == "--tick-interval" ||
				arg == "--sample-every" || arg == "--view-distance" || arg == "--fancy")
			{
				const long_t number = stress::parseInteger(value);
				if (number < 0 || number > 1000000)
					throw std::invalid_argument("Out of range: " + arg);
				const int parsed = static_cast<int>(number);
				if (arg == "--frames") options.frames = parsed;
				if (arg == "--warmup") options.warmupFrames = parsed;
				if (arg == "--tick-interval") options.tickInterval = parsed;
				if (arg == "--sample-every") options.sampleEvery = parsed;
				if (arg == "--view-distance") options.viewDistance = parsed;
				if (arg == "--fancy") options.fancyGraphics = parsed;
			}
			else
				options.params.values[arg.substr(2)] = value;
		}
		stress::validateOptions(options);
	}
	catch (const std::exception &error)
	{
		std::cerr << "stress: " << error.what() << '\n';
		stressUsage();
		return 2;
	}
	return stress::run(options);
}

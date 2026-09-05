#include "tools/SoundSmoke.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "client/Minecraft.h"
#include "client/Options.h"
#include "client/sound/SoundEngine.h"
#include "java/File.h"
#include "java/String.h"

namespace
{

int failures = 0;

void check(bool condition, const std::string &what)
{
	std::cout << (condition ? "[PASS] " : "[FAIL] ") << what << '\n';
	if (!condition)
		failures++;
}


void loadAll(SoundEngine &engine, File *dir, const jstring &prefix)
{
	if (!dir || !dir->exists() || !dir->isDirectory())
		return;

	auto files = dir->listFiles();
	for (auto &file : files)
	{
		if (file->isDirectory())
		{
			loadAll(engine, file.get(), prefix + file->getName() + u"/");
		}
		else
		{
			jstring name = file->getName();
			if (name.length() < 4)
				continue;
			jstring ext = name.substr(name.length() - 4);
			if (ext != u".ogg" && ext != u".mus" && ext != u".wav")
				continue;

			jstring fullName = prefix + name;
			size_t slashPos = fullName.find(u'/');
			if (slashPos == jstring::npos)
				continue;
			jstring category = fullName.substr(0, slashPos);
			jstring soundName = fullName.substr(slashPos + 1);
			if (category == u"sound" || category == u"newsound")
				engine.add(soundName, String::toUTF8(file->toString()));
		}
	}
}

void sleepMs(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

}

int runSoundSmoke()
{
	std::cout << "=== sound smoke ===\n";

	Minecraft minecraft(1, 1, false);
	Options &options = minecraft.options;
	options.sound = 1.0f;
	SoundEngine engine;
	engine.init(&options);

	std::unique_ptr<File> resourceDir(File::openResourceDirectory());
	loadAll(engine, resourceDir.get(), u"");

	
	check(engine.debugHasSound(u"random.hurt"), "random.hurt resolves in the repository");
	check(engine.debugHasSound(u"random.pop"), "random.pop resolves");
	check(engine.debugHasSound(u"step.grass"), "step.grass resolves");
	check(engine.debugHasSound(u"ambient.weather.rain"), "ambient.weather.rain resolves");

	
	engine.play(u"random.hurt", 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	check(engine.debugPlayingSources() == 1, "random.hurt is audibly playing (1 playing source)");

	
	for (int i = 0; i < 40; ++i)
		engine.play(u"random.pop", 0.0f, 0.0f, 0.0f, 0.2f, 1.0f);
	int_t playing = engine.debugPlayingSources();
	std::cout << "  burst: " << engine.debugActiveSources() << " active, " << playing << " playing\n";
	check(playing > 0 && playing <= 28, "burst saturates but stays within the 28-source pool");

	
	
	engine.debugDrops = 0;
	engine.play(u"random.hurt", 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	check(engine.debugDrops == 0, "hurt steals a non-priority channel when the pool is full");

	
	sleepMs(2500);
	engine.play(u"step.grass", 0.0f, 0.0f, 0.0f, 1.0f, 1.0f); 
	sleepMs(700);
	engine.play(u"random.hurt", 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	check(engine.debugPlayingSources() >= 1, "hurt still plays after the burst (sources reclaimed, no leak)");
	sleepMs(1200);

	
	for (int i = 0; i < 300; ++i)
	{
		engine.play(i % 3 == 0 ? u"random.hurt" : (i % 3 == 1 ? u"step.grass" : u"random.pop"),
			0.0f, 0.0f, 0.0f, 0.7f, 1.0f);
		sleepMs(10);
	}
	sleepMs(2500);
	engine.play(u"random.hurt", 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	check(engine.debugPlayingSources() >= 1, "hurt still plays after 300-sound stress (no cumulative leak)");

	std::cout << (failures == 0 ? "ALL PASS\n" : "FAILURES: " + std::to_string(failures) + "\n");
	return failures == 0 ? 0 : 1;
}

#pragma once

#include "client/sound/SoundRepository.h"
#include "client/Options.h"
#include "java/String.h"
#include "java/Random.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <string>
#include <vector>
#include <unordered_map>

class Mob;

class SoundEngine
{
private:
	static bool loaded;
	ALCdevice *device = nullptr;
	ALCcontext *context = nullptr;

	SoundRepository sounds;
	SoundRepository streamingSounds;
	SoundRepository songs;
	int_t idCounter = 0;
	Options *options = nullptr;
	Random random;
	int_t noMusicDelay = 0;

	
	struct SourceInfo
	{
		float x, y, z;
		float distOrRoll;    
		float sourceVolume;  
		int attModel;        
		bool priority;       

		SourceInfo() : x(0), y(0), z(0), distOrRoll(0), sourceVolume(1.0f), attModel(0), priority(false) {}
		SourceInfo(float x, float y, float z, float distOrRoll, float sourceVolume, int attModel, bool priority = false)
			: x(x), y(y), z(z), distOrRoll(distOrRoll), sourceVolume(sourceVolume), attModel(attModel), priority(priority) {}
	};

	std::unordered_map<std::string, SourceInfo> sourceInfoMap;
	
	
	std::vector<ALuint> channelSources;
	std::vector<std::string> channelIds;
	int_t nextNormalChannel = 0;
	std::unordered_map<std::string, ALuint> soundBuffers;
	ALuint musicSource = 0;
	ALuint streamingSource = 0;
	
	static constexpr int_t MAX_SOURCES = 28;

	float listenerX = 0.0f, listenerY = 0.0f, listenerZ = 0.0f;

	bool initOpenAL();
	void cleanupOpenAL();
	ALuint getOrCreateSource(const std::string &id, bool streaming, bool priority = false);
	void releaseSource(const std::string &id);
	ALuint loadOGGFile(const std::string &filePath, bool isMUS = false);
	bool loadSound(const Sound &sound, ALuint &buffer, bool isMUS = false);
	void checkAndReleaseFinishedSources();
	void updateSourceGains();

public:
	SoundEngine();
	~SoundEngine();

	void init(Options *options);
	void updateOptions();
	void destroy();

	void add(const jstring &name, const std::string &filePath);
	void addStreaming(const jstring &name, const std::string &filePath);
	void addMusic(const jstring &name, const std::string &filePath);

	void playMusicTick();
	void update(Mob *player, float a);

	void playStreaming(const jstring &name, float x, float y, float z, float volume, float pitch);
	void play(const jstring &name, float x, float y, float z, float volume, float pitch);

	
	bool debugHasSound(const jstring &name) { return sounds.get(name) != nullptr; }
	int_t debugActiveSources();
	int_t debugPlayingSources();
	int_t debugDrops = 0;
	void playUI(const jstring &name, float volume, float pitch);
};

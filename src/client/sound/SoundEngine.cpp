#include "client/sound/SoundEngine.h"

#include "world/entity/Mob.h"
#include "util/Mth.h"
#include "java/String.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cfloat>
#include <algorithm>


extern "C" {
	typedef unsigned char uint8;
	int stb_vorbis_decode_memory(const uint8 *mem, int len, int *channels, int *sample_rate, short **output);
}

bool SoundEngine::loaded = false;

SoundEngine::SoundEngine()
{
	noMusicDelay = random.nextInt(12000);
}

SoundEngine::~SoundEngine()
{
	destroy();
}


void SoundEngine::init(Options *options)
{
	this->options = options;
	this->streamingSounds.setTrimDigits(false);

	if (!loaded && (options == nullptr || options->sound != 0.0f || options->music != 0.0f))
	{
		if (initOpenAL())
			loaded = true;
	}
}

bool SoundEngine::initOpenAL()
{
	device = alcOpenDevice(nullptr);
	if (!device)
	{
		std::cerr << "Failed to open OpenAL device" << std::endl;
		return false;
	}

	context = alcCreateContext(device, nullptr);
	if (!context)
	{
		std::cerr << "Failed to create OpenAL context" << std::endl;
		alcCloseDevice(device);
		device = nullptr;
		return false;
	}

	if (!alcMakeContextCurrent(context))
	{
		std::cerr << "Failed to make OpenAL context current" << std::endl;
		alcDestroyContext(context);
		alcCloseDevice(device);
		context = nullptr;
		device = nullptr;
		return false;
	}

	channelSources.resize(MAX_SOURCES);
	channelIds.assign(MAX_SOURCES, std::string());
	nextNormalChannel = 0;
	alGenSources((ALsizei)channelSources.size(), channelSources.data());
	ALenum err = alGetError();
	if (err != AL_NO_ERROR)
	{
		std::cerr << "Failed to generate OpenAL sources: " << alGetString(err) << std::endl;
		cleanupOpenAL();
		return false;
	}

	alGenSources(1, &musicSource);
	alGenSources(1, &streamingSource);

	
	alDistanceModel(AL_INVERSE_DISTANCE);
	alDopplerFactor(1.0f);
	alDopplerVelocity(343.0f);

	alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
	alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	float orientation[6] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	alListenerfv(AL_ORIENTATION, orientation);

	alGetError();
	return true;
}

void SoundEngine::cleanupOpenAL()
{
	for (ALuint source : channelSources)
	{
		alSourceStop(source);
		alSourcei(source, AL_BUFFER, 0);
	}
	sourceInfoMap.clear();

	if (!channelSources.empty())
	{
		alDeleteSources((ALsizei)channelSources.size(), channelSources.data());
		channelSources.clear();
		channelIds.clear();
	}

	if (musicSource != 0)
	{
		alSourceStop(musicSource);
		alSourcei(musicSource, AL_BUFFER, 0);
		alDeleteSources(1, &musicSource);
		musicSource = 0;
	}

	if (streamingSource != 0)
	{
		alSourceStop(streamingSource);
		alSourcei(streamingSource, AL_BUFFER, 0);
		alDeleteSources(1, &streamingSource);
		streamingSource = 0;
	}

	for (auto &pair : soundBuffers)
		alDeleteBuffers(1, &pair.second);
	soundBuffers.clear();

	if (context)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
		context = nullptr;
	}

	if (device)
	{
		alcCloseDevice(device);
		device = nullptr;
	}
}


void SoundEngine::updateOptions()
{
	if (!loaded && options && (options->sound != 0.0f || options->music != 0.0f))
	{
		if (initOpenAL())
			loaded = true;
	}

	if (loaded)
	{
		if (options->music == 0.0f)
		{
			if (musicSource != 0)
			{
				alSourceStop(musicSource);
				alSourcei(musicSource, AL_BUFFER, 0);
			}
		}
		else if (musicSource != 0)
		{
			ALint state;
			alGetSourcei(musicSource, AL_SOURCE_STATE, &state);
			if (state == AL_PLAYING || state == AL_PAUSED)
				alSourcef(musicSource, AL_GAIN, options->music);
		}
	}
}


void SoundEngine::destroy()
{
	if (loaded)
	{
		cleanupOpenAL();
		loaded = false;
	}
}


void SoundEngine::add(const jstring &name, const std::string &filePath)
{
	sounds.add(name, filePath);
}


void SoundEngine::addStreaming(const jstring &name, const std::string &filePath)
{
	streamingSounds.add(name, filePath);
}


void SoundEngine::addMusic(const jstring &name, const std::string &filePath)
{
	songs.add(name, filePath);
}


void SoundEngine::playMusicTick()
{
	if (!loaded || !options || options->music == 0.0f || musicSource == 0)
		return;

	ALint musicState;
	alGetSourcei(musicSource, AL_SOURCE_STATE, &musicState);

	ALint streamState = AL_STOPPED;
	if (streamingSource != 0)
		alGetSourcei(streamingSource, AL_SOURCE_STATE, &streamState);

	if (musicState == AL_PLAYING || streamState == AL_PLAYING)
	{
		if (noMusicDelay > 0)
			noMusicDelay--;
		return;
	}

	if (noMusicDelay > 0)
	{
		noMusicDelay--;
		return;
	}

	Sound *song = songs.any();
	if (song != nullptr)
	{
		noMusicDelay = random.nextInt(12000) + 12000;

		bool isMus = song->filePath.size() >= 4 && song->filePath.compare(song->filePath.size() - 4, 4, ".mus") == 0;
		ALuint buffer = loadOGGFile(song->filePath, isMus);
		if (buffer != 0)
		{
			alSourceStop(musicSource);
			alSourcei(musicSource, AL_BUFFER, buffer);
			alSourcei(musicSource, AL_LOOPING, AL_TRUE);
			alSourcef(musicSource, AL_GAIN, options->music);
			alSource3f(musicSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSourcef(musicSource, AL_REFERENCE_DISTANCE, 1.0f);
			alSourcef(musicSource, AL_MAX_DISTANCE, 10000.0f);
			alSourcePlay(musicSource);
		}
	}
}


void SoundEngine::update(Mob *player, float a)
{
	if (!loaded || !options || options->sound == 0.0f || !player)
		return;

	float yRot = player->yRotO + (player->yRot - player->yRotO) * a;
	double x = player->xo + (player->x - player->xo) * (double)a;
	double y = player->yo + (player->y - player->yo) * (double)a;
	double z = player->zo + (player->z - player->zo) * (double)a;

	float yCos = Mth::cos(-yRot * Mth::DEGRAD - Mth::PI);
	float ySin = Mth::sin(-yRot * Mth::DEGRAD - Mth::PI);
	float forwardX = -ySin;
	float forwardY = 0.0f;
	float forwardZ = -yCos;

	alListener3f(AL_POSITION, (float)x, (float)y, (float)z);
	float orientation[6] = { forwardX, forwardY, forwardZ, 0.0f, 1.0f, 0.0f };
	alListenerfv(AL_ORIENTATION, orientation);

	listenerX = (float)x;
	listenerY = (float)y;
	listenerZ = (float)z;

	updateSourceGains();
	checkAndReleaseFinishedSources();
}

void SoundEngine::checkAndReleaseFinishedSources()
{
	for (size_t n = 0; n < channelSources.size(); ++n)
	{
		if (channelIds[n].empty())
			continue;

		ALuint source = channelSources[n];
		ALint state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);

		if (state == AL_STOPPED || state == AL_INITIAL)
		{
			alSourceStop(source);
			alSourcei(source, AL_BUFFER, 0);
			alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
			alSourcef(source, AL_GAIN, 1.0f);
			alSourcef(source, AL_PITCH, 1.0f);
			alSourcef(source, AL_REFERENCE_DISTANCE, 1.0f);
			alSourcef(source, AL_MAX_DISTANCE, FLT_MAX);
			alSourcef(source, AL_ROLLOFF_FACTOR, 1.0f);
			alSourcei(source, AL_LOOPING, AL_FALSE);
			alGetError();

			sourceInfoMap.erase(channelIds[n]);
			channelIds[n].clear();
		}
	}
}



static float calculateLinearGain(float srcX, float srcY, float srcZ,
                                  float lisX, float lisY, float lisZ,
                                  float distOrRoll)
{
	float dX = srcX - lisX;
	float dY = srcY - lisY;
	float dZ = srcZ - lisZ;
	float dist = std::sqrt(dX * dX + dY * dY + dZ * dZ);

	float gain;
	if (dist <= 0.0f)
		gain = 1.0f;
	else if (dist >= distOrRoll)
		gain = 0.0f;
	else
		gain = 1.0f - dist / distOrRoll;

	if (gain > 1.0f) gain = 1.0f;
	if (gain < 0.0f) gain = 0.0f;
	return gain;
}


void SoundEngine::updateSourceGains()
{
	for (size_t n = 0; n < channelSources.size(); ++n)
	{
		if (channelIds[n].empty())
			continue;

		ALuint source = channelSources[n];
		ALint state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING && state != AL_PAUSED)
			continue;

		auto infoIt = sourceInfoMap.find(channelIds[n]);
		if (infoIt == sourceInfoMap.end())
			continue;

		const SourceInfo &info = infoIt->second;
		float gain = 1.0f;
		if (info.attModel == 2)
			gain = calculateLinearGain(info.x, info.y, info.z, listenerX, listenerY, listenerZ, info.distOrRoll);

		alSourcef(source, AL_GAIN, gain * info.sourceVolume);
	}

	
	auto streamingInfoIt = sourceInfoMap.find("streaming");
	if (streamingInfoIt != sourceInfoMap.end() && streamingSource != 0)
	{
		ALint state;
		alGetSourcei(streamingSource, AL_SOURCE_STATE, &state);
		if (state == AL_PLAYING || state == AL_PAUSED)
		{
			const SourceInfo &info = streamingInfoIt->second;
			float gain = 1.0f;
			if (info.attModel == 2)
				gain = calculateLinearGain(info.x, info.y, info.z, listenerX, listenerY, listenerZ, info.distOrRoll);
			alSourcef(streamingSource, AL_GAIN, gain * info.sourceVolume);
		}
	}
}


void SoundEngine::playStreaming(const jstring &name, float x, float y, float z, float volume, float pitch)
{
	if (!loaded || !options || options->sound == 0.0f || streamingSource == 0)
		return;

	std::string id = "streaming";

	
	ALint state;
	alGetSourcei(streamingSource, AL_SOURCE_STATE, &state);
	if (state == AL_PLAYING || state == AL_PAUSED)
	{
		alSourceStop(streamingSource);
		alSourcei(streamingSource, AL_BUFFER, 0);
	}

	if (name.empty())
	{
		alSourceStop(streamingSource);
		alSourcei(streamingSource, AL_BUFFER, 0);
		sourceInfoMap.erase(id);
		return;
	}

	Sound *sound = streamingSounds.get(name);
	if (sound == nullptr || volume <= 0.0f)
		return;

	
	if (musicSource != 0)
	{
		alGetSourcei(musicSource, AL_SOURCE_STATE, &state);
		if (state == AL_PLAYING)
		{
			alSourceStop(musicSource);
			alSourcei(musicSource, AL_BUFFER, 0);
		}
	}

	
	float dist = 16.0f * 4.0f;

	
	bool isMus = sound->filePath.size() >= 4 && sound->filePath.compare(sound->filePath.size() - 4, 4, ".mus") == 0;
	ALuint buffer = loadOGGFile(sound->filePath, isMus);
	if (buffer == 0)
	{
		sourceInfoMap.erase(id);
		return;
	}

	alSourcei(streamingSource, AL_BUFFER, 0);
	alSource3f(streamingSource, AL_POSITION, x, y, z);
	alSource3f(streamingSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSourcei(streamingSource, AL_SOURCE_RELATIVE, AL_FALSE);
	alSourcei(streamingSource, AL_BUFFER, buffer);

	
	alSourcef(streamingSource, AL_ROLLOFF_FACTOR, 0.0f);

	alSourcef(streamingSource, AL_PITCH, pitch);

	
	float sourceVolume = 0.5f * options->sound;
	sourceInfoMap[id] = SourceInfo(x, y, z, dist, sourceVolume, 2);

	float gain = calculateLinearGain(x, y, z, listenerX, listenerY, listenerZ, dist);
	alSourcef(streamingSource, AL_GAIN, gain * sourceVolume);
	alSourcei(streamingSource, AL_LOOPING, AL_FALSE);

	alGetError();
	alSourcePlay(streamingSource);
}


void SoundEngine::play(const jstring &name, float x, float y, float z, float volume, float pitch)
{
	if (!loaded || !options || options->sound == 0.0f)
		return;

	Sound *sound = sounds.get(name);
	if (sound == nullptr || volume <= 0.0f)
		return;

	
	idCounter = (idCounter + 1) % 256;
	std::string id = "sound_" + std::to_string(idCounter);

	
	float dist = 16.0f;
	if (volume > 1.0f)
		dist *= volume;

	
	bool priority = volume > 1.0f;

	ALuint source = getOrCreateSource(id, false, priority);
	if (source == 0)
	{
		debugDrops++;
		return;
	}

	ALuint buffer;
	if (!loadSound(*sound, buffer))
		return;

	alSourceStop(source);
	alSourcei(source, AL_BUFFER, 0);
	alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

	alSourcei(source, AL_BUFFER, buffer);
	alSource3f(source, AL_POSITION, x, y, z);
	alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);

	
	alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);

	
	float clampedPitch = pitch;
	if (clampedPitch < 0.5f)
		clampedPitch = 0.5f;
	else if (clampedPitch > 2.0f)
		clampedPitch = 2.0f;
	alSourcef(source, AL_PITCH, clampedPitch);

	
	float finalVolume = volume > 1.0f ? 1.0f : volume;
	float sourceVolume = finalVolume * options->sound;
	sourceInfoMap[id] = SourceInfo(x, y, z, dist, sourceVolume, 2, priority);

	float gain = calculateLinearGain(x, y, z, listenerX, listenerY, listenerZ, dist);
	alSourcef(source, AL_GAIN, gain * sourceVolume);
	alSourcei(source, AL_LOOPING, AL_FALSE);

	alGetError();
	alSourcePlay(source);
}


void SoundEngine::playUI(const jstring &name, float volume, float pitch)
{
	if (!loaded || !options || options->sound == 0.0f)
		return;

	Sound *sound = sounds.get(name);
	if (sound == nullptr)
		return;

	idCounter = (idCounter + 1) % 256;
	std::string id = "sound_" + std::to_string(idCounter);

	
	if (volume > 1.0f)
		volume = 1.0f;
	volume *= 0.25f;

	ALuint source = getOrCreateSource(id, false);
	if (source == 0)
		return;

	ALuint buffer;
	if (!loadSound(*sound, buffer))
		return;

	
	alSourceStop(source);
	alSourcei(source, AL_BUFFER, 0);
	alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

	alSourcei(source, AL_BUFFER, buffer);
	alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
	alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);
	alSourcef(source, AL_PITCH, pitch);
	alSourcef(source, AL_GAIN, volume * options->sound);
	alSourcei(source, AL_LOOPING, AL_FALSE);

	alGetError();
	alSourcePlay(source);
}


ALuint SoundEngine::getOrCreateSource(const std::string &id, bool streaming, bool priority)
{
	(void)priority;
	if (streaming)
		return streamingSource;

	int_t channels = static_cast<int_t>(channelSources.size());
	if (channels == 0)
		return 0;

	auto takeChannel = [this](int_t n) -> ALuint
	{
		ALuint source = channelSources[n];

		alSourceStop(source);
		alSourcei(source, AL_BUFFER, 0);
		alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
		alSourcef(source, AL_GAIN, 1.0f);
		alSourcef(source, AL_PITCH, 1.0f);
		alSourcef(source, AL_REFERENCE_DISTANCE, 1.0f);
		alSourcei(source, AL_LOOPING, AL_FALSE);

		if (!channelIds[n].empty())
			sourceInfoMap.erase(channelIds[n]);
		return source;
	};

	
	for (int_t n = 0; n < channels; ++n)
	{
		if (channelIds[n] == id)
			return takeChannel(n);
	}

	
	
	int_t n = nextNormalChannel;
	for (int_t x = 0; x < channels; ++x)
	{
		bool playing = false;
		if (!channelIds[n].empty())
		{
			ALint state;
			alGetSourcei(channelSources[n], AL_SOURCE_STATE, &state);
			playing = state == AL_PLAYING;
		}

		if (!playing)
		{
			nextNormalChannel = (n + 1) % channels;
			ALuint source = takeChannel(n);
			channelIds[n] = id;
			return source;
		}

		n = (n + 1) % channels;
	}

	
	
	
	n = nextNormalChannel;
	for (int_t x = 0; x < channels; ++x)
	{
		auto info = sourceInfoMap.find(channelIds[n]);
		bool protectedChannel = info != sourceInfoMap.end() && info->second.priority;

		if (!protectedChannel)
		{
			nextNormalChannel = (n + 1) % channels;
			ALuint source = takeChannel(n);
			channelIds[n] = id;
			return source;
		}

		n = (n + 1) % channels;
	}

	return 0;
}

void SoundEngine::releaseSource(const std::string &id)
{
	for (size_t n = 0; n < channelSources.size(); ++n)
	{
		if (channelIds[n] == id)
		{
			alSourceStop(channelSources[n]);
			alSourcei(channelSources[n], AL_BUFFER, 0);
			channelIds[n].clear();
			sourceInfoMap.erase(id);
			return;
		}
	}
}



ALuint SoundEngine::loadOGGFile(const std::string &filePath, bool isMUS)
{
	std::string cacheKey = filePath + (isMUS ? "_mus" : "");
	auto it = soundBuffers.find(cacheKey);
	if (it != soundBuffers.end())
		return it->second;

	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "Failed to open sound file: " << filePath << std::endl;
		return 0;
	}

	size_t fileSize = (size_t)file.tellg();
	if (fileSize == 0 || fileSize > 100 * 1024 * 1024)
	{
		file.close();
		return 0;
	}

	file.seekg(0, std::ios::beg);
	std::vector<unsigned char> fileData(fileSize);
	if (!file.read((char *)fileData.data(), fileSize))
	{
		file.close();
		return 0;
	}
	file.close();

	
	if (isMUS)
	{
		
		size_t lastSlash = filePath.find_last_of("/\\");
		std::string filename = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath;

		
		int32_t seed = 0;
		for (size_t i = 0; i < filename.length(); i++)
			seed = (int32_t)((uint32_t)seed * 31u + (uint32_t)(unsigned char)filename[i]);

		
		for (size_t i = 0; i < fileData.size(); i++)
		{
			uint8_t key = (uint8_t)((uint32_t)seed >> 8);
			uint8_t val = (uint8_t)(fileData[i] ^ key);
			fileData[i] = val;
			
			seed = (int32_t)((uint32_t)seed * 498729871u +
			                  (uint32_t)(85731 * (int32_t)(int8_t)val));
		}
	}

	int channels, sampleRate;
	short *output = nullptr;
	int samples = stb_vorbis_decode_memory((const uint8 *)fileData.data(), (int)fileSize, &channels, &sampleRate, &output);

	if (samples <= 0 || output == nullptr || channels <= 0 || channels > 2)
	{
		if (output) free(output);
		return 0;
	}

	ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	ALuint buffer = 0;
	alGenBuffers(1, &buffer);
	if (buffer == 0)
	{
		free(output);
		return 0;
	}

	alBufferData(buffer, format, output, samples * channels * (int)sizeof(short), sampleRate);
	free(output);

	ALenum err = alGetError();
	if (err != AL_NO_ERROR)
	{
		alDeleteBuffers(1, &buffer);
		return 0;
	}

	soundBuffers[cacheKey] = buffer;
	return buffer;
}

bool SoundEngine::loadSound(const Sound &sound, ALuint &buffer, bool isMUS)
{
	buffer = loadOGGFile(sound.filePath, isMUS);
	return buffer != 0;
}

int_t SoundEngine::debugActiveSources()
{
	int_t active = 0;
	for (const std::string &id : channelIds)
	{
		if (!id.empty())
			active++;
	}
	return active;
}

int_t SoundEngine::debugPlayingSources()
{
	int_t playing = 0;
	for (size_t n = 0; n < channelSources.size(); ++n)
	{
		if (channelIds[n].empty())
			continue;
		ALint state;
		alGetSourcei(channelSources[n], AL_SOURCE_STATE, &state);
		if (state == AL_PLAYING)
			playing++;
	}
	return playing;
}

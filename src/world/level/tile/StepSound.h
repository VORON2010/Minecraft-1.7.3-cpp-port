#pragma once

#include "java/String.h"

struct StepSound
{
	jstring name;
	float volume;
	float pitch;

	StepSound(const jstring &name, float volume, float pitch)
		: name(name), volume(volume), pitch(pitch) {}

	float getVolume() const { return volume; }
	float getPitch() const { return pitch; }

	
	virtual jstring stepSoundDir() const { return u"step." + name; }
	
	virtual jstring getStepResourcePath() const { return u"step." + name; }

	virtual ~StepSound() = default;
};


struct StepSoundSand : StepSound
{
	using StepSound::StepSound;
	jstring stepSoundDir() const override { return u"step.gravel"; }
};


struct StepSoundStone : StepSound
{
	using StepSound::StepSound;
	jstring stepSoundDir() const override { return u"random.glass"; }
};

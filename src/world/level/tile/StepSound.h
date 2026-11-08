#pragma once

#include "java/String.h"

// 1.5.2 Block.Sounds: "dig.<name>" when broken or placed, "step.<name>" when walked on
struct StepSound
{
	jstring name;
	float volume;
	float pitch;

	StepSound(const jstring &name, float volume, float pitch)
		: name(name), volume(volume), pitch(pitch) {}

	float getVolume() const { return volume; }
	float getPitch() const { return pitch; }

	virtual jstring getBreakSound() const { return u"dig." + name; }
	virtual jstring getStepSound() const { return u"step." + name; }
	virtual jstring getPlaceSound() const { return getBreakSound(); }

	virtual ~StepSound() = default;
};

// 1.5.2 GLASS_SOUNDS
struct StepSoundGlass : StepSound
{
	using StepSound::StepSound;
	jstring getBreakSound() const override { return u"random.glass"; }
	jstring getPlaceSound() const override { return u"step.stone"; }
};

// 1.5.2 LADDER_SOUNDS
struct StepSoundLadder : StepSound
{
	using StepSound::StepSound;
	jstring getBreakSound() const override { return u"dig.wood"; }
};

// 1.5.2 ANVIL_SOUNDS
struct StepSoundAnvil : StepSound
{
	using StepSound::StepSound;
	jstring getBreakSound() const override { return u"dig.stone"; }
	jstring getPlaceSound() const override { return u"random.anvil_land"; }
};

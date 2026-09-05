#pragma once

#include "client/sound/Sound.h"
#include "java/String.h"
#include "java/Random.h"

#include <unordered_map>
#include <vector>
#include <string>

class SoundRepository
{
private:
	Random random;
	std::unordered_map<jstring, std::vector<Sound>> urls;
	std::vector<Sound> all;
	bool trimDigits = true;

public:
	int_t count = 0;

	Sound add(const jstring &name, const std::string &filePath);
	Sound *get(const jstring &name);
	Sound *any();

	void setTrimDigits(bool trim) { trimDigits = trim; }
};

#pragma once

#include "java/String.h"

#include <string>

class Sound
{
public:
	jstring name;
	std::string filePath;

	Sound(const jstring &name, const std::string &filePath)
		: name(name), filePath(filePath) {}
};

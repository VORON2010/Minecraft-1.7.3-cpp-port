#pragma once

#include "java/String.h"

class StatCollector
{
public:
	static jstring translate(const jstring &key);
	static jstring translate(const jstring &key, const jstring &argument);
};

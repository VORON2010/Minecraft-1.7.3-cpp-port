#pragma once

#include <vector>
#include "java/String.h"
#include "java/Type.h"

class Font;

namespace ChatLine
{
	std::vector<jstring> split(Font &font, const jstring &text, int_t maxWidth);
}

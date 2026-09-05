#pragma once

#include <cstdint>
#include <vector>

#include "java/String.h"
#include "java/Type.h"

class Minecraft;

namespace TextureItemFX
{
	bool loadIconPixels(Minecraft &minecraft, const jstring &resourceName, int_t iconIndex, std::vector<uint32_t> &out);
	bool loadWholePixels(Minecraft &minecraft, const jstring &resourceName, std::vector<uint32_t> &out);
	void applyAnaglyph(int_t &red, int_t &green, int_t &blue, bool enabled);
}

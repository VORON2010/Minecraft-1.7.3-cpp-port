#pragma once

#include <cstdint>
#include <vector>

#include "client/renderer/texturefx/TextureFX.h"

class Minecraft;

class TextureWatchFX : public TextureFX
{
private:
	Minecraft &minecraft;
	std::vector<uint32_t> watchIconImageData;
	std::vector<uint32_t> dialImageData;
	double rotation = 0.0;
	double rotationDelta = 0.0;

public:
	explicit TextureWatchFX(Minecraft &minecraft);

	void onTick() override;
};

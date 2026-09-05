#pragma once

#include <cstdint>
#include <vector>

#include "client/renderer/texturefx/TextureFX.h"

class Minecraft;

class TextureCompassFX : public TextureFX
{
private:
	Minecraft &minecraft;
	std::vector<uint32_t> compassIconImageData;
	double rotation = 0.0;
	double rotationDelta = 0.0;

public:
	explicit TextureCompassFX(Minecraft &minecraft);

	void onTick() override;
};

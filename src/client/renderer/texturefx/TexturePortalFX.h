#pragma once

#include <vector>

#include "client/renderer/texturefx/TextureFX.h"

class TexturePortalFX : public TextureFX
{
private:
	std::vector<std::vector<byte_t>> frames;
	int_t portalTickCounter = 0;

public:
	explicit TexturePortalFX(int_t iconIndex);

	void onTick() override;
};

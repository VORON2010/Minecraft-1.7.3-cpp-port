#pragma once

#include <vector>

#include "client/renderer/texturefx/TextureFX.h"

class TextureFlamesFX : public TextureFX
{
private:
	std::vector<float> current;
	std::vector<float> next;

public:
	explicit TextureFlamesFX(int_t variant);

	void onTick() override;
};

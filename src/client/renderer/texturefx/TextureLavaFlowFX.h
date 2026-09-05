#pragma once

#include <vector>

#include "client/renderer/texturefx/TextureFX.h"

class TextureLavaFlowFX : public TextureFX
{
private:
	std::vector<float> red0;
	std::vector<float> red1;
	std::vector<float> green0;
	std::vector<float> green1;
	int_t tickCounter = 0;

public:
	TextureLavaFlowFX();

	void onTick() override;
};

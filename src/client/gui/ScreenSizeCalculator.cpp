#include "client/gui/ScreenSizeCalculator.h"

#include <cmath>

#include "client/Options.h"

ScreenSizeCalculator::ScreenSizeCalculator(const Options &options, int_t width, int_t height)
{
	w = width;
	h = height;
	scale = 1;

	int_t maxScale = options.guiScale;
	if (maxScale == 0)
		maxScale = 1000;

	while (scale < maxScale && width / (scale + 1) >= 320 && height / (scale + 1) >= 240)
		scale++;

	w = static_cast<int_t>(std::ceil(static_cast<double>(width) / static_cast<double>(scale)));
	h = static_cast<int_t>(std::ceil(static_cast<double>(height) / static_cast<double>(scale)));
}

int ScreenSizeCalculator::getWidth() const
{
	return w;
}

int ScreenSizeCalculator::getHeight() const
{
	return h;
}

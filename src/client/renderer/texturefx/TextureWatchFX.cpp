#include "client/renderer/texturefx/TextureWatchFX.h"

#include <cmath>
#include "java/Math.h"

#include "client/Minecraft.h"
#include "client/renderer/texturefx/TextureItemFX.h"
#include "client/renderer/texturefx/TileSize.h"
#include "util/Mth.h"
#include "world/level/dimension/Dimension.h"

namespace
{
	constexpr int_t CLOCK_ICON_INDEX = 70;
}

TextureWatchFX::TextureWatchFX(Minecraft &minecraft) : TextureFX(CLOCK_ICON_INDEX), minecraft(minecraft),
	watchIconImageData(TileSize::numPixels), dialImageData(TileSize::numPixels)
{
	tileImage = 1;
	TextureItemFX::loadIconPixels(minecraft, u"/gui/items.png", iconIndex, watchIconImageData);
	TextureItemFX::loadWholePixels(minecraft, u"/misc/dial.png", dialImageData);
}

void TextureWatchFX::onTick()
{
	double angle = 0.0;
	if (minecraft.level != nullptr && minecraft.player != nullptr)
	{
		angle = static_cast<double>(-minecraft.level->getTimeOfDay(1.0f) * Mth::PI * 2.0f);
		if (minecraft.level->dimension != nullptr && minecraft.level->dimension->foggy)
			angle = Math::random() * static_cast<double>(Mth::PI) * 2.0;
	}

	double delta = angle - rotation;
	while (delta < -3.141592653589793)
		delta += 6.283185307179586;
	while (delta >= 3.141592653589793)
		delta -= 6.283185307179586;
	if (delta < -1.0)
		delta = -1.0;
	if (delta > 1.0)
		delta = 1.0;

	rotationDelta += delta * 0.1;
	rotationDelta *= 0.8;
	rotation += rotationDelta;
	double sinRot = std::sin(rotation);
	double cosRot = std::cos(rotation);

	for (int_t i = 0; i < TileSize::numPixels; ++i)
	{
		int_t alpha = (watchIconImageData[i] >> 24) & 255;
		int_t red = (watchIconImageData[i] >> 16) & 255;
		int_t green = (watchIconImageData[i] >> 8) & 255;
		int_t blue = watchIconImageData[i] & 255;
		if (red == blue && green == 0 && blue > 0)
		{
			double sampleX = -((static_cast<double>(i % TileSize::size) / TileSize::sizeMinus1Double) - 0.5);
			double sampleY = (static_cast<double>(i / TileSize::size) / TileSize::sizeMinus1Double) - 0.5;
			int_t dialX = static_cast<int_t>((sampleX * cosRot + sampleY * sinRot + 0.5) * TileSize::sizeDouble);
			int_t dialY = static_cast<int_t>((sampleY * cosRot - sampleX * sinRot + 0.5) * TileSize::sizeDouble);
			int_t dialPixel = (dialX & TileSize::sizeMinus1) + (dialY & TileSize::sizeMinus1) * TileSize::size;
			int_t intensity = red;
			alpha = (dialImageData[dialPixel] >> 24) & 255;
			red = ((dialImageData[dialPixel] >> 16) & 255) * red / 255;
			green = ((dialImageData[dialPixel] >> 8) & 255) * intensity / 255;
			blue = (dialImageData[dialPixel] & 255) * intensity / 255;
		}

		TextureItemFX::applyAnaglyph(red, green, blue, anaglyphEnabled);
		imageData[i * 4 + 0] = static_cast<byte_t>(red);
		imageData[i * 4 + 1] = static_cast<byte_t>(green);
		imageData[i * 4 + 2] = static_cast<byte_t>(blue);
		imageData[i * 4 + 3] = static_cast<byte_t>(alpha);
	}
}

#include "client/renderer/texturefx/TextureCompassFX.h"

#include <cmath>
#include "java/Math.h"

#include "client/Minecraft.h"
#include "client/renderer/texturefx/TextureItemFX.h"
#include "client/renderer/texturefx/TileSize.h"
#include "util/Mth.h"
#include "world/level/dimension/Dimension.h"

namespace
{
	constexpr int_t COMPASS_ICON_INDEX = 54;
}

TextureCompassFX::TextureCompassFX(Minecraft &minecraft) : TextureFX(COMPASS_ICON_INDEX), minecraft(minecraft),
	compassIconImageData(TileSize::numPixels)
{
	tileImage = 1;
	TextureItemFX::loadIconPixels(minecraft, u"/gui/items.png", iconIndex, compassIconImageData);
}

void TextureCompassFX::onTick()
{
	for (int_t i = 0; i < TileSize::numPixels; ++i)
	{
		int_t alpha = (compassIconImageData[i] >> 24) & 255;
		int_t red = (compassIconImageData[i] >> 16) & 255;
		int_t green = (compassIconImageData[i] >> 8) & 255;
		int_t blue = compassIconImageData[i] & 255;
		TextureItemFX::applyAnaglyph(red, green, blue, anaglyphEnabled);
		imageData[i * 4 + 0] = static_cast<byte_t>(red);
		imageData[i * 4 + 1] = static_cast<byte_t>(green);
		imageData[i * 4 + 2] = static_cast<byte_t>(blue);
		imageData[i * 4 + 3] = static_cast<byte_t>(alpha);
	}

	double angle = 0.0;
	if (minecraft.level != nullptr && minecraft.player != nullptr)
	{
		double dx = static_cast<double>(minecraft.level->xSpawn) - minecraft.player->x;
		double dz = static_cast<double>(minecraft.level->zSpawn) - minecraft.player->z;
		angle = static_cast<double>(minecraft.player->yRot - 90.0f) * 3.141592653589793 / 180.0 - std::atan2(dz, dx);
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

	for (int_t i = TileSize::compassCrossMin; i <= TileSize::compassCrossMax; ++i)
	{
		int_t x = static_cast<int_t>(TileSize::compassCenterMax + cosRot * i * 0.3);
		int_t y = static_cast<int_t>(TileSize::compassCenterMin - sinRot * i * 0.3 * 0.5);
		int_t pixel = y * TileSize::size + x;
		int_t red = 100;
		int_t green = 100;
		int_t blue = 100;
		TextureItemFX::applyAnaglyph(red, green, blue, anaglyphEnabled);
		imageData[pixel * 4 + 0] = static_cast<byte_t>(red);
		imageData[pixel * 4 + 1] = static_cast<byte_t>(green);
		imageData[pixel * 4 + 2] = static_cast<byte_t>(blue);
		imageData[pixel * 4 + 3] = static_cast<byte_t>(255);
	}

	for (int_t i = TileSize::compassNeedleMin; i <= TileSize::compassNeedleMax; ++i)
	{
		int_t x = static_cast<int_t>(TileSize::compassCenterMax + sinRot * i * 0.3);
		int_t y = static_cast<int_t>(TileSize::compassCenterMin + cosRot * i * 0.3 * 0.5);
		int_t pixel = y * TileSize::size + x;
		int_t red = i >= 0 ? 255 : 100;
		int_t green = i >= 0 ? 20 : 100;
		int_t blue = i >= 0 ? 20 : 100;
		TextureItemFX::applyAnaglyph(red, green, blue, anaglyphEnabled);
		imageData[pixel * 4 + 0] = static_cast<byte_t>(red);
		imageData[pixel * 4 + 1] = static_cast<byte_t>(green);
		imageData[pixel * 4 + 2] = static_cast<byte_t>(blue);
		imageData[pixel * 4 + 3] = static_cast<byte_t>(255);
	}
}

#include "client/renderer/texturefx/TextureItemFX.h"

#include "client/Minecraft.h"
#include "client/renderer/texturefx/TileSize.h"
#include "java/BufferedImage.h"

namespace
{
	uint32_t packPixel(const unsigned char *rawPixels, int_t pixelIndex)
	{
		uint32_t red = rawPixels[pixelIndex * 4 + 0];
		uint32_t green = rawPixels[pixelIndex * 4 + 1];
		uint32_t blue = rawPixels[pixelIndex * 4 + 2];
		uint32_t alpha = rawPixels[pixelIndex * 4 + 3];
		return (alpha << 24) | (red << 16) | (green << 8) | blue;
	}

	bool loadRegion(const BufferedImage &image, int_t startX, int_t startY, std::vector<uint32_t> &out)
	{
		if (image.getWidth() < startX + TileSize::size || image.getHeight() < startY + TileSize::size)
			return false;

		out.resize(TileSize::numPixels);
		const unsigned char *rawPixels = image.getRawPixels();
		for (int_t y = 0; y < TileSize::size; ++y)
		{
			for (int_t x = 0; x < TileSize::size; ++x)
			{
				int_t pixelIndex = (startX + x) + (startY + y) * image.getWidth();
				out[y * TileSize::size + x] = packPixel(rawPixels, pixelIndex);
			}
		}
		return true;
	}

	BufferedImage loadImage(Minecraft &minecraft, const jstring &resourceName)
	{
		return minecraft.textures.getResourceImage(resourceName);
	}
}

bool TextureItemFX::loadIconPixels(Minecraft &minecraft, const jstring &resourceName, int_t iconIndex, std::vector<uint32_t> &out)
{
	try
	{
		BufferedImage image = loadImage(minecraft, resourceName);
		return loadRegion(image, (iconIndex % 16) * TileSize::size, (iconIndex / 16) * TileSize::size, out);
	}
	catch (...)
	{
		return false;
	}
}

bool TextureItemFX::loadWholePixels(Minecraft &minecraft, const jstring &resourceName, std::vector<uint32_t> &out)
{
	try
	{
		BufferedImage image = loadImage(minecraft, resourceName);
		return loadRegion(image, 0, 0, out);
	}
	catch (...)
	{
		return false;
	}
}

void TextureItemFX::applyAnaglyph(int_t &red, int_t &green, int_t &blue, bool enabled)
{
	if (!enabled)
		return;

	int_t grayRed = (red * 30 + green * 59 + blue * 11) / 100;
	int_t grayGreen = (red * 30 + green * 70) / 100;
	int_t grayBlue = (red * 30 + blue * 70) / 100;
	red = grayRed;
	green = grayGreen;
	blue = grayBlue;
}

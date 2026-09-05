#include "client/renderer/texturefx/TextureFlamesFX.h"

#include "java/Math.h"

#include "client/renderer/texturefx/TileSize.h"
#include "world/level/tile/FireTile.h"

TextureFlamesFX::TextureFlamesFX(int_t variant) : TextureFX(Tile::fire.tex + variant * 16),
	current(TileSize::flameArraySize), next(TileSize::flameArraySize)
{
}

void TextureFlamesFX::onTick()
{
	for (int_t x = 0; x < TileSize::size; ++x)
	{
		for (int_t y = 0; y < TileSize::flameHeight; ++y)
		{
			int_t count = 18;
			float total = current[x + ((y + 1) % TileSize::flameHeight) * TileSize::size] * static_cast<float>(count);

			for (int_t sx = x - 1; sx <= x + 1; ++sx)
			{
				for (int_t sy = y; sy <= y + 1; ++sy)
				{
					if (sx >= 0 && sy >= 0 && sx < TileSize::size && sy < TileSize::flameHeight)
						total += current[sx + sy * TileSize::size];
					++count;
				}
			}

			next[x + y * TileSize::size] = total / (static_cast<float>(count) * TileSize::flameNudge);
			if (y >= TileSize::flameHeightMinus1)
			{
				double r0 = Math::random();
				double r1 = Math::random();
				double r2 = Math::random();
				double r3 = Math::random();
				next[x + y * TileSize::size] = static_cast<float>(r0 * r1 * r2 * 4.0 + r3 * static_cast<double>(0.1f) + static_cast<double>(0.2f));
			}
		}
	}

	current.swap(next);

	for (int_t i = 0; i < TileSize::numPixels; ++i)
	{
		float flame = current[i] * 1.8f;
		if (flame > 1.0f)
			flame = 1.0f;
		if (flame < 0.0f)
			flame = 0.0f;

		int_t red = static_cast<int_t>(flame * 155.0f + 100.0f);
		int_t green = static_cast<int_t>(flame * flame * 255.0f);
		int_t blue = static_cast<int_t>(flame * flame * flame * flame * flame * flame * flame * flame * flame * flame * 255.0f);
		int_t alpha = flame < 0.5f ? 0 : 255;

		if (anaglyphEnabled)
		{
			int_t grayRed = (red * 30 + green * 59 + blue * 11) / 100;
			int_t grayGreen = (red * 30 + green * 70) / 100;
			int_t grayBlue = (red * 30 + blue * 70) / 100;
			red = grayRed;
			green = grayGreen;
			blue = grayBlue;
		}

		imageData[i * 4 + 0] = static_cast<byte_t>(red);
		imageData[i * 4 + 1] = static_cast<byte_t>(green);
		imageData[i * 4 + 2] = static_cast<byte_t>(blue);
		imageData[i * 4 + 3] = static_cast<byte_t>(alpha);
	}
}

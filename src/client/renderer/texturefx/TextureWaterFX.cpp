#include "client/renderer/texturefx/TextureWaterFX.h"

#include "java/Math.h"
#include "java/Number.h"

#include "world/level/tile/Tile.h"
#include "world/level/tile/LiquidTile.h"
#include "client/renderer/texturefx/TileSize.h"

TextureWaterFX::TextureWaterFX() : TextureFX(Tile::water.tex),
	red0(TileSize::numPixels), red1(TileSize::numPixels), green0(TileSize::numPixels), green1(TileSize::numPixels)
{
}

void TextureWaterFX::onTick()
{
	tickCounter = Java::intFromBits(static_cast<uint_t>(tickCounter) + 1u);

	for (int_t x = 0; x < TileSize::size; ++x)
	{
		for (int_t y = 0; y < TileSize::size; ++y)
		{
			float val = 0.0f;

			for (int_t xx = x - 1; xx <= x + 1; ++xx)
			{
				int_t xi = xx & TileSize::sizeMinus1;
				int_t yi = y & TileSize::sizeMinus1;
				val += red0[xi + yi * TileSize::size];
			}

			red1[x + y * TileSize::size] = val / 3.3f + green0[x + y * TileSize::size] * 0.8f;
		}
	}

	for (int_t x = 0; x < TileSize::size; ++x)
	{
		for (int_t y = 0; y < TileSize::size; ++y)
		{
			green0[x + y * TileSize::size] += green1[x + y * TileSize::size] * 0.05f;
			if (green0[x + y * TileSize::size] < 0.0f)
				green0[x + y * TileSize::size] = 0.0f;

			green1[x + y * TileSize::size] -= 0.1f;
			if (Math::random() < 0.05)
				green1[x + y * TileSize::size] = 0.5f;
		}
	}

	
	red0.swap(red1);

	for (int_t i = 0; i < TileSize::numPixels; ++i)
	{
		float val = red0[i];
		if (val > 1.0f) val = 1.0f;
		if (val < 0.0f) val = 0.0f;

		float sq = val * val;
		int_t r = static_cast<int_t>(32.0f + sq * 32.0f);
		int_t g = static_cast<int_t>(50.0f + sq * 64.0f);
		int_t b = 255;
		int_t a = static_cast<int_t>(146.0f + sq * 50.0f);

		if (anaglyphEnabled)
		{
			int_t rr = (r * 30 + g * 59 + b * 11) / 100;
			int_t gg = (r * 30 + g * 70) / 100;
			int_t bb = (r * 30 + b * 70) / 100;
			r = rr;
			g = gg;
			b = bb;
		}

		imageData[i * 4 + 0] = static_cast<byte_t>(r);
		imageData[i * 4 + 1] = static_cast<byte_t>(g);
		imageData[i * 4 + 2] = static_cast<byte_t>(b);
		imageData[i * 4 + 3] = static_cast<byte_t>(a);
	}
}

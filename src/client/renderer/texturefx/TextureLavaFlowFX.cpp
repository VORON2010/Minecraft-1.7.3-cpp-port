#include "client/renderer/texturefx/TextureLavaFlowFX.h"

#include "java/Math.h"
#include "java/Number.h"
#include "util/Mth.h"

#include "world/level/tile/Tile.h"
#include "world/level/tile/LiquidTile.h"
#include "client/renderer/texturefx/TileSize.h"

TextureLavaFlowFX::TextureLavaFlowFX() : TextureFX(Tile::lava.tex + 1),
	red0(TileSize::numPixels), red1(TileSize::numPixels), green0(TileSize::numPixels), green1(TileSize::numPixels)
{
	tileSize = 2;
}

void TextureLavaFlowFX::onTick()
{
	tickCounter = Java::intFromBits(static_cast<uint_t>(tickCounter) + 1u);

	for (int_t x = 0; x < TileSize::size; ++x)
	{
		for (int_t y = 0; y < TileSize::size; ++y)
		{
			float val = 0.0f;
			int_t sinY = static_cast<int_t>(Mth::sin(static_cast<float>(y) * Mth::PI * 2.0f / TileSize::sizeFloat) * 1.2f);
			int_t sinX = static_cast<int_t>(Mth::sin(static_cast<float>(x) * Mth::PI * 2.0f / TileSize::sizeFloat) * 1.2f);

			for (int_t xx = x - 1; xx <= x + 1; ++xx)
			{
				for (int_t yy = y - 1; yy <= y + 1; ++yy)
				{
					int_t xi = (xx + sinY) & TileSize::sizeMinus1;
					int_t yi = (yy + sinX) & TileSize::sizeMinus1;
					val += red0[xi + yi * TileSize::size];
				}
			}

			red1[x + y * TileSize::size] = val / 10.0f +
				(green0[((x + 0) & TileSize::sizeMinus1) + ((y + 0) & TileSize::sizeMinus1) * TileSize::size] +
				 green0[((x + 1) & TileSize::sizeMinus1) + ((y + 0) & TileSize::sizeMinus1) * TileSize::size] +
				 green0[((x + 1) & TileSize::sizeMinus1) + ((y + 1) & TileSize::sizeMinus1) * TileSize::size] +
				 green0[((x + 0) & TileSize::sizeMinus1) + ((y + 1) & TileSize::sizeMinus1) * TileSize::size]) / 4.0f * 0.8f;

			green0[x + y * TileSize::size] += green1[x + y * TileSize::size] * 0.01f;
			if (green0[x + y * TileSize::size] < 0.0f)
				green0[x + y * TileSize::size] = 0.0f;

			green1[x + y * TileSize::size] -= 0.06f;
			if (Math::random() < 0.005)
				green1[x + y * TileSize::size] = 1.5f;
		}
	}

	
	red0.swap(red1);

	for (int_t i = 0; i < TileSize::numPixels; ++i)
	{
		float val = red0[(static_cast<uint_t>(i) - static_cast<uint_t>(tickCounter / 3) * static_cast<uint_t>(TileSize::size)) & static_cast<uint_t>(TileSize::numPixelsMinus1)] * 2.0f;
		if (val > 1.0f) val = 1.0f;
		if (val < 0.0f) val = 0.0f;

		int_t r = static_cast<int_t>(val * 100.0f + 155.0f);
		int_t g = static_cast<int_t>(val * val * 255.0f);
		int_t b = static_cast<int_t>(val * val * val * val * 128.0f);

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
		imageData[i * 4 + 3] = static_cast<byte_t>(255);
	}
}

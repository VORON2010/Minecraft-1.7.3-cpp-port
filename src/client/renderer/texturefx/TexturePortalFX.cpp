#include "client/renderer/texturefx/TexturePortalFX.h"

#include <cmath>

#include "client/renderer/texturefx/TileSize.h"
#include "java/Random.h"
#include "java/Number.h"
#include "util/Mth.h"

TexturePortalFX::TexturePortalFX(int_t iconIndex) : TextureFX(iconIndex),
	frames(32, std::vector<byte_t>(TileSize::numBytes))
{
	Random random(100L);
	for (int_t frame = 0; frame < 32; ++frame)
	{
		for (int_t x = 0; x < TileSize::size; ++x)
		{
			for (int_t y = 0; y < TileSize::size; ++y)
			{
				float value = 0.0f;
				for (int_t layer = 0; layer < 2; ++layer)
				{
					float xOffset = static_cast<float>(layer * TileSize::sizeHalf);
					float yOffset = static_cast<float>(layer * TileSize::sizeHalf);
					float sampleX = (static_cast<float>(x) - xOffset) / TileSize::sizeFloat * 2.0f;
					float sampleY = (static_cast<float>(y) - yOffset) / TileSize::sizeFloat * 2.0f;
					if (sampleX < -1.0f)
						sampleX += 2.0f;
					if (sampleX >= 1.0f)
						sampleX -= 2.0f;
					if (sampleY < -1.0f)
						sampleY += 2.0f;
					if (sampleY >= 1.0f)
						sampleY -= 2.0f;

					float distance = sampleX * sampleX + sampleY * sampleY;
					float angle = static_cast<float>(std::atan2(static_cast<double>(sampleY), static_cast<double>(sampleX)))
						+ (static_cast<float>(frame) / 32.0f * Mth::PI * 2.0f - distance * 10.0f + static_cast<float>(layer * 2))
							* static_cast<float>(layer * 2 - 1);
					float swirl = (Mth::sin(angle) + 1.0f) / 2.0f;
					swirl /= distance + 1.0f;
					value += swirl * 0.5f;
				}

				value += random.nextFloat() * 0.1f;
				int_t blue = static_cast<int_t>(value * 100.0f + 155.0f);
				int_t red = static_cast<int_t>(value * value * 200.0f + 55.0f);
				int_t green = static_cast<int_t>(value * value * value * value * 255.0f);
				int_t alpha = static_cast<int_t>(value * 100.0f + 155.0f);
				int_t pixel = y * TileSize::size + x;
				frames[frame][pixel * 4 + 0] = static_cast<byte_t>(red);
				frames[frame][pixel * 4 + 1] = static_cast<byte_t>(green);
				frames[frame][pixel * 4 + 2] = static_cast<byte_t>(blue);
				frames[frame][pixel * 4 + 3] = static_cast<byte_t>(alpha);
			}
		}
	}
}

void TexturePortalFX::onTick()
{
	portalTickCounter = Java::intFromBits(static_cast<uint_t>(portalTickCounter) + 1u);
	const std::vector<byte_t> &frame = frames[portalTickCounter & 31];
	for (int_t i = 0; i < TileSize::numPixels; ++i)
	{
		int_t red = frame[i * 4 + 0] & 255;
		int_t green = frame[i * 4 + 1] & 255;
		int_t blue = frame[i * 4 + 2] & 255;
		int_t alpha = frame[i * 4 + 3] & 255;
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

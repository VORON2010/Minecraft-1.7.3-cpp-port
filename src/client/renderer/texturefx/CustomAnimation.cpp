#include "client/renderer/texturefx/CustomAnimation.h"

#include <cstring>

#include "client/renderer/Textures.h"
#include "client/renderer/texturefx/TileSize.h"
#include "java/BufferedImage.h"
#include "java/Random.h"

namespace
{
	Random random;
}

CustomAnimation::CustomAnimation(Textures &textures, int_t tileNumber, int_t tileImage, int_t tileSize,
	const jstring &name, int_t minScrollDelay, int_t maxScrollDelay)
	: TextureFX(tileNumber), minScrollDelay(minScrollDelay), maxScrollDelay(maxScrollDelay)
{
	this->tileImage = tileImage;
	this->tileSize = tileSize;
	scrolling = minScrollDelay >= 0;

	BufferedImage custom;
	try
	{
		custom = textures.getResourceImage(u"/custom_" + name + u".png");
	}
	catch (...)
	{
	}

	if (custom.getWidth() == 0)
	{
		BufferedImage tiles;
		try
		{
			tiles = textures.getResourceImage(u"/terrain.png");
		}
		catch (...)
		{
			return;
		}
		int_t tileX = (tileNumber % 16) * TileSize::size;
		int_t tileY = (tileNumber / 16) * TileSize::size;
		tiles.getRGB(tileX, tileY, TileSize::size, TileSize::size,
			reinterpret_cast<unsigned char *>(imageData.data()));
		if (scrolling)
			temp.resize(TileSize::size * 4);
	}
	else
	{
		numFrames = custom.getHeight() / custom.getWidth();
		source.resize(custom.getWidth() * custom.getHeight() * 4);
		custom.getRGB(0, 0, custom.getWidth(), custom.getHeight(),
			reinterpret_cast<unsigned char *>(source.data()));
	}
}

void CustomAnimation::onTick()
{
	if (!source.empty())
	{
		if (++frame >= numFrames)
			frame = 0;
		std::memcpy(imageData.data(), source.data() + frame * TileSize::numBytes, TileSize::numBytes);
	}
	else if (scrolling)
	{
		if (maxScrollDelay <= 0 || --timer <= 0)
		{
			if (maxScrollDelay > 0)
				timer = random.nextInt(maxScrollDelay - minScrollDelay + 1) + minScrollDelay;
			std::memcpy(temp.data(), imageData.data() + (TileSize::size - 1) * TileSize::size * 4, TileSize::size * 4);
			std::memmove(imageData.data() + TileSize::size * 4, imageData.data(), TileSize::size * (TileSize::size - 1) * 4);
			std::memcpy(imageData.data(), temp.data(), TileSize::size * 4);
		}
	}
}

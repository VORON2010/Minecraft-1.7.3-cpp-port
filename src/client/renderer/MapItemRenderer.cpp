#include "client/renderer/MapItemRenderer.h"

#include "client/gui/Font.h"
#include "client/Options.h"
#include "client/renderer/Textures.h"
#include "client/renderer/Tesselator.h"
#include "world/level/MapData.h"
#include "world/level/material/MapColor.h"

#include "OpenGL.h"

MapItemRenderer::MapItemRenderer(Font &font, Options &options) : font(font), options(options)
{
	std::vector<int_t> ib(1);
	MemoryTracker::genTextures(ib);
	textureId = ib[0];
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void MapItemRenderer::render(MapData &data, Textures &textures)
{
	for (int_t i = 0; i < 16384; ++i)
	{
		byte_t colorByte = data.colors[i];
		if (colorByte / 4 == 0)
		{
			pixels[i] = (((i + i / 128) & 1) * 8 + 16) << 24;
		}
		else
		{
			int_t colorValue = MapColor::mapColorArray[colorByte / 4]->colorValue;
			int_t shade = colorByte & 3;
			short_t brightness = 220;
			if (shade == 2)
				brightness = 255;
			else if (shade == 0)
				brightness = 180;

			int_t r = ((colorValue >> 16) & 0xFF) * brightness / 255;
			int_t g = ((colorValue >> 8) & 0xFF) * brightness / 255;
			int_t b = (colorValue & 0xFF) * brightness / 255;

			if (options.anaglyph3d)
			{
				int_t rr = (r * 30 + g * 59 + b * 11) / 100;
				int_t gg = (r * 30 + g * 70) / 100;
				int_t bb = (r * 30 + b * 70) / 100;
				r = rr;
				g = gg;
				b = bb;
			}

			
			pixels[i] = 0xFF000000 | (b << 16) | (g << 8) | r;
		}
	}

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	Tesselator &t = Tesselator::instance;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_ALPHA_TEST);

	t.begin();
	t.vertexUV(0.0, 128.0, -0.01, 0.0, 1.0);
	t.vertexUV(128.0, 128.0, -0.01, 1.0, 1.0);
	t.vertexUV(128.0, 0.0, -0.01, 1.0, 0.0);
	t.vertexUV(0.0, 0.0, -0.01, 0.0, 0.0);
	t.end();

	glEnable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);

	
	int_t iconTex = textures.loadTexture(u"/misc/mapicons.png");
	glBindTexture(GL_TEXTURE_2D, iconTex);

	for (const auto &coord : data.mapCoords)
	{
		glPushMatrix();
		glTranslatef(coord->x / 2.0f + 64.0f, coord->z / 2.0f + 64.0f, -0.02f);
		glRotatef(coord->rot * 360.0f / 16.0f, 0.0f, 0.0f, 1.0f);
		glScalef(4.0f, 4.0f, 3.0f);
		glTranslatef(-(2.0f / 16.0f), 2.0f / 16.0f, 0.0f);

		float u0 = (coord->icon % 4 + 0) / 4.0f;
		float v0 = (coord->icon / 4 + 0) / 4.0f;
		float u1 = (coord->icon % 4 + 1) / 4.0f;
		float v1 = (coord->icon / 4 + 1) / 4.0f;

		t.begin();
		t.vertexUV(-1.0, 1.0, 0.0, u0, v0);
		t.vertexUV(1.0, 1.0, 0.0, u1, v0);
		t.vertexUV(1.0, -1.0, 0.0, u1, v1);
		t.vertexUV(-1.0, -1.0, 0.0, u0, v1);
		t.end();

		glPopMatrix();
	}

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.04f);
	glScalef(1.0f, 1.0f, 1.0f);
	font.draw(data.id, 0, 0, 0xFF000000);
	glPopMatrix();
}

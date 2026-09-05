#include "client/gui/Font.h"

#include "SharedConstants.h"
#include "client/Options.h"
#include "client/renderer/Textures.h"
#include "client/renderer/Tesselator.h"

#include "java/BufferedImage.h"

Font::Font(Options &options, const jstring &name, Textures &textures)
{
	initialize(options, name, textures);
}

void Font::initialize(Options &options, const jstring &name, Textures &textures)
{
	BufferedImage img = textures.getResourceImage(name);

	int_t w = img.getWidth();
	const unsigned char *rawPixels = img.getRawPixels();
	int_t cellSize = w / 16;

	
	for (int_t i = 0; i < 256; i++)
	{
		int_t xt = i % 16;
		int_t yt = i / 16;

		int_t x = cellSize - 1;
		for (; x >= 0; x--)
		{
			int_t xPixel = xt * cellSize + x;
			bool emptyColumn = true;
			for (int_t y = 0; y < cellSize && emptyColumn; y++)
			{
				int_t yPixel = (yt * cellSize + y) * w;
				int_t pixel = rawPixels[(xPixel + yPixel) * 4 + 3] & 0xFF;
				if (pixel > 0)
					emptyColumn = false;
			}
			if (!emptyColumn)
				break;
		}

		if (i == 32) x = w / 64;
		charWidths[i] = (128 * x + 256) / w;
	}

	fontTexture = textures.getTexture(img);

	listPos = MemoryTracker::genLists(256 + 32);
	Tesselator &t = Tesselator::instance;
	for (int_t j = 0; j < 256; j++)
	{
		glNewList(listPos + j, GL_COMPILE);

		t.begin();
		
		int_t ix = j % 16 * 8;
		int_t iy = j / 16 * 8;

		float s = 7.99f;

		float uo = 0.0f;
		float vo = 0.0f;

		t.vertexUV(0.0, (0.0f + s), 0.0, (ix / 128.0f + uo), ((iy + s) / 128.0f + vo));
		t.vertexUV((0.0f + s), (0.0f + s), 0.0, ((ix + s) / 128.0f + uo), ((iy + s) / 128.0f + vo));
		t.vertexUV((0.0f + s), 0.0, 0.0, ((ix + s) / 128.0f + uo), (iy / 128.0f + vo));
		t.vertexUV(0.0, 0.0, 0.0, (ix / 128.0f + uo), (iy / 128.0f + vo));

		t.end();

		glTranslatef(charWidths[j], 0.0f, 0.0f);
		glEndList();
	}

	for (int_t j = 0; j < 32; j++)
	{
		int_t br = ((j >> 3) & 1) * 85;
		int_t r = ((j >> 2) & 1) * 170 + br;
		int_t g = ((j >> 1) & 1) * 170 + br;
		int_t b = ((j >> 0) & 1) * 170 + br;
		if (j == 6)
			r += 85;
	
		bool darken = (j >= 16);
	
		if (options.anaglyph3d)
		{
			int_t cr = (r * 30 + g * 59 + b * 11) / 100;
			int_t cg = (r * 30 + g * 70) / 100;
			int_t cb = (r * 30 + b * 70) / 100;
			r = cr;
			g = cg;
			b = cb;
		}
	
		if (darken)
		{
			r /= 4;
			g /= 4;
			b /= 4;
		}
	
		colorCodes[j] = (r << 16) | (g << 8) | b;
	
		glNewList(listPos + 256 + j, GL_COMPILE);
		glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);
		glEndList();
	}
}

void Font::drawShadow(const jstring &str, int_t x, int_t y, int_t color)
{
	draw(str, x + 1, y + 1, color, true);
	draw(str, x, y, color);
}

void Font::draw(const jstring &str, int_t x, int_t y, int_t color)
{
	draw(str, x, y, color, false);
}

void Font::draw(const jstring &str, int_t x, int_t y, int_t color, bool darken)
{
	if (darken)
	{
		int_t oldAlpha = color & 0xFF000000;
		color = (color & 0xFCFCFC) >> 2;
		color += oldAlpha;
	}

	glBindTexture(GL_TEXTURE_2D, fontTexture);

	float r = ((color >> 16) & 0xFF) / 255.0f;
	float g = ((color >> 8) & 0xFF) / 255.0f;
	float b = (color & 0xFF) / 255.0f;
	float a = ((color >> 24) & 0xFF) / 255.0f;
	if ((color & 0xFF000000) == 0) a = 1.0f;
	
	glColor4f(r, g, b, a);
	
	ib.clear();
	glPushMatrix();
	glTranslatef(x, y, 0.0f);
	
	auto flushGlyphs = [&]() {
		if (!ib.empty())
		{
			glCallLists(ib.size(), GL_UNSIGNED_INT, ib.data());
			ib.clear();
		}
	};
	
	for (int_t i = 0; i < str.length(); i++)
	{
		if (str[i] == 167 && str.length() > i + 1)
		{
			static const jstring codes = u"0123456789abcdef";
			char_t code = str[i + 1];
			if (code >= u'A' && code <= u'F')
				code = static_cast<char_t>(code - u'A' + u'a');
			int_t cc = codes.find(code);
			if (cc == jstring::npos || cc > 15) cc = 15;
			flushGlyphs();
			int_t packed = colorCodes[cc + (darken ? 16 : 0)];
			float cr = ((packed >> 16) & 0xFF) / 255.0f;
			float cg = ((packed >> 8) & 0xFF) / 255.0f;
			float cb = (packed & 0xFF) / 255.0f;
			glColor4f(cr, cg, cb, a);
			i++;
			continue;
		}
	
		int_t ch = SharedConstants::acceptableLetters.find(str[i]);
		if (ch != jstring::npos)
			ib.push_back(listPos + ch + 32);
	}
	
	flushGlyphs();
	glPopMatrix();
}

int_t Font::width(const jstring &str)
{
	int_t len = 0;

	for (int_t i = 0; i < str.length(); i++)
	{
		char_t c = str[i];
		if (c == 167 && i + 1 < str.length())
		{
			i++;
			continue;
		}

		int_t ch = SharedConstants::acceptableLetters.find(c);
		if (ch != jstring::npos)
			len += charWidths.at(ch + 32);
	}

	return len;
}

namespace
{
	std::vector<jstring> splitText(const jstring &text, char_t delimiter)
	{
		std::vector<jstring> parts;
		size_t start = 0;
		while (true)
		{
			size_t end = text.find(delimiter, start);
			parts.push_back(text.substr(start, end == jstring::npos ? end : end - start));
			if (end == jstring::npos)
				return parts;
			start = end + 1;
		}
	}

	jstring trim(const jstring &text)
	{
		size_t first = text.find_first_not_of(u" \t\r\n");
		if (first == jstring::npos)
			return u"";
		size_t last = text.find_last_not_of(u" \t\r\n");
		return text.substr(first, last - first + 1);
	}

	std::vector<jstring> wrap(Font &font, const jstring &text, int_t maxWidth)
	{
		std::vector<jstring> lines;
		for (const jstring &paragraph : splitText(text, u'\n'))
		{
			auto words = splitText(paragraph, u' ');
			size_t word = 0;
			while (word < words.size())
			{
				jstring line = words[word++] + u" ";
				while (word < words.size() && font.width(line + words[word]) < maxWidth)
					line += words[word++] + u" ";

				while (font.width(line) > maxWidth)
				{
					size_t split = 0;
					while (split < line.size() && font.width(line.substr(0, split + 1)) <= maxWidth)
						++split;
					if (split == 0)
						split = 1;
					jstring part = line.substr(0, split);
					if (!trim(part).empty())
						lines.push_back(part);
					line = line.substr(split);
				}

				if (!trim(line).empty())
					lines.push_back(line);
			}
		}
		if (lines.empty())
			lines.push_back(u"");
		return lines;
	}
}

void Font::drawWordWrap(const jstring &str, int_t x, int_t y, int_t maxWidth, int_t color)
{
	for (const jstring &line : wrap(*this, str, maxWidth))
	{
		draw(line, x, y, color);
		y += 8;
	}
}

int_t Font::wordWrapHeight(const jstring &str, int_t maxWidth)
{
	return static_cast<int_t>(wrap(*this, str, maxWidth).size()) * 8;
}

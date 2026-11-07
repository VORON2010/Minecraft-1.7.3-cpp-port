#include "client/gui/WinScreen.h"

#include <memory>

#include "OpenGL.h"
#include "client/Minecraft.h"
#include "client/renderer/Tesselator.h"
#include "java/Resource.h"
#include "lwjgl/Keyboard.h"

WinScreen::WinScreen(Minecraft &minecraft) : Screen(minecraft)
{
}

void WinScreen::tick()
{
	ticks++;
	float end = static_cast<float>(totalHeight + height + height + 24) / speed;
	if (static_cast<float>(ticks) > end)
		respawn();
}

void WinScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	(void)eventCharacter;
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE)
		respawn();
}

void WinScreen::respawn()
{
	minecraft.setScreen(nullptr);
	minecraft.respawnPlayer(0, true);
}

bool WinScreen::isPauseScreen()
{
	return true;
}

namespace
{
	jstring replaceAll(jstring s, const jstring &from, const jstring &to)
	{
		size_t pos = 0;
		while ((pos = s.find(from, pos)) != jstring::npos)
		{
			s.replace(pos, from.size(), to);
			pos += to.size();
		}
		return s;
	}
}

void WinScreen::readLines(const jstring &path, bool tabs, Random *obfuscate)
{
	std::unique_ptr<std::istream> is(Resource::getResource(path));
	if (is == nullptr)
		return;
	const jstring player = minecraft.user != nullptr ? minecraft.user->name : u"Player";
	const jstring marker = u"§f§k§a§b";
	std::string raw;
	while (std::getline(*is, raw))
	{
		if (!raw.empty() && raw.back() == '\r')
			raw.pop_back();
		jstring line = replaceAll(String::fromUTF8(raw), u"PLAYERNAME", player);
		if (tabs)
			line = replaceAll(line, u"\t", u"    ");
		if (obfuscate != nullptr)
		{
			size_t pos;
			while ((pos = line.find(marker)) != jstring::npos)
				line = line.substr(0, pos) + u"§f§k" + jstring(u"XXXXXXXX").substr(0, obfuscate->nextInt(4) + 3) + line.substr(pos + marker.size());
		}
		if (line.empty())
			lines.push_back(line);
		lines.push_back(line);
	}
}

void WinScreen::init()
{
	if (loaded)
		return;
	loaded = true;
	Random random(8124371L);
	readLines(u"/title/win.txt", false, &random);
	for (int_t i = 0; i < 16; ++i)
		lines.push_back(u"");
	readLines(u"/title/credits.txt", true, nullptr);

	int_t w = 274;
	font.fontHeight = 12;
	for (const jstring &line : lines)
	{
		int_t h = font.splitAndGetHeight(line, w);
		lineHeights.push_back(h);
		totalHeight += h;
	}
	font.fontHeight = 8;
}

void WinScreen::renderBg(float a)
{
	Tesselator &t = Tesselator::instance;
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"%blur%/gui/background.png"));
	t.begin();
	t.color(1.0f, 1.0f, 1.0f, 1.0f);
	float time = static_cast<float>(ticks) + a;
	float v0 = 0.0f - time * 0.5f * speed;
	float v1 = static_cast<float>(height) - time * 0.5f * speed;
	float s = 0.015625f;
	float br = time * 0.02f;
	float end = static_cast<float>(totalHeight + height + height + 24) / speed;
	float fade = (end - 20.0f - time) * 0.005f;
	if (fade < br)
		br = fade;
	if (br > 1.0f)
		br = 1.0f;
	br *= br;
	br = br * 96.0f / 255.0f;
	t.color(br, br, br);
	t.vertexUV(0.0, height, blitOffset, 0.0, v0 * s);
	t.vertexUV(width, height, blitOffset, width * s, v0 * s);
	t.vertexUV(width, 0.0, blitOffset, width * s, v1 * s);
	t.vertexUV(0.0, 0.0, blitOffset, 0.0, v1 * s);
	t.end();
}

void WinScreen::render(int_t xm, int_t ym, float a)
{
	renderBg(a);
	Tesselator &t = Tesselator::instance;
	int_t w = 274;
	int_t x0 = width / 2 - w / 2;
	int_t y0 = height + 50;
	float scroll = -(static_cast<float>(ticks) + a) * speed;
	glPushMatrix();
	glTranslatef(0.0f, scroll, 0.0f);
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/title/mclogo.png"));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	blit(x0, y0, 0, 0, 155, 44);
	blit(x0 + 155, y0, 0, 45, 155, 44);
	t.color(0xFFFFFF);
	font.fontHeight = 12;
	int_t y = y0 + 200;
	for (size_t i = 0; i < lines.size(); ++i)
	{
		int_t h = lineHeights[i];
		if (i == lines.size() - 1)
		{
			float over = static_cast<float>(y) + scroll - static_cast<float>(height / 2 - h / 2);
			if (over < 0.0f)
				glTranslatef(0.0f, -over, 0.0f);
		}
		if (static_cast<float>(y) + scroll + static_cast<float>(h) + 8.0f > 0.0f && static_cast<float>(y) + scroll < static_cast<float>(height))
		{
			const jstring &line = lines[i];
			if (line.rfind(u"[C]", 0) == 0)
			{
				jstring text = line.substr(3);
				font.drawShadow(text, x0 + (w - font.width(text)) / 2, y, 0xFFFFFF);
			}
			else
			{
				long_t seed = static_cast<long_t>(i) * 4238972211LL + static_cast<long_t>(ticks / 4);
				font.random.setSeed(seed);
				font.splitAndDraw(line, x0 + 1, y + 1, w, 0xFFFFFF, true);
				font.random.setSeed(seed);
				font.splitAndDraw(line, x0, y, w, 0xFFFFFF, false);
			}
		}
		y += h;
	}
	font.fontHeight = 8;
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"%blur%/misc/vignette.png"));
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	t.begin();
	t.color(1.0f, 1.0f, 1.0f, 1.0f);
	t.vertexUV(0.0, height, blitOffset, 0.0, 1.0);
	t.vertexUV(width, height, blitOffset, 1.0, 1.0);
	t.vertexUV(width, 0.0, blitOffset, 1.0, 0.0);
	t.vertexUV(0.0, 0.0, blitOffset, 0.0, 0.0);
	t.end();
	glDisable(GL_BLEND);
	Screen::render(xm, ym, a);
}

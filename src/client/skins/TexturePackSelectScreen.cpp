#include "client/skins/TexturePackSelectScreen.h"

#include <algorithm>

#include "client/Minecraft.h"
#include "client/locale/Language.h"

#include "client/gui/GuiSlot.h"
#include "client/gui/SmallButton.h"
#include "client/skins/TexturePack.h"
#include "client/skins/TexturePackRepository.h"

#include "java/File.h"

#include "OpenGL.h"
#include "SDL.h"

namespace
{

std::string toOpenUrl(const File &file)
{
	std::string path = String::toUTF8(file.toString());
	std::replace(path.begin(), path.end(), '\\', '/');
	return "file:///" + path;
}

class TexturePackList : public GuiSlot
{
private:
	TexturePackSelectScreen &screen;

public:
	TexturePackList(Minecraft &minecraft, TexturePackSelectScreen &screen)
		: GuiSlot(minecraft, screen.width, screen.height, 32, screen.height - 55 + 4, 36), screen(screen)
	{

	}

protected:
	int_t getSize() override
	{
		return static_cast<int_t>(minecraft.texturePackRepository.getTexturePacks().size());
	}

	void elementClicked(int_t index, bool doubleClick) override
	{
		const std::vector<TexturePack *> &texturePacks = minecraft.texturePackRepository.getTexturePacks();
		if (index < 0 || index >= static_cast<int_t>(texturePacks.size()))
			return;

		minecraft.texturePackRepository.setSelected(texturePacks[index]);
		minecraft.textures.reloadAll();
	}

	bool isSelected(int_t index) override
	{
		const std::vector<TexturePack *> &texturePacks = minecraft.texturePackRepository.getTexturePacks();
		return index >= 0 && index < static_cast<int_t>(texturePacks.size()) && minecraft.texturePackRepository.selected == texturePacks[index];
	}

	void drawBackground() override
	{
		screen.renderBackground();
	}

	void drawSlot(int_t index, int_t x, int_t y, int_t height, Tesselator &t) override
	{
		const std::vector<TexturePack *> &texturePacks = minecraft.texturePackRepository.getTexturePacks();
		TexturePack *texturePack = texturePacks[index];
		texturePack->bindTexture(minecraft);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		t.begin();
		t.color(0xFFFFFF);
		t.vertexUV(x, y + height, 0.0, 0.0, 1.0);
		t.vertexUV(x + 32, y + height, 0.0, 1.0, 1.0);
		t.vertexUV(x + 32, y, 0.0, 1.0, 0.0);
		t.vertexUV(x, y, 0.0, 0.0, 0.0);
		t.end();

		Font &font = *minecraft.font;
		screen.drawString(font, texturePack->name, x + 34, y + 1, 0xFFFFFF);
		screen.drawString(font, texturePack->desc1, x + 34, y + 12, 0x808080);
		screen.drawString(font, texturePack->desc2, x + 34, y + 22, 0x808080);
	}
};

}

TexturePackSelectScreen::TexturePackSelectScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen) : Screen(minecraft), lastScreen(lastScreen)
{

}

void TexturePackSelectScreen::init()
{
	Language &language = Language::getInstance();
	minecraft.texturePackRepository.updateList();
	texturePackList = Util::make_shared<TexturePackList>(minecraft, *this);
	buttons.push_back(Util::make_shared<SmallButton>(5, width / 2 - 154, height - 48, language.getElement(u"texturePack.openFolder")));
	buttons.push_back(Util::make_shared<SmallButton>(6, width / 2 + 4, height - 48, language.getElement(u"gui.done")));
}

void TexturePackSelectScreen::tick()
{
	Screen::tick();
	--refreshTicks;
}

void TexturePackSelectScreen::buttonClicked(Button &button)
{
	if (!button.active)
		return;

	if (button.id == 5)
	{
		std::unique_ptr<File> texturePacksDir(File::open(*minecraft.getWorkingDirectory(), u"texturepacks"));
		if (!texturePacksDir->exists())
			texturePacksDir->mkdirs();
		SDL_OpenURL(toOpenUrl(*texturePacksDir).c_str());
	}
	else if (button.id == 6)
	{
		minecraft.textures.reloadAll();
		minecraft.setScreen(lastScreen);
	}
}

void TexturePackSelectScreen::mouseScrolled(int_t x, int_t y, int_t scrollAmount)
{
	if (texturePackList != nullptr)
		texturePackList->mouseScrolled(scrollAmount);
}

void TexturePackSelectScreen::render(int_t xm, int_t ym, float a)
{
	if (texturePackList != nullptr)
		texturePackList->drawScreen(xm, ym, a);
	else
		renderBackground();

	if (refreshTicks <= 0)
	{
		minecraft.texturePackRepository.updateList();
		refreshTicks += 20;
	}

	Language &language = Language::getInstance();
	drawCenteredString(font, language.getElement(u"texturePack.title"), width / 2, 16, 0xFFFFFF);
	drawCenteredString(font, language.getElement(u"texturePack.folderInfo"), width / 2 - 77, height - 26, 0x808080);
	Screen::render(xm, ym, a);
}

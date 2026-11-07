#include "client/gui/EnchantmentScreen.h"

#include <cmath>

#include "OpenGL.h"
#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gamemode/GameMode.h"
#include "client/gui/Font.h"
#include "client/gui/ScreenSizeCalculator.h"
#include "client/model/BookModel.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "lwjgl/Keyboard.h"
#include "util/GLU.h"
#include "util/Mth.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/ContainerMenus.h"

namespace
{
	// 1.0 EnchantingPhrases
	class Phrases
	{
		Random random;
		std::vector<jstring> words;

	public:
		Phrases()
		{
			jstring all = u"the elder scrolls klaatu berata niktu xyzzy bless curse light darkness fire air earth water hot dry cold wet ignite snuff embiggen twist shorten stretch fiddle destroy imbue galvanize enchant free limited range of towards inside sphere cube self other ball mental physical grow shrink demon elemental spirit animal creature beast humanoid undead fresh stale ";
			size_t start = 0;
			for (size_t i = 0; i <= all.size(); ++i)
			{
				if (i == all.size() || all[i] == u' ')
				{
					if (i > start)
						words.push_back(all.substr(start, i - start));
					start = i + 1;
				}
			}
		}
		void setSeed(long_t seed) { random.setSeed(seed); }
		jstring next()
		{
			int_t n = random.nextInt(2) + 3;
			jstring s;
			for (int_t i = 0; i < n; ++i)
			{
				if (i > 0)
					s += u" ";
				s += words[random.nextInt(static_cast<int_t>(words.size()))];
			}
			return s;
		}
	};

	Phrases &phrases()
	{
		static Phrases p;
		return p;
	}

	BookModel &bookModel()
	{
		static BookModel model;
		return model;
	}
}

EnchantmentScreen::EnchantmentScreen(Minecraft &minecraft, std::shared_ptr<ContainerEnchantment> menu)
	: ContainerScreen(minecraft), menu(std::move(menu))
{
	passEvents = true;
}

int_t EnchantmentScreen::getSlotAt(int_t x, int_t y) const
{
	int_t relX = x - getGuiLeft();
	int_t relY = y - getGuiTop();
	for (size_t i = 0; i < menu->slots.size(); ++i)
	{
		const Slot &slot = *menu->slots[i];
		if (relX >= slot.x - 1 && relX < slot.x + 17 && relY >= slot.y - 1 && relY < slot.y + 17)
			return static_cast<int_t>(i);
	}
	return -1;
}

void EnchantmentScreen::tick()
{
	ContainerScreen::tick();
	// 1.0 tickEnchantingScreen
	const ItemInstance *current = menu->slots[0]->getItem();
	ItemInstance now = current != nullptr ? *current : ItemInstance();
	bool same = now.itemID == last.itemID && now.stackSize == last.stackSize && now.itemDamage == last.itemDamage && now.enchantments == last.enchantments;
	if (!same)
	{
		last = now;
		do
			flipT += static_cast<float>(random.nextInt(4) - random.nextInt(4));
		while (flip <= flipT + 1.0f && flip >= flipT - 1.0f);
	}
	time++;
	oFlip = flip;
	oOpen = open;
	bool any = menu->costs[0] != 0 || menu->costs[1] != 0 || menu->costs[2] != 0;
	open += any ? 0.2f : -0.2f;
	if (open < 0.0f)
		open = 0.0f;
	if (open > 1.0f)
		open = 1.0f;
	float step = (flipT - flip) * 0.4f;
	float max = 0.2f;
	if (step < -max)
		step = -max;
	if (step > max)
		step = max;
	flipA += (step - flipA) * 0.9f;
	flip += flipA;
}

void EnchantmentScreen::renderBook(float a)
{
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	ScreenSizeCalculator ssc(minecraft.options, minecraft.width, minecraft.height);
	glViewport((ssc.getWidth() - 320) / 2 * ssc.scale, (ssc.getHeight() - 240) / 2 * ssc.scale, 320 * ssc.scale, 240 * ssc.scale);
	glTranslatef(-0.34f, 0.23f, 0.0f);
	gluPerspective(90.0f, 1.3333334f, 9.0f, 80.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	Lighting::turnOn();
	glTranslatef(0.0f, 3.3f, -16.0f);
	glScalef(5.0f, 5.0f, 5.0f);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/item/book.png"));
	glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
	float o = oOpen + (open - oOpen) * a;
	glTranslatef((1.0f - o) * 0.2f, (1.0f - o) * 0.1f, (1.0f - o) * 0.25f);
	glRotatef(-(1.0f - o) * 90.0f - 90.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	float f1 = oFlip + (flip - oFlip) * a + 0.25f;
	float f2 = oFlip + (flip - oFlip) * a + 0.75f;
	f1 = (f1 - static_cast<float>(std::floor(f1))) * 1.6f - 0.3f;
	f2 = (f2 - static_cast<float>(std::floor(f2))) * 1.6f - 0.3f;
	f1 = f1 < 0.0f ? 0.0f : (f1 > 1.0f ? 1.0f : f1);
	f2 = f2 < 0.0f ? 0.0f : (f2 > 1.0f ? 1.0f : f2);
	glEnable(GL_RESCALE_NORMAL);
	bookModel().render(0.0f, f1, f2, o, 0.0625f);
	glDisable(GL_RESCALE_NORMAL);
	Lighting::turnOff();
	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, minecraft.width, minecraft.height);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	Lighting::turnOff();
}

void EnchantmentScreen::renderBg(int_t xm, int_t ym, float a)
{
	int_t tex = minecraft.textures.loadTexture(u"/gui/enchant.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(tex);
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	blit(xo, yo, 0, 0, imageWidth, imageHeight);
	renderBook(a);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	static Font galactic(minecraft.options, u"/font/alternate.png", minecraft.textures);
	phrases().setSeed(menu->nameSeed);
	for (int_t i = 0; i < 3; ++i)
	{
		jstring phrase = phrases().next();
		blitOffset = 0.0f;
		minecraft.textures.bind(tex);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		int_t cost = menu->costs[i];
		int_t bx = xo + 60, by = yo + 14 + 19 * i;
		if (cost == 0)
		{
			blit(bx, by, 0, 185, 108, 19);
			continue;
		}
		jstring costText = String::toString(cost);
		int_t color = 6839882;
		if (minecraft.player->experienceLevel < cost)
		{
			blit(bx, by, 0, 185, 108, 19);
			galactic.splitAndDraw(phrase, xo + 62, yo + 16 + 19 * i, 104, (color & 16711422) >> 1);
			font.drawShadow(costText, xo + 62 + 104 - font.width(costText), yo + 16 + 19 * i + 7, 4226832);
		}
		else
		{
			int_t rx = xm - bx, ry = ym - by;
			if (rx >= 0 && ry >= 0 && rx < 108 && ry < 19)
			{
				blit(bx, by, 0, 204, 108, 19);
				color = 16777088;
			}
			else
				blit(bx, by, 0, 166, 108, 19);
			galactic.splitAndDraw(phrase, xo + 62, yo + 16 + 19 * i, 104, color);
			font.drawShadow(costText, xo + 62 + 104 - font.width(costText), yo + 16 + 19 * i + 7, 8453920);
		}
	}
}

void EnchantmentScreen::render(int_t xm, int_t ym, float a)
{
	renderBackground();
	if (minecraft.player == nullptr || menu == nullptr)
		return;
	renderBg(xm, ym, a);

	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	int_t relX = xm - xo;
	int_t relY = ym - yo;
	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);

	glPushMatrix();
	glTranslatef(static_cast<float>(xo), static_cast<float>(yo), 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();
	int_t hovered = getSlotAt(xm, ym);
	for (auto &slotPtr : menu->slots)
	{
		ItemInstance *stack = slotPtr->getItem();
		if (stack != nullptr && !stack->isEmpty())
		{
			itemRenderer.renderGuiItem(font, minecraft.textures, *stack, slotPtr->x, slotPtr->y);
			itemRenderer.renderGuiItemDecorations(font, minecraft.textures, *stack, slotPtr->x, slotPtr->y);
		}
	}
	if (hovered >= 0)
	{
		const Slot &slot = *menu->slots[hovered];
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		fillGradient(slot.x, slot.y, slot.x + 16, slot.y + 16, 0x80FFFFFF, 0x80FFFFFF);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}
	ItemInstance *carried = minecraft.player->inventory.getCarried();
	if (carried != nullptr && !carried->isEmpty())
	{
		glTranslatef(0.0f, 0.0f, 32.0f);
		itemRenderer.renderGuiItem(font, minecraft.textures, *carried, relX - 8, relY - 8);
		itemRenderer.renderGuiItemDecorations(font, minecraft.textures, *carried, relX - 8, relY - 8);
	}
	glDisable(GL_RESCALE_NORMAL);
	Lighting::turnOff();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	font.draw(u"Enchant", 12, 6, 0x404040);
	font.draw(u"Inventory", 8, imageHeight - 96 + 2, 0x404040);
	if (carried == nullptr && hovered >= 0)
	{
		const ItemInstance *stack = menu->slots[hovered]->getItem();
		if (stack != nullptr && !stack->isEmpty())
		{
			auto lines = getTooltipLines(*stack, &minecraft.options);
			if (!lines.empty())
				renderTooltip(lines, relX + 12, relY - 12);
		}
	}
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

void EnchantmentScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE || eventKey == minecraft.options.keyInventory.key)
	{
		minecraft.player->closeContainer();
		minecraft.grabMouse();
		return;
	}
	Screen::keyPressed(eventCharacter, eventKey);
}

void EnchantmentScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	if (minecraft.player == nullptr || menu == nullptr || (buttonNum != 0 && buttonNum != 1))
		return;
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	if (buttonNum == 0)
	{
		for (int_t i = 0; i < 3; ++i)
		{
			int_t rx = x - (xo + 60);
			int_t ry = y - (yo + 14 + 19 * i);
			if (rx >= 0 && ry >= 0 && rx < 108 && ry < 19 && menu->clickMenuButton(*minecraft.player, i))
				return;
		}
	}
	bool outside = x < xo || y < yo || x >= xo + imageWidth || y >= yo + imageHeight;
	int_t slot = outside ? -999 : getSlotAt(x, y);
	if (slot == -1)
		return;
	bool shift = slot != -999 && (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) || lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT));
	minecraft.gameMode->clickContainer(menu->windowId, slot, buttonNum, shift, *minecraft.player);
}

void EnchantmentScreen::removed()
{
	if (minecraft.player != nullptr && menu != nullptr)
		minecraft.gameMode->closeContainer(menu->windowId, *minecraft.player);
	Screen::removed();
}

#include "client/gui/RepairScreen.h"

#include "OpenGL.h"
#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gamemode/GameMode.h"
#include "client/gui/GuiTextField.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "lwjgl/Keyboard.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/ContainerRepair.h"
#include "world/stats/StatCollector.h"

namespace
{
	jstring plainName(const jstring &tooltipName)
	{
		// the tooltip name may start with a colour code
		if (tooltipName.size() >= 2 && tooltipName[0] == 167)
			return tooltipName.substr(2);
		return tooltipName;
	}
}

RepairScreen::RepairScreen(Minecraft &minecraft, std::shared_ptr<ContainerRepair> menu)
	: ContainerScreen(minecraft), menu(std::move(menu))
{
	passEvents = true;
}

void RepairScreen::init()
{
	nameField = Util::make_shared<GuiTextField>(*this, font, getGuiLeft() + 62, getGuiTop() + 24, 103, 12, u"");
	nameField->setMaxStringLength(30);
	nameField->isFocused = true;
	lastFirst = ItemInstance();
	checkFirstSlot();
}

void RepairScreen::checkFirstSlot()
{
	// 1.4.7 onSlotChanged(slot 0): the field shows the item's name
	const ItemInstance *first = menu->slots[0]->getItem();
	ItemInstance now = first != nullptr ? *first : ItemInstance();
	if (now.itemID == lastFirst.itemID && now.itemDamage == lastFirst.itemDamage && now.stackSize == lastFirst.stackSize && now.sameTag(lastFirst))
		return;
	lastFirst = now;
	if (nameField == nullptr)
		return;
	nameField->setText(first == nullptr ? u"" : plainName(getTooltipName(*first)));
	nameField->isEnabled = first != nullptr;
	if (first != nullptr)
		menu->setItemName(nameField->getText());
}

void RepairScreen::tick()
{
	ContainerScreen::tick();
	if (nameField != nullptr)
		nameField->updateCursorCounter();
	checkFirstSlot();
}

int_t RepairScreen::getSlotAt(int_t x, int_t y) const
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

void RepairScreen::render(int_t xm, int_t ym, float a)
{
	(void)a;
	renderBackground();
	if (minecraft.player == nullptr || menu == nullptr)
		return;
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	int_t tex = minecraft.textures.loadTexture(u"/gui/repair.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(tex);
	blit(xo, yo, 0, 0, imageWidth, imageHeight);
	bool hasFirst = menu->slots[0]->hasItem();
	blit(xo + 59, yo + 20, 0, imageHeight + (hasFirst ? 0 : 16), 110, 16);
	if ((hasFirst || menu->slots[1]->hasItem()) && !menu->slots[2]->hasItem())
		blit(xo + 99, yo + 45, imageWidth, 0, 28, 21);

	int_t relX = xm - xo;
	int_t relY = ym - yo;
	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);
	glPushMatrix();
	glTranslatef(static_cast<float>(xo), static_cast<float>(yo), 0.0f);
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
		glTranslatef(0.0f, 0.0f, -32.0f);
	}
	glDisable(GL_RESCALE_NORMAL);
	Lighting::turnOff();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	// 1.4.7 renderLabels
	font.draw(StatCollector::translate(u"container.repair"), 60, 6, 0x404040);
	font.draw(u"Inventory", 8, imageHeight - 96 + 2, 0x404040);
	if (menu->cost > 0)
	{
		int_t color = 8453920;
		bool show = true;
		jstring text = StatCollector::translate(u"container.repair.cost");
		size_t pos = text.find(u"%1$d");
		if (pos != jstring::npos)
			text.replace(pos, 4, String::toString(menu->cost));
		if (menu->cost >= 40 && !minecraft.player->abilities.instabuild)
		{
			text = StatCollector::translate(u"container.repair.expensive");
			color = 16736352;
		}
		else if (!menu->slots[2]->hasItem())
			show = false;
		else if (!menu->slots[2]->mayPickup(*minecraft.player))
			color = 16736352;
		if (show)
		{
			int_t shadow = static_cast<int_t>(0xFF000000u | static_cast<unsigned>((color & 16579836) >> 2) | static_cast<unsigned>(color & 0xFF000000));
			int_t tx = imageWidth - 8 - font.width(text);
			int_t ty = 67;
			font.draw(text, tx, ty + 1, shadow);
			font.draw(text, tx + 1, ty, shadow);
			font.draw(text, tx + 1, ty + 1, shadow);
			font.draw(text, tx, ty, color);
		}
	}
	if (nameField != nullptr && nameField->isEnabled)
	{
		bool cursor = nameField->isFocused && (minecraft.level != nullptr ? minecraft.level->time : 0) / 6 % 2 == 0;
		font.drawShadow(nameField->getText() + (cursor ? u"_" : u""), 62 + 4 - 4, 24 + 2, 0xFFFFFF);
	}
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

void RepairScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE)
	{
		minecraft.player->closeContainer();
		minecraft.grabMouse();
		return;
	}
	if (nameField != nullptr && nameField->isEnabled && nameField->isFocused)
	{
		jstring before = nameField->getText();
		nameField->textboxKeyTyped(eventCharacter, eventKey);
		if (nameField->getText() != before)
			menu->setItemName(nameField->getText());
		return;
	}
	if (eventKey == minecraft.options.keyInventory.key)
	{
		minecraft.player->closeContainer();
		minecraft.grabMouse();
		return;
	}
	Screen::keyPressed(eventCharacter, eventKey);
}

void RepairScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	if (minecraft.player == nullptr || menu == nullptr || (buttonNum != 0 && buttonNum != 1))
		return;
	if (nameField != nullptr)
		nameField->mouseClicked(x, y, buttonNum);
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	bool outside = x < xo || y < yo || x >= xo + imageWidth || y >= yo + imageHeight;
	int_t slot = outside ? -999 : getSlotAt(x, y);
	if (slot == -1)
		return;
	bool shift = slot != -999 && (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) || lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT));
	minecraft.gameMode->clickContainer(menu->windowId, slot, buttonNum, shift, *minecraft.player);
}

void RepairScreen::removed()
{
	if (minecraft.player != nullptr && menu != nullptr)
		minecraft.gameMode->closeContainer(menu->windowId, *minecraft.player);
	Screen::removed();
}

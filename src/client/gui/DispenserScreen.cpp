#include "client/gui/DispenserScreen.h"

#include "OpenGL.h"
#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gamemode/GameMode.h"
#include "client/locale/Language.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "lwjgl/Keyboard.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/inventory/Container.h"
#include "world/item/Item.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/DispenserTileEntity.h"

namespace
{
	constexpr int_t SLOT_NONE = -1;
	constexpr int_t SLOT_DISPENSER_BASE = 200;

}

DispenserScreen::DispenserScreen(Minecraft &minecraft, std::shared_ptr<DispenserTileEntity> dispenser)
	: ContainerScreen(minecraft), dispenser(std::move(dispenser)), containerMenu(minecraft.player->craftingInventory)
{
	passEvents = true;
}

void DispenserScreen::render(int_t xm, int_t ym, float a)
{
	xMouse = static_cast<float>(xm);
	yMouse = static_cast<float>(ym);
	renderBackground();
	renderBg();

	if (minecraft.player == nullptr || dispenser == nullptr)
		return;

	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	int_t relX = xm - xo;
	int_t relY = ym - yo;
	hoveredSlot = SLOT_NONE;
	hoveredSlotX = -1;
	hoveredSlotY = -1;

	glPushMatrix();
	glTranslatef(static_cast<float>(xo), static_cast<float>(yo), 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_RESCALE_NORMAL);

	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();

	for (int_t slot = 0; slot < dispenser->getContainerSize(); ++slot)
	{
		int_t slotX = getDispenserSlotX(slot);
		int_t slotY = getDispenserSlotY(slot);
		renderSlot(dispenser->getItem(slot), slotX, slotY, a);
		if (isPointInSlot(relX, relY, slotX, slotY))
		{
			hoveredSlot = SLOT_DISPENSER_BASE + slot;
			hoveredSlotX = slotX;
			hoveredSlotY = slotY;
		}
	}

	for (int_t slot = 9; slot < 36; ++slot)
	{
		int_t slotX = getInventorySlotX(slot);
		int_t slotY = getInventorySlotY(slot);
		renderSlot(minecraft.player->inventory.mainInventory[slot], slotX, slotY, a);
		if (isPointInSlot(relX, relY, slotX, slotY))
		{
			hoveredSlot = slot;
			hoveredSlotX = slotX;
			hoveredSlotY = slotY;
		}
	}

	for (int_t slot = 0; slot < 9; ++slot)
	{
		int_t slotX = getInventorySlotX(slot);
		int_t slotY = getInventorySlotY(slot);
		renderSlot(minecraft.player->inventory.mainInventory[slot], slotX, slotY, a);
		if (isPointInSlot(relX, relY, slotX, slotY))
		{
			hoveredSlot = slot;
			hoveredSlotX = slotX;
			hoveredSlotY = slotY;
		}
	}

	if (hoveredSlotX >= 0 && hoveredSlotY >= 0)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		fillGradient(hoveredSlotX, hoveredSlotY, hoveredSlotX + 16, hoveredSlotY + 16, 0x80FFFFFF, 0x80FFFFFF);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}

	ItemInstance *carried = minecraft.player->inventory.getCarried();
	if (carried != nullptr && !carried->isEmpty())
	{
		static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);
		glTranslatef(0.0f, 0.0f, 32.0f);
		itemRenderer.renderGuiItem(font, minecraft.textures, *carried, relX - 8, relY - 8);
		itemRenderer.renderGuiItemDecorations(font, minecraft.textures, *carried, relX - 8, relY - 8);
	}

	glDisable(GL_RESCALE_NORMAL);
	Lighting::turnOff();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	renderLabels();
	if (carried == nullptr && hoveredSlot != SLOT_NONE)
	{
		const ItemInstance *hoveredItem = getSlotItem(hoveredSlot);
		if (hoveredItem != nullptr && !hoveredItem->isEmpty())
		{
			jstring tooltip = getTooltipName(*hoveredItem);
			if (!tooltip.empty())
			{
				int_t tooltipX = relX + 12;
				int_t tooltipY = relY - 12;
				int_t tooltipW = font.width(tooltip);
				fillGradient(tooltipX - 3, tooltipY - 3, tooltipX + tooltipW + 3, tooltipY + 11, 0xC0000000, 0xC0000000);
				font.drawShadow(tooltip, tooltipX, tooltipY, 0xFFFFFF);
			}
		}
	}
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

void DispenserScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE || eventKey == minecraft.options.keyInventory.key)
	{
		minecraft.player->closeContainer();
		minecraft.grabMouse();
		return;
	}
	Screen::keyPressed(eventCharacter, eventKey);
}

void DispenserScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	if (minecraft.player == nullptr || dispenser == nullptr || (buttonNum != 0 && buttonNum != 1))
		return;
	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	bool outside = x < xo || y < yo || x >= xo + imageWidth || y >= yo + imageHeight;
	int_t protocolSlot = -1;
	if (outside)
		protocolSlot = -999;
	else
	{
		int_t slot = getSlotAt(x, y);
		if (slot >= SLOT_DISPENSER_BASE)
			protocolSlot = slot - SLOT_DISPENSER_BASE;
		else if (slot >= 9 && slot < 36)
			protocolSlot = slot;
		else if (slot >= 0 && slot < 9)
			protocolSlot = 36 + slot;
	}
	if (protocolSlot != -1)
	{
		bool shiftClick = protocolSlot != -999
			&& (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT)
				|| lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT));
		minecraft.gameMode->clickContainer(minecraft.player->craftingInventory->windowId,
			protocolSlot, buttonNum, shiftClick, *minecraft.player);
	}
}

bool DispenserScreen::isPauseScreen()
{
	return false;
}

int_t DispenserScreen::getGuiLeft() const { return (width - imageWidth) / 2; }
int_t DispenserScreen::getGuiTop() const { return (height - imageHeight) / 2; }
int_t DispenserScreen::getDispenserSlotX(int_t slot) const { return 62 + (slot % 3) * 18; }
int_t DispenserScreen::getDispenserSlotY(int_t slot) const { return 17 + (slot / 3) * 18; }
int_t DispenserScreen::getInventorySlotX(int_t slot) const { return 8 + (slot % 9) * 18; }
int_t DispenserScreen::getInventorySlotY(int_t slot) const { return slot < 9 ? 142 : 84 + ((slot - 9) / 9) * 18; }

int_t DispenserScreen::getSlotAt(int_t x, int_t y) const
{
	int_t relX = x - getGuiLeft();
	int_t relY = y - getGuiTop();
	for (int_t slot = 0; slot < 9; ++slot)
		if (isPointInSlot(relX, relY, getDispenserSlotX(slot), getDispenserSlotY(slot))) return SLOT_DISPENSER_BASE + slot;
	for (int_t slot = 9; slot < 36; ++slot)
		if (isPointInSlot(relX, relY, getInventorySlotX(slot), getInventorySlotY(slot))) return slot;
	for (int_t slot = 0; slot < 9; ++slot)
		if (isPointInSlot(relX, relY, getInventorySlotX(slot), getInventorySlotY(slot))) return slot;
	return SLOT_NONE;
}

const ItemInstance *DispenserScreen::getSlotItem(int_t slot) const
{
	if (dispenser == nullptr) return nullptr;
	if (slot >= SLOT_DISPENSER_BASE && slot < SLOT_DISPENSER_BASE + 9)
	{
		const ItemInstance &stack = dispenser->getItem(slot - SLOT_DISPENSER_BASE);
		return stack.isEmpty() ? nullptr : &stack;
	}
	if (minecraft.player == nullptr) return nullptr;
	const ItemInstance &stack = minecraft.player->inventory.mainInventory[slot];
	return stack.isEmpty() ? nullptr : &stack;
}

void DispenserScreen::renderBg()
{
	int_t tex = minecraft.textures.loadTexture(u"/gui/trap.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(tex);
	blit(getGuiLeft(), getGuiTop(), 0, 0, imageWidth, imageHeight);
}

void DispenserScreen::renderLabels()
{
	font.draw(u"Dispenser", 60, 6, 0x00404040);
	font.draw(u"Inventory", 8, imageHeight - 96 + 2, 0x00404040);
}

void DispenserScreen::renderSlot(ItemInstance &stack, int_t x, int_t y, float a)
{
	if (stack.isEmpty()) return;
	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);

	itemRenderer.renderGuiItem(font, minecraft.textures, stack, x, y);
	itemRenderer.renderGuiItemDecorations(font, minecraft.textures, stack, x, y);
}

void DispenserScreen::handleRegularSlotClick(ItemInstance &slotStack, int_t buttonNum)
{
	InventoryPlayer &inventory = minecraft.player->inventory;
	ItemInstance *carried = inventory.getCarried();
	if (slotStack.isEmpty())
	{
		if (carried == nullptr) return;
		int_t toPlace = buttonNum == 0 ? carried->stackSize.load(std::memory_order_relaxed) : 1;
		if (toPlace > carried->getMaxStackSize()) toPlace = carried->getMaxStackSize();
		if (toPlace <= 0) return;
		slotStack = ItemInstance(carried->itemID, toPlace, carried->itemDamage);
		carried->stackSize -= toPlace;
		if (carried->isEmpty()) inventory.setCarriedNull();
		return;
	}
	if (carried == nullptr)
	{
		int_t toTake = buttonNum == 0 ? slotStack.stackSize.load(std::memory_order_relaxed)
			: (slotStack.stackSize + 1) / 2;
		inventory.setCarried(ItemInstance(slotStack.itemID, toTake, slotStack.itemDamage));
		slotStack.stackSize -= toTake;
		if (slotStack.isEmpty()) slotStack = ItemInstance();
		return;
	}
	bool canMerge = slotStack.sameItem(*carried) && slotStack.isStackable() && carried->isStackable();
	if (!canMerge)
	{
		ItemInstance swapped = slotStack;
		slotStack = *carried;
		inventory.setCarried(swapped);
		return;
	}
	int_t space = slotStack.getMaxStackSize() - slotStack.stackSize;
	if (space <= 0) return;
	int_t toMove = buttonNum == 0 ? carried->stackSize.load(std::memory_order_relaxed) : 1;
	if (toMove > space) toMove = space;
	if (toMove <= 0) return;
	slotStack.stackSize += toMove;
	carried->stackSize -= toMove;
	if (carried->isEmpty()) inventory.setCarriedNull();
}

void DispenserScreen::handleSlotClick(int_t slot, int_t buttonNum)
{
	if (slot >= SLOT_DISPENSER_BASE && slot < SLOT_DISPENSER_BASE + 9)
	{
		ItemInstance &slotStack = dispenser->getItem(slot - SLOT_DISPENSER_BASE);
		handleRegularSlotClick(slotStack, buttonNum);
		dispenser->setChanged();
		return;
	}
	ItemInstance &slotStack = minecraft.player->inventory.mainInventory[slot];
	handleRegularSlotClick(slotStack, buttonNum);
}

bool DispenserScreen::isPointInSlot(int_t relX, int_t relY, int_t slotX, int_t slotY) const
{
	return relX >= slotX - 1 && relX < slotX + 16 + 1 && relY >= slotY - 1 && relY < slotY + 16 + 1;
}

void DispenserScreen::removed()
{
	if (minecraft.player != nullptr && containerMenu != nullptr)
		minecraft.gameMode->closeContainer(containerMenu->windowId, *minecraft.player);
	Screen::removed();
}

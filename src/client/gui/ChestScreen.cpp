#include "client/gui/ChestScreen.h"

#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gamemode/GameMode.h"
#include "client/locale/Language.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "java/String.h"
#include "world/CompoundContainer.h"
#include "world/inventory/BasicInventory.h"
#include "world/inventory/Container.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/item/EntityMinecart.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/ChestTileEntity.h"

#include "OpenGL.h"
#include "lwjgl/Keyboard.h"

namespace
{
	constexpr int_t SLOT_NONE = -1;
	constexpr int_t SLOT_CHEST_BASE = 200;

}

ChestScreen::ChestScreen(Minecraft &minecraft, std::shared_ptr<ChestTileEntity> chest)
	: ContainerScreen(minecraft), chest(chest), containerMenu(minecraft.player->craftingInventory)
{
	passEvents = true;
	inventoryRows = getChestSize() / 9;
	imageHeight = 114 + inventoryRows * 18;
}

ChestScreen::ChestScreen(Minecraft &minecraft, std::shared_ptr<CompoundContainer> chest)
	: ContainerScreen(minecraft), compoundChest(chest), containerMenu(minecraft.player->craftingInventory)
{
	passEvents = true;
	inventoryRows = getChestSize() / 9;
	imageHeight = 114 + inventoryRows * 18;
}
ChestScreen::ChestScreen(Minecraft &minecraft, std::shared_ptr<EntityMinecart> chest)
	: ContainerScreen(minecraft), chestMinecart(chest), containerMenu(minecraft.player->craftingInventory)
{
	passEvents = true;
	inventoryRows = getChestSize() / 9;
	imageHeight = 114 + inventoryRows * 18;
}

ChestScreen::ChestScreen(Minecraft &minecraft, std::shared_ptr<BasicInventory> chest)
	: ContainerScreen(minecraft), basicChest(std::move(chest)), containerMenu(minecraft.player->craftingInventory)
{
	passEvents = true;
	inventoryRows = getChestSize() / 9;
	imageHeight = 114 + inventoryRows * 18;
}

void ChestScreen::tick()
{
	ContainerScreen::tick();
	if (minecraft.player == nullptr || getChestSize() <= 0
		|| (!minecraft.player->level.isOnline && !canUseChest(*minecraft.player)))
	{
		minecraft.setScreen(nullptr);
		return;
	}

}

void ChestScreen::render(int_t xm, int_t ym, float a)
{
	xMouse = static_cast<float>(xm);
	yMouse = static_cast<float>(ym);

	renderBackground();
	renderBg();

	if (minecraft.player == nullptr || getChestSize() <= 0)
		return;

	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	int_t relX = xm - xo;
	int_t relY = ym - yo;
	int_t hoveredSlot = SLOT_NONE;
	int_t hoveredSlotX = -1;
	int_t hoveredSlotY = -1;

	glPushMatrix();
	glTranslatef(static_cast<float>(xo), static_cast<float>(yo), 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_RESCALE_NORMAL);

	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();

	for (int_t slot = 0; slot < getChestSize(); ++slot)
	{
		int_t slotX = getChestSlotX(slot);
		int_t slotY = getChestSlotY(slot);
		renderSlot(getChestItem(slot), slotX, slotY, a);
		if (isPointInSlot(relX, relY, slotX, slotY))
		{
			hoveredSlot = SLOT_CHEST_BASE + slot;
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
	glPopMatrix();

	renderLabels();
	if (carried == nullptr && hoveredSlot != SLOT_NONE)
	{
		const ItemInstance *hoveredItem = getSlotItem(hoveredSlot);
		if (hoveredItem != nullptr && !hoveredItem->isEmpty())
		{
			jstring tooltip = getTooltipName(*hoveredItem);
			if (!tooltip.empty())
			{
				int_t tooltipX = static_cast<int_t>(xMouse) + 12;
				int_t tooltipY = static_cast<int_t>(yMouse) - 12;
				int_t tooltipW = font.width(tooltip);
				fillGradient(tooltipX - 3, tooltipY - 3, tooltipX + tooltipW + 3, tooltipY + 11, 0xC0000000, 0xC0000000);
				font.drawShadow(tooltip, tooltipX, tooltipY, 0xFFFFFF);
			}
		}
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

bool ChestScreen::isPauseScreen()
{
	return false;
}

void ChestScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE || eventKey == minecraft.options.keyInventory.key)
	{
		minecraft.player->closeContainer();
		minecraft.grabMouse();
		return;
	}

	Screen::keyPressed(eventCharacter, eventKey);
}

void ChestScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	if (minecraft.player == nullptr || getChestSize() <= 0 || (buttonNum != 0 && buttonNum != 1))
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
		if (slot >= SLOT_CHEST_BASE)
			protocolSlot = slot - SLOT_CHEST_BASE;
		else if (slot >= 9 && slot < 36)
			protocolSlot = getChestSize() + slot - 9;
		else if (slot >= 0 && slot < 9)
			protocolSlot = getChestSize() + 27 + slot;
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

void ChestScreen::removed()
{
	if (minecraft.player != nullptr && containerMenu != nullptr)
		minecraft.gameMode->closeContainer(containerMenu->windowId, *minecraft.player);
	Screen::removed();
}

int_t ChestScreen::getGuiLeft() const
{
	return (width - imageWidth) / 2;
}

int_t ChestScreen::getGuiTop() const
{
	return (height - imageHeight) / 2;
}

int_t ChestScreen::getChestSize() const
{
	if (basicChest != nullptr)
		return basicChest->getContainerSize();
	if (compoundChest != nullptr)
		return compoundChest->getContainerSize();
	if (chest != nullptr)
		return chest->getContainerSize();
	if (chestMinecart != nullptr)
		return chestMinecart->getContainerSize();
	return 0;
}

int_t ChestScreen::getChestSlotX(int_t slot) const
{
	return 8 + (slot % 9) * 18;
}

int_t ChestScreen::getChestSlotY(int_t slot) const
{
	return 18 + (slot / 9) * 18;
}

int_t ChestScreen::getInventorySlotX(int_t slot) const
{
	return 8 + (slot % 9) * 18;
}

int_t ChestScreen::getInventorySlotY(int_t slot) const
{
	if (slot < 9)
		return imageHeight - 25;
	return imageHeight - 83 + ((slot - 9) / 9) * 18;
}

int_t ChestScreen::getSlotAt(int_t x, int_t y) const
{
	int_t relX = x - getGuiLeft();
	int_t relY = y - getGuiTop();

	for (int_t slot = 0; slot < getChestSize(); ++slot)
		if (isPointInSlot(relX, relY, getChestSlotX(slot), getChestSlotY(slot)))
			return SLOT_CHEST_BASE + slot;

	for (int_t slot = 9; slot < 36; ++slot)
		if (isPointInSlot(relX, relY, getInventorySlotX(slot), getInventorySlotY(slot)))
			return slot;

	for (int_t slot = 0; slot < 9; ++slot)
		if (isPointInSlot(relX, relY, getInventorySlotX(slot), getInventorySlotY(slot)))
			return slot;

	return SLOT_NONE;
}

bool ChestScreen::canUseChest(Player &player) const
{
	if (basicChest != nullptr)
		return basicChest->canUse(player);
	if (compoundChest != nullptr)
		return compoundChest->canUse(player);
	if (chest != nullptr)
		return chest->canUse(player);
	if (chestMinecart != nullptr)
		return chestMinecart->canUse(player);
	return false;
}

jstring ChestScreen::getChestName() const
{
	if (basicChest != nullptr)
		return basicChest->getName();
	if (compoundChest != nullptr)
		return compoundChest->getName();
	if (chest != nullptr)
		return chest->getName();
	if (chestMinecart != nullptr)
		return chestMinecart->getName();
	return u"Chest";
}

ItemInstance &ChestScreen::getChestItem(int_t slot)
{
	if (basicChest != nullptr)
		return basicChest->getItem(slot);
	if (compoundChest != nullptr)
		return compoundChest->getItem(slot);
	if (chest != nullptr)
		return chest->getItem(slot);
	return chestMinecart->getItem(slot);
}

const ItemInstance &ChestScreen::getChestItem(int_t slot) const
{
	if (basicChest != nullptr)
		return basicChest->getItem(slot);
	if (compoundChest != nullptr)
		return compoundChest->getItem(slot);
	if (chest != nullptr)
		return chest->getItem(slot);
	return chestMinecart->getItem(slot);
}

void ChestScreen::setChestChanged()
{
	if (basicChest != nullptr)
		basicChest->setChanged();
	else if (compoundChest != nullptr)
		compoundChest->setChanged();
	else if (chest != nullptr)
		chest->setChanged();
	
}

void ChestScreen::renderBg()
{
	int_t tex = minecraft.textures.loadTexture(u"/gui/container.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(tex);
	blit(getGuiLeft(), getGuiTop(), 0, 0, imageWidth, inventoryRows * 18 + 17);
	blit(getGuiLeft(), getGuiTop() + inventoryRows * 18 + 17, 0, 126, imageWidth, 96);
}

void ChestScreen::renderLabels()
{
	font.draw(getChestName(), getGuiLeft() + 8, getGuiTop() + 6, 0x00404040);
	font.draw(u"Inventory", getGuiLeft() + 8, getGuiTop() + imageHeight - 96 + 2, 0x00404040);
}

void ChestScreen::renderSlot(ItemInstance &stack, int_t x, int_t y, float a)
{
	if (stack.isEmpty())
		return;

	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);

	itemRenderer.renderGuiItem(font, minecraft.textures, stack, x, y);
	itemRenderer.renderGuiItemDecorations(font, minecraft.textures, stack, x, y);
}

const ItemInstance *ChestScreen::getSlotItem(int_t slot) const
{
	if (slot >= SLOT_CHEST_BASE && slot < SLOT_CHEST_BASE + getChestSize())
	{
		const ItemInstance &stack = getChestItem(slot - SLOT_CHEST_BASE);
		return stack.isEmpty() ? nullptr : &stack;
	}

	if (minecraft.player == nullptr || slot < 0 || slot >= 36)
		return nullptr;

	const ItemInstance &stack = minecraft.player->inventory.mainInventory[slot];
	return stack.isEmpty() ? nullptr : &stack;
}

void ChestScreen::handleRegularSlotClick(ItemInstance &slotStack, int_t buttonNum)
{
	InventoryPlayer &inventory = minecraft.player->inventory;
	ItemInstance *carried = inventory.getCarried();
	if (slotStack.isEmpty())
	{
		if (carried == nullptr)
			return;
		int_t toPlace = buttonNum == 0 ? carried->stackSize.load(std::memory_order_relaxed) : 1;
		if (toPlace > carried->getMaxStackSize())
			toPlace = carried->getMaxStackSize();
		if (toPlace <= 0)
			return;
		slotStack = ItemInstance(carried->itemID, toPlace, carried->itemDamage);
		carried->stackSize -= toPlace;
		if (carried->isEmpty())
			inventory.setCarriedNull();
		return;
	}

	if (carried == nullptr)
	{
		int_t toTake = buttonNum == 0 ? slotStack.stackSize.load(std::memory_order_relaxed)
			: (slotStack.stackSize + 1) / 2;
		inventory.setCarried(ItemInstance(slotStack.itemID, toTake, slotStack.itemDamage));
		slotStack.stackSize -= toTake;
		if (slotStack.isEmpty())
			slotStack = ItemInstance();
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
	if (space <= 0)
		return;
	int_t toMove = buttonNum == 0 ? carried->stackSize.load(std::memory_order_relaxed) : 1;
	if (toMove > space)
		toMove = space;
	if (toMove <= 0)
		return;
	
	slotStack.stackSize += toMove;
	carried->stackSize -= toMove;
	if (carried->isEmpty())
		inventory.setCarriedNull();
}

void ChestScreen::handleSlotClick(int_t slot, int_t buttonNum)
{
	if (slot >= SLOT_CHEST_BASE && slot < SLOT_CHEST_BASE + getChestSize())
	{
		ItemInstance &slotStack = getChestItem(slot - SLOT_CHEST_BASE);
		handleRegularSlotClick(slotStack, buttonNum);
		setChestChanged();
		return;
	}

	if (slot >= 0 && slot < 36)
		handleRegularSlotClick(minecraft.player->inventory.mainInventory[slot], buttonNum);
}

bool ChestScreen::isPointInSlot(int_t relX, int_t relY, int_t slotX, int_t slotY) const
{
	return relX >= slotX - 1 && relX < slotX + 17 && relY >= slotY - 1 && relY < slotY + 17;
}

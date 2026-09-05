#include "client/gui/InventoryScreen.h"

#include <cmath>

#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gamemode/GameMode.h"
#include "client/locale/Language.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "java/String.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/Container.h"
#include "world/item/Item.h"
#include "world/item/ItemArmor.h"
#include "world/item/Items.h"
#include "world/item/crafting/CraftingContainer.h"
#include "world/item/crafting/Recipes.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/stats/AchievementList.h"
#include "world/stats/Achievement.h"
#include "world/item/ItemPickaxe.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/WorkbenchTile.h"

#include "OpenGL.h"
#include "lwjgl/Keyboard.h"

namespace
{
	constexpr int_t SLOT_NONE = -1;
	constexpr int_t SLOT_CRAFTING_BASE = 100;
	constexpr int_t SLOT_RESULT = 200;
	constexpr int_t SLOT_ARMOR_BASE = 300;

	bool isPointInSlot(int_t relX, int_t relY, int_t slotX, int_t slotY)
	{
		return relX >= slotX - 1 && relX < slotX + 17 && relY >= slotY - 1 && relY < slotY + 17;
	}

	class GridCraftingContainer : public CraftingContainer
	{
	private:
		const std::vector<ItemInstance> &slots;
		int_t width = 0;
		int_t height = 0;

	public:
		GridCraftingContainer(const std::vector<ItemInstance> &slots, int_t width, int_t height)
			: slots(slots), width(width), height(height)
		{
		}

		ItemInstance getItem(int_t x, int_t y) const override
		{
			if (x < 0 || y < 0 || x >= width || y >= height)
				return ItemInstance();
			return slots[x + y * width];
		}
	};

}

InventoryScreen::InventoryScreen(Minecraft &minecraft, int_t craftingWidth, int_t craftingHeight)
	: ContainerScreen(minecraft), craftingSlots(craftingWidth * craftingHeight), craftingWidth(craftingWidth), craftingHeight(craftingHeight)
{
	if (minecraft.player != nullptr)
		containerMenu = craftingWidth == 2 && craftingHeight == 2
			? minecraft.player->inventorySlots : minecraft.player->craftingInventory;
	passEvents = true;
	syncCraftingSlotsFromMenu();
	if (craftingWidth == 2 && craftingHeight == 2 && minecraft.player != nullptr)
		minecraft.player->addStat(*AchievementList::openInventory, 1);
}

void InventoryScreen::render(int_t xm, int_t ym, float a)
{
	syncCraftingSlotsFromMenu();

	renderBackground();

	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	renderBg(a);

	if (minecraft.player == nullptr)
		return;

	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(static_cast<float>(xo), static_cast<float>(yo), 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_RESCALE_NORMAL);

	int_t hoveredSlot = SLOT_NONE;
	int_t hoveredSlotX = -1;
	int_t hoveredSlotY = -1;
	int_t relX = xm - xo;
	int_t relY = ym - yo;

	for (int_t slot = 0; slot < static_cast<int_t>(craftingSlots.size()); ++slot)
	{
		int_t slotX = getCraftingSlotX(slot);
		int_t slotY = getCraftingSlotY(slot);
		renderSlot(craftingSlots[slot], slotX, slotY, a);
		if (isPointInSlot(relX, relY, slotX, slotY))
		{
			hoveredSlot = SLOT_CRAFTING_BASE + slot;
			hoveredSlotX = slotX;
			hoveredSlotY = slotY;
		}
	}

	int_t resultSlotX = getResultSlotX();
	int_t resultSlotY = getResultSlotY();
	renderSlot(craftingResult, resultSlotX, resultSlotY, a);
	if (isPointInSlot(relX, relY, resultSlotX, resultSlotY))
	{
		hoveredSlot = SLOT_RESULT;
		hoveredSlotX = resultSlotX;
		hoveredSlotY = resultSlotY;
	}

	for (int_t row = 0; row < 3; ++row)
	{
		for (int_t col = 0; col < 9; ++col)
		{
			int_t slot = 9 + col + row * 9;
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

	
	for (int_t slot = 0; craftingWidth == 2 && craftingHeight == 2 && slot < 4; ++slot)
	{
		int_t slotX = getArmorSlotX(slot);
		int_t slotY = getArmorSlotY(slot);
		int_t armorIdx = 3 - slot; 
		renderSlot(minecraft.player->inventory.armorInventory[armorIdx], slotX, slotY, a);
		if (isPointInSlot(relX, relY, slotX, slotY))
		{
			hoveredSlot = SLOT_ARMOR_BASE + slot;
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

	
	xMouse = static_cast<float>(xm);
	yMouse = static_cast<float>(ym);
}

bool InventoryScreen::isPauseScreen()
{
	return false;
}

void InventoryScreen::removed()
{
	if (minecraft.player != nullptr && containerMenu != nullptr)
	{
		minecraft.gameMode->closeContainer(containerMenu->windowId, *minecraft.player);
		if (craftingWidth == 3 && craftingHeight == 3)
			containerMenu->onClosed(*minecraft.player);
	}
	Screen::removed();
}

void InventoryScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE || eventKey == minecraft.options.keyInventory.key)
	{
		minecraft.player->closeContainer();
		minecraft.grabMouse();
		return;
	}

	Screen::keyPressed(eventCharacter, eventKey);
}

void InventoryScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	if (minecraft.player == nullptr || (buttonNum != 0 && buttonNum != 1))
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
		if (slot == SLOT_RESULT)
			protocolSlot = 0;
		else if (slot >= SLOT_CRAFTING_BASE && slot < SLOT_CRAFTING_BASE + craftingWidth * craftingHeight)
			protocolSlot = 1 + slot - SLOT_CRAFTING_BASE;
		else if (craftingWidth == 2 && slot >= SLOT_ARMOR_BASE && slot < SLOT_ARMOR_BASE + 4)
			protocolSlot = 5 + slot - SLOT_ARMOR_BASE;
		else if (slot >= 9 && slot < 36)
			protocolSlot = craftingWidth == 2 ? slot : slot + 1;
		else if (slot >= 0 && slot < 9)
			protocolSlot = (craftingWidth == 2 ? 36 : 37) + slot;
	}
	if (protocolSlot != -1 && containerMenu != nullptr)
	{
		bool shiftClick = protocolSlot != -999
			&& (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT)
				|| lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT));
		minecraft.gameMode->clickContainer(containerMenu->windowId, protocolSlot,
			buttonNum, shiftClick, *minecraft.player);
		syncCraftingSlotsFromMenu();
	}
}

int_t InventoryScreen::getGuiLeft() const
{
	return (width - imageWidth) / 2;
}

int_t InventoryScreen::getGuiTop() const
{
	return (height - imageHeight) / 2;
}

int_t InventoryScreen::getInventorySlotX(int_t slot) const
{
	if (slot >= 0 && slot < 9)
		return 8 + slot * 18;
	return 8 + ((slot - 9) % 9) * 18;
}

int_t InventoryScreen::getInventorySlotY(int_t slot) const
{
	if (slot >= 0 && slot < 9)
		return 142;
	return 84 + ((slot - 9) / 9) * 18;
}

int_t InventoryScreen::getCraftingSlotX(int_t slot) const
{
	return getCraftingGridLeft() + (slot % craftingWidth) * 18;
}

int_t InventoryScreen::getCraftingSlotY(int_t slot) const
{
	return getCraftingGridTop() + (slot / craftingWidth) * 18;
}

int_t InventoryScreen::getSlotAt(int_t x, int_t y) const
{
	int_t relX = x - getGuiLeft();
	int_t relY = y - getGuiTop();

	if (isPointInSlot(relX, relY, getResultSlotX(), getResultSlotY()))
		return SLOT_RESULT;

	for (int_t slot = 0; slot < static_cast<int_t>(craftingSlots.size()); ++slot)
	{
		if (isPointInSlot(relX, relY, getCraftingSlotX(slot), getCraftingSlotY(slot)))
			return SLOT_CRAFTING_BASE + slot;
	}

	for (int_t slot = 0; slot < 36; ++slot)
	{
		if (isPointInSlot(relX, relY, getInventorySlotX(slot), getInventorySlotY(slot)))
			return slot;
	}

	for (int_t slot = 0; craftingWidth == 2 && craftingHeight == 2 && slot < 4; ++slot)
	{
		if (isPointInSlot(relX, relY, getArmorSlotX(slot), getArmorSlotY(slot)))
			return SLOT_ARMOR_BASE + slot;
	}

	return SLOT_NONE;
}

void InventoryScreen::updateCraftingResult()
{
	GridCraftingContainer container(craftingSlots, craftingWidth, craftingHeight);
	craftingResult = Recipes::getInstance().getItemFor(container);
}

void InventoryScreen::syncCraftingSlotsFromMenu()
{
	if (containerMenu == nullptr)
		return;
	for (int_t slot = 0; slot < static_cast<int_t>(craftingSlots.size()); ++slot)
	{
		const ItemInstance *item = containerMenu->getSlot(slot + 1).getItem();
		craftingSlots[slot] = item == nullptr ? ItemInstance() : *item;
	}
	const ItemInstance *result = containerMenu->getSlot(0).getItem();
	craftingResult = result == nullptr ? ItemInstance() : *result;
}

void InventoryScreen::consumeCraftingIngredients()
{
	for (ItemInstance &stack : craftingSlots)
	{
		if (stack.isEmpty())
			continue;
		--stack.stackSize;
		if (stack.isEmpty())
			stack = ItemInstance();
	}
}

void InventoryScreen::dropCraftingContents()
{
	if (minecraft.player == nullptr)
		return;

	for (ItemInstance &stack : craftingSlots)
	{
		if (stack.isEmpty())
			continue;
		minecraft.player->drop(stack);
		stack = ItemInstance();
	}
	craftingResult = ItemInstance();
}

void InventoryScreen::renderBg(float a)
{
	(void)a;
	int_t tex = minecraft.textures.loadTexture(getBackgroundTexture());
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(tex);

	int_t xo = getGuiLeft();
	int_t yo = getGuiTop();
	blit(xo, yo, 0, 0, imageWidth, imageHeight);

	if (minecraft.player == nullptr || !shouldRenderPlayerModel())
		return;

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_LIGHTING);
	renderPlayerModel(xo + 51, yo + 75, 30);
	glDisable(GL_RESCALE_NORMAL);
}

void InventoryScreen::renderLabels()
{
	jstring title = getTitleText();
	if (!title.empty())
		font.draw(title, getTitleX(), getTitleY(), 0x00404040);
}

void InventoryScreen::renderPlayerModel(int_t x, int_t y, int_t scale)
{
	float oldBodyRot = minecraft.player->yBodyRot;
	float oldYRot = minecraft.player->yRot;
	float oldXRot = minecraft.player->xRot;

	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), 50.0f);
	glScalef(static_cast<float>(-scale), static_cast<float>(scale), static_cast<float>(scale));
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);

	float xd = static_cast<float>(x) - xMouse;
	float yd = static_cast<float>(y - 50) - yMouse;

	glRotatef(135.0f, 0.0f, 1.0f, 0.0f);
	Lighting::turnOn();
	glRotatef(-135.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(-static_cast<float>(std::atan(yd / 40.0f)) * 20.0f, 1.0f, 0.0f, 0.0f);

	minecraft.player->yBodyRot = static_cast<float>(std::atan(xd / 40.0f)) * 20.0f;
	minecraft.player->yRot = static_cast<float>(std::atan(xd / 40.0f)) * 40.0f;
	minecraft.player->xRot = -static_cast<float>(std::atan(yd / 40.0f)) * 20.0f;

	minecraft.player->entityBrightness = 1.0f;
	glTranslatef(0.0f, minecraft.player->heightOffset, 0.0f);
	EntityRenderDispatcher::instance.playerRotY = 180.0f;
	EntityRenderDispatcher::instance.render(*minecraft.player, 0.0, 0.0, 0.0, 0.0f, 1.0f);
	minecraft.player->entityBrightness = 0.0f;

	minecraft.player->yBodyRot = oldBodyRot;
	minecraft.player->yRot = oldYRot;
	minecraft.player->xRot = oldXRot;

	glPopMatrix();
	Lighting::turnOff();
}

void InventoryScreen::renderSlot(ItemInstance &stack, int_t x, int_t y, float a)
{
	if (stack.isEmpty())
		return;

	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);

	itemRenderer.renderGuiItem(font, minecraft.textures, stack, x, y);

	itemRenderer.renderGuiItemDecorations(font, minecraft.textures, stack, x, y);
}

const ItemInstance *InventoryScreen::getSlotItem(int_t slot) const
{
	if (slot == SLOT_RESULT)
		return craftingResult.isEmpty() ? nullptr : &craftingResult;
	if (slot >= SLOT_CRAFTING_BASE && slot < SLOT_CRAFTING_BASE + static_cast<int_t>(craftingSlots.size()))
	{
		int_t craftingSlot = slot - SLOT_CRAFTING_BASE;
		if (!craftingSlots[craftingSlot].isEmpty())
			return &craftingSlots[craftingSlot];
		return nullptr;
	}
	if (slot >= 0 && slot < 36)
	{
		const ItemInstance &stack = minecraft.player->inventory.mainInventory[slot];
		return stack.isEmpty() ? nullptr : &stack;
	}

	
	if (craftingWidth == 2 && craftingHeight == 2
		&& slot >= SLOT_ARMOR_BASE && slot < SLOT_ARMOR_BASE + 4)
	{
		int_t armorIdx = 3 - (slot - SLOT_ARMOR_BASE);
		const ItemInstance &stack = minecraft.player->inventory.armorInventory[armorIdx];
		return stack.isEmpty() ? nullptr : &stack;
	}

	return nullptr;
}

void InventoryScreen::handleRegularSlotClick(ItemInstance &slotStack, int_t buttonNum)
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

void InventoryScreen::handleSlotClick(int_t slot, int_t buttonNum)
{
	InventoryPlayer &inventory = minecraft.player->inventory;
	if (slot == SLOT_RESULT)
	{
		if (craftingResult.isEmpty())
			return;

		ItemInstance result = craftingResult;
		ItemInstance *carried = inventory.getCarried();
		if (carried == nullptr)
			inventory.setCarried(result);
		else
		{
			bool canMerge = carried->sameItem(result) && carried->isStackable() && result.isStackable();
			if (!canMerge)
				return;

			int_t space = carried->getMaxStackSize() - carried->stackSize;
			if (space < result.stackSize)
				return;
			carried->stackSize += result.stackSize;
		}
		result.onCrafted(*minecraft.level, *minecraft.player);

		if (result.itemID == Tile::workBench.id)
			minecraft.player->addStat(*AchievementList::buildWorkBench, 1);
		else if (result.itemID == Items::pickaxeWood->getShiftedIndex())
			minecraft.player->addStat(*AchievementList::buildPickaxe, 1);
		else if (result.itemID == Tile::furnace.id)
			minecraft.player->addStat(*AchievementList::buildFurnace, 1);
		else if (result.itemID == Items::hoeWood->getShiftedIndex())
			minecraft.player->addStat(*AchievementList::buildHoe, 1);
		else if (result.itemID == Items::bread->getShiftedIndex())
			minecraft.player->addStat(*AchievementList::makeBread, 1);
		else if (result.itemID == Items::cake->getShiftedIndex())
			minecraft.player->addStat(*AchievementList::bakeCake, 1);
		else if (result.itemID == Items::pickaxeStone->getShiftedIndex())
			minecraft.player->addStat(*AchievementList::buildBetterPickaxe, 1);
		else if (result.itemID == Items::swordWood->getShiftedIndex())
			minecraft.player->addStat(*AchievementList::buildSword, 1);
		consumeCraftingIngredients();
		updateCraftingResult();
		return;
	}

	if (slot >= SLOT_CRAFTING_BASE)
	{
		handleRegularSlotClick(craftingSlots[slot - SLOT_CRAFTING_BASE], buttonNum);
		updateCraftingResult();
		return;
	}

	if (slot >= 0 && slot < 36)
	{
		handleRegularSlotClick(minecraft.player->inventory.mainInventory[slot], buttonNum);
		return;
	}

	
	if (slot >= SLOT_ARMOR_BASE && slot < SLOT_ARMOR_BASE + 4)
	{
		int_t armorDisplaySlot = slot - SLOT_ARMOR_BASE;
		int_t armorIdx = 3 - armorDisplaySlot; 
		ItemInstance &armorSlot = minecraft.player->inventory.armorInventory[armorIdx];
		ItemInstance *carried = inventory.getCarried();

		auto isValidArmorItem = [&](const ItemInstance &stack) {
			if (stack.isEmpty())
				return false;
			Item *stackItem = stack.getItem();
			auto *carriedArmor = dynamic_cast<ItemArmor *>(stackItem);
			if (carriedArmor != nullptr)
				return carriedArmor->armorType == armorDisplaySlot;
			return armorDisplaySlot == 0 && stack.itemID == Tile::pumpkin.id;
		};

		if (carried != nullptr && !carried->isEmpty() && !isValidArmorItem(*carried))
			return;

		if (armorSlot.isEmpty())
		{
			if (carried == nullptr || carried->isEmpty())
				return;
			armorSlot = ItemInstance(carried->itemID, 1, carried->itemDamage);
			carried->stackSize--;
			if (carried->isEmpty())
				inventory.setCarriedNull();
			return;
		}

		if (carried == nullptr || carried->isEmpty())
		{
			inventory.setCarried(armorSlot);
			armorSlot = ItemInstance();
			return;
		}

		if (carried->stackSize > 1)
			return;

		ItemInstance swapped = armorSlot;
		armorSlot = *carried;
		inventory.setCarried(swapped);
		return;
	}
}

jstring InventoryScreen::getBackgroundTexture() const
{
	return u"/gui/inventory.png";
}

int_t InventoryScreen::getCraftingGridLeft() const
{
	return 88;
}

int_t InventoryScreen::getCraftingGridTop() const
{
	return 26;
}

int_t InventoryScreen::getResultSlotX() const
{
	return 144;
}

int_t InventoryScreen::getResultSlotY() const
{
	return 36;
}

int_t InventoryScreen::getTitleX() const
{
	return 86;
}

int_t InventoryScreen::getTitleY() const
{
	return 16;
}

jstring InventoryScreen::getTitleText() const
{
	return u"Crafting";
}

bool InventoryScreen::shouldRenderPlayerModel() const
{
	return true;
}

int_t InventoryScreen::getArmorSlotX(int_t slot) const
{
	return 8;
}

int_t InventoryScreen::getArmorSlotY(int_t slot) const
{
	return 8 + slot * 18;
}

bool InventoryScreen::isArmorSlot(int_t slot) const
{
	return slot >= SLOT_ARMOR_BASE && slot < SLOT_ARMOR_BASE + 4;
}

int_t InventoryScreen::armorSlotToArmorIndex(int_t slot) const
{
	return 3 - (slot - SLOT_ARMOR_BASE);
}

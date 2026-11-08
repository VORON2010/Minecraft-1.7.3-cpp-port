#include "client/gui/CreativeInventoryScreen.h"

#include <algorithm>
#include <cmath>

#include "OpenGL.h"
#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/Options.h"
#include "client/gui/ContainerScreen.h"
#include "client/gui/GuiTextField.h"
#include "client/gui/RecipeScreen.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "lwjgl/Keyboard.h"
#include "lwjgl/Mouse.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/Container.h"
#include "world/inventory/Slot.h"
#include "world/item/CreativeModeTab.h"
#include "world/item/Item.h"
#include "world/stats/StatCollector.h"

int_t CreativeInventoryScreen::selectedTab = CreativeModeTab::BUILDING_BLOCKS;

namespace
{
	ItemRenderer &itemRenderer()
	{
		static ItemRenderer renderer(EntityRenderDispatcher::instance);
		return renderer;
	}

	jstring lower(const jstring &s)
	{
		jstring out = s;
		for (auto &c : out)
			if (c >= u'A' && c <= u'Z')
				c = static_cast<char16_t>(c - u'A' + u'a');
		return out;
	}

	bool sameItem(const ItemInstance &a, const ItemInstance &b)
	{
		return a.itemID == b.itemID && a.itemDamage == b.itemDamage;
	}
}

CreativeInventoryScreen::CreativeInventoryScreen(Minecraft &minecraft) : Screen(minecraft)
{
	passEvents = true;
}

void CreativeInventoryScreen::init()
{
	buttons.clear();
	leftPos = (width - imageWidth) / 2;
	topPos = (height - imageHeight) / 2;
	searchBox = Util::make_shared<GuiTextField>(*this, font, leftPos + 82, topPos + 6, 89, font.fontHeight, u"");
	searchBox->setMaxStringLength(15);
	searchBox->isFocused = false;
	int_t tab = selectedTab;
	selectedTab = -1;
	selectTab(tab);
}

// ---- items and tabs ----

void CreativeInventoryScreen::selectTab(int_t tab)
{
	selectedTab = tab;
	items.clear();
	if (tab != CreativeModeTab::SEARCH && tab != CreativeModeTab::INVENTORY)
		CreativeModeTab::addItems(tab, items);
	if (searchBox != nullptr)
	{
		if (tab == CreativeModeTab::SEARCH)
		{
			searchBox->isFocused = true;
			searchBox->setText(u"");
			refreshSearch();
		}
		else
			searchBox->isFocused = false;
	}
	scrollOffs = 0.0f;
}

void CreativeInventoryScreen::refreshSearch()
{
	items.clear();
	std::vector<ItemInstance> all;
	CreativeModeTab::addAllItems(all);
	jstring query = lower(searchBox->getText());
	for (const ItemInstance &stack : all)
	{
		for (const jstring &line : ContainerScreen::getTooltipLines(stack, &minecraft.options))
		{
			if (lower(line).find(query) != jstring::npos)
			{
				items.push_back(stack);
				break;
			}
		}
	}
	scrollOffs = 0.0f;
}

bool CreativeInventoryScreen::canScroll() const
{
	return selectedTab != CreativeModeTab::INVENTORY && CreativeModeTab::hasScrollbar(selectedTab) && items.size() > 45;
}

int_t CreativeInventoryScreen::scrollRow() const
{
	int_t rows = static_cast<int_t>(items.size()) / 9 - 5 + 1;
	int_t row = static_cast<int_t>(scrollOffs * rows + 0.5);
	return row < 0 ? 0 : row;
}

ItemInstance *CreativeInventoryScreen::listItem(int_t cell)
{
	int_t index = cell % 9 + (cell / 9 + scrollRow()) * 9;
	if (index >= 0 && index < static_cast<int_t>(items.size()))
		return &items[index];
	return nullptr;
}

// ---- slots ----

std::vector<CreativeInventoryScreen::SlotRef> CreativeInventoryScreen::getSlots() const
{
	std::vector<SlotRef> slots;
	if (selectedTab == CreativeModeTab::INVENTORY)
	{
		// 1.5.2 setSelectedTab(INVENTORY): the player's own menu, laid out on survival_inv.png
		for (int_t i = 5; i < 45; ++i)
		{
			SlotRef s;
			s.kind = SlotRef::PLAYER;
			s.index = i;
			if (i < 9)
			{
				s.x = 9 + (i - 5) / 2 * 54;
				s.y = 6 + (i - 5) % 2 * 27;
			}
			else
			{
				s.x = 9 + (i - 9) % 9 * 18;
				s.y = i >= 36 ? 112 : 54 + (i - 9) / 9 * 18;
			}
			slots.push_back(s);
		}
		SlotRef bin;
		bin.kind = SlotRef::BIN;
		bin.x = 173;
		bin.y = 112;
		slots.push_back(bin);
		return slots;
	}
	for (int_t row = 0; row < 5; ++row)
		for (int_t col = 0; col < 9; ++col)
		{
			SlotRef s;
			s.kind = SlotRef::LIST;
			s.index = col + row * 9;
			s.x = 9 + col * 18;
			s.y = 18 + row * 18;
			slots.push_back(s);
		}
	for (int_t col = 0; col < 9; ++col)
	{
		SlotRef s;
		s.kind = SlotRef::HOTBAR;
		s.index = col;
		s.x = 9 + col * 18;
		s.y = 112;
		slots.push_back(s);
	}
	return slots;
}

CreativeInventoryScreen::SlotRef CreativeInventoryScreen::findSlot(int_t mx, int_t my) const
{
	int_t rx = mx - leftPos;
	int_t ry = my - topPos;
	for (const SlotRef &s : getSlots())
		if (rx >= s.x - 1 && rx < s.x + 16 + 1 && ry >= s.y - 1 && ry < s.y + 16 + 1)
			return s;
	return SlotRef();
}

ItemInstance *CreativeInventoryScreen::slotItem(const SlotRef &slot)
{
	switch (slot.kind)
	{
	case SlotRef::LIST:
		return listItem(slot.index);
	case SlotRef::HOTBAR:
		return minecraft.player->inventory.getItem(slot.index);
	case SlotRef::PLAYER:
		return minecraft.player->inventorySlots->getSlot(slot.index).getItem();
	default:
		return nullptr;
	}
}

void CreativeInventoryScreen::dropFromMenu(const ItemInstance &stack)
{
	ItemInstance copy = stack;
	minecraft.player->drop(copy, true);
}

// 1.5.2 CreativeInventoryScreen.clickSlot
void CreativeInventoryScreen::slotClicked(const SlotRef &slot, int_t button, bool shift, bool outside)
{
	scrolling = false;
	InventoryPlayer &inventory = minecraft.player->inventory;
	ItemInstance *carried = inventory.getCarried();
	bool hasCarried = carried != nullptr && !carried->isEmpty();

	if (slot.kind == SlotRef::NONE && selectedTab != CreativeModeTab::INVENTORY)
	{
		if (outside && hasCarried)
		{
			if (button == 0)
			{
				dropFromMenu(*carried);
				inventory.setCarriedNull();
			}
			else if (button == 1)
			{
				ItemInstance one = carried->remove(1);
				dropFromMenu(one);
				if (carried->stackSize == 0)
					inventory.setCarriedNull();
			}
		}
		return;
	}

	if (slot.kind == SlotRef::BIN)
	{
		if (shift)
		{
			// shift-click on the bin empties the whole inventory
			for (int_t i = 0; i < inventory.getSizeInventory(); ++i)
				inventory.setItem(i, ItemInstance());
		}
		else
			inventory.setCarriedNull();
		return;
	}

	if (selectedTab == CreativeModeTab::INVENTORY)
	{
		if (slot.kind == SlotRef::NONE)
		{
			if (outside && hasCarried)
			{
				dropFromMenu(*carried);
				inventory.setCarriedNull();
			}
			return;
		}
		minecraft.player->inventorySlots->click(slot.index, button, shift, *minecraft.player);
		return;
	}

	if (slot.kind == SlotRef::HOTBAR)
	{
		if (shift)
		{
			// CreativePlayerMenu.quickMoveItem: shift-click clears a hotbar slot
			inventory.setItem(slot.index, ItemInstance());
			return;
		}
		minecraft.player->inventorySlots->click(36 + slot.index, button, false, *minecraft.player);
		return;
	}

	// a cell of the item list
	ItemInstance *stack = listItem(slot.index);
	if (button == 2)
	{
		if (!hasCarried && stack != nullptr)
		{
			ItemInstance copy = *stack;
			copy.stackSize = copy.getMaxStackSize();
			inventory.setCarried(copy);
		}
		return;
	}
	if (hasCarried && stack != nullptr && sameItem(*carried, *stack))
	{
		if (button == 0)
		{
			if (shift)
				carried->stackSize = carried->getMaxStackSize();
			else if (carried->stackSize < carried->getMaxStackSize())
				carried->stackSize = carried->stackSize + 1;
		}
		else if (carried->stackSize <= 1)
			inventory.setCarriedNull();
		else
			carried->stackSize = carried->stackSize - 1;
	}
	else if (stack != nullptr && !hasCarried)
	{
		ItemInstance copy = *stack;
		copy.stackSize = shift ? copy.getMaxStackSize() : 1;
		inventory.setCarried(copy);
	}
	else
		inventory.setCarriedNull();
}

// ---- input ----

bool CreativeInventoryScreen::checkTabClicked(int_t tab, int_t mx, int_t my) const
{
	int_t col = CreativeModeTab::getColumn(tab);
	int_t x = 28 * col;
	int_t y = 0;
	if (col == 5)
		x = imageWidth - 28 + 2;
	else if (col > 0)
		x += col;
	if (CreativeModeTab::isTopRow(tab))
		y -= 32;
	else
		y += imageHeight;
	return mx >= x && mx <= x + 28 && my >= y && my <= y + 32;
}

void CreativeInventoryScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	if (buttonNum == 0)
	{
		for (int_t tab = 0; tab < CreativeModeTab::COUNT; ++tab)
			if (checkTabClicked(tab, x - leftPos, y - topPos))
				return;
		int_t barX = leftPos + 175;
		int_t barY = topPos + 18;
		if (x >= barX && y >= barY && x < barX + 14 && y < barY + 112 && canScroll())
		{
			scrolling = true;
			return;
		}
	}
	SlotRef slot = findSlot(x, y);
	bool outside = x < leftPos || y < topPos || x >= leftPos + imageWidth || y >= topPos + imageHeight;
	bool shift = lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) || lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT);
	if (buttonNum == 0 || buttonNum == 1 || buttonNum == 2)
		slotClicked(slot, buttonNum, shift, outside);
}

void CreativeInventoryScreen::mouseReleased(int_t x, int_t y, int_t buttonNum)
{
	if (buttonNum == 0)
	{
		scrolling = false;
		for (int_t tab = 0; tab < CreativeModeTab::COUNT; ++tab)
			if (checkTabClicked(tab, x - leftPos, y - topPos))
			{
				selectTab(tab);
				return;
			}
	}
}

void CreativeInventoryScreen::mouseScrolled(int_t x, int_t y, int_t scrollAmount)
{
	(void)x;
	(void)y;
	if (scrollAmount == 0 || !canScroll())
		return;
	int_t rows = static_cast<int_t>(items.size()) / 9 - 5 + 1;
	int_t dir = scrollAmount > 0 ? 1 : -1;
	scrollOffs = static_cast<float>(scrollOffs - static_cast<double>(dir) / rows);
	scrollOffs = std::max(0.0f, std::min(1.0f, scrollOffs));
}

void CreativeInventoryScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE)
	{
		minecraft.setScreen(nullptr);
		minecraft.grabMouse();
		return;
	}

	int_t xm = lwjgl::Mouse::getX() * width / minecraft.width;
	int_t ym = height - lwjgl::Mouse::getY() * height / minecraft.height - 1;
	SlotRef hovered = findSlot(xm, ym);

	// 1.5.2 moveHoveredSlotToHotbar: number keys put the hovered item on the hotbar
	if (eventKey >= lwjgl::Keyboard::KEY_1 && eventKey <= lwjgl::Keyboard::KEY_9 && minecraft.player->inventory.getCarried() == nullptr)
	{
		int_t hotbar = eventKey - lwjgl::Keyboard::KEY_1;
		if (hovered.kind == SlotRef::LIST)
		{
			ItemInstance *stack = listItem(hovered.index);
			if (stack != nullptr)
			{
				ItemInstance copy = *stack;
				copy.stackSize = copy.getMaxStackSize();
				minecraft.player->inventory.setItem(hotbar, copy);
			}
			return;
		}
		if (hovered.kind == SlotRef::PLAYER && hovered.index != 36 + hotbar)
		{
			Container &menu = *minecraft.player->inventorySlots;
			ItemInstance *a = menu.getSlot(hovered.index).getItem();
			ItemInstance *b = minecraft.player->inventory.getItem(hotbar);
			ItemInstance first = a != nullptr ? *a : ItemInstance();
			ItemInstance second = b != nullptr ? *b : ItemInstance();
			menu.getSlot(hovered.index).set(second.isEmpty() ? nullptr : &second);
			minecraft.player->inventory.setItem(hotbar, first);
			return;
		}
	}

	if (eventKey == lwjgl::Keyboard::KEY_F4 && hovered.kind != SlotRef::NONE && hovered.kind != SlotRef::BIN)
	{
		ItemInstance *stack = slotItem(hovered);
		if (stack != nullptr && !stack->isEmpty())
		{
			minecraft.setScreen(Util::make_shared<RecipeScreen>(minecraft, minecraft.screen, stack->itemID, stack->itemDamage));
			return;
		}
	}

	if (selectedTab != CreativeModeTab::SEARCH)
	{
		if (eventKey == minecraft.options.keyChat.key)
		{
			selectTab(CreativeModeTab::SEARCH);
			return;
		}
		if (eventKey == minecraft.options.keyInventory.key)
		{
			minecraft.setScreen(nullptr);
			minecraft.grabMouse();
		}
		return;
	}

	jstring before = searchBox->getText();
	searchBox->textboxKeyTyped(eventCharacter, eventKey);
	if (searchBox->getText() != before)
		refreshSearch();
}

// ---- rendering ----

void CreativeInventoryScreen::renderTab(int_t tab)
{
	bool selected = tab == selectedTab;
	bool top = CreativeModeTab::isTopRow(tab);
	int_t col = CreativeModeTab::getColumn(tab);
	int_t u = col * 28;
	int_t v = selected ? 32 : 0;
	int_t x = leftPos + 28 * col;
	int_t y = topPos;
	if (col == 5)
		x = leftPos + imageWidth - 28;
	else if (col > 0)
		x += col;
	if (top)
		y -= 28;
	else
	{
		v += 64;
		y += imageHeight - 4;
	}
	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(minecraft.textures.loadTexture(u"/gui/allitems.png"));
	blit(x, y, u, v, 28, 32);
	x += 6;
	y += 8 + (top ? 1 : -1);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 100.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_RESCALE_NORMAL);
	ItemInstance icon(CreativeModeTab::getIconItem(tab), 1, 0);
	itemRenderer().renderGuiItem(font, minecraft.textures, icon, x, y);
	itemRenderer().renderGuiItemDecorations(font, minecraft.textures, icon, x, y);
	glDisable(GL_LIGHTING);
	glPopMatrix();
}

bool CreativeInventoryScreen::renderTabTooltip(int_t tab, int_t mx, int_t my)
{
	int_t col = CreativeModeTab::getColumn(tab);
	int_t x = 28 * col;
	int_t y = 0;
	if (col == 5)
		x = imageWidth - 28 + 2;
	else if (col > 0)
		x += col;
	if (CreativeModeTab::isTopRow(tab))
		y -= 32;
	else
		y += imageHeight;
	int_t rx = mx - leftPos;
	int_t ry = my - topPos;
	if (rx >= x + 3 - 1 && rx < x + 3 + 23 + 1 && ry >= y + 3 - 1 && ry < y + 3 + 27 + 1)
	{
		renderTooltip({StatCollector::translate(u"itemGroup." + CreativeModeTab::getName(tab))}, mx + 12, my - 12);
		return true;
	}
	return false;
}

void CreativeInventoryScreen::renderPlayer(int_t x, int_t y, int_t scale, float xd, float yd)
{
	Player &player = *minecraft.player;
	float oldBody = player.yBodyRot, oldY = player.yRot, oldX = player.xRot, oldHead = player.yHeadRot, oldHeadO = player.yHeadRotO;
	glEnable(GL_COLOR_MATERIAL);
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), 50.0f);
	glScalef(static_cast<float>(-scale), static_cast<float>(scale), static_cast<float>(scale));
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(135.0f, 0.0f, 1.0f, 0.0f);
	Lighting::turnOn();
	glRotatef(-135.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(-static_cast<float>(std::atan(yd / 40.0f)) * 20.0f, 1.0f, 0.0f, 0.0f);
	player.yBodyRot = static_cast<float>(std::atan(xd / 40.0f)) * 20.0f;
	player.yRot = static_cast<float>(std::atan(xd / 40.0f)) * 40.0f;
	player.xRot = -static_cast<float>(std::atan(yd / 40.0f)) * 20.0f;
	player.yHeadRot = player.yRot;
	player.yHeadRotO = player.yRot;
	glTranslatef(0.0f, player.heightOffset, 0.0f);
	EntityRenderDispatcher::instance.playerRotY = 180.0f;
	EntityRenderDispatcher::instance.render(player, 0.0, 0.0, 0.0, 0.0f, 1.0f);
	player.yBodyRot = oldBody;
	player.yRot = oldY;
	player.xRot = oldX;
	player.yHeadRot = oldHead;
	player.yHeadRotO = oldHeadO;
	glPopMatrix();
	Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);
}

void CreativeInventoryScreen::render(int_t xm, int_t ym, float a)
{
	(void)a;
	if (minecraft.player == nullptr)
		return;
	renderBackground();

	// scroll bar dragging
	bool down = lwjgl::Mouse::isButtonDown(0);
	int_t barX0 = leftPos + 175;
	int_t barY0 = topPos + 18;
	int_t barX1 = barX0 + 14;
	int_t barY1 = barY0 + 112;
	if (!wasClicking && down && xm >= barX0 && ym >= barY0 && xm < barX1 && ym < barY1)
		scrolling = canScroll();
	if (!down)
		scrolling = false;
	wasClicking = down;
	if (scrolling)
	{
		scrollOffs = (static_cast<float>(ym - barY0) - 7.5f) / (static_cast<float>(barY1 - barY0) - 15.0f);
		scrollOffs = std::max(0.0f, std::min(1.0f, scrollOffs));
	}

	// renderMenuBackground: the other tabs behind, the panel, then the selected tab
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	for (int_t tab = 0; tab < CreativeModeTab::COUNT; ++tab)
		if (tab != selectedTab)
			renderTab(tab);
	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(minecraft.textures.loadTexture(jstring(u"/gui/creative_inv/") + CreativeModeTab::getTexture(selectedTab)));
	blit(leftPos, topPos, 0, 0, imageWidth, imageHeight);
	if (selectedTab == CreativeModeTab::SEARCH)
	{
		bool cursor = searchBox->isFocused && (minecraft.level != nullptr ? minecraft.level->time : 0) / 6 % 2 == 0;
		font.drawShadow(searchBox->getText() + (cursor ? u"_" : u""), leftPos + 82, topPos + 6, 0xFFFFFF);
	}
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(minecraft.textures.loadTexture(u"/gui/allitems.png"));
	if (CreativeModeTab::hasScrollbar(selectedTab))
		blit(barX0, barY0 + static_cast<int_t>((barY1 - barY0 - 17) * scrollOffs), 232 + (canScroll() ? 0 : 12), 0, 12, 15);
	renderTab(selectedTab);
	if (selectedTab == CreativeModeTab::INVENTORY)
		renderPlayer(leftPos + 43, topPos + 45, 20, static_cast<float>(leftPos + 43 - xm), static_cast<float>(topPos + 45 - 30 - ym));

	// slots
	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(static_cast<float>(leftPos), static_cast<float>(topPos), 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_RESCALE_NORMAL);
	SlotRef hovered = findSlot(xm, ym);
	for (const SlotRef &s : getSlots())
	{
		ItemInstance *stack = slotItem(s);
		if (stack == nullptr || stack->isEmpty())
		{
			if (s.kind == SlotRef::PLAYER && s.index >= 5 && s.index < 9)
			{
				// empty armour slots show their outline icon
				int_t icon = minecraft.player->inventorySlots->getSlot(s.index).getBackgroundIcon();
				if (icon >= 0)
				{
					glDisable(GL_LIGHTING);
					minecraft.textures.bind(minecraft.textures.loadTexture(u"/gui/items.png"));
					blit(s.x, s.y, icon % 16 * 16, icon / 16 * 16, 16, 16);
					glEnable(GL_LIGHTING);
				}
			}
			continue;
		}
		itemRenderer().renderGuiItem(font, minecraft.textures, *stack, s.x, s.y);
		itemRenderer().renderGuiItemDecorations(font, minecraft.textures, *stack, s.x, s.y);
	}
	if (hovered.kind != SlotRef::NONE)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
		fillGradient(hovered.x, hovered.y, hovered.x + 16, hovered.y + 16, 0x80FFFFFF, 0x80FFFFFF);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}
	ItemInstance *carried = minecraft.player->inventory.getCarried();
	if (carried != nullptr && !carried->isEmpty())
	{
		glTranslatef(0.0f, 0.0f, 32.0f);
		itemRenderer().renderGuiItem(font, minecraft.textures, *carried, xm - leftPos - 8, ym - topPos - 8);
		itemRenderer().renderGuiItemDecorations(font, minecraft.textures, *carried, xm - leftPos - 8, ym - topPos - 8);
		glTranslatef(0.0f, 0.0f, -32.0f);
	}
	glDisable(GL_RESCALE_NORMAL);
	Lighting::turnOff();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	// renderLabels
	if (CreativeModeTab::hasTooltips(selectedTab))
		font.draw(StatCollector::translate(u"itemGroup." + CreativeModeTab::getName(selectedTab)), 8, 6, 0x404040);
	glPopMatrix();

	// tooltips
	if (carried == nullptr || carried->isEmpty())
	{
		ItemInstance *stack = hovered.kind != SlotRef::NONE ? slotItem(hovered) : nullptr;
		if (stack != nullptr && !stack->isEmpty())
		{
			auto lines = ContainerScreen::getTooltipLines(*stack, &minecraft.options);
			if (!lines.empty())
				renderTooltip(lines, xm + 12, ym - 12);
		}
	}
	for (int_t tab = 0; tab < CreativeModeTab::COUNT; ++tab)
		if (renderTabTooltip(tab, xm, ym))
			break;
	if (hovered.kind == SlotRef::BIN)
		renderTooltip({StatCollector::translate(u"inventory.binSlot")}, xm + 12, ym - 12);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

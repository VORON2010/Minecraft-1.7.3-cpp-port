#include "pc/vulkan/VulkanMatrixStack.h"
#include "client/gui/RecipeScreen.h"

#include <algorithm>
#include <set>

#include "client/Minecraft.h"
#include "client/Lighting.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/gui/ContainerScreen.h"
#include "world/item/crafting/Recipes.h"
#include "world/item/crafting/FurnaceRecipes.h"
#include "world/item/Item.h"
#include "world/level/tile/Tile.h"
#include "lwjgl/Keyboard.h"
#include "OpenGL.h"

RecipeScreen::RecipeScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, int_t targetItemId, int_t targetDamage)
	: Screen(minecraft), lastScreen(lastScreen), targetItemId(targetItemId), targetDamage(targetDamage)
{
	loadRecipes();
	if (targetItemId >= 0)
	{
		filterRecipes();
		browseMode = false;
	}
	else
	{
		browseMode = true;
	}
}

void RecipeScreen::loadRecipes()
{
	allRecipes.clear();
	uniqueOutputs.clear();
	std::set<int_t> seenItemIds;

	// 1. Crafting recipes
	for (const auto &r : Recipes::getInstance().getRecipes())
	{
		if (r.result.isEmpty())
			continue;

		RecipeEntry entry;
		entry.type = r.shapeless ? RecipeEntry::Type::CRAFTING_SHAPELESS : RecipeEntry::Type::CRAFTING_SHAPED;
		entry.width = r.width;
		entry.height = r.height;
		entry.result = r.result;
		entry.inputs.resize(9);

		if (r.shapeless)
		{
			for (size_t i = 0; i < r.items.size() && i < 9; i++)
				entry.inputs[i] = r.items[i];
		}
		else
		{
			for (int_t y = 0; y < r.height && y < 3; y++)
			{
				for (int_t x = 0; x < r.width && x < 3; x++)
				{
					entry.inputs[x + y * 3] = r.items[x + y * r.width];
				}
			}
		}

		allRecipes.push_back(entry);
		if (seenItemIds.find(r.result.itemID) == seenItemIds.end())
		{
			seenItemIds.insert(r.result.itemID);
			uniqueOutputs.push_back(r.result);
		}
	}

	// 2. Furnace recipes
	for (const auto &pair : FurnaceRecipes::getInstance().getRecipes())
	{
		int_t inputId = pair.first;
		const ItemInstance &result = pair.second;
		if (result.isEmpty())
			continue;

		RecipeEntry entry;
		entry.type = RecipeEntry::Type::SMELTING;
		entry.width = 1;
		entry.height = 1;
		entry.result = result;
		entry.inputs.resize(9);
		entry.inputs[0] = ItemInstance(inputId, 1, 0);

		allRecipes.push_back(entry);
		if (seenItemIds.find(result.itemID) == seenItemIds.end())
		{
			seenItemIds.insert(result.itemID);
			uniqueOutputs.push_back(result);
		}
	}
}

void RecipeScreen::filterRecipes()
{
	filteredIndices.clear();
	currentRecipeIdx = 0;

	if (targetItemId < 0)
	{
		for (size_t i = 0; i < allRecipes.size(); i++)
			filteredIndices.push_back(i);
		return;
	}

	// First find recipes that produce targetItemId
	for (size_t i = 0; i < allRecipes.size(); i++)
	{
		if (allRecipes[i].result.itemID == targetItemId)
		{
			if (targetDamage < 0 || allRecipes[i].result.itemDamage == targetDamage || allRecipes[i].result.itemDamage == -1)
				filteredIndices.push_back(i);
		}
	}

	// If no recipes produce it, find recipes that use it as ingredient
	if (filteredIndices.empty())
	{
		for (size_t i = 0; i < allRecipes.size(); i++)
		{
			for (const auto &in : allRecipes[i].inputs)
			{
				if (!in.isEmpty() && in.itemID == targetItemId)
				{
					filteredIndices.push_back(i);
					break;
				}
			}
		}
	}

	if (filteredIndices.empty())
	{
		// Fallback to all recipes
		for (size_t i = 0; i < allRecipes.size(); i++)
			filteredIndices.push_back(i);
	}
}

void RecipeScreen::setTargetItem(int_t itemId, int_t damage)
{
	targetItemId = itemId;
	targetDamage = damage;
	browseMode = false;
	filterRecipes();
	init();
}

void RecipeScreen::init()
{
	buttons.clear();
	int_t leftPos = (width - imageWidth) / 2;
	int_t topPos = (height - imageHeight) / 2;

	if (browseMode)
	{
		buttons.push_back(Util::make_shared<Button>(1, leftPos + 10, topPos + 140, 45, 20, u"< Prev"));
		buttons.push_back(Util::make_shared<Button>(2, leftPos + 60, topPos + 140, 45, 20, u"Next >"));
		buttons.push_back(Util::make_shared<Button>(4, leftPos + 115, topPos + 140, 50, 20, u"Done"));
	}
	else
	{
		buttons.push_back(Util::make_shared<Button>(1, leftPos + 10, topPos + 140, 24, 20, u"<"));
		buttons.push_back(Util::make_shared<Button>(2, leftPos + 38, topPos + 140, 24, 20, u">"));
		buttons.push_back(Util::make_shared<Button>(3, leftPos + 66, topPos + 140, 50, 20, u"All Items"));
		buttons.push_back(Util::make_shared<Button>(4, leftPos + 120, topPos + 140, 46, 20, u"Done"));
	}
}

void RecipeScreen::buttonClicked(Button &button)
{
	if (!button.active) return;

	if (button.id == 1) // Prev
	{
		if (browseMode)
		{
			if (browsePage > 0) browsePage--;
		}
		else if (!filteredIndices.empty())
		{
			if (currentRecipeIdx == 0)
				currentRecipeIdx = filteredIndices.size() - 1;
			else
				currentRecipeIdx--;
		}
	}
	else if (button.id == 2) // Next
	{
		if (browseMode)
		{
			int_t maxPages = (static_cast<int_t>(uniqueOutputs.size()) + 39) / 40;
			if (browsePage + 1 < maxPages) browsePage++;
		}
		else if (!filteredIndices.empty())
		{
			currentRecipeIdx = (currentRecipeIdx + 1) % filteredIndices.size();
		}
	}
	else if (button.id == 3) // All Items
	{
		minecraft.setScreen(Util::make_shared<RecipeScreen>(minecraft, lastScreen));
	}
	else if (button.id == 4) // Done
	{
		minecraft.setScreen(lastScreen);
	}
}

void RecipeScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE)
	{
		minecraft.setScreen(lastScreen);
		return;
	}
	if (eventKey == lwjgl::Keyboard::KEY_LEFT)
	{
		Button b(1, 0, 0, u"");
		buttonClicked(b);
		return;
	}
	if (eventKey == lwjgl::Keyboard::KEY_RIGHT)
	{
		Button b(2, 0, 0, u"");
		buttonClicked(b);
		return;
	}

	Screen::keyPressed(eventCharacter, eventKey);
}

void RecipeScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	Screen::mouseClicked(x, y, buttonNum);

	int_t leftPos = (width - imageWidth) / 2;
	int_t topPos = (height - imageHeight) / 2;

	if (browseMode)
	{
		int_t startIndex = browsePage * 40;
		for (int_t i = 0; i < 40 && (startIndex + i) < static_cast<int_t>(uniqueOutputs.size()); i++)
		{
			int_t col = i % 8;
			int_t row = i / 8;
			int_t slotX = leftPos + 16 + col * 18;
			int_t slotY = topPos + 24 + row * 18;
			if (x >= slotX && x < slotX + 18 && y >= slotY && y < slotY + 18)
			{
				setTargetItem(uniqueOutputs[startIndex + i].itemID, uniqueOutputs[startIndex + i].itemDamage);
				return;
			}
		}
	}
	else if (!filteredIndices.empty())
	{
		const auto &recipe = allRecipes[filteredIndices[currentRecipeIdx]];
		// Check result slot
		int_t resX = leftPos + 124;
		int_t resY = topPos + 35;
		if (x >= resX && x < resX + 18 && y >= resY && y < resY + 18)
		{
			setTargetItem(recipe.result.itemID, recipe.result.itemDamage);
			return;
		}

		// Check input slots
		if (recipe.type == RecipeEntry::Type::SMELTING)
		{
			int_t inX = leftPos + 48;
			int_t inY = topPos + 35;
			if (x >= inX && x < inX + 18 && y >= inY && y < inY + 18 && !recipe.inputs[0].isEmpty())
			{
				setTargetItem(recipe.inputs[0].itemID, recipe.inputs[0].itemDamage);
				return;
			}
		}
		else
		{
			for (int_t r = 0; r < 3; r++)
			{
				for (int_t c = 0; c < 3; c++)
				{
					int_t slotX = leftPos + 30 + c * 18;
					int_t slotY = topPos + 17 + r * 18;
					if (x >= slotX && x < slotX + 18 && y >= slotY && y < slotY + 18)
					{
						const auto &in = recipe.inputs[c + r * 3];
						if (!in.isEmpty())
						{
							setTargetItem(in.itemID, in.itemDamage);
							return;
						}
					}
				}
			}
		}
	}
}

void RecipeScreen::render(int_t xm, int_t ym, float a)
{
	renderBackground();

	int_t leftPos = (width - imageWidth) / 2;
	int_t topPos = (height - imageHeight) / 2;

	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);
	const ItemInstance *hoveredItem = nullptr;

	if (browseMode)
	{
		// Draw frame
		fillGradient(leftPos, topPos, leftPos + imageWidth, topPos + imageHeight, 0xD0101010, 0xD0101010);
		drawCenteredString(font, u"Craftable Items", width / 2, topPos + 8, 0xFFFFFF);

		int_t maxPages = std::max(1, (static_cast<int_t>(uniqueOutputs.size()) + 39) / 40);
		jstring pageStr = u"Page " + String::toString(browsePage + 1) + u" / " + String::toString(maxPages);
		drawCenteredString(font, pageStr, width / 2, topPos + 122, 0xAAAAAA);

		VulkanMatrixStack::get().pushMatrix();
		VulkanMatrixStack::get().rotatef(120.0f, 1.0f, 0.0f, 0.0f);
		Lighting::turnOn();
		VulkanMatrixStack::get().popMatrix();

		VulkanMatrixStack::get().pushMatrix();
		glEnable(GL_RESCALE_NORMAL);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);

		int_t startIndex = browsePage * 40;
		for (int_t i = 0; i < 40 && (startIndex + i) < static_cast<int_t>(uniqueOutputs.size()); i++)
		{
			int_t col = i % 8;
			int_t row = i / 8;
			int_t slotX = leftPos + 16 + col * 18;
			int_t slotY = topPos + 24 + row * 18;

			fillGradient(slotX, slotY, slotX + 16, slotY + 16, 0x40FFFFFF, 0x40FFFFFF);
			ItemInstance stack = uniqueOutputs[startIndex + i];
			itemRenderer.renderGuiItem(font, minecraft.textures, stack, slotX, slotY);
			itemRenderer.renderGuiItemDecorations(font, minecraft.textures, stack, slotX, slotY);

			if (xm >= slotX && xm < slotX + 18 && ym >= slotY && ym < slotY + 18)
				hoveredItem = &uniqueOutputs[startIndex + i];
		}

		glDisable(GL_RESCALE_NORMAL);
		Lighting::turnOff();
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		VulkanMatrixStack::get().popMatrix();
	}
	else if (!filteredIndices.empty())
	{
		const auto &recipe = allRecipes[filteredIndices[currentRecipeIdx]];

		// Draw crafting / furnace panel
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_ALPHA_TEST);
		Textures &t = minecraft.textures;
		t.bind(t.loadTexture(u"/gui/crafting.png"));
		blit(leftPos, topPos, 0, 0, imageWidth, 80);
		glDisable(GL_BLEND);

		// Header: Result item name
		jstring titleName = ContainerScreen::getTooltipName(recipe.result);
		if (recipe.result.stackSize > 1)
			titleName += u" x" + String::toString(recipe.result.stackSize);
		font.draw(titleName, width / 2 - font.width(titleName) / 2, topPos + 4, 0x404040);

		// Subtitle: Recipe type and counter
		jstring typeStr;
		if (recipe.type == RecipeEntry::Type::SMELTING)
			typeStr = u"Furnace Smelting";
		else if (recipe.type == RecipeEntry::Type::CRAFTING_SHAPELESS)
			typeStr = u"Shapeless Crafting";
		else
			typeStr = u"Shaped Crafting (" + String::toString(recipe.width) + u"x" + String::toString(recipe.height) + u")";

		jstring countStr = String::toString(static_cast<int_t>(currentRecipeIdx + 1)) + u" / " + String::toString(static_cast<int_t>(filteredIndices.size()));
		drawString(font, typeStr, leftPos + 10, topPos + 84, 0xFFFFFF);
		drawString(font, countStr, leftPos + 10, topPos + 100, 0xAAAAAA);
		drawString(font, u"Hint: click ingredient to view recipe", leftPos + 10, topPos + 116, 0x888888);

		VulkanMatrixStack::get().pushMatrix();
		VulkanMatrixStack::get().rotatef(120.0f, 1.0f, 0.0f, 0.0f);
		Lighting::turnOn();
		VulkanMatrixStack::get().popMatrix();

		VulkanMatrixStack::get().pushMatrix();
		glEnable(GL_RESCALE_NORMAL);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);

		// Render inputs
		if (recipe.type == RecipeEntry::Type::SMELTING)
		{
			int_t inX = leftPos + 48;
			int_t inY = topPos + 35;
			if (!recipe.inputs[0].isEmpty())
			{
				ItemInstance inCopy = recipe.inputs[0];
				itemRenderer.renderGuiItem(font, minecraft.textures, inCopy, inX, inY);
				itemRenderer.renderGuiItemDecorations(font, minecraft.textures, inCopy, inX, inY);
				if (xm >= inX && xm < inX + 18 && ym >= inY && ym < inY + 18)
					hoveredItem = &recipe.inputs[0];
			}
		}
		else
		{
			for (int_t r = 0; r < 3; r++)
			{
				for (int_t c = 0; c < 3; c++)
				{
					int_t slotX = leftPos + 30 + c * 18;
					int_t slotY = topPos + 17 + r * 18;
					const auto &in = recipe.inputs[c + r * 3];
					if (!in.isEmpty())
					{
						ItemInstance inCopy = in;
						itemRenderer.renderGuiItem(font, minecraft.textures, inCopy, slotX, slotY);
						itemRenderer.renderGuiItemDecorations(font, minecraft.textures, inCopy, slotX, slotY);
						if (xm >= slotX && xm < slotX + 18 && ym >= slotY && ym < slotY + 18)
							hoveredItem = &in;
					}
				}
			}
		}

		// Render output
		int_t resX = leftPos + 124;
		int_t resY = topPos + 35;
		ItemInstance resCopy = recipe.result;
		itemRenderer.renderGuiItem(font, minecraft.textures, resCopy, resX, resY);
		itemRenderer.renderGuiItemDecorations(font, minecraft.textures, resCopy, resX, resY);
		if (xm >= resX && xm < resX + 18 && ym >= resY && ym < resY + 18)
			hoveredItem = &recipe.result;

		glDisable(GL_RESCALE_NORMAL);
		Lighting::turnOff();
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		VulkanMatrixStack::get().popMatrix();
	}

	Screen::render(xm, ym, a);

	// Tooltip
	if (hoveredItem != nullptr && !hoveredItem->isEmpty())
	{
		auto lines = ContainerScreen::getTooltipLines(*hoveredItem, &minecraft.options);
		lines.push_back(u"\xA77[Click / F4]: View recipe");

		int_t maxW = 0;
		for (const auto &line : lines)
		{
			int_t w = font.width(line);
			if (w > maxW) maxW = w;
		}
		int_t h = static_cast<int_t>(lines.size()) * 10;
		int_t tx = xm + 12;
		int_t ty = ym - 12;
		fillGradient(tx - 3, ty - 3, tx + maxW + 3, ty + h + 1, 0xD0000000, 0xD0000000);
		for (size_t i = 0; i < lines.size(); i++)
		{
			int_t color = (i == 0) ? 0xFFFFFF : ((i == 1 && lines.size() > 2) ? 0x55FF55 : 0xAAAAAA);
			font.drawShadow(lines[i], tx, ty + static_cast<int_t>(i) * 10, color);
		}
	}
}

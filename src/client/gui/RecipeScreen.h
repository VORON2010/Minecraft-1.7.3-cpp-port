#pragma once

#include <vector>
#include <memory>
#include "client/gui/Screen.h"
#include "world/item/ItemInstance.h"

class RecipeScreen : public Screen
{
public:
	struct RecipeEntry
	{
		enum class Type { CRAFTING_SHAPED, CRAFTING_SHAPELESS, SMELTING };
		Type type = Type::CRAFTING_SHAPED;
		int_t width = 3;
		int_t height = 3;
		std::vector<ItemInstance> inputs;
		ItemInstance result;
	};

private:
	std::shared_ptr<Screen> lastScreen;
	int_t targetItemId = -1;
	int_t targetDamage = -1;

	std::vector<RecipeEntry> allRecipes;
	std::vector<size_t> filteredIndices;
	size_t currentRecipeIdx = 0;

	std::vector<ItemInstance> uniqueOutputs;
	int_t browsePage = 0;
	bool browseMode = false;

	int_t imageWidth = 176;
	int_t imageHeight = 166;

public:
	RecipeScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, int_t targetItemId = -1, int_t targetDamage = -1);

	void init() override;
	void render(int_t xm, int_t ym, float a) override;

protected:
	void buttonClicked(Button &button) override;
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;

private:
	void loadRecipes();
	void filterRecipes();
	void setTargetItem(int_t itemId, int_t damage = -1);
};

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "world/item/ItemInstance.h"

class CraftingContainer;

class Recipes
{
public:
	static Recipes &getInstance();

	ItemInstance getItemFor(const CraftingContainer &container) const;

	struct Recipe
	{
		bool shapeless = false;
		int_t width = 0;
		int_t height = 0;
		std::vector<ItemInstance> items;
		ItemInstance result;

		int_t size() const { return shapeless ? static_cast<int_t>(items.size()) : width * height; }
	};

	const std::vector<Recipe> &getRecipes() const { return recipes; }

private:
	std::vector<Recipe> recipes;

	Recipes();
	void addShapedRecipe(const ItemInstance &result, const std::vector<std::string> &pattern,
		const std::vector<std::pair<char, ItemInstance>> &mapping);
	void addShapelessRecipe(const ItemInstance &result, const std::vector<ItemInstance> &ingredients);
};

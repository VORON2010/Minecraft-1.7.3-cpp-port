#pragma once

#include <unordered_map>

#include "world/item/ItemInstance.h"

class FurnaceRecipes
{
private:
	std::unordered_map<int_t, ItemInstance> recipes;

	FurnaceRecipes();

public:
	static FurnaceRecipes &getInstance();

	ItemInstance getResult(const ItemInstance &input) const;
	const std::unordered_map<int_t, ItemInstance> &getRecipes() const { return recipes; }
};

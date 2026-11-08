#include "world/entity/npc/Merchant.h"

#include "nbt/CompoundTag.h"
#include "nbt/ListTag.h"
#include "util/Memory.h"

bool MerchantRecipe::isSame(const MerchantRecipe &other) const
{
	if (buyA.itemID != other.buyA.itemID || sell.itemID != other.sell.itemID)
		return false;
	return (!hasBuyB && !other.hasBuyB) || (hasBuyB && other.hasBuyB && buyB.itemID == other.buyB.itemID);
}

bool MerchantRecipe::isSameSameButBetter(const MerchantRecipe &other) const
{
	return isSame(other) && (buyA.stackSize < other.buyA.stackSize || (hasBuyB && buyB.stackSize < other.buyB.stackSize));
}

void MerchantRecipe::load(CompoundTag &tag)
{
	buyA = ItemInstance(*tag.getCompound(u"buy"));
	sell = ItemInstance(*tag.getCompound(u"sell"));
	if (tag.contains(u"buyB"))
	{
		buyB = ItemInstance(*tag.getCompound(u"buyB"));
		hasBuyB = true;
	}
	if (tag.contains(u"uses"))
		uses = tag.getInt(u"uses");
	maxUses = tag.contains(u"maxUses") ? tag.getInt(u"maxUses") : 7;
}

std::shared_ptr<CompoundTag> MerchantRecipe::createTag() const
{
	auto tag = Util::make_shared<CompoundTag>();
	auto buy = Util::make_shared<CompoundTag>();
	buyA.save(*buy);
	tag->put(u"buy", buy);
	auto sellTag = Util::make_shared<CompoundTag>();
	sell.save(*sellTag);
	tag->put(u"sell", sellTag);
	if (hasBuyB)
	{
		auto b = Util::make_shared<CompoundTag>();
		buyB.save(*b);
		tag->put(u"buyB", b);
	}
	tag->putInt(u"uses", uses);
	tag->putInt(u"maxUses", maxUses);
	return tag;
}

MerchantRecipe *MerchantRecipeList::getRecipeFor(const ItemInstance &buyA, const ItemInstance *buyB, int_t selectionHint)
{
	if (selectionHint > 0 && selectionHint < static_cast<int_t>(recipes.size()))
	{
		MerchantRecipe &r = *recipes[selectionHint];
		if (buyA.itemID != r.getBuyA().itemID)
			return nullptr;
		if ((buyB != nullptr || r.hasSecondCostItem()) && (!r.hasSecondCostItem() || buyB == nullptr || r.getBuyB()->itemID != buyB->itemID))
			return nullptr;
		if (buyA.stackSize < r.getBuyA().stackSize)
			return nullptr;
		if (r.hasSecondCostItem() && buyB->stackSize < r.getBuyB()->stackSize)
			return nullptr;
		return &r;
	}
	for (auto &rp : recipes)
	{
		MerchantRecipe &r = *rp;
		if (buyA.itemID == r.getBuyA().itemID && buyA.stackSize >= r.getBuyA().stackSize
			&& ((!r.hasSecondCostItem() && buyB == nullptr)
				|| (r.hasSecondCostItem() && buyB != nullptr && r.getBuyB()->itemID == buyB->itemID && buyB->stackSize >= r.getBuyB()->stackSize)))
			return &r;
	}
	return nullptr;
}

void MerchantRecipeList::addIfNewOrBetter(std::shared_ptr<MerchantRecipe> recipe)
{
	for (auto &existing : recipes)
	{
		if (recipe->isSame(*existing))
		{
			if (recipe->isSameSameButBetter(*existing))
				existing = recipe;
			return;
		}
	}
	recipes.push_back(std::move(recipe));
}

void MerchantRecipeList::load(CompoundTag &tag)
{
	std::shared_ptr<ListTag> list = tag.getList(u"Recipes");
	if (list == nullptr)
		return;
	for (int_t i = 0; i < list->size(); ++i)
		recipes.push_back(std::make_shared<MerchantRecipe>(*std::static_pointer_cast<CompoundTag>(list->get(i))));
}

std::shared_ptr<CompoundTag> MerchantRecipeList::createTag() const
{
	auto tag = Util::make_shared<CompoundTag>();
	auto list = Util::make_shared<ListTag>();
	for (const auto &r : recipes)
		list->add(r->createTag());
	tag->put(u"Recipes", list);
	return tag;
}

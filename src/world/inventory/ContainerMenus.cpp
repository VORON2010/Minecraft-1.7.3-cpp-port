#include "world/inventory/ContainerMenus.h"
#include "world/item/PotionItem.h"
#include "world/item/enchantment/Enchantment.h"
#include "world/level/tile/EnchantmentTableTile.h"
#include "world/level/tile/BookshelfTile.h"
#include "world/stats/Achievement.h"
#include "world/item/Item.h"
#include "world/stats/AchievementList.h"
#include "world/level/tile/entity/BrewingStandTileEntity.h"
#include "world/item/Items.h"

#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/IInventory.h"
#include "world/inventory/Slot.h"
#include "world/item/crafting/Recipes.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/WorkbenchTile.h"
#include "world/level/tile/entity/FurnaceTileEntity.h"

ContainerPlayer::ContainerPlayer(InventoryPlayer &inventory)
	: ContainerPlayer(inventory, true)
{
}

ContainerPlayer::ContainerPlayer(InventoryPlayer &inventory, bool singlePlayer)
	: craftMatrix(*this, 2, 2), isSinglePlayer(singlePlayer)
{
	Player &player = inventory.getPlayer();
	addSlot(std::make_unique<CraftingSlot>(player, craftMatrix, craftResult, 0, 144, 36));
	for (int_t row = 0; row < 2; ++row)
		for (int_t column = 0; column < 2; ++column)
			addSlot(std::make_unique<Slot>(craftMatrix, column + row * 2,
				88 + column * 18, 26 + row * 18));
	for (int_t armor = 0; armor < 4; ++armor)
		addSlot(std::make_unique<ArmorSlot>(inventory, inventory.getContainerSize() - 1 - armor,
			8, 8 + armor * 18, armor));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + (row + 1) * 9,
				8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
	onCraftMatrixChanged(craftMatrix);
}

void ContainerPlayer::onCraftMatrixChanged(IInventory &inventory)
{
	(void)inventory;
	craftResult.setInventorySlotContents(0, Recipes::getInstance().getItemFor(craftMatrix));
}

void ContainerPlayer::onClosed(Player &player)
{
	Container::onClosed(player);
	for (int_t slot = 0; slot < 4; ++slot)
	{
		ItemInstance *item = craftMatrix.getStackInSlot(slot);
		if (item != nullptr)
		{
			ItemInstance dropped = *item;
			player.drop(dropped);
			craftMatrix.setInventorySlotContents(slot, ItemInstance());
		}
	}
}

bool ContainerPlayer::canUse(Player &player) const
{
	(void)player;
	return true;
}

std::unique_ptr<ItemInstance> ContainerPlayer::quickMoveStack(int_t slot)
{
	if (slot == 0)
		return quickMoveStackTo(slot, 9, 45, true);
	if (slot >= 9 && slot < 36)
		return quickMoveStackTo(slot, 36, 45, false);
	if (slot >= 36 && slot < 45)
		return quickMoveStackTo(slot, 9, 36, false);
	return quickMoveStackTo(slot, 9, 45, false);
}

ContainerChest::ContainerChest(InventoryPlayer &inventory, std::shared_ptr<IInventory> chest)
	: chest(std::move(chest)), rows(this->chest->getSizeInventory() / 9)
{
	int_t offset = (rows - 4) * 18;
	for (int_t row = 0; row < rows; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(*this->chest, column + row * 9,
				8 + column * 18, 18 + row * 18));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9,
				8 + column * 18, 103 + row * 18 + offset));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 161 + offset));
}

bool ContainerChest::canUse(Player &player) const
{
	return chest->canInteractWith(player);
}

std::unique_ptr<ItemInstance> ContainerChest::quickMoveStack(int_t slotNumber)
{
	Slot &slot = getSlot(slotNumber);
	ItemInstance *slotItem = slot.getItem();
	if (slotItem == nullptr)
		return nullptr;
	ItemInstance original = *slotItem;
	if (slotNumber < rows * 9)
		moveItemStackTo(*slotItem, rows * 9, static_cast<int_t>(slots.size()), true);
	else
		moveItemStackTo(*slotItem, 0, rows * 9, false);
	if (slotItem->stackSize == 0)
		slot.set(nullptr);
	else
		slot.setChanged();
	return std::make_unique<ItemInstance>(original);
}

ContainerFurnace::ContainerFurnace(InventoryPlayer &inventory, std::shared_ptr<FurnaceTileEntity> furnace)
	: furnace(std::move(furnace))
{
	Player &player = inventory.getPlayer();
	addSlot(std::make_unique<Slot>(*this->furnace, 0, 56, 17));
	addSlot(std::make_unique<Slot>(*this->furnace, 1, 56, 53));
	addSlot(std::make_unique<FurnaceResultSlot>(player, *this->furnace, 2, 116, 35));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9,
				8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
}

void ContainerFurnace::updateProgressBar(int_t id, int_t value)
{
	if (id == 0)
		furnace->cookTime = value;
	if (id == 1)
		furnace->burnTime = value;
	if (id == 2)
		furnace->currentItemBurnTime = value;
}

bool ContainerFurnace::canUse(Player &player) const
{
	return furnace->canUse(player);
}

std::unique_ptr<ItemInstance> ContainerFurnace::quickMoveStack(int_t slot)
{
	if (slot == 2)
		return quickMoveStackTo(slot, 3, 39, true);
	if (slot >= 3 && slot < 30)
		return quickMoveStackTo(slot, 30, 39, false);
	if (slot >= 30 && slot < 39)
		return quickMoveStackTo(slot, 3, 30, false);
	return quickMoveStackTo(slot, 3, 39, false);
}

ContainerDispenser::ContainerDispenser(InventoryPlayer &inventory, std::shared_ptr<IInventory> dispenser)
	: dispenser(std::move(dispenser))
{
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 3; ++column)
			addSlot(std::make_unique<Slot>(*this->dispenser, column + row * 3,
				62 + column * 18, 17 + row * 18));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9,
				8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
}

bool ContainerDispenser::canUse(Player &player) const
{
	return dispenser->canInteractWith(player);
}

ContainerWorkbench::ContainerWorkbench(InventoryPlayer &inventory, Level &level,
	int_t x, int_t y, int_t z)
	: level(level), x(x), y(y), z(z), craftMatrix(*this, 3, 3)
{
	Player &player = inventory.getPlayer();
	addSlot(std::make_unique<CraftingSlot>(player, craftMatrix, craftResult, 0, 124, 35));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 3; ++column)
			addSlot(std::make_unique<Slot>(craftMatrix, column + row * 3,
				30 + column * 18, 17 + row * 18));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9,
				8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
	onCraftMatrixChanged(craftMatrix);
}

void ContainerWorkbench::onCraftMatrixChanged(IInventory &inventory)
{
	(void)inventory;
	craftResult.setInventorySlotContents(0, Recipes::getInstance().getItemFor(craftMatrix));
}

void ContainerWorkbench::onClosed(Player &player)
{
	Container::onClosed(player);
	if (!level.isOnline)
	{
		for (int_t slot = 0; slot < 9; ++slot)
		{
			ItemInstance *item = craftMatrix.getStackInSlot(slot);
			if (item != nullptr)
			{
				ItemInstance dropped = *item;
				player.drop(dropped);
			}
		}
	}
}

bool ContainerWorkbench::canUse(Player &player) const
{
	return level.getTile(x, y, z) == Tile::workBench.id
		&& player.distanceToSqr(static_cast<double>(x) + 0.5,
			static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5) <= 64.0;
}

std::unique_ptr<ItemInstance> ContainerWorkbench::quickMoveStack(int_t slot)
{
	if (slot == 0)
		return quickMoveStackTo(slot, 10, 46, true);
	if (slot >= 10 && slot < 37)
		return quickMoveStackTo(slot, 37, 46, false);
	if (slot >= 37 && slot < 46)
		return quickMoveStackTo(slot, 10, 37, false);
	return quickMoveStackTo(slot, 10, 46, false);
}

namespace
{
	class PotionSlot : public Slot
	{
		Player &player;

	public:
		PotionSlot(Player &player, IInventory &inventory, int_t slot, int_t x, int_t y) : Slot(inventory, slot, x, y), player(player) {}
		bool mayPlace(const ItemInstance &item) const override
		{
			return item.itemID == Items::potion->getShiftedIndex() || item.itemID == Items::glassBottle->getShiftedIndex();
		}
		int_t getMaxStackSize() const override { return 1; }
		void onPickup(ItemInstance &item) override
		{
			if (item.itemID == Items::potion->getShiftedIndex() && item.itemDamage > 0)
				player.addStat(*AchievementList::potion, 1);
			Slot::onPickup(item);
		}
	};

	class IngredientSlot : public Slot
	{
	public:
		using Slot::Slot;
		bool mayPlace(const ItemInstance &item) const override
		{
			Item *it = item.getItem();
			return it != nullptr && it->hasPotionBrewingFormula();
		}
		int_t getMaxStackSize() const override { return 64; }
	};
}

ContainerBrewingStand::ContainerBrewingStand(InventoryPlayer &inventory, std::shared_ptr<BrewingStandTileEntity> stand)
	: stand(std::move(stand))
{
	Player &player = inventory.getPlayer();
	addSlot(std::make_unique<PotionSlot>(player, *this->stand, 0, 56, 46));
	addSlot(std::make_unique<PotionSlot>(player, *this->stand, 1, 79, 53));
	addSlot(std::make_unique<PotionSlot>(player, *this->stand, 2, 102, 46));
	addSlot(std::make_unique<IngredientSlot>(*this->stand, 3, 79, 17));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
}

void ContainerBrewingStand::updateProgressBar(int_t id, int_t value)
{
	if (id == 0)
		stand->setBrewTime(value);
}

bool ContainerBrewingStand::canUse(Player &player) const
{
	return stand->canInteractWith(player);
}

std::unique_ptr<ItemInstance> ContainerBrewingStand::quickMoveStack(int_t slot)
{
	if ((slot < 0 || slot > 2) && slot != 3)
	{
		if (slot >= 4 && slot < 31)
			return quickMoveStackTo(slot, 31, 40, false);
		if (slot >= 31 && slot < 40)
			return quickMoveStackTo(slot, 4, 31, false);
		return quickMoveStackTo(slot, 4, 40, false);
	}
	return quickMoveStackTo(slot, 4, 40, true);
}

ItemInstance EnchantInventory::decrStackSize(int_t slot, int_t count)
{
	if (slot != 0 || item.isEmpty())
		return ItemInstance();
	ItemInstance taken = item.remove(count);
	taken.enchantments = item.enchantments;
	if (item.stackSize <= 0)
		item = ItemInstance();
	onInventoryChanged();
	return taken;
}

void EnchantInventory::setInventorySlotContents(int_t slot, const ItemInstance &stack)
{
	if (slot != 0)
		return;
	item = stack;
	onInventoryChanged();
}

void EnchantInventory::onInventoryChanged()
{
	menu.slotsChanged();
}

namespace
{
	class EnchantSlot : public Slot
	{
	public:
		using Slot::Slot;
		bool mayPlace(const ItemInstance &item) const override { (void)item; return true; }
	};
}

ContainerEnchantment::ContainerEnchantment(InventoryPlayer &inventory, Level &level, int_t x, int_t y, int_t z)
	: level(level), x(x), y(y), z(z), enchantSlots(*this)
{
	addSlot(std::make_unique<EnchantSlot>(enchantSlots, 0, 25, 47));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
}

int_t ContainerEnchantment::countBookshelves() const
{
	int_t shelves = 0;
	int_t book = Tile::bookshelf.id;
	for (int_t dz = -1; dz <= 1; ++dz)
	{
		for (int_t dx = -1; dx <= 1; ++dx)
		{
			if ((dz == 0 && dx == 0) || !level.isEmptyTile(x + dx, y, z + dz) || !level.isEmptyTile(x + dx, y + 1, z + dz))
				continue;
			if (level.getTile(x + dx * 2, y, z + dz * 2) == book)
				shelves++;
			if (level.getTile(x + dx * 2, y + 1, z + dz * 2) == book)
				shelves++;
			if (dx != 0 && dz != 0)
			{
				if (level.getTile(x + dx * 2, y, z + dz) == book)
					shelves++;
				if (level.getTile(x + dx * 2, y + 1, z + dz) == book)
					shelves++;
				if (level.getTile(x + dx, y, z + dz * 2) == book)
					shelves++;
				if (level.getTile(x + dx, y + 1, z + dz * 2) == book)
					shelves++;
			}
		}
	}
	return shelves;
}

void ContainerEnchantment::slotsChanged()
{
	const ItemInstance *item = enchantSlots.getStackInSlot(0);
	if (item != nullptr && item->isEnchantable())
	{
		nameSeed = random.nextLong();
		if (!level.isOnline)
		{
			int_t shelves = countBookshelves();
			for (int_t i = 0; i < 3; ++i)
				costs[i] = EnchantmentHelper::getEnchantmentCost(random, i, shelves, *item);
		}
	}
	else
	{
		for (int_t i = 0; i < 3; ++i)
			costs[i] = 0;
	}
}

bool ContainerEnchantment::clickMenuButton(Player &player, int_t id)
{
	ItemInstance *item = enchantSlots.getStackInSlot(0);
	if (id < 0 || id > 2 || costs[id] <= 0 || item == nullptr || player.experienceLevel < costs[id])
		return false;
	if (!level.isOnline)
	{
		std::vector<EnchantmentHelper::Entry> chosen = EnchantmentHelper::selectEnchantment(random, *item, costs[id]);
		if (!chosen.empty())
		{
			// 1.0 PlayerEntity.addXp(levels) takes the levels away
			player.experienceLevel -= costs[id];
			if (player.experienceLevel < 0)
				player.experienceLevel = 0;
			// 1.4.7: a book becomes an enchanted book with one of the rolled enchantments
			bool book = item->itemID == Items::book->getShiftedIndex();
			int_t keep = book ? random.nextInt(static_cast<int_t>(chosen.size())) : -1;
			if (book)
				item->itemID = Items::enchantedBook->getShiftedIndex();
			for (int_t i = 0; i < static_cast<int_t>(chosen.size()); ++i)
			{
				if (book && i != keep)
					continue;
				if (book)
					EnchantedBookItem::addEnchantment(*item, chosen[i].enchantment->id, chosen[i].level);
				else
					item->enchant(chosen[i].enchantment->id, chosen[i].level);
			}
			slotsChanged();
		}
	}
	return true;
}

void ContainerEnchantment::onClosed(Player &player)
{
	Container::onClosed(player);
	if (!level.isOnline)
	{
		ItemInstance *item = enchantSlots.getStackInSlot(0);
		if (item != nullptr)
		{
			ItemInstance dropped = *item;
			enchantSlots.setInventorySlotContents(0, ItemInstance());
			player.drop(dropped);
		}
	}
}

bool ContainerEnchantment::canUse(Player &player) const
{
	if (level.getTile(x, y, z) != Tile::enchantTable.id)
		return false;
	return player.distanceToSqr(x + 0.5, y + 0.5, z + 0.5) <= 64.0;
}

std::unique_ptr<ItemInstance> ContainerEnchantment::quickMoveStack(int_t slot)
{
	if (slot != 0)
		return nullptr;
	return quickMoveStackTo(slot, 1, 37, true);
}

void ContainerEnchantment::updateProgressBar(int_t id, int_t value)
{
	if (id >= 0 && id <= 2)
		costs[id] = value;
}

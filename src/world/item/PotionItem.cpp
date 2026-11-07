#include "world/item/PotionItem.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/DeadBushTile.h"
#include "world/level/tile/TallGrassTile.h"
#include "world/level/tile/VineTile.h"

#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/entity/projectile/ThrownPotion.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/item/alchemy/PotionBrewing.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/Tile.h"
#include "world/phys/HitResult.h"
#include "world/phys/Vec3.h"
#include "world/stats/StatCollector.h"

namespace
{
	jstring trimmed(const jstring &s)
	{
		size_t a = 0, b = s.size();
		while (a < b && s[a] <= u' ')
			++a;
		while (b > a && s[b - 1] <= u' ')
			--b;
		return s.substr(a, b - a);
	}

	jstring tr(const jstring &key)
	{
		return trimmed(StatCollector::translate(key));
	}
}

PotionItem::PotionItem(int_t baseId) : Item(baseId)
{
	setMaxStackSize(1);
	setHasSubtypes(true);
	setMaxDamage(0);
}

const std::vector<MobEffectInstance> &PotionItem::getMobEffects(int_t data) const
{
	auto it = cache.find(data);
	if (it == cache.end())
		it = cache.emplace(data, PotionBrewing::getEffects(data, false)).first;
	return it->second;
}

bool PotionItem::hasInstantenousEffects(int_t data) const
{
	for (const MobEffectInstance &e : getMobEffects(data))
		if (MobEffect::effects[e.getId()]->isInstantenous())
			return true;
	return false;
}

void PotionItem::useTimeDepleted(ItemInstance &stack, Level &level, Player &player) const
{
	stack.stackSize--;
	if (!level.isOnline)
	{
		for (const MobEffectInstance &e : getMobEffects(stack.itemDamage))
			player.addEffect(e);
	}
	if (stack.stackSize <= 0)
	{
		stack = ItemInstance(Items::glassBottle->getShiftedIndex(), 1, 0);
	}
	else
	{
		ItemInstance bottle(Items::glassBottle->getShiftedIndex(), 1, 0);
		player.inventory.add(bottle);
	}
}

void PotionItem::use(ItemInstance &stack, Level &level, Player &player) const
{
	if (isThrowable(stack.itemDamage))
	{
		stack.stackSize--;
		level.playSoundAtEntity(player, u"random.bow", 0.5f, 0.4f / (itemRandom.nextFloat() * 0.4f + 0.8f));
		if (!level.isOnline)
			level.addEntity(std::make_shared<ThrownPotion>(level, player, stack.itemDamage));
		return;
	}
	player.startUsingItem(stack, getUseDuration(stack));
}

bool PotionItem::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)stack; (void)player; (void)level; (void)x; (void)y; (void)z; (void)face;
	return false;
}

int_t PotionItem::getIcon(const ItemInstance &stack) const
{
	return isThrowable(stack.itemDamage) ? 154 : 140;
}

int_t PotionItem::getColor(int_t data) const
{
	return PotionBrewing::getColorValue(data, false);
}

jstring PotionItem::getHoverName(const ItemInstance &stack) const
{
	if (stack.itemDamage == 0)
		return tr(u"item.emptyPotion.name");
	jstring name = tr(getDescriptionId(stack) + u".name");
	if (isThrowable(stack.itemDamage))
		name = tr(u"potion.prefix.grenade") + u" " + name;
	const std::vector<MobEffectInstance> &effects = getMobEffects(stack.itemDamage);
	if (!effects.empty())
		return name + u" " + tr(MobEffect::effects[effects[0].getId()]->getDescriptionId() + u".postfix");
	return tr(PotionBrewing::getAppearanceName(stack.itemDamage)) + u" " + name;
}

void PotionItem::appendHoverText(const ItemInstance &stack, std::vector<jstring> &lines) const
{
	if (stack.itemDamage == 0)
		return;
	const std::vector<MobEffectInstance> &effects = getMobEffects(stack.itemDamage);
	if (effects.empty())
	{
		lines.push_back(u"§7" + tr(u"potion.empty"));
		return;
	}
	for (const MobEffectInstance &e : effects)
	{
		const MobEffect *effect = MobEffect::effects[e.getId()];
		jstring line = tr(effect->getDescriptionId());
		if (e.getAmplifier() > 0)
			line += u" " + tr(u"potion.potency." + String::toString(e.getAmplifier()));
		if (e.getDuration() > 20)
			line += u" (" + MobEffectInstance::formatDuration(e) + u")";
		lines.push_back((effect->isHarmful() ? u"§c" : u"§7") + line);
	}
}

bool PotionItem::isFoil(const ItemInstance &stack) const
{
	return !getMobEffects(stack.itemDamage).empty();
}

void BottleItem::use(ItemInstance &stack, Level &level, Player &player) const
{
	float xRot = player.xRot;
	float yRot = player.yRot;
	double px = player.x;
	double py = player.y + 1.62 - player.heightOffset;
	double pz = player.z;
	Vec3 *pos = Vec3::newTemp(px, py, pz);
	float c = Mth::cos(-yRot * 0.017453292f - Mth::PI);
	float s = Mth::sin(-yRot * 0.017453292f - Mth::PI);
	float c2 = -Mth::cos(-xRot * 0.017453292f);
	float s2 = Mth::sin(-xRot * 0.017453292f);
	Vec3 *to = pos->add(s * c2 * 5.0, s2 * 5.0, c * c2 * 5.0);
	HitResult hit = level.clip(*pos, *to, true);
	if (hit.type != HitResult::Type::TILE)
		return;
	const Material &mat = level.getMaterial(hit.x, hit.y, hit.z);
	if (&mat != static_cast<const Material *>(&Material::water))
		return;
	stack.stackSize--;
	if (stack.stackSize <= 0)
	{
		stack = ItemInstance(Items::potion->getShiftedIndex(), 1, 0);
		return;
	}
	ItemInstance water(Items::potion->getShiftedIndex(), 1, 0);
	if (!player.inventory.add(water))
		player.drop(water);
}

bool TilePlanterItem::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	// 1.0 PlaceableItem.useOn
	int_t clicked = level.getTile(x, y, z);
	if (clicked == Tile::snow.id)
		face = Facing::UP;
	else if (clicked != Tile::vine.id && clicked != Tile::tallGrass.id && clicked != Tile::deadBush.id)
	{
		if (face == Facing::DOWN) y--;
		if (face == Facing::UP) y++;
		if (face == Facing::NORTH) z--;
		if (face == Facing::SOUTH) z++;
		if (face == Facing::WEST) x--;
		if (face == Facing::EAST) x++;
	}
	if (stack.stackSize == 0)
		return false;
	Tile *tile = Tile::tiles[tileId];
	if (y >= 127 && tile->material.isSolid())
		return false;
	if (!level.mayPlace(tileId, x, y, z, false, face))
		return false;
	if (!level.setTile(x, y, z, tileId))
		return false;
	if (level.getTile(x, y, z) == tileId)
	{
		tile->setPlacedOnFace(level, x, y, z, face);
		tile->setPlacedBy(level, x, y, z, player);
	}
	if (tile->soundType != nullptr)
		level.playSoundEffect(x + 0.5, y + 0.5, z + 0.5, tile->soundType->getStepResourcePath(), (tile->soundType->getVolume() + 1.0f) / 2.0f, tile->soundType->getPitch() * 0.8f);
	stack.stackSize--;
	return true;
}

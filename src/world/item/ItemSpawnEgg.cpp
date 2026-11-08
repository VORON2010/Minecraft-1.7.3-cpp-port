#include "world/item/ItemSpawnEgg.h"

#include "client/locale/Language.h"
#include "world/entity/EntityIO.h"
#include "world/entity/Mob.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "util/Mth.h"

ItemSpawnEgg::ItemSpawnEgg(int_t baseId) : Item(baseId)
{
	setHasSubtypes(true);
}

jstring ItemSpawnEgg::getHoverName(const ItemInstance &stack) const
{
	Language &language = Language::getInstance();
	jstring name = language.getElement(getDescriptionId(stack) + u".name");
	jstring key = EntityIO::getEncodeId(stack.getAuxValue());
	if (!key.empty())
		name += u" " + language.getElement(u"entity." + key + u".name");
	return name;
}

int_t ItemSpawnEgg::getLayerIcon(const ItemInstance &stack, int_t layer) const
{
	return layer > 0 ? 169 : getIcon(stack);
}

int_t ItemSpawnEgg::getLayerColor(const ItemInstance &stack, int_t layer) const
{
	const EntityIO::SpawnEggData *data = EntityIO::getSpawnEgg(stack.getAuxValue());
	if (data == nullptr)
		return 0xFFFFFF;
	return layer == 0 ? data->baseColor : data->spotsColor;
}

bool ItemSpawnEgg::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	if (level.isOnline)
		return true;

	int_t tile = level.getTile(x, y, z);
	if (face == Facing::DOWN) y--;
	if (face == Facing::UP) y++;
	if (face == Facing::NORTH) z--;
	if (face == Facing::SOUTH) z++;
	if (face == Facing::WEST) x--;
	if (face == Facing::EAST) x++;

	double yOff = 0.0;
	// fences (render shape 11) put the mob on top
	if (face == Facing::UP && Tile::tiles[tile] != nullptr && Tile::tiles[tile]->getRenderShape() == 11)
		yOff = 0.5;

	std::shared_ptr<Entity> entity = spawnEntity(level, stack.getAuxValue(), x + 0.5, y + yOff, z + 0.5);
	if (entity != nullptr && !player.abilities.instabuild)
		stack.stackSize--;
	return true;
}

std::shared_ptr<Entity> ItemSpawnEgg::spawnEntity(Level &level, int_t id, double x, double y, double z)
{
	if (EntityIO::getSpawnEgg(id) == nullptr)
		return nullptr;
	std::shared_ptr<Entity> entity = EntityIO::newEntity(id, level);
	Mob *mob = dynamic_cast<Mob *>(entity.get());
	if (mob != nullptr)
	{
		entity->moveTo(x, y, z, Mth::wrapDegrees(level.random.nextFloat() * 360.0f), 0.0f);
		mob->yHeadRot = mob->yRot;
		mob->yBodyRot = mob->yRot;
		mob->setEntityVariety();
		level.addEntity(entity);
		mob->playAmbientSound();
	}
	return entity;
}

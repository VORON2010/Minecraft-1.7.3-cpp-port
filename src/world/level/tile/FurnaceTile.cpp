#include "world/level/tile/FurnaceTile.h"

#include "client/player/LocalPlayer.h"
#include "util/Mth.h"
#include "util/Memory.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/entity/FurnaceTileEntity.h"

namespace
{
	constexpr int_t SIDE_TEXTURE = 45;
	constexpr int_t FRONT_IDLE_TEXTURE = 44;
	constexpr int_t FRONT_LIT_TEXTURE = 61;
	constexpr int_t TOP_BOTTOM_TEXTURE = 62;
}

bool FurnaceTile::keepContents = false;

FurnaceTile::FurnaceTile(int_t id, bool lit) : Tile(id, SIDE_TEXTURE, Material::stone), lit(lit)
{
	Tile::isEntityTile[id] = true;
}

int_t FurnaceTile::getTexture(Facing face, int_t data)
{
	if (face == Facing::UP || face == Facing::DOWN)
		return TOP_BOTTOM_TEXTURE;
	if (data < static_cast<int_t>(Facing::NORTH) || data > static_cast<int_t>(Facing::EAST))
		data = static_cast<int_t>(Facing::WEST);
	return data == static_cast<int_t>(face) ? (lit ? FRONT_LIT_TEXTURE : FRONT_IDLE_TEXTURE) : SIDE_TEXTURE;
}

int_t FurnaceTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Tile::furnace.id;
}

void FurnaceTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (!lit)
		return;

	int_t data = level.getData(x, y, z);
	float centerX = static_cast<float>(x) + 0.5f;
	float centerY = static_cast<float>(y) + random.nextFloat() * 6.0f / 16.0f;
	float centerZ = static_cast<float>(z) + 0.5f;
	float offset = 0.52f;
	float randomOffset = random.nextFloat() * 0.6f - 0.3f;

	if (data == static_cast<int_t>(Facing::WEST))
	{
		level.addParticle(u"smoke", centerX - offset, centerY, centerZ + randomOffset, 0.0, 0.0, 0.0);
		level.addParticle(u"flame", centerX - offset, centerY, centerZ + randomOffset, 0.0, 0.0, 0.0);
	}
	else if (data == static_cast<int_t>(Facing::EAST))
	{
		level.addParticle(u"smoke", centerX + offset, centerY, centerZ + randomOffset, 0.0, 0.0, 0.0);
		level.addParticle(u"flame", centerX + offset, centerY, centerZ + randomOffset, 0.0, 0.0, 0.0);
	}
	else if (data == static_cast<int_t>(Facing::NORTH))
	{
		level.addParticle(u"smoke", centerX + randomOffset, centerY, centerZ - offset, 0.0, 0.0, 0.0);
		level.addParticle(u"flame", centerX + randomOffset, centerY, centerZ - offset, 0.0, 0.0, 0.0);
	}
	else if (data == static_cast<int_t>(Facing::SOUTH))
	{
		level.addParticle(u"smoke", centerX + randomOffset, centerY, centerZ + offset, 0.0, 0.0, 0.0);
		level.addParticle(u"flame", centerX + randomOffset, centerY, centerZ + offset, 0.0, 0.0, 0.0);
	}
}

void FurnaceTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	level.setTileEntity(x, y, z, Util::make_shared<FurnaceTileEntity>());
	setDefaultDirection(level, x, y, z);
}

void FurnaceTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	if (!keepContents)
		dropContents(level, x, y, z);
	level.removeTileEntity(x, y, z);
}

bool FurnaceTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	if (level.isOnline)
		return true;
	LocalPlayer *localPlayer = dynamic_cast<LocalPlayer *>(&player);
	if (localPlayer == nullptr)
		return false;
	auto tileEntity = std::dynamic_pointer_cast<FurnaceTileEntity>(level.getTileEntity(x, y, z));
	if (tileEntity == nullptr)
		return false;
	localPlayer->startFurnace(tileEntity);
	return true;
}

void FurnaceTile::setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	int_t rotation = Mth::floor(static_cast<double>(player.yRot) * 4.0 / 360.0 + 0.5) & 3;
	if (rotation == 0)
		level.setData(x, y, z, static_cast<int_t>(Facing::NORTH));
	else if (rotation == 1)
		level.setData(x, y, z, static_cast<int_t>(Facing::EAST));
	else if (rotation == 2)
		level.setData(x, y, z, static_cast<int_t>(Facing::SOUTH));
	else if (rotation == 3)
		level.setData(x, y, z, static_cast<int_t>(Facing::WEST));
}

void FurnaceTile::setLitState(bool lit, Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	auto tileEntity = level.getTileEntity(x, y, z);
	keepContents = true;
	level.setTile(x, y, z, lit ? Tile::furnaceLit.id : Tile::furnace.id);
	keepContents = false;
	level.setData(x, y, z, data);
	tileEntity->clearRemoved();
	level.setTileEntity(x, y, z, tileEntity);
}

void FurnaceTile::setDefaultDirection(Level &level, int_t x, int_t y, int_t z) const
{
	if (level.isOnline)
		return;

	int_t north = level.getTile(x, y, z - 1);
	int_t south = level.getTile(x, y, z + 1);
	int_t west = level.getTile(x - 1, y, z);
	int_t east = level.getTile(x + 1, y, z);
	int_t data = static_cast<int_t>(Facing::SOUTH);
	if (Tile::solid[north] && !Tile::solid[south])
		data = static_cast<int_t>(Facing::SOUTH);
	if (Tile::solid[south] && !Tile::solid[north])
		data = static_cast<int_t>(Facing::NORTH);
	if (Tile::solid[west] && !Tile::solid[east])
		data = static_cast<int_t>(Facing::EAST);
	if (Tile::solid[east] && !Tile::solid[west])
		data = static_cast<int_t>(Facing::WEST);
	level.setData(x, y, z, data);
}

void FurnaceTile::dropContents(Level &level, int_t x, int_t y, int_t z) const
{
	auto furnace = std::dynamic_pointer_cast<FurnaceTileEntity>(level.getTileEntity(x, y, z));
	if (furnace == nullptr)
		return;

	for (int_t slot = 0; slot < 3; ++slot)
	{
		ItemInstance &stack = furnace->getItem(slot);
		if (stack.isEmpty())
			continue;

		float xo = level.random.nextFloat() * 0.8f + 0.1f;
		float yo = level.random.nextFloat() * 0.8f + 0.1f;
		float zo = level.random.nextFloat() * 0.8f + 0.1f;
		while (stack.stackSize > 0)
		{
			int_t amount = level.random.nextInt(21) + 10;
			if (amount > stack.stackSize)
				amount = stack.stackSize;
			stack.stackSize -= amount;
			ItemInstance dropped(stack.itemID, amount, stack.itemDamage);
			auto entity = std::make_shared<EntityItem>(level, x + xo, y + yo, z + zo, dropped);
			float velocity = 0.05f;
			entity->xd = (level.random.nextFloat() - level.random.nextFloat()) * velocity;
			entity->yd = (level.random.nextFloat() - level.random.nextFloat()) * velocity + 0.2f;
			entity->zd = (level.random.nextFloat() - level.random.nextFloat()) * velocity;
			level.addEntity(entity);
		}
		stack = ItemInstance();
	}
}

#include "world/item/ItemEnderEye.h"

#include "util/Mth.h"
#include "world/entity/EyeOfEnderSignal.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/EndPortalFrameTile.h"
#include "world/level/tile/EndPortalTile.h"
#include "world/phys/HitResult.h"
#include "world/phys/Vec3.h"

namespace
{
	// 1.0 Facings
	const int_t CLOCKWISE[] = {1, 2, 3, 0};
	const int_t X_OFFSET[] = {0, -1, 0, 1};
	const int_t Z_OFFSET[] = {1, 0, -1, 0};
}

ItemEnderEye::ItemEnderEye(int_t id) : Item(id)
{
}

bool ItemEnderEye::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	(void)face;
	int_t tile = level.getTile(x, y, z);
	int_t data = level.getData(x, y, z);
	if (tile != Tile::endPortalFrame.id || EndPortalFrameTile::hasEye(data))
		return false;
	if (level.isOnline)
		return true;

	level.setData(x, y, z, data + 4);
	// 1.5.2 has no sound here; the frame's stone placing sound so the click is heard
	level.playSoundEffect(x + 0.5, y + 0.5, z + 0.5, u"dig.stone", 1.0f, 0.8f + level.random.nextFloat() * 0.2f);
	stack.stackSize--;
	for (int_t i = 0; i < 16; i++)
	{
		double px = x + (5.0f + itemRandom.nextFloat() * 6.0f) / 16.0f;
		double py = y + 0.8125f;
		double pz = z + (5.0f + itemRandom.nextFloat() * 6.0f) / 16.0f;
		level.addParticle(u"smoke", px, py, pz, 0.0, 0.0, 0.0);
	}

	int_t dir = data & 3;
	int_t first = 0;
	int_t last = 0;
	bool foundFirst = false;
	bool complete = true;
	int_t side = CLOCKWISE[dir];
	for (int_t i = -2; i <= 2; i++)
	{
		int_t fx = x + X_OFFSET[side] * i;
		int_t fz = z + Z_OFFSET[side] * i;
		if (level.getTile(fx, y, fz) == Tile::endPortalFrame.id)
		{
			if (!EndPortalFrameTile::hasEye(level.getData(fx, y, fz)))
			{
				complete = false;
				break;
			}
			if (!foundFirst)
			{
				first = i;
				last = i;
				foundFirst = true;
			}
			else
			{
				last = i;
			}
		}
	}

	if (complete && last == first + 2)
	{
		for (int_t i = first; i <= last; i++)
		{
			int_t fx = x + X_OFFSET[side] * i + X_OFFSET[dir] * 4;
			int_t fz = z + Z_OFFSET[side] * i + Z_OFFSET[dir] * 4;
			if (level.getTile(fx, y, fz) != Tile::endPortalFrame.id || !EndPortalFrameTile::hasEye(level.getData(fx, y, fz)))
			{
				complete = false;
				break;
			}
		}
		for (int_t i = first - 1; i <= last + 1; i += 4)
		{
			for (int_t j = 1; j <= 3; j++)
			{
				int_t fx = x + X_OFFSET[side] * i + X_OFFSET[dir] * j;
				int_t fz = z + Z_OFFSET[side] * i + Z_OFFSET[dir] * j;
				if (level.getTile(fx, y, fz) != Tile::endPortalFrame.id || !EndPortalFrameTile::hasEye(level.getData(fx, y, fz)))
				{
					complete = false;
					break;
				}
			}
		}
		if (complete)
		{
			for (int_t i = first; i <= last; i++)
			{
				for (int_t j = 1; j <= 3; j++)
				{
					int_t fx = x + X_OFFSET[side] * i + X_OFFSET[dir] * j;
					int_t fz = z + Z_OFFSET[side] * i + Z_OFFSET[dir] * j;
					level.setTile(fx, y, fz, Tile::endPortal.id);
				}
			}
		}
	}
	return true;
}

void ItemEnderEye::use(ItemInstance &stack, Level &level, Player &player) const
{
	// Item.getUseTarget: 5 block ray from the eyes; aiming at a frame is left to useOn
	float xRot = player.xRotO + (player.xRot - player.xRotO);
	float yRot = player.yRotO + (player.yRot - player.yRotO);
	double px = player.x;
	double py = player.y + 1.62 - player.heightOffset;
	double pz = player.z;
	Vec3 *from = Vec3::newTemp(px, py, pz);
	float c = Mth::cos(-yRot * 0.017453292f - Mth::PI);
	float s = Mth::sin(-yRot * 0.017453292f - Mth::PI);
	float c2 = -Mth::cos(-xRot * 0.017453292f);
	float s2 = Mth::sin(-xRot * 0.017453292f);
	Vec3 *to = from->add(s * c2 * 5.0, s2 * 5.0, c * c2 * 5.0);
	HitResult hit = level.clip(*from, *to, false);
	if (hit.type == HitResult::Type::TILE && level.getTile(hit.x, hit.y, hit.z) == Tile::endPortalFrame.id)
		return;

	if (level.isOnline)
		return;
	int_t sx = 0, sy = 0, sz = 0;
	if (!level.findNearestStronghold(static_cast<int_t>(player.x), static_cast<int_t>(player.y), static_cast<int_t>(player.z), sx, sy, sz))
		return;
	auto eye = std::make_shared<EyeOfEnderSignal>(level, player.x, player.y + 1.62 - player.heightOffset, player.z);
	eye->signalTo(sx, sy, sz);
	level.addEntity(eye);
	level.playSoundAtEntity(player, u"random.bow", 0.5f, 0.4f / (itemRandom.nextFloat() * 0.4f + 0.8f));
	level.levelEvent(1002, static_cast<int_t>(player.x), static_cast<int_t>(player.y), static_cast<int_t>(player.z), 0);
	if (!player.abilities.instabuild)
		stack.stackSize--;
}

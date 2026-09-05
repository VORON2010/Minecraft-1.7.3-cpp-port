#include "world/item/ItemBucket.h"

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/Tile.h"
#include "world/phys/HitResult.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"

ItemBucket::ItemBucket(int_t baseId, int_t containedBlockId)
	: Item(baseId), containedBlockId(containedBlockId)
{
	setMaxStackSize(1);
}

void ItemBucket::use(ItemInstance &stack, Level &level, Player &player) const
{
	float a = 1.0f;
	float xRot = player.xRotO + (player.xRot - player.xRotO) * a;
	float yRot = player.yRotO + (player.yRot - player.yRotO) * a;
	double px = player.xo + (player.x - player.xo) * a;
	double py = player.yo + (player.y - player.yo) * a + 1.62 - player.heightOffset;
	double pz = player.zo + (player.z - player.zo) * a;
	Vec3 *pos = Vec3::newTemp(px, py, pz);
	float c = Mth::cos(-yRot * 0.017453292f - Mth::PI);
	float s = Mth::sin(-yRot * 0.017453292f - Mth::PI);
	float c2 = -Mth::cos(-xRot * 0.017453292f);
	float s2 = Mth::sin(-xRot * 0.017453292f);
	float vx = s * c2;
	float vy = s2;
	float vz = c * c2;
	Vec3 *to = pos->add(vx * 5.0, vy * 5.0, vz * 5.0);
	HitResult hit = level.clip(*pos, *to, containedBlockId == 0);

	if (hit.type == HitResult::Type::NONE)
		return;

	if (hit.type == HitResult::Type::TILE)
	{
		int_t x = hit.x;
		int_t y = hit.y;
		int_t z = hit.z;

		if (containedBlockId == 0)
		{
			
			const Material &mat = level.getMaterial(x, y, z);
			if (&mat == static_cast<const Material *>(&Material::water) && level.getData(x, y, z) == 0)
			{
				level.setTile(x, y, z, 0);
				stack = ItemInstance(Items::bucketWater->getShiftedIndex(), 1, 0);
				return;
			}
			if (&mat == static_cast<const Material *>(&Material::lava) && level.getData(x, y, z) == 0)
			{
				level.setTile(x, y, z, 0);
				stack = ItemInstance(Items::bucketLava->getShiftedIndex(), 1, 0);
				return;
			}
		}
		else
		{
			if (containedBlockId < 0)
			{
				
				stack = ItemInstance(Items::bucketEmpty->getShiftedIndex(), 1, 0);
				return;
			}

			int_t placeX = x, placeY = y, placeZ = z;
			switch (hit.f)
			{
				case Facing::DOWN: placeY--; break;
				case Facing::UP: placeY++; break;
				case Facing::NORTH: placeZ--; break;
				case Facing::SOUTH: placeZ++; break;
				case Facing::WEST: placeX--; break;
				case Facing::EAST: placeX++; break;
			}

			if (level.getTile(placeX, placeY, placeZ) == 0 || !level.getMaterial(placeX, placeY, placeZ).isSolid())
			{
				if (level.dimension->ultraWarm && containedBlockId == Tile::water.id)
				{
					level.playSoundEffect(x + 0.5, y + 0.5, z + 0.5, u"random.fizz", 0.5f, 2.6f + (level.random.nextFloat() - level.random.nextFloat()) * 0.8f);
					for (int_t i = 0; i < 8; ++i)
					{
						level.addParticle(u"largesmoke", placeX + level.random.nextFloat(), placeY + level.random.nextFloat(), placeZ + level.random.nextFloat(), 0.0, 0.0, 0.0);
					}
				}
				else
				{
					level.setTileAndData(placeX, placeY, placeZ, containedBlockId, 0);
				}
				stack = ItemInstance(Items::bucketEmpty->getShiftedIndex(), 1, 0);
				return;
			}
		}
	}
}

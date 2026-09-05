#include "world/item/ItemBoat.h"

#include "world/entity/item/EntityBoat.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/phys/HitResult.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"

ItemBoat::ItemBoat(int_t baseId) : Item(baseId)
{
	setMaxStackSize(1);
}

void ItemBoat::use(ItemInstance &stack, Level &level, Player &player) const
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
	HitResult hit = level.clip(*pos, *to, true);

	if (hit.type == HitResult::Type::NONE)
		return;

	if (hit.type == HitResult::Type::TILE)
	{
		if (!level.isOnline)
			level.addEntity(std::make_shared<EntityBoat>(level, hit.x + 0.5, hit.y + 1.5, hit.z + 0.5));
		stack.stackSize--;
	}
}

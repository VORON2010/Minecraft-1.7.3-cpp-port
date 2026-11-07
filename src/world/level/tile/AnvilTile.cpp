#include "world/level/tile/AnvilTile.h"
#include "world/level/material/Material.h"

#include "client/player/LocalPlayer.h"
#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/LevelSource.h"

AnvilTile::AnvilTile(int_t id) : SandTile(id, 215, Material::iron)
{
	Tile::lightBlock[id] = 0;
}

int_t AnvilTile::getTexture(Facing face, int_t data)
{
	if (renderPart == 3 && face == Facing::UP)
	{
		switch (data >> 2)
		{
		case 1: return tex + 1;
		case 2: return tex + 16 + 1;
		default: return tex + 16;
		}
	}
	return tex;
}

void AnvilTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	int_t dir = level.getData(x, y, z) & 3;
	if (dir != 3 && dir != 1)
		setShape(0.125f, 0.0f, 0.0f, 0.875f, 1.0f, 1.0f);
	else
		setShape(0.0f, 0.0f, 0.125f, 1.0f, 1.0f, 0.875f);
}

void AnvilTile::setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	int_t dir = Mth::floor(player.yRot * 4.0f / 360.0f + 0.5) & 3;
	int_t damage = level.getData(x, y, z) >> 2;
	dir = (dir + 1) % 4;
	static const int_t facing[4] = {2, 3, 0, 1};
	level.setData(x, y, z, facing[dir] | damage << 2);
	level.levelEvent(1022, x, y, z, 0);
}

bool AnvilTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	if (level.isOnline)
		return true;
	if (LocalPlayer *local = dynamic_cast<LocalPlayer *>(&player))
		local->startRepairing(x, y, z);
	return true;
}

#include "world/level/tile/PaneTile.h"

#include "world/level/Level.h"
#include "world/level/LevelSource.h"
#include "world/level/tile/GlassTile.h"

PaneTile::PaneTile(int_t id, int_t tex, int_t topTex, const Material &material) : Tile(id, tex, material), topTex(topTex)
{
	updateCachedProperties();
}

bool PaneTile::isCubeShaped()
{
	return false;
}

bool PaneTile::isSolidRender()
{
	return false;
}

Tile::Shape PaneTile::getRenderShape()
{
	return SHAPE_PANE;
}

bool PaneTile::shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	if (level.getTile(x, y, z) == id)
		return false;
	return Tile::shouldRenderFace(level, x, y, z, face);
}

bool PaneTile::attachesTo(int_t tile) const
{
	return Tile::solid[tile] || tile == id || tile == Tile::glass.id;
}

void PaneTile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
	bool n = attachesTo(level.getTile(x, y, z - 1));
	bool s = attachesTo(level.getTile(x, y, z + 1));
	bool w = attachesTo(level.getTile(x - 1, y, z));
	bool e = attachesTo(level.getTile(x + 1, y, z));

	if ((!w || !e) && (w || e || n || s))
	{
		if (w && !e)
		{
			setShape(0.0f, 0.0f, 0.4375f, 0.5f, 1.0f, 0.5625f);
			Tile::addAABBs(level, x, y, z, bb, aabbList);
		}
		else if (!w && e)
		{
			setShape(0.5f, 0.0f, 0.4375f, 1.0f, 1.0f, 0.5625f);
			Tile::addAABBs(level, x, y, z, bb, aabbList);
		}
	}
	else
	{
		setShape(0.0f, 0.0f, 0.4375f, 1.0f, 1.0f, 0.5625f);
		Tile::addAABBs(level, x, y, z, bb, aabbList);
	}

	if ((!n || !s) && (w || e || n || s))
	{
		if (n && !s)
		{
			setShape(0.4375f, 0.0f, 0.0f, 0.5625f, 1.0f, 0.5f);
			Tile::addAABBs(level, x, y, z, bb, aabbList);
		}
		else if (!n && s)
		{
			setShape(0.4375f, 0.0f, 0.5f, 0.5625f, 1.0f, 1.0f);
			Tile::addAABBs(level, x, y, z, bb, aabbList);
		}
	}
	else
	{
		setShape(0.4375f, 0.0f, 0.0f, 0.5625f, 1.0f, 1.0f);
		Tile::addAABBs(level, x, y, z, bb, aabbList);
	}
}

void PaneTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void PaneTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	float x0 = 0.4375f, x1 = 0.5625f, z0 = 0.4375f, z1 = 0.5625f;
	bool n = attachesTo(level.getTile(x, y, z - 1));
	bool s = attachesTo(level.getTile(x, y, z + 1));
	bool w = attachesTo(level.getTile(x - 1, y, z));
	bool e = attachesTo(level.getTile(x + 1, y, z));

	if ((!w || !e) && (w || e || n || s))
	{
		if (w && !e) x0 = 0.0f;
		else if (!w && e) x1 = 1.0f;
	}
	else
	{
		x0 = 0.0f;
		x1 = 1.0f;
	}

	if ((!n || !s) && (w || e || n || s))
	{
		if (n && !s) z0 = 0.0f;
		else if (!n && s) z1 = 1.0f;
	}
	else
	{
		z0 = 0.0f;
		z1 = 1.0f;
	}

	setShape(x0, 0.0f, z0, x1, 1.0f, z1);
}

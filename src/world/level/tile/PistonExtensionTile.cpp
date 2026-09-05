#include "world/level/tile/PistonExtensionTile.h"

#include "world/level/Level.h"
#include "world/level/tile/PistonBaseTile.h"
#include "world/level/tile/PistonTextures.h"
#include "world/level/tile/Tile.h"

PistonExtensionTile::PistonExtensionTile(int_t id, int_t tex) : Tile(id, tex, Material::piston)
{
	setDestroyTime(0.5f);
	setSoundType(soundStoneFootstep);
	updateCachedProperties();
}

void PistonExtensionTile::setHeadTexture(int_t tex)
{
	headTextureOverride = tex;
}

void PistonExtensionTile::clearHeadTexture()
{
	headTextureOverride = -1;
}

void PistonExtensionTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	int_t dir = getDirection(level.getData(x, y, z));
	int_t bx = x - PistonTextures::offsetX[dir];
	int_t by = y - PistonTextures::offsetY[dir];
	int_t bz = z - PistonTextures::offsetZ[dir];
	int_t baseId = level.getTile(bx, by, bz);
	if ((baseId == Tile::pistonBase.id || baseId == Tile::pistonStickyBase.id) && PistonBaseTile::isPowered(level.getData(bx, by, bz)))
	{
		Tile::tiles[baseId]->spawnResources(level, bx, by, bz, level.getData(bx, by, bz));
		level.setTile(bx, by, bz, 0);
	}
}

int_t PistonExtensionTile::getTexture(Facing face, int_t data)
{
	int_t dir = getDirection(data);
	int_t faceId = static_cast<int_t>(face);
	if (faceId == dir)
	{
		if (headTextureOverride >= 0)
			return headTextureOverride;
		if ((data & 8) != 0)
			return tex - 1;
		return tex;
	}
	if (faceId == PistonTextures::oppositeFace[dir])
		return 107;
	return 108;
}

Tile::Shape PistonExtensionTile::getRenderShape()
{
	return SHAPE_PISTON_EXTENSION;
}

bool PistonExtensionTile::isSolidRender()
{
	return false;
}

bool PistonExtensionTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	(void)level; (void)x; (void)y; (void)z;
	return false;
}

void PistonExtensionTile::setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	(void)level; (void)x; (void)y; (void)z; (void)face;
}

int_t PistonExtensionTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

void PistonExtensionTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	int_t dir = getDirection(level.getData(x, y, z));
	switch (dir)
	{
		case 0: setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f); break;
		case 1: setShape(0.0f, 0.75f, 0.0f, 1.0f, 1.0f, 1.0f); break;
		case 2: setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f); break;
		case 3: setShape(0.0f, 0.0f, 0.75f, 1.0f, 1.0f, 1.0f); break;
		case 4: setShape(0.0f, 0.0f, 0.0f, 0.25f, 1.0f, 1.0f); break;
		case 5: setShape(0.75f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f); break;
	}
}

void PistonExtensionTile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
	int_t data = level.getData(x, y, z);
	int_t dir = getDirection(data);
	if (dir > 5)
	{
		setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		return;
	}

	updateShape(level, x, y, z);
	AABB *aabb = getAABB(level, x, y, z);
	if (aabb != nullptr && aabb->intersects(bb))
		aabbList.push_back(aabb);

	
	float f = 6.0f / 16.0f;
	float t = 10.0f / 16.0f;
	switch (dir)
	{
		case 0:
			setShape(f, 0.25f, f, t, 1.0f, t);
			break;
		case 1:
			setShape(f, 0.0f, f, t, 0.75f, t);
			break;
		case 2:
			setShape(0.25f, f, 0.25f, 0.75f, t, 1.0f);
			break;
		case 3:
			setShape(0.25f, f, 0.0f, 0.75f, t, 0.75f);
			break;
		case 4:
			setShape(f, 0.25f, 0.25f, t, 0.75f, 1.0f);
			break;
		case 5:
			setShape(0.0f, f, 0.25f, 0.75f, t, 0.75f);
			break;
	}

	aabb = getAABB(level, x, y, z);
	if (aabb != nullptr && aabb->intersects(bb))
		aabbList.push_back(aabb);

	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void PistonExtensionTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	int_t dir = getDirection(level.getData(x, y, z));
	int_t bx = x - PistonTextures::offsetX[dir];
	int_t by = y - PistonTextures::offsetY[dir];
	int_t bz = z - PistonTextures::offsetZ[dir];
	int_t baseId = level.getTile(bx, by, bz);
	if (baseId != Tile::pistonBase.id && baseId != Tile::pistonStickyBase.id)
		level.setTile(x, y, z, 0);
	else
		Tile::tiles[baseId]->neighborChanged(level, bx, by, bz, baseId);
}

int_t PistonExtensionTile::getDirection(int_t data)
{
	return data & 7;
}
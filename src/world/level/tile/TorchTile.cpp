#include "world/level/tile/TorchTile.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/phys/AABB.h"
#include "world/phys/HitResult.h"
#include "world/phys/Vec3.h"
#include "java/Random.h"


TorchTile::TorchTile(int_t id, int_t tex) : Tile(id, tex, Material::circuits())
{
	setDestroyTime(0.0f);
	setLightEmission(14); 
	setTicking(true);
	updateCachedProperties();
}

AABB *TorchTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	return nullptr;
}

bool TorchTile::isSolidRender()
{
	return false;
}

bool TorchTile::isCubeShaped()
{
	return false;
}

Tile::Shape TorchTile::getRenderShape()
{
	return SHAPE_TORCH;
}

bool TorchTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	
	if (level.isSolidTile(x - 1, y, z))
		return true;
	if (level.isSolidTile(x + 1, y, z))
		return true;
	if (level.isSolidTile(x, y, z - 1))
		return true;
	if (level.isSolidTile(x, y, z + 1))
		return true;
	return level.isSolidTile(x, y - 1, z);
}

void TorchTile::setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	
	int_t data = level.getData(x, y, z);
	if (face == Facing::UP && level.isSolidTile(x, y - 1, z))
	{
		data = 5; 
	}
	else if (face == Facing::NORTH && level.isSolidTile(x, y, z + 1))
	{
		data = 4; 
	}
	else if (face == Facing::SOUTH && level.isSolidTile(x, y, z - 1))
	{
		data = 3; 
	}
	else if (face == Facing::WEST && level.isSolidTile(x + 1, y, z))
	{
		data = 2; 
	}
	else if (face == Facing::EAST && level.isSolidTile(x - 1, y, z))
	{
		data = 1; 
	}
	level.setData(x, y, z, data);
}

void TorchTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	Tile::tick(level, x, y, z, random);
	if (level.getData(x, y, z) == 0)
	{
		onPlace(level, x, y, z);
	}
}

void TorchTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	
	if (level.isSolidTile(x - 1, y, z))
	{
		level.setData(x, y, z, 1);
	}
	else if (level.isSolidTile(x + 1, y, z))
	{
		level.setData(x, y, z, 2);
	}
	else if (level.isSolidTile(x, y, z - 1))
	{
		level.setData(x, y, z, 3);
	}
	else if (level.isSolidTile(x, y, z + 1))
	{
		level.setData(x, y, z, 4);
	}
	else if (level.isSolidTile(x, y - 1, z))
	{
		level.setData(x, y, z, 5);
	}

	checkCanSurvive(level, x, y, z);
}

void TorchTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	
	if (checkCanSurvive(level, x, y, z))
	{
		int_t data = level.getData(x, y, z);
		bool shouldDrop = false;

		if (!level.isSolidTile(x - 1, y, z) && data == 1)
			shouldDrop = true;
		if (!level.isSolidTile(x + 1, y, z) && data == 2)
			shouldDrop = true;
		if (!level.isSolidTile(x, y, z - 1) && data == 3)
			shouldDrop = true;
		if (!level.isSolidTile(x, y, z + 1) && data == 4)
			shouldDrop = true;
		if (!level.isSolidTile(x, y - 1, z) && data == 5)
			shouldDrop = true;

		if (shouldDrop)
		{
			spawnResources(level, x, y, z, level.getData(x, y, z));
			level.setTile(x, y, z, 0);
		}
	}
}

bool TorchTile::checkCanSurvive(Level &level, int_t x, int_t y, int_t z)
{
	
	if (!mayPlace(level, x, y, z))
	{
		spawnResources(level, x, y, z, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
		return false;
	}
	return true;
}

HitResult TorchTile::clip(Level &level, int_t x, int_t y, int_t z, Vec3 &from, Vec3 &to)
{
	
	int_t data = level.getData(x, y, z) & 7;
	float size = 0.15f;

	if (data == 1)
	{
		setShape(0.0f, 0.2f, 0.5f - size, size * 2.0f, 0.8f, 0.5f + size);
	}
	else if (data == 2)
	{
		setShape(1.0f - size * 2.0f, 0.2f, 0.5f - size, 1.0f, 0.8f, 0.5f + size);
	}
	else if (data == 3)
	{
		setShape(0.5f - size, 0.2f, 0.0f, 0.5f + size, 0.8f, size * 2.0f);
	}
	else if (data == 4)
	{
		setShape(0.5f - size, 0.2f, 1.0f - size * 2.0f, 0.5f + size, 0.8f, 1.0f);
	}
	else
	{
		size = 0.1f;
		setShape(0.5f - size, 0.0f, 0.5f - size, 0.5f + size, 0.6f, 0.5f + size);
	}

	return Tile::clip(level, x, y, z, from, to);
}

void TorchTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	
	int_t data = level.getData(x, y, z);
	double px = (double)x + 0.5;
	double py = (double)y + 0.7;
	double pz = (double)z + 0.5;
	double var13 = 0.22;
	double var15 = 0.27;

	if (data == 1)
	{
		level.addParticle(u"smoke", px - var15, py + var13, pz, 0.0, 0.0, 0.0);
		level.addParticle(u"flame", px - var15, py + var13, pz, 0.0, 0.0, 0.0);
	}
	else if (data == 2)
	{
		level.addParticle(u"smoke", px + var15, py + var13, pz, 0.0, 0.0, 0.0);
		level.addParticle(u"flame", px + var15, py + var13, pz, 0.0, 0.0, 0.0);
	}
	else if (data == 3)
	{
		level.addParticle(u"smoke", px, py + var13, pz - var15, 0.0, 0.0, 0.0);
		level.addParticle(u"flame", px, py + var13, pz - var15, 0.0, 0.0, 0.0);
	}
	else if (data == 4)
	{
		level.addParticle(u"smoke", px, py + var13, pz + var15, 0.0, 0.0, 0.0);
		level.addParticle(u"flame", px, py + var13, pz + var15, 0.0, 0.0, 0.0);
	}
	else
	{
		level.addParticle(u"smoke", px, py, pz, 0.0, 0.0, 0.0);
		level.addParticle(u"flame", px, py, pz, 0.0, 0.0, 0.0);
	}
}
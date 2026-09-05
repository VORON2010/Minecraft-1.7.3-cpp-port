#include "world/level/tile/PistonBaseTile.h"

#include <cmath>

#include "world/level/Level.h"
#include "world/level/tile/PistonExtensionTile.h"
#include "world/level/tile/PistonMovingTile.h"
#include "world/level/tile/PistonTextures.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/PistonTileEntity.h"
#include "world/entity/player/Player.h"
#include "world/phys/AABB.h"
#include "util/Mth.h"

PistonBaseTile::PistonBaseTile(int_t id, int_t tex, bool sticky) : Tile(id, tex, Material::piston)
{
	isSticky = sticky;
	setDestroyTime(0.5f);
	setSoundType(soundStoneFootstep);
	updateCachedProperties();
}

int_t PistonBaseTile::getTexture(Facing face, int_t data)
{
	int_t dir = getDirection(data);
	if (dir > 5)
		return tex;
	int_t faceId = static_cast<int_t>(face);
	if (faceId == dir)
	{
		if (isPowered(data) || xx0 > 0.0 || yy0 > 0.0 || zz0 > 0.0 || xx1 < 1.0 || yy1 < 1.0 || zz1 < 1.0)
			return 110;
		return tex;
	}
	if (faceId == PistonTextures::oppositeFace[dir])
		return 109;
	return 108;
}

Tile::Shape PistonBaseTile::getRenderShape()
{
	return SHAPE_PISTON_BASE;
}

bool PistonBaseTile::isSolidRender()
{
	return false;
}

bool PistonBaseTile::isCubeShaped()
{
	return false;
}

bool PistonBaseTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)level; (void)x; (void)y; (void)z; (void)player;
	return false;
}

void PistonBaseTile::setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	int_t dir = getPlacementDirection(level, x, y, z, player);
	level.setData(x, y, z, dir);
	if (!level.isOnline)
		checkState(level, x, y, z);
}

void PistonBaseTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	if (!level.isOnline && !suppressNeighborUpdates)
		checkState(level, x, y, z);
}

void PistonBaseTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	if (!level.isOnline && level.getTileEntity(x, y, z) == nullptr)
		checkState(level, x, y, z);
}

void PistonBaseTile::playBlock(Level &level, int_t x, int_t y, int_t z, int_t type, int_t data)
{
	suppressNeighborUpdates = true;
	int_t dir = data;
	if (type == 0)
	{
		if (doExtend(level, x, y, z, dir))
		{
			level.setData(x, y, z, dir | 8);
			level.playSoundEffect(x + 0.5, y + 0.5, z + 0.5, u"tile.piston.out", 0.5f, level.random.nextFloat() * 0.25f + 0.6f);
		}
	}
	else if (type == 1)
	{
		auto te = level.getTileEntity(x + PistonTextures::offsetX[dir], y + PistonTextures::offsetY[dir], z + PistonTextures::offsetZ[dir]);
		if (te != nullptr)
		{
			auto *pistonTe = dynamic_cast<PistonTileEntity *>(te.get());
			if (pistonTe != nullptr)
				pistonTe->finishMovement();
		}
		level.setTileAndData(x, y, z, Tile::pistonMoving.id, dir);
		level.setTileEntity(x, y, z, PistonMovingTile::createTileEntity(id, dir, dir, false, true));
		if (isSticky)
		{
			int_t px = x + PistonTextures::offsetX[dir] * 2;
			int_t py = y + PistonTextures::offsetY[dir] * 2;
			int_t pz = z + PistonTextures::offsetZ[dir] * 2;
			int_t pulledId = level.getTile(px, py, pz);
			int_t pulledData = level.getData(px, py, pz);
			bool found = false;
			if (pulledId == Tile::pistonMoving.id)
			{
				auto pulledTe = level.getTileEntity(px, py, pz);
				if (pulledTe != nullptr)
				{
					auto *pistonPulled = dynamic_cast<PistonTileEntity *>(pulledTe.get());
					if (pistonPulled != nullptr && pistonPulled->getDirection() == dir && pistonPulled->isExtending())
					{
						pistonPulled->finishMovement();
						pulledId = pistonPulled->getStoredBlockID();
						pulledData = pistonPulled->getBlockMetadata();
						found = true;
					}
				}
			}
			if (!found && pulledId > 0 && canPushBlock(pulledId, level, px, py, pz, false)
				&& (Tile::tiles[pulledId]->getMobilityFlag() == 0 || pulledId == Tile::pistonBase.id || pulledId == Tile::pistonStickyBase.id))
			{
				suppressNeighborUpdates = false;
				level.setTile(px, py, pz, 0);
				suppressNeighborUpdates = true;
				
				
				x += PistonTextures::offsetX[dir];
				y += PistonTextures::offsetY[dir];
				z += PistonTextures::offsetZ[dir];
				level.setTileAndData(x, y, z, Tile::pistonMoving.id, pulledData);
				level.setTileEntity(x, y, z, PistonMovingTile::createTileEntity(pulledId, pulledData, dir, false, false));
			}
			else if (!found)
			{
				suppressNeighborUpdates = false;
				level.setTile(x + PistonTextures::offsetX[dir], y + PistonTextures::offsetY[dir], z + PistonTextures::offsetZ[dir], 0);
				suppressNeighborUpdates = true;
			}
		}
		else
		{
			suppressNeighborUpdates = false;
			level.setTile(x + PistonTextures::offsetX[dir], y + PistonTextures::offsetY[dir], z + PistonTextures::offsetZ[dir], 0);
			suppressNeighborUpdates = true;
		}
		level.playSoundEffect(x + 0.5, y + 0.5, z + 0.5, u"tile.piston.in", 0.5f, level.random.nextFloat() * 0.15f + 0.6f);
	}
	suppressNeighborUpdates = false;
}

void PistonBaseTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	if (isPowered(data))
	{
		switch (getDirection(data))
		{
			case 0: setShape(0.0f, 0.25f, 0.0f, 1.0f, 1.0f, 1.0f); break;
			case 1: setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f); break;
			case 2: setShape(0.0f, 0.0f, 0.25f, 1.0f, 1.0f, 1.0f); break;
			case 3: setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.75f); break;
			case 4: setShape(0.25f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f); break;
			case 5: setShape(0.0f, 0.0f, 0.0f, 0.75f, 1.0f, 1.0f); break;
		}
	}
	else
	{
		setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
}

void PistonBaseTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void PistonBaseTile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
	(void)level; (void)x; (void)y; (void)z;
	updateDefaultShape();
	AABB *aabb = getAABB(level, x, y, z);
	if (aabb != nullptr && aabb->intersects(bb))
		aabbList.push_back(aabb);
}

int_t PistonBaseTile::getResourceCount(Random &random)
{
	(void)random;
	return 1;
}

int_t PistonBaseTile::getDirection(int_t data)
{
	return data & 7;
}

bool PistonBaseTile::isPowered(int_t data)
{
	return (data & 8) != 0;
}

bool PistonBaseTile::canPushBlock(int_t tileId, Level &level, int_t x, int_t y, int_t z, bool allowBreak)
{
	if (tileId == Tile::obsidian.id)
		return false;
	if (tileId == Tile::pistonBase.id || tileId == Tile::pistonStickyBase.id)
	{
		if (isPowered(level.getData(x, y, z)))
			return false;
	}
	else
	{
		if (Tile::tiles[tileId] == nullptr)
			return false;
		if (Tile::tiles[tileId]->getDestroyTime() == -1.0f)
			return false;
		if (Tile::tiles[tileId]->getMobilityFlag() == 2)
			return false;
		if (!allowBreak && Tile::tiles[tileId]->getMobilityFlag() == 1)
			return false;
	}
	return level.getTileEntity(x, y, z) == nullptr;
}

void PistonBaseTile::checkState(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	int_t dir = getDirection(data);
	bool powered = isIndirectlyPowered(level, x, y, z, dir);
	if (data == 7)
		return;
	if (powered && !isPowered(data))
	{
		if (canExtend(level, x, y, z, dir))
		{
			level.setDataNoUpdate(x, y, z, dir | 8);
			level.playNoteAt(x, y, z, 0, dir);
		}
	}
	else if (!powered && isPowered(data))
	{
		level.setDataNoUpdate(x, y, z, dir);
		level.playNoteAt(x, y, z, 1, dir);
	}
}

int_t PistonBaseTile::getPlacementDirection(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)level; (void)x; (void)y; (void)z;
	if (std::abs(static_cast<float>(player.x) - x) < 2.0f && std::abs(static_cast<float>(player.z) - z) < 2.0f)
	{
		double eyeY = player.y + 1.82 - player.heightOffset;
		if (eyeY - y > 2.0)
			return 1;
		if (y - eyeY > 0.0)
			return 0;
	}
	int_t yaw = Mth::floor(player.yRot * 4.0f / 360.0f + 0.5f) & 3;
	if (yaw == 0) return 2;
	if (yaw == 1) return 5;
	if (yaw == 2) return 3;
	if (yaw == 3) return 4;
	return 0;
}

bool PistonBaseTile::canExtend(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	int_t cx = x + PistonTextures::offsetX[dir];
	int_t cy = y + PistonTextures::offsetY[dir];
	int_t cz = z + PistonTextures::offsetZ[dir];
	for (int_t i = 0; i < 13; ++i)
	{
		if (cy <= 0 || cy >= 127)
			return false;
		int_t tileId = level.getTile(cx, cy, cz);
		if (tileId == 0)
			break;
		if (!canPushBlock(tileId, level, cx, cy, cz, true))
			return false;
		if (Tile::tiles[tileId]->getMobilityFlag() == 1)
			break;
		if (i == 12)
			return false;
		cx += PistonTextures::offsetX[dir];
		cy += PistonTextures::offsetY[dir];
		cz += PistonTextures::offsetZ[dir];
	}
	return true;
}

bool PistonBaseTile::doExtend(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	int_t cx = x + PistonTextures::offsetX[dir];
	int_t cy = y + PistonTextures::offsetY[dir];
	int_t cz = z + PistonTextures::offsetZ[dir];
	int_t i = 0;
	for (; i < 13; ++i)
	{
		if (cy <= 0 || cy >= 127)
			return false;
		int_t tileId = level.getTile(cx, cy, cz);
		if (tileId == 0)
			break;
		if (!canPushBlock(tileId, level, cx, cy, cz, true))
			return false;
		if (Tile::tiles[tileId]->getMobilityFlag() == 1)
		{
			Tile::tiles[tileId]->spawnResources(level, cx, cy, cz, level.getData(cx, cy, cz));
			level.setTile(cx, cy, cz, 0);
			break;
		}
		if (i == 12)
			return false;
		cx += PistonTextures::offsetX[dir];
		cy += PistonTextures::offsetY[dir];
		cz += PistonTextures::offsetZ[dir];
	}
	while (cx != x || cy != y || cz != z)
	{
		int_t px = cx - PistonTextures::offsetX[dir];
		int_t py = cy - PistonTextures::offsetY[dir];
		int_t pz = cz - PistonTextures::offsetZ[dir];
		int_t prevId = level.getTile(px, py, pz);
		int_t prevData = level.getData(px, py, pz);
		if (prevId == id && px == x && py == y && pz == z)
		{
			level.setTileAndData(cx, cy, cz, Tile::pistonMoving.id, dir | (isSticky ? 8 : 0));
			level.setTileEntity(cx, cy, cz, PistonMovingTile::createTileEntity(Tile::pistonExtension.id, dir | (isSticky ? 8 : 0), dir, true, false));
		}
		else
		{
			level.setTileAndData(cx, cy, cz, Tile::pistonMoving.id, prevData);
			level.setTileEntity(cx, cy, cz, PistonMovingTile::createTileEntity(prevId, prevData, dir, true, false));
		}
		cx = px;
		cy = py;
		cz = pz;
	}
	return true;
}

bool PistonBaseTile::isIndirectlyPowered(Level &level, int_t x, int_t y, int_t z, int_t dir)
{
	if (dir != 0 && level.getSignal(x, y - 1, z, 0))
		return true;
	if (dir != 1 && level.getSignal(x, y + 1, z, 1))
		return true;
	if (dir != 2 && level.getSignal(x, y, z - 1, 2))
		return true;
	if (dir != 3 && level.getSignal(x, y, z + 1, 3))
		return true;
	if (dir != 5 && level.getSignal(x + 1, y, z, 5))
		return true;
	if (dir != 4 && level.getSignal(x - 1, y, z, 4))
		return true;
	if (level.getSignal(x, y, z, 0))
		return true;
	if (level.getSignal(x, y + 2, z, 1))
		return true;
	if (level.getSignal(x, y + 1, z - 1, 2))
		return true;
	if (level.getSignal(x, y + 1, z + 1, 3))
		return true;
	if (level.getSignal(x - 1, y + 1, z, 4))
		return true;
	return level.getSignal(x + 1, y + 1, z, 5);
}
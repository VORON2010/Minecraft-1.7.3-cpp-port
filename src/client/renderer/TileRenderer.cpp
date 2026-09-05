#include "client/renderer/TileRenderer.h"

#include "client/renderer/Tesselator.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/TallGrassTile.h"
#include "world/level/tile/StairTile.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/RedStoneDustTile.h"
#include "world/level/tile/RepeaterTile.h"
#include "world/phys/Vec3.h"
#include "world/level/material/LiquidMaterial.h"

#include "util/Mth.h"
#include "world/level/tile/PistonBaseTile.h"
#include "world/level/tile/PistonExtensionTile.h"
#include "world/level/tile/BedTile.h"
#include <cmath>

namespace
{
	constexpr double FLOW_TEXTURE_NONE = -1000.0;
	constexpr double HALF_PI = 3.14159265358979323846 * 0.5;

	int_t getEffectiveFlowDepth(LevelSource &level, int_t x, int_t y, int_t z, const Material &material)
	{
		if (&level.getMaterial(x, y, z) != &material)
			return -1;

		int_t depth = level.getData(x, y, z);
		if (depth >= 8)
			depth = 0;
		return depth;
	}

	bool blocksFlowAngle(LevelSource &level, int_t x, int_t y, int_t z, const Material &material)
	{
		const Material &adjacent = level.getMaterial(x, y, z);
		if (&adjacent == &material || &adjacent == &Material::ice())
			return false;
		return adjacent.isSolid();
	}

	double getLiquidFlowAngle(LevelSource &level, int_t x, int_t y, int_t z, const Material &material)
	{
		int_t depth = getEffectiveFlowDepth(level, x, y, z, material);
		if (depth < 0)
			return FLOW_TEXTURE_NONE;

		double flowX = 0.0;
		double flowZ = 0.0;

		for (int_t direction = 0; direction < 4; ++direction)
		{
			int_t nx = x;
			int_t nz = z;
			if (direction == 0) nx = x - 1;
			if (direction == 1) nz = z - 1;
			if (direction == 2) nx = x + 1;
			if (direction == 3) nz = z + 1;

			int_t adjacentDepth = getEffectiveFlowDepth(level, nx, y, nz, material);
			if (adjacentDepth < 0)
			{
				if (!blocksFlowAngle(level, nx, y, nz, material))
				{
					adjacentDepth = getEffectiveFlowDepth(level, nx, y - 1, nz, material);
					if (adjacentDepth >= 0)
					{
						int_t delta = adjacentDepth - (depth - 8);
						flowX += (nx - x) * delta;
						flowZ += (nz - z) * delta;
					}
				}
			}
			else
			{
				int_t delta = adjacentDepth - depth;
				flowX += (nx - x) * delta;
				flowZ += (nz - z) * delta;
			}
		}

		if (flowX == 0.0 && flowZ == 0.0)
			return FLOW_TEXTURE_NONE;

		return std::atan2(flowZ, flowX) - HALF_PI;
	}
}

TileRenderer::TileRenderer(LevelSource *levelSource, bool ambientOcclusion, bool fancyGrass)
	: level(levelSource), ambientOcclusion(ambientOcclusion), fancyGrass(fancyGrass)
	, northFlip(FLIP_NONE), southFlip(FLIP_NONE), eastFlip(FLIP_NONE), westFlip(FLIP_NONE), upFlip(FLIP_NONE), downFlip(FLIP_NONE)
	{
}

TileRenderer::TileRenderer(bool ambientOcclusion, bool fancyGrass)
	: ambientOcclusion(ambientOcclusion), fancyGrass(fancyGrass)
	, northFlip(FLIP_NONE), southFlip(FLIP_NONE), eastFlip(FLIP_NONE), westFlip(FLIP_NONE), upFlip(FLIP_NONE), downFlip(FLIP_NONE)
	{
}

void TileRenderer::tesselateInWorld(Tile &tile, int_t x, int_t y, int_t z, int_t fixedTexture)
{
	this->fixedTexture = fixedTexture;
	tesselateInWorld(tile, x, y, z);
	this->fixedTexture = -1;
}

void TileRenderer::tesselateInWorldNoCulling(Tile &tile, int_t x, int_t y, int_t z)
{
	noCulling = true;
	tesselateInWorld(tile, x, y, z);
	noCulling = false;
}

bool TileRenderer::tesselateInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	int_t shape = tt.getRenderShape();
	tt.updateShape(*level, x, y, z);

	if (shape == Tile::SHAPE_BLOCK)
		return tesselateBlockInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_CROSS_TEXTURE)
		return tesselateCrossTextureInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_CACTUS)
		return tesselateCactusInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_WATER)
		return tesselateLiquidInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_TORCH)
		return tesselateTorchInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_STAIRS)
		return tesselateStairsInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_FENCE)
		return tesselateFenceInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_FIRE)
		return tesselateFireInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_LADDER)
		return tesselateLadderInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_RAIL)
		return tesselateRailInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_DOOR)
		return tesselateDoorInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_RED_DUST)
		return tesselateDustInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_REPEATER)
		return tesselateRepeaterInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_LEVER)
		return tesselateLeverInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_PISTON_BASE)
		return tesselatePistonBaseInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_PISTON_EXTENSION)
		return tesselatePistonExtensionInWorld(tt, x, y, z);
	if (shape == Tile::SHAPE_BED)
		return tesselateBedInWorld(tt, x, y, z);
	return false;
}

bool TileRenderer::tesselateFireInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	Tesselator &t = Tesselator::instance;
	int_t tex = tt.getTexture(Facing::UP, 0);
	if (fixedTexture >= 0)
		tex = fixedTexture;

	float br = tt.getBrightness(*level, x, y, z);
	t.color(br, br, br);

	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	double u0 = static_cast<double>(xt) / 256.0;
	double u1 = static_cast<double>(xt + 15.99) / 256.0;
	double v0 = static_cast<double>(yt) / 256.0;
	double v1 = static_cast<double>(yt + 15.99) / 256.0;
	float height = 1.4f;
	double var19, var21, var23, var25, var27, var29, var31;

	bool solidBelow = level->isSolidTile(x, y - 1, z);
	bool canCatchFireBelow = FireTile::canBlockCatchFire(*level, x, y - 1, z);
	if (!solidBelow && !canCatchFireBelow)
	{
		float sideOffset = 0.2f;
		float offset = 1.0f / 16.0f;

		if ((x + y + z & 1) == 1)
		{
			u0 = static_cast<double>(xt) / 256.0;
			u1 = static_cast<double>(xt + 15.99) / 256.0;
			v0 = static_cast<double>(yt + 16) / 256.0;
			v1 = static_cast<double>(yt + 31.99) / 256.0;
		}

		if (((x / 2 + y / 2 + z / 2) & 1) == 1)
		{
			var19 = u1;
			u1 = u0;
			u0 = var19;
		}

		if (FireTile::canBlockCatchFire(*level, x - 1, y, z))
		{
			t.vertexUV(static_cast<double>(x) + sideOffset, static_cast<double>(y) + height + offset, static_cast<double>(z + 1), u1, v0);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + offset, static_cast<double>(z + 1), u1, v1);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + offset, static_cast<double>(z), u0, v1);
			t.vertexUV(static_cast<double>(x) + sideOffset, static_cast<double>(y) + height + offset, static_cast<double>(z), u0, v0);
			t.vertexUV(static_cast<double>(x) + sideOffset, static_cast<double>(y) + height + offset, static_cast<double>(z), u0, v0);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + offset, static_cast<double>(z), u0, v1);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + offset, static_cast<double>(z + 1), u1, v1);
			t.vertexUV(static_cast<double>(x) + sideOffset, static_cast<double>(y) + height + offset, static_cast<double>(z + 1), u1, v0);
		}

		if (FireTile::canBlockCatchFire(*level, x + 1, y, z))
		{
			t.vertexUV(static_cast<double>(x + 1) - sideOffset, static_cast<double>(y) + height + offset, static_cast<double>(z), u0, v0);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + offset, static_cast<double>(z), u0, v1);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + offset, static_cast<double>(z + 1), u1, v1);
			t.vertexUV(static_cast<double>(x + 1) - sideOffset, static_cast<double>(y) + height + offset, static_cast<double>(z + 1), u1, v0);
			t.vertexUV(static_cast<double>(x + 1) - sideOffset, static_cast<double>(y) + height + offset, static_cast<double>(z + 1), u1, v0);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + offset, static_cast<double>(z + 1), u1, v1);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + offset, static_cast<double>(z), u0, v1);
			t.vertexUV(static_cast<double>(x + 1) - sideOffset, static_cast<double>(y) + height + offset, static_cast<double>(z), u0, v0);
		}

		if (FireTile::canBlockCatchFire(*level, x, y, z - 1))
		{
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + height + offset, static_cast<double>(z) + sideOffset, u1, v0);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + offset, static_cast<double>(z), u1, v1);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + offset, static_cast<double>(z), u0, v1);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + height + offset, static_cast<double>(z) + sideOffset, u0, v0);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + height + offset, static_cast<double>(z) + sideOffset, u0, v0);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + offset, static_cast<double>(z), u0, v1);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + offset, static_cast<double>(z), u1, v1);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + height + offset, static_cast<double>(z) + sideOffset, u1, v0);
		}

		if (FireTile::canBlockCatchFire(*level, x, y, z + 1))
		{
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + height + offset, static_cast<double>(z + 1) - sideOffset, u0, v0);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + offset, static_cast<double>(z + 1), u0, v1);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + offset, static_cast<double>(z + 1), u1, v1);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + height + offset, static_cast<double>(z + 1) - sideOffset, u1, v0);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + height + offset, static_cast<double>(z + 1) - sideOffset, u1, v0);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y) + offset, static_cast<double>(z + 1), u1, v1);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + offset, static_cast<double>(z + 1), u0, v1);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + height + offset, static_cast<double>(z + 1) - sideOffset, u0, v0);
		}

		if (FireTile::canBlockCatchFire(*level, x, y + 1, z))
		{
			var19 = static_cast<double>(x) + 1.0;
			var21 = static_cast<double>(x);
			var23 = static_cast<double>(z) + 1.0;
			var25 = static_cast<double>(z);
			var27 = static_cast<double>(x);
			var29 = static_cast<double>(x) + 1.0;
			var31 = static_cast<double>(z);
			double var35 = static_cast<double>(z) + 1.0;
			u0 = static_cast<double>(xt) / 256.0;
			u1 = static_cast<double>(xt + 15.99) / 256.0;
			v0 = static_cast<double>(yt) / 256.0;
			v1 = static_cast<double>(yt + 15.99) / 256.0;
			int_t yAbove = y + 1;
			height = -0.2f;
			if (((x + yAbove + z) & 1) == 0)
			{
				t.vertexUV(var27, static_cast<double>(yAbove) + height, static_cast<double>(z), u1, v0);
				t.vertexUV(var19, static_cast<double>(yAbove), static_cast<double>(z), u1, v1);
				t.vertexUV(var19, static_cast<double>(yAbove), static_cast<double>(z + 1), u0, v1);
				t.vertexUV(var27, static_cast<double>(yAbove) + height, static_cast<double>(z + 1), u0, v0);
				u0 = static_cast<double>(xt) / 256.0;
				u1 = static_cast<double>(xt + 15.99) / 256.0;
				v0 = static_cast<double>(yt + 16) / 256.0;
				v1 = static_cast<double>(yt + 31.99) / 256.0;
				t.vertexUV(var29, static_cast<double>(yAbove) + height, static_cast<double>(z + 1), u1, v0);
				t.vertexUV(var21, static_cast<double>(yAbove), static_cast<double>(z + 1), u1, v1);
				t.vertexUV(var21, static_cast<double>(yAbove), static_cast<double>(z), u0, v1);
				t.vertexUV(var29, static_cast<double>(yAbove) + height, static_cast<double>(z), u0, v0);
			}
			else
			{
				t.vertexUV(static_cast<double>(x), static_cast<double>(yAbove) + height, var35, u1, v0);
				t.vertexUV(static_cast<double>(x), static_cast<double>(yAbove), var25, u1, v1);
				t.vertexUV(static_cast<double>(x + 1), static_cast<double>(yAbove), var25, u0, v1);
				t.vertexUV(static_cast<double>(x + 1), static_cast<double>(yAbove) + height, var35, u0, v0);
				u0 = static_cast<double>(xt) / 256.0;
				u1 = static_cast<double>(xt + 15.99) / 256.0;
				v0 = static_cast<double>(yt + 16) / 256.0;
				v1 = static_cast<double>(yt + 31.99) / 256.0;
				t.vertexUV(static_cast<double>(x + 1), static_cast<double>(yAbove) + height, var31, u1, v0);
				t.vertexUV(static_cast<double>(x + 1), static_cast<double>(yAbove), var23, u1, v1);
				t.vertexUV(static_cast<double>(x), static_cast<double>(yAbove), var23, u0, v1);
				t.vertexUV(static_cast<double>(x), static_cast<double>(yAbove) + height, var31, u0, v0);
			}
		}
	}
	else
	{
		double var33 = static_cast<double>(x) + 0.7;
		var19 = static_cast<double>(x) + 0.3;
		var21 = static_cast<double>(z) + 0.7;
		var23 = static_cast<double>(z) + 0.3;
		var25 = static_cast<double>(x) + 0.2;
		var27 = static_cast<double>(x) + 0.8;
		var29 = static_cast<double>(z) + 0.2;
		var31 = static_cast<double>(z) + 0.8;

		t.vertexUV(var25, static_cast<double>(y) + height, static_cast<double>(z + 1), u1, v0);
		t.vertexUV(var33, static_cast<double>(y), static_cast<double>(z + 1), u1, v1);
		t.vertexUV(var33, static_cast<double>(y), static_cast<double>(z), u0, v1);
		t.vertexUV(var25, static_cast<double>(y) + height, static_cast<double>(z), u0, v0);
		t.vertexUV(var27, static_cast<double>(y) + height, static_cast<double>(z), u1, v0);
		t.vertexUV(var19, static_cast<double>(y), static_cast<double>(z), u1, v1);
		t.vertexUV(var19, static_cast<double>(y), static_cast<double>(z + 1), u0, v1);
		t.vertexUV(var27, static_cast<double>(y) + height, static_cast<double>(z + 1), u0, v0);

		u0 = static_cast<double>(xt) / 256.0;
		u1 = static_cast<double>(xt + 15.99) / 256.0;
		v0 = static_cast<double>(yt + 16) / 256.0;
		v1 = static_cast<double>(yt + 31.99) / 256.0;

		t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + height, var31, u1, v0);
		t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y), var23, u1, v1);
		t.vertexUV(static_cast<double>(x), static_cast<double>(y), var23, u0, v1);
		t.vertexUV(static_cast<double>(x), static_cast<double>(y) + height, var31, u0, v0);
		t.vertexUV(static_cast<double>(x), static_cast<double>(y) + height, var29, u1, v0);
		t.vertexUV(static_cast<double>(x), static_cast<double>(y), var21, u1, v1);
		t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y), var21, u0, v1);
		t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + height, var29, u0, v0);

		var33 = static_cast<double>(x);
		var19 = static_cast<double>(x + 1);
		var21 = static_cast<double>(z);
		var23 = static_cast<double>(z + 1);
		var25 = static_cast<double>(x) + 0.1;
		var27 = static_cast<double>(x) + 0.9;
		var29 = static_cast<double>(z) + 0.1;
		var31 = static_cast<double>(z) + 0.9;

		t.vertexUV(var25, static_cast<double>(y) + height, static_cast<double>(z), u0, v0);
		t.vertexUV(var33, static_cast<double>(y), static_cast<double>(z), u0, v1);
		t.vertexUV(var33, static_cast<double>(y), static_cast<double>(z + 1), u1, v1);
		t.vertexUV(var25, static_cast<double>(y) + height, static_cast<double>(z + 1), u1, v0);
		t.vertexUV(var27, static_cast<double>(y) + height, static_cast<double>(z + 1), u0, v0);
		t.vertexUV(var19, static_cast<double>(y), static_cast<double>(z + 1), u0, v1);
		t.vertexUV(var19, static_cast<double>(y), static_cast<double>(z), u1, v1);
		t.vertexUV(var27, static_cast<double>(y) + height, static_cast<double>(z), u1, v0);

		u0 = static_cast<double>(xt) / 256.0;
		u1 = static_cast<double>(xt + 15.99) / 256.0;
		v0 = static_cast<double>(yt) / 256.0;
		v1 = static_cast<double>(yt + 15.99) / 256.0;

		t.vertexUV(static_cast<double>(x), static_cast<double>(y) + height, var31, u0, v0);
		t.vertexUV(static_cast<double>(x), static_cast<double>(y), var23, u0, v1);
		t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y), var23, u1, v1);
		t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + height, var31, u1, v0);
		t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y) + height, var29, u0, v0);
		t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y), var21, u0, v1);
		t.vertexUV(static_cast<double>(x), static_cast<double>(y), var21, u1, v1);
		t.vertexUV(static_cast<double>(x), static_cast<double>(y) + height, var29, u1, v0);
	}

	return true;
}


bool TileRenderer::tesselateRailInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	Tesselator &t = Tesselator::instance;
	int_t data = level->getData(x, y, z);
	int_t tex = tt.getTexture(*level, x, y, z, Facing::NORTH);
	if (fixedTexture >= 0) tex = fixedTexture;
	if (tt.id == 27 || tt.id == 28) data &= 7;

	float br = tt.getBrightness(*level, x, y, z);
	t.color(br, br, br);

	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	double u0 = xt / 256.0;
	double u1 = (xt + 15.99) / 256.0;
	double v0 = yt / 256.0;
	double v1 = (yt + 15.99) / 256.0;

	double x0 = x + 1.0;
	double x1 = x + 1.0;
	double x2 = x + 0.0;
	double x3 = x + 0.0;
	double z0 = z + 0.0;
	double z1 = z + 1.0;
	double z2 = z + 1.0;
	double z3 = z + 0.0;
	double y0 = y + 1.0 / 16.0;
	double y1 = y + 1.0 / 16.0;
	double y2 = y + 1.0 / 16.0;
	double y3 = y + 1.0 / 16.0;

	if (data == 1 || data == 2 || data == 3 || data == 7)
	{
		x3 = x + 1.0;
		x0 = x3;
		x2 = x + 0.0;
		x1 = x2;
		z1 = z + 1.0;
		z0 = z1;
		z3 = z + 0.0;
		z2 = z3;
	}
	else if (data == 8)
	{
		x1 = x + 0.0;
		x0 = x1;
		x3 = x + 1.0;
		x2 = x3;
		z3 = z + 1.0;
		z0 = z3;
		z2 = z + 0.0;
		z1 = z2;
	}
	else if (data == 9)
	{
		x3 = x + 0.0;
		x0 = x3;
		x2 = x + 1.0;
		x1 = x2;
		z1 = z + 0.0;
		z0 = z1;
		z3 = z + 1.0;
		z2 = z3;
	}

	if (data == 2 || data == 4)
	{
		y0 += 1.0;
		y3 += 1.0;
	}
	else if (data == 3 || data == 5)
	{
		y1 += 1.0;
		y2 += 1.0;
	}

	t.vertexUV(x0, y0, z0, u1, v0);
	t.vertexUV(x1, y1, z1, u1, v1);
	t.vertexUV(x2, y2, z2, u0, v1);
	t.vertexUV(x3, y3, z3, u0, v0);
	t.vertexUV(x3, y3, z3, u0, v0);
	t.vertexUV(x2, y2, z2, u0, v1);
	t.vertexUV(x1, y1, z1, u1, v1);
	t.vertexUV(x0, y0, z0, u1, v0);
	return true;
}

bool TileRenderer::tesselateCactusInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	float c10 = 0.5f;
	float c11 = 1.0f;
	float c2 = 0.8f;
	float c3 = 0.6f;
	float s = 1.0f / 16.0f;
	bool changed = false;

	int_t col = tt.getColor(*level, x, y, z);
	float r = static_cast<float>((col >> 16) & 0xFF) / 255.0f;
	float g = static_cast<float>((col >> 8) & 0xFF) / 255.0f;
	float b = static_cast<float>(col & 0xFF) / 255.0f;
	float lightValueOwn = tt.getBrightness(*level, x, y, z);

	double oldxx0 = tt.xx0;
	double oldyy0 = tt.yy0;
	double oldzz0 = tt.zz0;
	double oldxx1 = tt.xx1;
	double oldyy1 = tt.yy1;
	double oldzz1 = tt.zz1;
	double inset = 1.0 / 16.0;
	double cactusxx0 = inset;
	double cactusyy0 = 0.0;
	double cactuszz0 = inset;
	double cactusxx1 = 1.0 - inset;
	double cactusyy1 = 1.0;
	double cactuszz1 = 1.0 - inset;

	if (noCulling || tt.shouldRenderFace(*level, x, y - 1, z, Facing::DOWN))
	{
		float br = tt.getBrightness(*level, x, y - 1, z);
		tt.setShape(static_cast<float>(cactusxx0), static_cast<float>(cactusyy0), static_cast<float>(cactuszz0), static_cast<float>(cactusxx1), static_cast<float>(cactusyy1), static_cast<float>(cactuszz1));
		Tesselator::instance.color(c10 * r * br, c10 * g * br, c10 * b * br);
		renderFaceUp(tt, x, y, z, tt.getTexture(*level, x, y, z, Facing::DOWN));
		changed = true;
	}

	if (noCulling || tt.shouldRenderFace(*level, x, y + 1, z, Facing::UP))
	{
		float br = tt.getBrightness(*level, x, y + 1, z);
		if (cactusyy1 != 1.0 && !tt.material.isLiquid()) br = lightValueOwn;
		tt.setShape(static_cast<float>(cactusxx0), static_cast<float>(cactusyy0), static_cast<float>(cactuszz0), static_cast<float>(cactusxx1), static_cast<float>(cactusyy1), static_cast<float>(cactuszz1));
		Tesselator::instance.color(c11 * r * br, c11 * g * br, c11 * b * br);
		renderFaceDown(tt, x, y, z, tt.getTexture(*level, x, y, z, Facing::UP));
		changed = true;
	}

	tt.setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	if (noCulling || tt.shouldRenderFace(*level, x, y, z - 1, Facing::NORTH))
	{
		float br = tt.getBrightness(*level, x, y, z - 1);
		Tesselator::instance.color(c2 * r * br, c2 * g * br, c2 * b * br);
		renderNorth(tt, x, y, z + s, tt.getTexture(*level, x, y, z, Facing::NORTH));
		changed = true;
	}

	if (noCulling || tt.shouldRenderFace(*level, x, y, z + 1, Facing::SOUTH))
	{
		float br = tt.getBrightness(*level, x, y, z + 1);
		Tesselator::instance.color(c2 * r * br, c2 * g * br, c2 * b * br);
		renderSouth(tt, x, y, z - s, tt.getTexture(*level, x, y, z, Facing::SOUTH));
		changed = true;
	}

	if (noCulling || tt.shouldRenderFace(*level, x - 1, y, z, Facing::WEST))
	{
		float br = tt.getBrightness(*level, x - 1, y, z);
		Tesselator::instance.color(c3 * r * br, c3 * g * br, c3 * b * br);
		renderWest(tt, x + s, y, z, tt.getTexture(*level, x, y, z, Facing::WEST));
		changed = true;
	}

	if (noCulling || tt.shouldRenderFace(*level, x + 1, y, z, Facing::EAST))
	{
		float br = tt.getBrightness(*level, x + 1, y, z);
		Tesselator::instance.color(c3 * r * br, c3 * g * br, c3 * b * br);
		renderEast(tt, x - s, y, z, tt.getTexture(*level, x, y, z, Facing::EAST));
		changed = true;
	}

	tt.setShape(static_cast<float>(oldxx0), static_cast<float>(oldyy0), static_cast<float>(oldzz0), static_cast<float>(oldxx1), static_cast<float>(oldyy1), static_cast<float>(oldzz1));
	enableAO = false;
	return changed;
}

bool TileRenderer::tesselateCrossTextureInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	Tesselator &t = Tesselator::instance;
	int_t tex = tt.getTexture(*level, x, y, z, Facing::NORTH);
	if (fixedTexture >= 0) tex = fixedTexture;

	int_t color = tt.getColor(*level, x, y, z);
	float r = static_cast<float>((color >> 16) & 0xFF) / 255.0f;
	float g = static_cast<float>((color >> 8) & 0xFF) / 255.0f;
	float b = static_cast<float>(color & 0xFF) / 255.0f;
	float bright = tt.getBrightness(*level, x, y, z);
	t.color(r * bright, g * bright, b * bright);

	int_t xt = (tex & 0xF) << 4;
	int_t yt = tex & 0xF0;
	double u0 = static_cast<double>(xt) / 256.0;
	double u1 = (static_cast<double>(xt) + 15.99) / 256.0;
	double v0 = static_cast<double>(yt) / 256.0;
	double v1 = (static_cast<double>(yt) + 15.99) / 256.0;

	
	double xo = 0.0;
	double yo = 0.0;
	double zo = 0.0;
	double cropYOffset = tt.id == 59 ? -(1.0 / 16.0) : 0.0;
	if (tt.id == Tile::tallGrass.id)
	{
		long_t offset = (static_cast<long_t>(x) * 3129871LL) ^ (static_cast<long_t>(z) * 116129781LL) ^ static_cast<long_t>(y);
		offset = offset * offset * 42317861LL + offset * 11LL;
		xo = ((static_cast<double>((offset >> 16) & 15LL) / 15.0) - 0.5) * 0.5;
		yo = ((static_cast<double>((offset >> 20) & 15LL) / 15.0) - 1.0) * 0.2;
		zo = ((static_cast<double>((offset >> 24) & 15LL) / 15.0) - 0.5) * 0.5;
	}

	double x0 = x + xo + 0.5 - 0.45;
	double x1 = x + xo + 0.5 + 0.45;
	double y0 = y + yo + cropYOffset;
	double y1 = y + yo + cropYOffset + 1.0;
	double z0 = z + zo + 0.5 - 0.45;
	double z1 = z + zo + 0.5 + 0.45;

	auto renderQuad = [&](double ax, double az, double bx, double bz) {
		t.vertexUV(ax, y1, az, u0, v0);
		t.vertexUV(ax, y0, az, u0, v1);
		t.vertexUV(bx, y0, bz, u1, v1);
		t.vertexUV(bx, y1, bz, u1, v0);
		t.vertexUV(bx, y1, bz, u0, v0);
		t.vertexUV(bx, y0, bz, u0, v1);
		t.vertexUV(ax, y0, az, u1, v1);
		t.vertexUV(ax, y1, az, u1, v0);
	};

	renderQuad(x0, z0, x1, z1);
	renderQuad(x0, z1, x1, z0);
	return true;
}

bool TileRenderer::tesselateBlockInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	int_t col = tt.getColor(*level, x, y, z);
	float r = ((col >> 16) & 0xFF) / 255.0f;
	float g = ((col >> 8) & 0xFF) / 255.0f;
	float b = (col & 0xFF) / 255.0f;
	return tesselateBlockInWorld(tt, x, y, z, r, g, b);
}

bool TileRenderer::tesselateBlockInWorld(Tile &tt, int_t x, int_t y, int_t z, float r, float g, float b)
{
	Tesselator &t = Tesselator::instance;
	bool changed = false;
	float c10 = 0.5f;
	float c11 = 1.0f;
	float c2 = 0.8f;
	float c3 = 0.6f;

	float grassR = r;
	float grassG = g;
	float grassB = b;
	bool isGrass = &tt == reinterpret_cast<Tile*>(&Tile::grass);
	bool useFancyGrass = fancyGrass && fixedTexture < 0 && isGrass;

	bool cactusShape = tt.getRenderShape() == Tile::SHAPE_CACTUS;
	double cactusInset = cactusShape ? (1.0 / 16.0) : 0.0;

	float r11 = c11 * r;
	float g11 = c11 * g;
	float b11 = c11 * b;

	if (isGrass)
	{
		b = 1.0f;
		g = 1.0f;
		r = 1.0f;
	}

	float r10 = c10 * r;
	float r2 = c2 * r;
	float r3 = c3 * r;

	float g10 = c10 * g;
	float g2 = c2 * g;
	float g3 = c3 * g;

	float b10 = c10 * b;
	float b2 = c2 * b;
	float b3 = c3 * b;

	float lightValueOwn = tt.getBrightness(*level, x, y, z);
	bool useAO = ambientOcclusion && !noCulling && !cactusShape && tt.xx0 == 0.0 && tt.xx1 == 1.0 && tt.yy0 == 0.0 && tt.yy1 == 1.0 && tt.zz0 == 0.0 && tt.zz1 == 1.0;

	auto sample = [&](int_t sx, int_t sy, int_t sz) {
		return tt.getBrightness(*level, sx, sy, sz);
	};
	auto setVertexColors = [&](float v0, float v1, float v2, float v3, float cr, float cg, float cb) {
		enableAO = true;
		colorR0 = cr * v0; colorG0 = cg * v0; colorB0 = cb * v0;
		colorR1 = cr * v1; colorG1 = cg * v1; colorB1 = cb * v1;
		colorR2 = cr * v2; colorG2 = cg * v2; colorB2 = cb * v2;
		colorR3 = cr * v3; colorG3 = cg * v3; colorB3 = cb * v3;
	};
	auto tintVertexColors = [&](float tr, float tg, float tb) {
		colorR0 *= tr; colorG0 *= tg; colorB0 *= tb;
		colorR1 *= tr; colorG1 *= tg; colorB1 *= tb;
		colorR2 *= tr; colorG2 *= tg; colorB2 *= tb;
		colorR3 *= tr; colorG3 *= tg; colorB3 *= tb;
	};

	
	float aoXNeg = 0, aoYNeg = 0, aoZNeg = 0, aoXPos = 0, aoYPos = 0, aoZPos = 0;
	bool tXpYp = false, tXpYn = false, tXpZp = false, tXpZn = false;
	bool tXnYp = false, tXnYn = false, tXnZn = false, tXnZp = false;
	bool tYpZp = false, tYpZn = false, tYnZp = false, tYnZn = false;

	if (useAO)
	{
		aoXNeg = sample(x - 1, y, z);
		aoYNeg = sample(x, y - 1, z);
		aoZNeg = sample(x, y, z - 1);
		aoXPos = sample(x + 1, y, z);
		aoYPos = sample(x, y + 1, z);
		aoZPos = sample(x, y, z + 1);

		
		auto cbg = [&](int_t sx, int_t sy, int_t sz) { return !Tile::solid[level->getTile(sx, sy, sz)]; };
		tXpYp = cbg(x + 1, y + 1, z); tXpYn = cbg(x + 1, y - 1, z);
		tXpZp = cbg(x + 1, y, z + 1); tXpZn = cbg(x + 1, y, z - 1);
		tXnYp = cbg(x - 1, y + 1, z); tXnYn = cbg(x - 1, y - 1, z);
		tXnZn = cbg(x - 1, y, z - 1); tXnZp = cbg(x - 1, y, z + 1);
		tYpZp = cbg(x, y + 1, z + 1); tYpZn = cbg(x, y + 1, z - 1);
		tYnZp = cbg(x, y - 1, z + 1); tYnZn = cbg(x, y - 1, z - 1);
	}

	
	if (noCulling || tt.shouldRenderFace(*level, x, y - 1, z, Facing::DOWN))
	{
		if (useAO)
		{
			float eXn = sample(x - 1, y - 1, z);
			float eZn = sample(x, y - 1, z - 1);
			float eZp = sample(x, y - 1, z + 1);
			float eXp = sample(x + 1, y - 1, z);
			
			float cXnZn = (tYnZn || tXnYn) ? sample(x - 1, y - 1, z - 1) : eXn;
			float cXnZp = (tYnZp || tXnYn) ? sample(x - 1, y - 1, z + 1) : eXn;
			float cXpZn = (tYnZn || tXpYn) ? sample(x + 1, y - 1, z - 1) : eXp;
			float cXpZp = (tYnZp || tXpYn) ? sample(x + 1, y - 1, z + 1) : eXp;
			setVertexColors(
				(cXnZp + eXn + eZp + aoYNeg) * 0.25f,
				(eXn + cXnZn + aoYNeg + eZn) * 0.25f,
				(aoYNeg + eZn + eXp + cXpZn) * 0.25f,
				(eZp + aoYNeg + cXpZp + eXp) * 0.25f,
				r10, g10, b10);
		}
		else
		{
			enableAO = false;
			float br = sample(x, y - 1, z);
			t.color(r10 * br, g10 * br, b10 * br);
		}
		renderFaceUp(tt, x, y, z, tt.getTexture(*level, x, y, z, Facing::DOWN));
		changed = true;
	}

	
	if (noCulling || tt.shouldRenderFace(*level, x, y + 1, z, Facing::UP))
	{
		if (useAO)
		{
			float eXn = sample(x - 1, y + 1, z);
			float eXp = sample(x + 1, y + 1, z);
			float eZn = sample(x, y + 1, z - 1);
			float eZp = sample(x, y + 1, z + 1);
			float cXnZn = (tYpZn || tXnYp) ? sample(x - 1, y + 1, z - 1) : eXn;
			float cXpZn = (tYpZn || tXpYp) ? sample(x + 1, y + 1, z - 1) : eXp;
			float cXnZp = (tYpZp || tXnYp) ? sample(x - 1, y + 1, z + 1) : eXn;
			float cXpZp = (tYpZp || tXpYp) ? sample(x + 1, y + 1, z + 1) : eXp;
			setVertexColors(
				(eZp + aoYPos + cXpZp + eXp) * 0.25f,
				(aoYPos + eZn + eXp + cXpZn) * 0.25f,
				(eXn + cXnZn + aoYPos + eZn) * 0.25f,
				(cXnZp + eXn + eZp + aoYPos) * 0.25f,
				r11, g11, b11);
		}
		else
		{
			enableAO = false;
			float br = sample(x, y + 1, z);
			if (tt.yy1 != 1.0 && !tt.material.isLiquid()) br = lightValueOwn;
			t.color(r11 * br, g11 * br, b11 * br);
		}
		renderFaceDown(tt, x, y, z, tt.getTexture(*level, x, y, z, Facing::UP));
		changed = true;
	}

	
	if (noCulling || tt.shouldRenderFace(*level, x, y, z - 1, Facing::NORTH))
	{
		int_t tex = tt.getTexture(*level, x, y, z, Facing::NORTH);
		float br = 0.0f;
		if (useAO)
		{
			float eXn = sample(x - 1, y, z - 1);
			float eYn = sample(x, y - 1, z - 1);
			float eYp = sample(x, y + 1, z - 1);
			float eXp = sample(x + 1, y, z - 1);
			float cXnYn = (tXnZn || tYnZn) ? sample(x - 1, y - 1, z - 1) : eXn;
			float cXnYp = (tXnZn || tYpZn) ? sample(x - 1, y + 1, z - 1) : eXn;
			float cXpYn = (tXpZn || tYnZn) ? sample(x + 1, y - 1, z - 1) : eXp;
			float cXpYp = (tXpZn || tYpZn) ? sample(x + 1, y + 1, z - 1) : eXp;
			setVertexColors(
				(eXn + cXnYp + aoZNeg + eYp) * 0.25f,
				(aoZNeg + eYp + eXp + cXpYp) * 0.25f,
				(eYn + aoZNeg + cXpYn + eXp) * 0.25f,
				(cXnYn + eXn + eYn + aoZNeg) * 0.25f,
				r2, g2, b2);
		}
		else
		{
			enableAO = false;
			br = sample(x, y, z - 1);
			if (tt.zz0 > 0.0) br = lightValueOwn;
			t.color(r2 * br, g2 * br, b2 * br);
		}
		renderNorth(tt, x, y, z + cactusInset, tex);
		if (useFancyGrass && tex == 3)
		{
			if (useAO)
			{
				tintVertexColors(grassR, grassG, grassB);
				renderNorth(tt, x, y, z + cactusInset, 38);
			}
			else
			{
				t.color(c2 * grassR * br, c2 * grassG * br, c2 * grassB * br);
				renderNorth(tt, x, y, z + cactusInset, 38);
			}
		}
		changed = true;
	}

	
	if (noCulling || tt.shouldRenderFace(*level, x, y, z + 1, Facing::SOUTH))
	{
		int_t tex = tt.getTexture(*level, x, y, z, Facing::SOUTH);
		float br = 0.0f;
		if (useAO)
		{
			float eXn = sample(x - 1, y, z + 1);
			float eXp = sample(x + 1, y, z + 1);
			float eYn = sample(x, y - 1, z + 1);
			float eYp = sample(x, y + 1, z + 1);
			float cXnYn = (tXnZp || tYnZp) ? sample(x - 1, y - 1, z + 1) : eXn;
			float cXnYp = (tXnZp || tYpZp) ? sample(x - 1, y + 1, z + 1) : eXn;
			float cXpYn = (tXpZp || tYnZp) ? sample(x + 1, y - 1, z + 1) : eXp;
			float cXpYp = (tXpZp || tYpZp) ? sample(x + 1, y + 1, z + 1) : eXp;
			setVertexColors(
				(eXn + cXnYp + aoZPos + eYp) * 0.25f,
				(cXnYn + eXn + eYn + aoZPos) * 0.25f,
				(eYn + aoZPos + cXpYn + eXp) * 0.25f,
				(aoZPos + eYp + eXp + cXpYp) * 0.25f,
				r2, g2, b2);
		}
		else
		{
			enableAO = false;
			br = sample(x, y, z + 1);
			if (tt.zz1 < 1.0) br = lightValueOwn;
			t.color(r2 * br, g2 * br, b2 * br);
		}
		renderSouth(tt, x, y, z - cactusInset, tex);
		if (useFancyGrass && tex == 3)
		{
			if (useAO)
			{
				tintVertexColors(grassR, grassG, grassB);
				renderSouth(tt, x, y, z - cactusInset, 38);
			}
			else
			{
				t.color(c2 * grassR * br, c2 * grassG * br, c2 * grassB * br);
				renderSouth(tt, x, y, z - cactusInset, 38);
			}
		}
		changed = true;
	}

	
	if (noCulling || tt.shouldRenderFace(*level, x - 1, y, z, Facing::WEST))
	{
		int_t tex = tt.getTexture(*level, x, y, z, Facing::WEST);
		float br = 0.0f;
		if (useAO)
		{
			float eYn = sample(x - 1, y - 1, z);
			float eZn = sample(x - 1, y, z - 1);
			float eZp = sample(x - 1, y, z + 1);
			float eYp = sample(x - 1, y + 1, z);
			float cYnZn = (tXnZn || tXnYn) ? sample(x - 1, y - 1, z - 1) : eZn;
			float cYnZp = (tXnZp || tXnYn) ? sample(x - 1, y - 1, z + 1) : eZp;
			float cYpZn = (tXnZn || tXnYp) ? sample(x - 1, y + 1, z - 1) : eZn;
			float cYpZp = (tXnZp || tXnYp) ? sample(x - 1, y + 1, z + 1) : eZp;
			setVertexColors(
				(aoXNeg + eZp + eYp + cYpZp) * 0.25f,
				(eZn + aoXNeg + cYpZn + eYp) * 0.25f,
				(cYnZn + eYn + eZn + aoXNeg) * 0.25f,
				(eYn + cYnZp + aoXNeg + eZp) * 0.25f,
				r3, g3, b3);
		}
		else
		{
			enableAO = false;
			br = sample(x - 1, y, z);
			if (tt.xx0 > 0.0) br = lightValueOwn;
			t.color(r3 * br, g3 * br, b3 * br);
		}
		renderWest(tt, x + cactusInset, y, z, tex);
		if (useFancyGrass && tex == 3)
		{
			if (useAO)
			{
				tintVertexColors(grassR, grassG, grassB);
				renderWest(tt, x + cactusInset, y, z, 38);
			}
			else
			{
				t.color(c3 * grassR * br, c3 * grassG * br, c3 * grassB * br);
				renderWest(tt, x + cactusInset, y, z, 38);
			}
		}
		changed = true;
	}

	
	if (noCulling || tt.shouldRenderFace(*level, x + 1, y, z, Facing::EAST))
	{
		int_t tex = tt.getTexture(*level, x, y, z, Facing::EAST);
		float br = 0.0f;
		if (useAO)
		{
			float eYn = sample(x + 1, y - 1, z);
			float eZn = sample(x + 1, y, z - 1);
			float eZp = sample(x + 1, y, z + 1);
			float eYp = sample(x + 1, y + 1, z);
			float cYnZn = (tXpYn || tXpZn) ? sample(x + 1, y - 1, z - 1) : eZn;
			float cYnZp = (tXpYn || tXpZp) ? sample(x + 1, y - 1, z + 1) : eZp;
			float cYpZn = (tXpYp || tXpZn) ? sample(x + 1, y + 1, z - 1) : eZn;
			float cYpZp = (tXpYp || tXpZp) ? sample(x + 1, y + 1, z + 1) : eZp;
			setVertexColors(
				(eYn + cYnZp + aoXPos + eZp) * 0.25f,
				(cYnZn + eYn + eZn + aoXPos) * 0.25f,
				(eZn + aoXPos + cYpZn + eYp) * 0.25f,
				(aoXPos + eZp + eYp + cYpZp) * 0.25f,
				r3, g3, b3);
		}
		else
		{
			enableAO = false;
			br = sample(x + 1, y, z);
			if (tt.xx1 < 1.0) br = lightValueOwn;
			t.color(r3 * br, g3 * br, b3 * br);
		}
		renderEast(tt, x - cactusInset, y, z, tex);
		if (useFancyGrass && tex == 3)
		{
			if (useAO)
			{
				tintVertexColors(grassR, grassG, grassB);
				renderEast(tt, x - cactusInset, y, z, 38);
			}
			else
			{
				t.color(c3 * grassR * br, c3 * grassG * br, c3 * grassB * br);
				renderEast(tt, x - cactusInset, y, z, 38);
			}
		}
		changed = true;
	}

	enableAO = false;
	return changed;
}

void TileRenderer::renderFaceUp(Tile &tt, double x, double y, double z, int_t tex)
	{
		Tesselator &t = Tesselator::instance;

		if (fixedTexture >= 0) tex = fixedTexture;

		int_t xt = (tex & 0xF) << 4;
		int_t yt = tex & 0xF0;

		double u00 = (xt + tt.xx0 * 16.0) / 256.0;
		double u11 = (xt + tt.xx1 * 16.0 - 0.01) / 256.0;
		double v00 = (yt + tt.zz0 * 16.0) / 256.0;
		double v11 = (yt + tt.zz1 * 16.0 - 0.01) / 256.0;

		if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
		{
			u00 = ((xt + 0.0f) / 256.0f);
			u11 = ((xt + 15.99f) / 256.0f);
		}

		if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
		{
			v00 = ((yt + 0.0f) / 256.0f);
			v11 = ((yt + 15.99f) / 256.0f);
		}

		double u01 = u11, u10 = u00, v01 = v00, v10 = v11;

		if (upFlip == FLIP_CW)
		{
			u00 = (xt + tt.zz0 * 16.0) / 256.0;
			v00 = (yt + (1.0 - tt.xx1) * 16.0) / 256.0;
			u11 = (xt + tt.zz1 * 16.0 - 0.01) / 256.0;
			v11 = (yt + (1.0 - tt.xx0) * 16.0 - 0.01) / 256.0;

			if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
			{
				u00 = ((xt + 0.0f) / 256.0f);
				u11 = ((xt + 15.99f) / 256.0f);
			}

			if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
			{
				v00 = ((yt + 0.0f) / 256.0f);
				v11 = ((yt + 15.99f) / 256.0f);
			}

			u01 = u11;
			u10 = u00;
			v01 = v00;
			v10 = v11;
			u01 = u00;
			u10 = u11;
			v00 = v11;
			v11 = v01;
		}

		else if (upFlip == FLIP_CCW)
		{
			u00 = (xt + (1.0 - tt.zz1) * 16.0) / 256.0;
			v00 = (yt + tt.xx0 * 16.0) / 256.0;
			u11 = (xt + (1.0 - tt.zz0) * 16.0 - 0.01) / 256.0;
			v11 = (yt + tt.xx1 * 16.0 - 0.01) / 256.0;

			if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
			{
				u00 = ((xt + 0.0f) / 256.0f);
				u11 = ((xt + 15.99f) / 256.0f);
			}

			if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
			{
				v00 = ((yt + 0.0f) / 256.0f);
				v11 = ((yt + 15.99f) / 256.0f);
			}

			u01 = u11;
			u10 = u00;
			v01 = v00;
			v10 = v11;
			u00 = u01;
			u11 = u10;
			v01 = v11;
			v10 = v00;
		}

		else if (upFlip == FLIP_180)
		{
			u00 = (xt + (1.0 - tt.xx0) * 16.0) / 256.0;
			u11 = (xt + (1.0 - tt.xx1) * 16.0 - 0.01) / 256.0;
			v00 = (yt + (1.0 - tt.zz0) * 16.0) / 256.0;
			v11 = (yt + (1.0 - tt.zz1) * 16.0 - 0.01) / 256.0;

			if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
			{
				u00 = ((xt + 0.0f) / 256.0f);
				u11 = ((xt + 15.99f) / 256.0f);
			}

			if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
			{
				v00 = ((yt + 0.0f) / 256.0f);
				v11 = ((yt + 15.99f) / 256.0f);
			}

			u01 = u11;
			u10 = u00;
			v01 = v00;
			v10 = v11;
		}


		double x0 = x + tt.xx0;
		double x1 = x + tt.xx1;
		double y0 = y + tt.yy0;
		double z0 = z + tt.zz0;
		double z1 = z + tt.zz1;


		if (enableAO)
		{
			t.color(colorR0, colorG0, colorB0);
			t.vertexUV(x0, y0, z1, u10, v10);
			t.color(colorR1, colorG1, colorB1);
			t.vertexUV(x0, y0, z0, u00, v00);
			t.color(colorR2, colorG2, colorB2);
			t.vertexUV(x1, y0, z0, u01, v01);
			t.color(colorR3, colorG3, colorB3);
			t.vertexUV(x1, y0, z1, u11, v11);
		}

		else
		{
			t.vertexUV(x0, y0, z1, u10, v10);
			t.vertexUV(x0, y0, z0, u00, v00);
			t.vertexUV(x1, y0, z0, u01, v01);
			t.vertexUV(x1, y0, z1, u11, v11);
		}
}

void TileRenderer::renderFaceDown(Tile &tt, double x, double y, double z, int_t tex)
	{
		Tesselator &t = Tesselator::instance;

		if (fixedTexture >= 0) tex = fixedTexture;

		int_t xt = (tex & 0xF) << 4;
		int_t yt = tex & 0xF0;

		double u00 = (xt + tt.xx0 * 16.0) / 256.0;
		double u11 = (xt + tt.xx1 * 16.0 - 0.01) / 256.0;
		double v00 = (yt + tt.zz0 * 16.0) / 256.0;
		double v11 = (yt + tt.zz1 * 16.0 - 0.01) / 256.0;

		if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
		{
			u00 = ((xt + 0.0f) / 256.0f);
			u11 = ((xt + 15.99f) / 256.0f);
		}

		if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
		{
			v00 = ((yt + 0.0f) / 256.0f);
			v11 = ((yt + 15.99f) / 256.0f);
		}

		double u01 = u11, u10 = u00, v01 = v00, v10 = v11;

		if (downFlip == FLIP_CW)
		{
			u00 = (xt + (1.0 - tt.zz1) * 16.0) / 256.0;
			v00 = (yt + tt.xx0 * 16.0) / 256.0;
			u11 = (xt + (1.0 - tt.zz0) * 16.0 - 0.01) / 256.0;
			v11 = (yt + tt.xx1 * 16.0 - 0.01) / 256.0;

			if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
			{
				u00 = ((xt + 0.0f) / 256.0f);
				u11 = ((xt + 15.99f) / 256.0f);
			}

			if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
			{
				v00 = ((yt + 0.0f) / 256.0f);
				v11 = ((yt + 15.99f) / 256.0f);
			}

			u01 = u11;
			u10 = u00;
			v01 = v00;
			v10 = v11;
			u00 = u01;
			u11 = u10;
			v01 = v11;
			v10 = v00;
		}

		else if (downFlip == FLIP_CCW)
		{
			u00 = (xt + tt.zz0 * 16.0) / 256.0;
			v00 = (yt + (1.0 - tt.xx1) * 16.0) / 256.0;
			u11 = (xt + tt.zz1 * 16.0 - 0.01) / 256.0;
			v11 = (yt + (1.0 - tt.xx0) * 16.0 - 0.01) / 256.0;

			if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
			{
				u00 = ((xt + 0.0f) / 256.0f);
				u11 = ((xt + 15.99f) / 256.0f);
			}

			if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
			{
				v00 = ((yt + 0.0f) / 256.0f);
				v11 = ((yt + 15.99f) / 256.0f);
			}

			u01 = u11;
			u10 = u00;
			v01 = v00;
			v10 = v11;
			u01 = u00;
			u10 = u11;
			v00 = v11;
			v11 = v01;
		}

		else if (downFlip == FLIP_180)
		{
			u00 = (xt + (1.0 - tt.xx0) * 16.0) / 256.0;
			u11 = (xt + (1.0 - tt.xx1) * 16.0 - 0.01) / 256.0;
			v00 = (yt + (1.0 - tt.zz0) * 16.0) / 256.0;
			v11 = (yt + (1.0 - tt.zz1) * 16.0 - 0.01) / 256.0;

			if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
			{
				u00 = ((xt + 0.0f) / 256.0f);
				u11 = ((xt + 15.99f) / 256.0f);
			}

			if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
			{
				v00 = ((yt + 0.0f) / 256.0f);
				v11 = ((yt + 15.99f) / 256.0f);
			}

			u01 = u11;
			u10 = u00;
			v01 = v00;
			v10 = v11;
		}


		double x0 = x + tt.xx0;
		double x1 = x + tt.xx1;
		double y1 = y + tt.yy1;
		double z0 = z + tt.zz0;
		double z1 = z + tt.zz1;


		if (enableAO)
		{
			t.color(colorR0, colorG0, colorB0);
			t.vertexUV(x1, y1, z1, u11, v11);
			t.color(colorR1, colorG1, colorB1);
			t.vertexUV(x1, y1, z0, u01, v01);
			t.color(colorR2, colorG2, colorB2);
			t.vertexUV(x0, y1, z0, u00, v00);
			t.color(colorR3, colorG3, colorB3);
			t.vertexUV(x0, y1, z1, u10, v10);
		}

		else
		{
			t.vertexUV(x1, y1, z1, u11, v11);
			t.vertexUV(x1, y1, z0, u01, v01);
			t.vertexUV(x0, y1, z0, u00, v00);
			t.vertexUV(x0, y1, z1, u10, v10);
		}
}

void TileRenderer::renderNorth(Tile &tt, double x, double y, double z, int_t tex)
{
	Tesselator &t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int_t xt = (tex & 0xF) << 4;
	int_t yt = tex & 0xF0;

	double u00 = (xt + tt.xx0 * 16.0) / 256.0;
	double u11 = (xt + tt.xx1 * 16.0 - 0.01) / 256.0;
	double v00 = (yt + (1.0 - tt.yy1) * 16.0) / 256.0;
	double v11 = (yt + (1.0 - tt.yy0) * 16.0 - 0.01) / 256.0;
	if (xFlipTexture)
	{
		double tmp = u00;
		u00 = u11;
		u11 = tmp;
	}

	if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
	{
		u00 = ((xt + 0.0f) / 256.0f);
		u11 = ((xt + 15.99f) / 256.0f);
	}
	if (tt.yy0 < 0.0 || tt.yy1 > 1.0)
	{
		v00 = ((yt + 0.0f) / 256.0f);
		v11 = ((yt + 15.99f) / 256.0f);
	}

	double u01 = u11, u10 = u00, v01 = v00, v10 = v11;

	if (northFlip == FLIP_CCW)
	{
		u00 = (xt + tt.yy0 * 16.0) / 256.0;
		v00 = (yt + (1.0 - tt.xx0) * 16.0) / 256.0;
		u11 = (xt + tt.yy1 * 16.0 - 0.01) / 256.0;
		v11 = (yt + (1.0 - tt.xx1) * 16.0 - 0.01) / 256.0;
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if (northFlip == FLIP_CW)
	{
		u00 = (xt + (1.0 - tt.yy1) * 16.0) / 256.0;
		v00 = (yt + tt.xx1 * 16.0) / 256.0;
		u11 = (xt + (1.0 - tt.yy0) * 16.0 - 0.01) / 256.0;
		v11 = (yt + tt.xx0 * 16.0 - 0.01) / 256.0;
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if (northFlip == FLIP_180)
	{
		u00 = (xt + (1.0 - tt.xx0) * 16.0) / 256.0;
		v00 = (yt + tt.yy1 * 16.0) / 256.0;
		u11 = (xt + (1.0 - tt.xx1) * 16.0 - 0.01) / 256.0;
		v11 = (yt + tt.yy0 * 16.0 - 0.01) / 256.0;
		u01 = u11; u10 = u00; v01 = v00; v10 = v11;
	}

	double x0 = x + tt.xx0;
	double x1 = x + tt.xx1;
	double y0 = y + tt.yy0;
	double y1 = y + tt.yy1;
	double z0 = z + tt.zz0;

	if (enableAO)
	{
		t.color(colorR0, colorG0, colorB0);
		t.vertexUV(x0, y1, z0, u01, v01);
		t.color(colorR1, colorG1, colorB1);
		t.vertexUV(x1, y1, z0, u00, v00);
		t.color(colorR2, colorG2, colorB2);
		t.vertexUV(x1, y0, z0, u10, v10);
		t.color(colorR3, colorG3, colorB3);
		t.vertexUV(x0, y0, z0, u11, v11);
	}
	else
	{
		t.vertexUV(x0, y1, z0, u01, v01);
		t.vertexUV(x1, y1, z0, u00, v00);
		t.vertexUV(x1, y0, z0, u10, v10);
		t.vertexUV(x0, y0, z0, u11, v11);
	}
}

void TileRenderer::renderSouth(Tile &tt, double x, double y, double z, int_t tex)
{
	Tesselator &t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int_t xt = (tex & 0xF) << 4;
	int_t yt = tex & 0xF0;

	double u00 = (xt + tt.xx0 * 16.0) / 256.0;
	double u11 = (xt + tt.xx1 * 16.0 - 0.01) / 256.0;
	double v00 = (yt + (1.0 - tt.yy1) * 16.0) / 256.0;
	double v11 = (yt + (1.0 - tt.yy0) * 16.0 - 0.01) / 256.0;
	if (xFlipTexture)
	{
		double tmp = u00;
		u00 = u11;
		u11 = tmp;
	}

	if (tt.xx0 < 0.0 || tt.xx1 > 1.0)
	{
		u00 = ((xt + 0.0f) / 256.0f);
		u11 = ((xt + 15.99f) / 256.0f);
	}
	if (tt.yy0 < 0.0 || tt.yy1 > 1.0)
	{
		v00 = ((yt + 0.0f) / 256.0f);
		v11 = ((yt + 15.99f) / 256.0f);
	}

	double u01 = u11, u10 = u00, v01 = v00, v10 = v11;

	if (southFlip == FLIP_CCW)
	{
		u00 = (xt + (1.0 - tt.yy1) * 16.0) / 256.0;
		v00 = (yt + tt.xx0 * 16.0) / 256.0;
		u11 = (xt + (1.0 - tt.yy0) * 16.0 - 0.01) / 256.0;
		v11 = (yt + tt.xx1 * 16.0 - 0.01) / 256.0;
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if (southFlip == FLIP_CW)
	{
		u00 = (xt + tt.yy0 * 16.0) / 256.0;
		v11 = (yt + (1.0 - tt.xx0) * 16.0 - 0.01) / 256.0;
		u11 = (xt + tt.yy1 * 16.0 - 0.01) / 256.0;
		v00 = (yt + (1.0 - tt.xx1) * 16.0) / 256.0;
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if (southFlip == FLIP_180)
	{
		u00 = (xt + (1.0 - tt.xx0) * 16.0) / 256.0;
		v00 = (yt + tt.yy1 * 16.0) / 256.0;
		u11 = (xt + (1.0 - tt.xx1) * 16.0 - 0.01) / 256.0;
		v11 = (yt + tt.yy0 * 16.0 - 0.01) / 256.0;
		u01 = u11; u10 = u00; v01 = v00; v10 = v11;
	}

	double x0 = x + tt.xx0;
	double x1 = x + tt.xx1;
	double y0 = y + tt.yy0;
	double y1 = y + tt.yy1;
	double z1 = z + tt.zz1;

	if (enableAO)
	{
		t.color(colorR0, colorG0, colorB0);
		t.vertexUV(x0, y1, z1, u00, v00);
		t.color(colorR1, colorG1, colorB1);
		t.vertexUV(x0, y0, z1, u10, v10);
		t.color(colorR2, colorG2, colorB2);
		t.vertexUV(x1, y0, z1, u11, v11);
		t.color(colorR3, colorG3, colorB3);
		t.vertexUV(x1, y1, z1, u01, v01);
	}
	else
	{
		t.vertexUV(x0, y1, z1, u00, v00);
		t.vertexUV(x0, y0, z1, u10, v10);
		t.vertexUV(x1, y0, z1, u11, v11);
		t.vertexUV(x1, y1, z1, u01, v01);
	}
}

void TileRenderer::renderWest(Tile &tt, double x, double y, double z, int_t tex)
{
	Tesselator &t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int_t xt = (tex & 0xF) << 4;
	int_t yt = tex & 0xF0;

	double u00 = (xt + tt.zz0 * 16.0) / 256.0;
	double u11 = (xt + tt.zz1 * 16.0 - 0.01) / 256.0;
	double v00 = (yt + (1.0 - tt.yy1) * 16.0) / 256.0;
	double v11 = (yt + (1.0 - tt.yy0) * 16.0 - 0.01) / 256.0;
	if (xFlipTexture)
	{
		double tmp = u00;
		u00 = u11;
		u11 = tmp;
	}

	if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
	{
		u00 = ((xt + 0.0f) / 256.0f);
		u11 = ((xt + 15.99f) / 256.0f);
	}
	if (tt.yy0 < 0.0 || tt.yy1 > 1.0)
	{
		v00 = ((yt + 0.0f) / 256.0f);
		v11 = ((yt + 15.99f) / 256.0f);
	}

	double u01 = u11, u10 = u00, v01 = v00, v10 = v11;

	if (westFlip == FLIP_CCW)
	{
		u00 = (xt + (1.0 - tt.yy1) * 16.0) / 256.0;
		v00 = (yt + tt.zz0 * 16.0) / 256.0;
		u11 = (xt + (1.0 - tt.yy0) * 16.0 - 0.01) / 256.0;
		v11 = (yt + tt.zz1 * 16.0 - 0.01) / 256.0;
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if (westFlip == FLIP_CW)
	{
		u00 = (xt + tt.yy0 * 16.0) / 256.0;
		v00 = (yt + (1.0 - tt.zz1) * 16.0) / 256.0;
		u11 = (xt + tt.yy1 * 16.0 - 0.01) / 256.0;
		v11 = (yt + (1.0 - tt.zz0) * 16.0 - 0.01) / 256.0;
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if (westFlip == FLIP_180)
	{
		u00 = (xt + (1.0 - tt.zz0) * 16.0) / 256.0;
		v00 = (yt + tt.yy1 * 16.0) / 256.0;
		u11 = (xt + (1.0 - tt.zz1) * 16.0 - 0.01) / 256.0;
		v11 = (yt + tt.yy0 * 16.0 - 0.01) / 256.0;
		u01 = u11; u10 = u00; v01 = v00; v10 = v11;
	}

	double x0 = x + tt.xx0;
	double y0 = y + tt.yy0;
	double y1 = y + tt.yy1;
	double z0 = z + tt.zz0;
	double z1 = z + tt.zz1;

	if (enableAO)
	{
		t.color(colorR0, colorG0, colorB0);
		t.vertexUV(x0, y1, z1, u01, v01);
		t.color(colorR1, colorG1, colorB1);
		t.vertexUV(x0, y1, z0, u00, v00);
		t.color(colorR2, colorG2, colorB2);
		t.vertexUV(x0, y0, z0, u10, v10);
		t.color(colorR3, colorG3, colorB3);
		t.vertexUV(x0, y0, z1, u11, v11);
	}
	else
	{
		t.vertexUV(x0, y1, z1, u01, v01);
		t.vertexUV(x0, y1, z0, u00, v00);
		t.vertexUV(x0, y0, z0, u10, v10);
		t.vertexUV(x0, y0, z1, u11, v11);
	}
}

void TileRenderer::renderEast(Tile &tt, double x, double y, double z, int_t tex)
{
	Tesselator &t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int_t xt = (tex & 0xF) << 4;
	int_t yt = tex & 0xF0;

	double u00 = (xt + tt.zz0 * 16.0) / 256.0;
	double u11 = (xt + tt.zz1 * 16.0 - 0.01) / 256.0;
	double v00 = (yt + (1.0 - tt.yy1) * 16.0) / 256.0;
	double v11 = (yt + (1.0 - tt.yy0) * 16.0 - 0.01) / 256.0;
	if (xFlipTexture)
	{
		double tmp = u00;
		u00 = u11;
		u11 = tmp;
	}

	if (tt.zz0 < 0.0 || tt.zz1 > 1.0)
	{
		u00 = ((xt + 0.0f) / 256.0f);
		u11 = ((xt + 15.99f) / 256.0f);
	}
	if (tt.yy0 < 0.0 || tt.yy1 > 1.0)
	{
		v00 = ((yt + 0.0f) / 256.0f);
		v11 = ((yt + 15.99f) / 256.0f);
	}

	double u01 = u11, u10 = u00, v01 = v00, v10 = v11;

	if (eastFlip == FLIP_CCW)
	{
		u00 = (xt + tt.yy0 * 16.0) / 256.0;
		v00 = (yt + (1.0 - tt.zz0) * 16.0) / 256.0;
		u11 = (xt + tt.yy1 * 16.0 - 0.01) / 256.0;
		v11 = (yt + (1.0 - tt.zz1) * 16.0 - 0.01) / 256.0;
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u01 = u00;
		u10 = u11;
		v00 = v11;
		v11 = v01;
	}
	else if (eastFlip == FLIP_CW)
	{
		u00 = (xt + (1.0 - tt.yy1) * 16.0) / 256.0;
		v00 = (yt + tt.zz1 * 16.0) / 256.0;
		u11 = (xt + (1.0 - tt.yy0) * 16.0 - 0.01) / 256.0;
		v11 = (yt + tt.zz0 * 16.0 - 0.01) / 256.0;
		u01 = u11;
		u10 = u00;
		v01 = v00;
		v10 = v11;
		u00 = u01;
		u11 = u10;
		v01 = v11;
		v10 = v00;
	}
	else if (eastFlip == FLIP_180)
	{
		u00 = (xt + (1.0 - tt.zz0) * 16.0) / 256.0;
		v00 = (yt + tt.yy1 * 16.0) / 256.0;
		u11 = (xt + (1.0 - tt.zz1) * 16.0 - 0.01) / 256.0;
		v11 = (yt + tt.yy0 * 16.0 - 0.01) / 256.0;
		u01 = u11; u10 = u00; v01 = v00; v10 = v11;
	}

	double x1 = x + tt.xx1;
	double y0 = y + tt.yy0;
	double y1 = y + tt.yy1;
	double z0 = z + tt.zz0;
	double z1 = z + tt.zz1;

	if (enableAO)
	{
		t.color(colorR0, colorG0, colorB0);
		t.vertexUV(x1, y0, z1, u10, v10);
		t.color(colorR1, colorG1, colorB1);
		t.vertexUV(x1, y0, z0, u11, v11);
		t.color(colorR2, colorG2, colorB2);
		t.vertexUV(x1, y1, z0, u01, v01);
		t.color(colorR3, colorG3, colorB3);
		t.vertexUV(x1, y1, z1, u00, v00);
	}
	else
	{
		t.vertexUV(x1, y0, z1, u10, v10);
		t.vertexUV(x1, y0, z0, u11, v11);
		t.vertexUV(x1, y1, z0, u01, v01);
		t.vertexUV(x1, y1, z1, u00, v00);
	}
}

void TileRenderer::renderCube(Tile &tile, float alpha)
{
	int shape = tile.getRenderShape();
	Tesselator &t = Tesselator::instance;
	

	if (shape == Tile::SHAPE_BLOCK)
	{
		tile.updateDefaultShape();
		glTranslatef(-0.5f, -0.5f, -0.5f);
		
		float sd = 0.5f;
		float su = 1.0f;
		float sns = 0.8f;
		float sew = 0.6f;
		
		t.begin();
		
		t.color(su, su, su, alpha);
		renderFaceUp(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::DOWN));
		t.color(sd, sd, sd, alpha);
		renderFaceDown(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::UP));
		t.color(sns, sns, sns, alpha);
		renderNorth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::NORTH));
		renderSouth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::SOUTH));
		t.color(sew, sew, sew, alpha);
		renderWest(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::WEST));
		renderEast(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::EAST));
		
		t.end();
		
		glTranslatef(0.5f, 0.5f, 0.5f);
	}
	else if (shape == Tile::SHAPE_CACTUS)
	{
		tile.updateDefaultShape();
		float s = 1.0f / 16.0f;
		glTranslatef(-0.5f, -0.5f, -0.5f);
		
		t.begin();
		t.normal(0.0f, -1.0f, 0.0f);
		renderFaceUp(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::DOWN));
		t.end();
		
		t.begin();
		t.normal(0.0f, 1.0f, 0.0f);
		renderFaceDown(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::UP));
		t.end();
		
		t.begin();
		t.normal(0.0f, 0.0f, -1.0f);
		t.addOffset(0.0f, 0.0f, s);
		renderNorth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::NORTH));
		t.addOffset(0.0f, 0.0f, -s);
		t.end();
		
		t.begin();
		t.normal(0.0f, 0.0f, 1.0f);
		t.addOffset(0.0f, 0.0f, -s);
		renderSouth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::SOUTH));
		t.addOffset(0.0f, 0.0f, s);
		t.end();
		
		t.begin();
		t.normal(-1.0f, 0.0f, 0.0f);
		t.addOffset(s, 0.0f, 0.0f);
		renderWest(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::WEST));
		t.addOffset(-s, 0.0f, 0.0f);
		t.end();
		
		t.begin();
		t.normal(1.0f, 0.0f, 0.0f);
		t.addOffset(-s, 0.0f, 0.0f);
		renderEast(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::EAST));
		t.addOffset(s, 0.0f, 0.0f);
		t.end();
		
		glTranslatef(0.5f, 0.5f, 0.5f);
	}
	
}

void TileRenderer::renderTile(Tile &tile, int_t data)
	{
	Tesselator &t = Tesselator::instance;
	if (tile.id == 23 || tile.id == 61 || tile.id == 62)
		data = static_cast<int_t>(Facing::SOUTH);
	int_t shape = tile.getRenderShape();
	tile.updateDefaultShape();

		if (shape == Tile::SHAPE_BLOCK)
		{
			glTranslatef(-0.5f, -0.5f, -0.5f);
			t.begin();
			t.normal(0.0f, -1.0f, 0.0f);
			renderFaceUp(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::DOWN, data));
			t.end();
			t.begin();
			t.normal(0.0f, 1.0f, 0.0f);
			renderFaceDown(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::UP, data));
			t.end();
			t.begin();
			t.normal(0.0f, 0.0f, -1.0f);
			renderNorth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::NORTH, data));
			t.end();
			t.begin();
			t.normal(0.0f, 0.0f, 1.0f);
			renderSouth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::SOUTH, data));
			t.end();
			t.begin();
			t.normal(-1.0f, 0.0f, 0.0f);
			renderWest(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::WEST, data));
			t.end();
			t.begin();
			t.normal(1.0f, 0.0f, 0.0f);
			renderEast(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::EAST, data));
			t.end();
			glTranslatef(0.5f, 0.5f, 0.5f);
		}
		else if (shape == Tile::SHAPE_CACTUS)
		{
			float s = 1.0f / 16.0f;
			glTranslatef(-0.5f, -0.5f, -0.5f);
			t.begin();
			t.normal(0.0f, -1.0f, 0.0f);
			renderFaceUp(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::DOWN));
			t.end();
			t.begin();
			t.normal(0.0f, 1.0f, 0.0f);
			renderFaceDown(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::UP));
			t.end();
			t.begin();
			t.normal(0.0f, 0.0f, -1.0f);
			t.addOffset(0.0f, 0.0f, s);
			renderNorth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::NORTH));
			t.addOffset(0.0f, 0.0f, -s);
			t.end();
			t.begin();
			t.normal(0.0f, 0.0f, 1.0f);
			t.addOffset(0.0f, 0.0f, -s);
			renderSouth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::SOUTH));
			t.addOffset(0.0f, 0.0f, s);
			t.end();
			t.begin();
			t.normal(-1.0f, 0.0f, 0.0f);
			t.addOffset(s, 0.0f, 0.0f);
			renderWest(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::WEST));
			t.addOffset(-s, 0.0f, 0.0f);
			t.end();
			t.begin();
			t.normal(1.0f, 0.0f, 0.0f);
			t.addOffset(-s, 0.0f, 0.0f);
			renderEast(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::EAST));
			t.addOffset(s, 0.0f, 0.0f);
			t.end();
			glTranslatef(0.5f, 0.5f, 0.5f);
		}
		else if (shape == Tile::SHAPE_TORCH)
		{
			int_t orient = data & 7;
			float w = 0.0625f * 2.0f; 
			float h = 0.625f;        

			float x0, x1, z0, z1, yBot, yTop;

			if (orient == 1) 
			{
				x0 = 0.0f; x1 = w;
				z0 = 0.5f - w; z1 = 0.5f + w;
				yBot = 0.2f; yTop = yBot + h * 10.0f / 16.0f;
			}
			else if (orient == 2) 
			{
				x0 = 1.0f - w; x1 = 1.0f;
				z0 = 0.5f - w; z1 = 0.5f + w;
				yBot = 0.2f; yTop = yBot + h * 10.0f / 16.0f;
			}
			else if (orient == 3) 
			{
				x0 = 0.5f - w; x1 = 0.5f + w;
				z0 = 0.0f; z1 = w;
				yBot = 0.2f; yTop = yBot + h * 10.0f / 16.0f;
			}
			else if (orient == 4) 
			{
				x0 = 0.5f - w; x1 = 0.5f + w;
				z0 = 1.0f - w; z1 = 1.0f;
				yBot = 0.2f; yTop = yBot + h * 10.0f / 16.0f;
			}
			else 
			{
				x0 = 0.5f - w; x1 = 0.5f + w;
				z0 = 0.5f - w; z1 = 0.5f + w;
				yBot = 0.0f; yTop = 0.625f;
			}

			int_t tex = tile.getTexture(Facing::NORTH, data);

			glTranslatef(-0.5f, -0.5f, -0.5f);

			
			t.begin();
			tile.setShape(x0, yBot, z0, x1, yTop, z1);
			renderFaceUp(tile, 0.0, 0.0, 0.0, tex);
			t.end();

			
			t.begin();
			renderFaceDown(tile, 0.0, 0.0, 0.0, tex);
			t.end();

			
			t.begin();
			renderNorth(tile, 0.0, 0.0, 0.0, tex);
			t.end();

			
			t.begin();
			renderSouth(tile, 0.0, 0.0, 0.0, tex);
			t.end();

			
			t.begin();
			renderWest(tile, 0.0, 0.0, 0.0, tex);
			t.end();

			
			t.begin();
			renderEast(tile, 0.0, 0.0, 0.0, tex);
			t.end();

			tile.setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
			glTranslatef(0.5f, 0.5f, 0.5f);
		}
		else if (shape == Tile::SHAPE_FENCE)
		{
			for (int_t piece = 0; piece < 4; ++piece)
			{
				float width = 2.0f / 16.0f;
				if (piece == 0)
					tile.setShape(0.5f - width, 0.0f, 0.0f,
						0.5f + width, 1.0f, width * 2.0f);
				if (piece == 1)
					tile.setShape(0.5f - width, 0.0f, 1.0f - width * 2.0f,
						0.5f + width, 1.0f, 1.0f);
				width = 1.0f / 16.0f;
				if (piece == 2)
					tile.setShape(0.5f - width, 1.0f - width * 3.0f, -width * 2.0f,
						0.5f + width, 1.0f - width, 1.0f + width * 2.0f);
				if (piece == 3)
					tile.setShape(0.5f - width, 0.5f - width * 3.0f, -width * 2.0f,
						0.5f + width, 0.5f - width, 1.0f + width * 2.0f);

				glTranslatef(-0.5f, -0.5f, -0.5f);
				t.begin();
				t.normal(0.0f, -1.0f, 0.0f);
				renderFaceUp(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::DOWN));
				t.end();
				t.begin();
				t.normal(0.0f, 1.0f, 0.0f);
				renderFaceDown(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::UP));
				t.end();
				t.begin();
				t.normal(0.0f, 0.0f, -1.0f);
				renderNorth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::NORTH));
				t.end();
				t.begin();
				t.normal(0.0f, 0.0f, 1.0f);
				renderSouth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::SOUTH));
				t.end();
				t.begin();
				t.normal(-1.0f, 0.0f, 0.0f);
				renderWest(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::WEST));
				t.end();
				t.begin();
				t.normal(1.0f, 0.0f, 0.0f);
				renderEast(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::EAST));
				t.end();
				glTranslatef(0.5f, 0.5f, 0.5f);
			}
			tile.setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		}
		else if (shape == Tile::SHAPE_STAIRS)
		{
			glTranslatef(-0.5f, -0.5f, -0.5f);
			for (int_t piece = 0; piece < 2; ++piece)
			{
				StairTile::setPieceShape(tile, data & 3, piece);
				t.begin();
				t.normal(0.0f, -1.0f, 0.0f);
				renderFaceUp(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::DOWN, data));
				t.end();
				t.begin();
				t.normal(0.0f, 1.0f, 0.0f);
				renderFaceDown(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::UP, data));
				t.end();
				t.begin();
				t.normal(0.0f, 0.0f, -1.0f);
				renderNorth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::NORTH, data));
				t.end();
				t.begin();
				t.normal(0.0f, 0.0f, 1.0f);
				renderSouth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::SOUTH, data));
				t.end();
				t.begin();
				t.normal(-1.0f, 0.0f, 0.0f);
				renderWest(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::WEST, data));
				t.end();
				t.begin();
				t.normal(1.0f, 0.0f, 0.0f);
				renderEast(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::EAST, data));
				t.end();
			}
			tile.updateDefaultShape();
			glTranslatef(0.5f, 0.5f, 0.5f);
		}
		else if (shape == Tile::SHAPE_PISTON_BASE)
		{
			int_t pistonData = (tile.id == Tile::pistonBase.id || tile.id == Tile::pistonStickyBase.id) ? 1 : data;
			glTranslatef(-0.5f, -0.5f, -0.5f);
			t.begin();
			t.normal(0.0f, -1.0f, 0.0f);
			renderFaceUp(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::DOWN, pistonData));
			t.end();
			t.begin();
			t.normal(0.0f, 1.0f, 0.0f);
			renderFaceDown(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::UP, pistonData));
			t.end();
			t.begin();
			t.normal(0.0f, 0.0f, -1.0f);
			renderNorth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::NORTH, pistonData));
			t.end();
			t.begin();
			t.normal(0.0f, 0.0f, 1.0f);
			renderSouth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::SOUTH, pistonData));
			t.end();
			t.begin();
			t.normal(-1.0f, 0.0f, 0.0f);
			renderWest(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::WEST, pistonData));
			t.end();
			t.begin();
			t.normal(1.0f, 0.0f, 0.0f);
			renderEast(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::EAST, pistonData));
			t.end();
			glTranslatef(0.5f, 0.5f, 0.5f);
		}
		else if (shape == Tile::SHAPE_PISTON_EXTENSION)
		{
			glTranslatef(-0.5f, -0.5f, -0.5f);
			t.begin();
			t.normal(0.0f, -1.0f, 0.0f);
			renderFaceUp(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::DOWN, data));
			t.end();
			t.begin();
			t.normal(0.0f, 1.0f, 0.0f);
			renderFaceDown(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::UP, data));
			t.end();
			t.begin();
			t.normal(0.0f, 0.0f, -1.0f);
			renderNorth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::NORTH, data));
			t.end();
			t.begin();
			t.normal(0.0f, 0.0f, 1.0f);
			renderSouth(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::SOUTH, data));
			t.end();
			t.begin();
			t.normal(-1.0f, 0.0f, 0.0f);
			renderWest(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::WEST, data));
			t.end();
			t.begin();
			t.normal(1.0f, 0.0f, 0.0f);
			renderEast(tile, 0.0, 0.0, 0.0, tile.getTexture(Facing::EAST, data));
			t.end();
			glTranslatef(0.5f, 0.5f, 0.5f);
		}
	}

bool TileRenderer::tesselateBedInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	Tesselator &t = Tesselator::instance;

	int_t data = level->getData(x, y, z);
	int_t direction = BedTile::getDirectionFromMetadata(data);
	bool isHead = !BedTile::isBlockFootOfBed(data);

	float c10 = 0.5f;
	float c11 = 1.0f;
	float c2 = 0.8f;
	float c3 = 0.6f;

	float centerBrightness = tt.getBrightness(*level, x, y, z);

	
	{
		t.color(c10 * centerBrightness, c10 * centerBrightness, c10 * centerBrightness);
		int_t tex = tt.getTexture(*level, x, y, z, Facing::DOWN);
		int_t xt = (tex & 0xF) << 4;
		int_t yt = tex & 0xF0;
		double u0 = static_cast<double>(xt) / 256.0;
		double u1 = static_cast<double>(xt + 16 - 0.01) / 256.0;
		double v0 = static_cast<double>(yt) / 256.0;
		double v1 = static_cast<double>(yt + 16 - 0.01) / 256.0;

		float x0 = static_cast<float>(x + tt.xx0);
		float x1 = static_cast<float>(x + tt.xx1);
		float y0 = static_cast<float>(y + tt.yy0 + 3.0 / 16.0);
		float z0 = static_cast<float>(z + tt.zz0);
		float z1 = static_cast<float>(z + tt.zz1);

		t.vertexUV(x0, y0, z1, u0, v1);
		t.vertexUV(x0, y0, z0, u0, v0);
		t.vertexUV(x1, y0, z0, u1, v0);
		t.vertexUV(x1, y0, z1, u1, v1);
	}

	
	{
		float topBrightness = tt.getBrightness(*level, x, y + 1, z);
		t.color(c11 * topBrightness, c11 * topBrightness, c11 * topBrightness);
		int_t tex = tt.getTexture(*level, x, y, z, Facing::UP);
		int_t xt = (tex & 0xF) << 4;
		int_t yt = tex & 0xF0;
		double u0 = static_cast<double>(xt) / 256.0;
		double u1 = static_cast<double>(xt + 16 - 0.01) / 256.0;
		double v0 = static_cast<double>(yt) / 256.0;
		double v1 = static_cast<double>(yt + 16 - 0.01) / 256.0;

		double topLeftU = u0, topRightU = u1;
		double topLeftV = v0, topRightV = v0;
		double bottomLeftU = u0, bottomRightU = u1;
		double bottomLeftV = v1, bottomRightV = v1;

		if (direction == 0) 
		{
			topRightU = u0;
			topLeftV = v1;
			bottomLeftU = u1;
			bottomRightV = v0;
		}
		else if (direction == 2) 
		{
			topLeftU = u1;
			topRightV = v1;
			bottomRightU = u0;
			bottomLeftV = v0;
		}
		else if (direction == 3) 
		{
			topLeftU = u1;
			topRightV = v1;
			bottomRightU = u0;
			bottomLeftV = v0;
			topRightU = u0;
			topLeftV = v1;
			bottomLeftU = u1;
			bottomRightV = v0;
		}

		float x0 = static_cast<float>(x + tt.xx0);
		float x1 = static_cast<float>(x + tt.xx1);
		float y1 = static_cast<float>(y + tt.yy1);
		float z0 = static_cast<float>(z + tt.zz0);
		float z1 = static_cast<float>(z + tt.zz1);

		t.vertexUV(x1, y1, z1, bottomLeftU, bottomLeftV);
		t.vertexUV(x1, y1, z0, topLeftU, topLeftV);
		t.vertexUV(x0, y1, z0, topRightU, topRightV);
		t.vertexUV(x0, y1, z1, bottomRightU, bottomRightV);
	}

	
	
	
	
	static constexpr int directionToFacing[4] = {3, 4, 2, 5};
	static constexpr int directionOpposite[4] = {2, 3, 0, 1};
	int_t skipEdge = directionToFacing[directionOpposite[direction]];
	if (isHead)
		skipEdge = directionToFacing[direction];

	
	int_t flipEdge = 4; 
	switch (direction)
	{
		case 0: flipEdge = 5; break; 
		case 1: flipEdge = 3; break; 
		case 2: flipEdge = 4; break; 
		case 3: flipEdge = 2; break; 
	}

	if (skipEdge != 2 && (noCulling || tt.shouldRenderFace(*level, x, y, z - 1, Facing::NORTH)))
	{
		float br = tt.getBrightness(*level, x, y, z - 1);
		t.color(c2 * br, c2 * br, c2 * br);
		xFlipTexture = flipEdge == 2;
		renderNorth(tt, x, y, z, tt.getTexture(*level, x, y, z, Facing::NORTH));
	}

	if (skipEdge != 3 && (noCulling || tt.shouldRenderFace(*level, x, y, z + 1, Facing::SOUTH)))
	{
		float br = tt.getBrightness(*level, x, y, z + 1);
		t.color(c2 * br, c2 * br, c2 * br);
		xFlipTexture = flipEdge == 3;
		renderSouth(tt, x, y, z, tt.getTexture(*level, x, y, z, Facing::SOUTH));
	}

	if (skipEdge != 4 && (noCulling || tt.shouldRenderFace(*level, x - 1, y, z, Facing::WEST)))
	{
		float br = tt.getBrightness(*level, x - 1, y, z);
		t.color(c3 * br, c3 * br, c3 * br);
		xFlipTexture = flipEdge == 4;
		renderWest(tt, x, y, z, tt.getTexture(*level, x, y, z, Facing::WEST));
	}

	if (skipEdge != 5 && (noCulling || tt.shouldRenderFace(*level, x + 1, y, z, Facing::EAST)))
	{
		float br = tt.getBrightness(*level, x + 1, y, z);
		t.color(c3 * br, c3 * br, c3 * br);
		xFlipTexture = flipEdge == 5;
		renderEast(tt, x, y, z, tt.getTexture(*level, x, y, z, Facing::EAST));
	}

	xFlipTexture = false;
	return true;
}

void TileRenderer::renderGuiTile(Tile &tile, int_t data)
{
	if (tile.id == 23 || tile.id == 61 || tile.id == 62)
		data = static_cast<int_t>(Facing::SOUTH);
	renderTile(tile, data);
}

bool TileRenderer::canRender(int_t renderShape)
	{
		return renderShape == Tile::SHAPE_BLOCK || renderShape == Tile::SHAPE_CACTUS ||
			renderShape == Tile::SHAPE_STAIRS || renderShape == Tile::SHAPE_FENCE ||
			renderShape == Tile::SHAPE_PISTON_BASE;
	}


float TileRenderer::getLiquidHeight(int_t x, int_t y, int_t z, const Material &material)
{
	int_t count = 0;
	float total = 0.0f;

	for (int_t i = 0; i < 4; ++i)
	{
		int_t bx = x - (i & 1);
		int_t bz = z - (i >> 1 & 1);

		if (&level->getMaterial(bx, y + 1, bz) == &material)
			return 1.0f;

		const Material &mat = level->getMaterial(bx, y, bz);
		if (&mat != &material)
		{
			if (!mat.isSolid())
			{
				++total;
				++count;
			}
		}
		else
		{
			int_t data = level->getData(bx, y, bz);
			if (data >= 8 || data == 0)
			{
				total += LiquidTile::getHeight(data) * 10.0f;
				count += 10;
			}
			total += LiquidTile::getHeight(data);
			++count;
		}
	}

	return 1.0f - total / static_cast<float>(count);
}

bool TileRenderer::tesselateLiquidInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	Tesselator &t = Tesselator::instance;

	bool renderTop = noCulling || tt.shouldRenderFace(*level, x, y + 1, z, Facing::UP);
	bool renderBot = noCulling || tt.shouldRenderFace(*level, x, y - 1, z, Facing::DOWN);
	bool renderSides[4] = {
		noCulling || tt.shouldRenderFace(*level, x, y, z - 1, Facing::NORTH),
		noCulling || tt.shouldRenderFace(*level, x, y, z + 1, Facing::SOUTH),
		noCulling || tt.shouldRenderFace(*level, x - 1, y, z, Facing::WEST),
		noCulling || tt.shouldRenderFace(*level, x + 1, y, z, Facing::EAST)
	};

	if (!renderTop && !renderBot && !renderSides[0] && !renderSides[1] && !renderSides[2] && !renderSides[3])
		return false;

	bool changed = false;
	const Material &material = tt.material;
	int_t data = level->getData(x, y, z);

	float h00 = getLiquidHeight(x, y, z, material);
	float h01 = getLiquidHeight(x, y, z + 1, material);
	float h11 = getLiquidHeight(x + 1, y, z + 1, material);
	float h10 = getLiquidHeight(x + 1, y, z, material);

	if (renderTop)
	{
		changed = true;
		int_t texTop = tt.getTexture(Facing::UP);
		float angle = static_cast<float>(LiquidTile::getSlopeAngle(*level, x, y, z, material));
		if (angle > -999.0f)
			texTop = tt.getTexture(Facing::NORTH);
		int_t xt = (texTop & 0xF) << 4;
		int_t yt = texTop & 0xF0;
		double u = (static_cast<double>(xt) + 8.0) / 256.0;
		double v = (static_cast<double>(yt) + 8.0) / 256.0;
		if (angle < -999.0f)
		{
			angle = 0.0f;
		}
		else
		{
			u = (static_cast<double>(xt) + 16.0) / 256.0;
			v = (static_cast<double>(yt) + 16.0) / 256.0;
		}
		float s = Mth::sin(angle) * 8.0f / 256.0f;
		float c = Mth::cos(angle) * 8.0f / 256.0f;

		float bright = tt.getBrightness(*level, x, y, z);
		t.color(bright, bright, bright);

		t.vertexUV(x + 0.0, y + h00, z + 0.0, u - c - s, v - c + s);
		t.vertexUV(x + 0.0, y + h01, z + 1.0, u - c + s, v + c + s);
		t.vertexUV(x + 1.0, y + h11, z + 1.0, u + c + s, v + c - s);
		t.vertexUV(x + 1.0, y + h10, z + 0.0, u + c - s, v - c - s);
	}

	if (renderBot)
	{
		changed = true;
		float bright = tt.getBrightness(*level, x, y - 1, z);
		t.color(0.5f * bright, 0.5f * bright, 0.5f * bright);
		renderFaceUp(tt, static_cast<double>(x), static_cast<double>(y), static_cast<double>(z), tt.getTexture(Facing::DOWN));
	}

	for (int_t side = 0; side < 4; ++side)
	{
		if (!renderSides[side])
			continue;

		changed = true;

		int_t sx = x, sz = z;
		float sH0, sH1;
		float sx0, sx1, sz0, sz1;

		if (side == 0)
		{
			sH0 = h00; sH1 = h10;
			sx0 = static_cast<float>(x); sx1 = static_cast<float>(x + 1);
			sz0 = static_cast<float>(z); sz1 = static_cast<float>(z);
			sz = z - 1;
		}
		else if (side == 1)
		{
			sH0 = h11; sH1 = h01;
			sx0 = static_cast<float>(x + 1); sx1 = static_cast<float>(x);
			sz0 = static_cast<float>(z + 1); sz1 = static_cast<float>(z + 1);
			sz = z + 1;
		}
		else if (side == 2)
		{
			sH0 = h01; sH1 = h00;
			sx0 = static_cast<float>(x); sx1 = static_cast<float>(x);
			sz0 = static_cast<float>(z + 1); sz1 = static_cast<float>(z);
			sx = x - 1;
		}
		else
		{
			sH0 = h10; sH1 = h11;
			sx0 = static_cast<float>(x + 1); sx1 = static_cast<float>(x + 1);
			sz0 = static_cast<float>(z); sz1 = static_cast<float>(z + 1);
			sx = x + 1;
		}

		int_t texSide = tt.getTexture(static_cast<Facing>(side + 2));
		int_t txi = (texSide & 0xF) << 4;
		int_t tyi = texSide & 0xF0;
		double u0 = static_cast<double>(txi) / 256.0;
		double u1 = (static_cast<double>(txi) + 16.0 - 0.01) / 256.0;
		double v0 = (static_cast<double>(tyi) + (1.0f - sH0) * 16.0f) / 256.0;
		double v1 = (static_cast<double>(tyi) + (1.0f - sH1) * 16.0f) / 256.0;
		double vBot = (static_cast<double>(tyi) + 16.0 - 0.01) / 256.0;

		float bright = tt.getBrightness(*level, sx, y, sz);
		float shade = side < 2 ? 0.8f : 0.6f;
		t.color(shade * bright, shade * bright, shade * bright);

		t.vertexUV(sx0, y + sH0, sz0, u0, v0);
		t.vertexUV(sx1, y + sH1, sz1, u1, v1);
		t.vertexUV(sx1, y + 0.0, sz1, u1, vBot);
		t.vertexUV(sx0, y + 0.0, sz0, u0, vBot);
	}

	return changed;
}

bool TileRenderer::tesselateTorchInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	int_t dir = level->getData(x, y, z);
	Tesselator &t = Tesselator::instance;
	float br = tt.getBrightness(*level, x, y, z);
	if (Tile::lightEmission[tt.id] > 0)
		br = 1.0f;

	t.color(br, br, br);
	double r = 0.4;
	double r2 = 0.5 - r;
	double h = 0.2;
	if (dir == 1)
		tesselateTorch(tt, static_cast<double>(x) - r2, static_cast<double>(y) + h, static_cast<double>(z), -r, 0.0);
	else if (dir == 2)
		tesselateTorch(tt, static_cast<double>(x) + r2, static_cast<double>(y) + h, static_cast<double>(z), r, 0.0);
	else if (dir == 3)
		tesselateTorch(tt, static_cast<double>(x), static_cast<double>(y) + h, static_cast<double>(z) - r2, 0.0, -r);
	else if (dir == 4)
		tesselateTorch(tt, static_cast<double>(x), static_cast<double>(y) + h, static_cast<double>(z) + r2, 0.0, r);
	else
		tesselateTorch(tt, static_cast<double>(x), static_cast<double>(y), static_cast<double>(z), 0.0, 0.0);
	return true;
}


bool TileRenderer::tesselateFenceInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	bool changed = false;

	float postLo = 6.0f / 16.0f;
	float postHi = 10.0f / 16.0f;
	tt.setShape(postLo, 0.0f, postLo, postHi, 1.0f, postHi);
	tesselateBlockInWorld(tt, x, y, z);
	changed = true;

	bool connectX = false;
	bool connectZ = false;
	if (level->getTile(x - 1, y, z) == tt.id || level->getTile(x + 1, y, z) == tt.id)
		connectX = true;
	if (level->getTile(x, y, z - 1) == tt.id || level->getTile(x, y, z + 1) == tt.id)
		connectZ = true;

	bool west = level->getTile(x - 1, y, z) == tt.id;
	bool east = level->getTile(x + 1, y, z) == tt.id;
	bool north = level->getTile(x, y, z - 1) == tt.id;
	bool south = level->getTile(x, y, z + 1) == tt.id;
	if (!connectX && !connectZ)
		connectX = true;

	float railLo = 7.0f / 16.0f;
	float railHi = 9.0f / 16.0f;
	float railBottom = 12.0f / 16.0f;
	float railTop = 15.0f / 16.0f;
	float x0 = west ? 0.0f : railLo;
	float x1 = east ? 1.0f : railHi;
	float z0 = north ? 0.0f : railLo;
	float z1 = south ? 1.0f : railHi;

	if (connectX)
	{
		tt.setShape(x0, railBottom, railLo, x1, railTop, railHi);
		tesselateBlockInWorld(tt, x, y, z);
		changed = true;
	}

	if (connectZ)
	{
		tt.setShape(railLo, railBottom, z0, railHi, railTop, z1);
		tesselateBlockInWorld(tt, x, y, z);
		changed = true;
	}

	railBottom = 6.0f / 16.0f;
	railTop = 9.0f / 16.0f;

	if (connectX)
	{
		tt.setShape(x0, railBottom, railLo, x1, railTop, railHi);
		tesselateBlockInWorld(tt, x, y, z);
		changed = true;
	}

	if (connectZ)
	{
		tt.setShape(railLo, railBottom, z0, railHi, railTop, z1);
		tesselateBlockInWorld(tt, x, y, z);
		changed = true;
	}

	tt.setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return changed;
}

bool TileRenderer::tesselateStairsInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	int_t data = level->getData(x, y, z) & 3;
	bool changed = false;
	double oldxx0 = tt.xx0;
	double oldyy0 = tt.yy0;
	double oldzz0 = tt.zz0;
	double oldxx1 = tt.xx1;
	double oldyy1 = tt.yy1;
	double oldzz1 = tt.zz1;
	for (int_t piece = 0; piece < 2; ++piece)
	{
		StairTile::setPieceShape(tt, data, piece);
		changed = tesselateBlockInWorld(tt, x, y, z) || changed;
	}
	tt.setShape(static_cast<float>(oldxx0), static_cast<float>(oldyy0), static_cast<float>(oldzz0), static_cast<float>(oldxx1), static_cast<float>(oldyy1), static_cast<float>(oldzz1));
	enableAO = false;
	return changed;
}

bool TileRenderer::tesselateLadderInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	Tesselator &t = Tesselator::instance;
	int_t tex = tt.getTexture(Facing::NORTH);
	if (fixedTexture >= 0)
		tex = fixedTexture;

	float bright = tt.getBrightness(*level, x, y, z);
	t.color(bright, bright, bright);

	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	double u0 = static_cast<double>(xt) / 256.0;
	double u1 = (static_cast<double>(xt) + 15.99) / 256.0;
	double v0 = static_cast<double>(yt) / 256.0;
	double v1 = (static_cast<double>(yt) + 15.99) / 256.0;
	int_t data = level->getData(x, y, z);
	float inset = 0.05f;
	float pad = 0.0f;

	if (data == 5)
{
		t.vertexUV(static_cast<double>(x) + inset, static_cast<double>(y + 1) + pad, static_cast<double>(z + 1) + pad, u0, v0);
		t.vertexUV(static_cast<double>(x) + inset, static_cast<double>(y) - pad, static_cast<double>(z + 1) + pad, u0, v1);
		t.vertexUV(static_cast<double>(x) + inset, static_cast<double>(y) - pad, static_cast<double>(z) - pad, u1, v1);
		t.vertexUV(static_cast<double>(x) + inset, static_cast<double>(y + 1) + pad, static_cast<double>(z) - pad, u1, v0);
	}
	if (data == 4)
	{
		t.vertexUV(static_cast<double>(x + 1) - inset, static_cast<double>(y) - pad, static_cast<double>(z + 1) + pad, u1, v1);
		t.vertexUV(static_cast<double>(x + 1) - inset, static_cast<double>(y + 1) + pad, static_cast<double>(z + 1) + pad, u1, v0);
		t.vertexUV(static_cast<double>(x + 1) - inset, static_cast<double>(y + 1) + pad, static_cast<double>(z) - pad, u0, v0);
		t.vertexUV(static_cast<double>(x + 1) - inset, static_cast<double>(y) - pad, static_cast<double>(z) - pad, u0, v1);
	}
	if (data == 3)
	{
		t.vertexUV(static_cast<double>(x + 1) + pad, static_cast<double>(y) - pad, static_cast<double>(z) + inset, u1, v1);
		t.vertexUV(static_cast<double>(x + 1) + pad, static_cast<double>(y + 1) + pad, static_cast<double>(z) + inset, u1, v0);
		t.vertexUV(static_cast<double>(x) - pad, static_cast<double>(y + 1) + pad, static_cast<double>(z) + inset, u0, v0);
		t.vertexUV(static_cast<double>(x) - pad, static_cast<double>(y) - pad, static_cast<double>(z) + inset, u0, v1);
	}
	if (data == 2)
	{
		t.vertexUV(static_cast<double>(x + 1) + pad, static_cast<double>(y + 1) + pad, static_cast<double>(z + 1) - inset, u0, v0);
		t.vertexUV(static_cast<double>(x + 1) + pad, static_cast<double>(y) - pad, static_cast<double>(z + 1) - inset, u0, v1);
		t.vertexUV(static_cast<double>(x) - pad, static_cast<double>(y) - pad, static_cast<double>(z + 1) - inset, u1, v1);
		t.vertexUV(static_cast<double>(x) - pad, static_cast<double>(y + 1) + pad, static_cast<double>(z + 1) - inset, u1, v0);
	}
	return true;
}

bool TileRenderer::tesselateDoorInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	int_t col = tt.getColor(*level, x, y, z);
	float r = ((col >> 16) & 0xFF) / 255.0f;
	float g = ((col >> 8) & 0xFF) / 255.0f;
	float b = (col & 0xFF) / 255.0f;
	float c10 = 0.5f;
	float c11 = 1.0f;
	float c2 = 0.8f;
	float c3 = 0.6f;
	float lightOwn = tt.getBrightness(*level, x, y, z);
	bool changed = false;
	enableAO = false;
	auto renderFace = [&](Facing face, float shade, float br, auto fn) {
		int_t tex = tt.getTexture(*level, x, y, z, face);
		xFlipTexture = false;
		if (tex < 0)
		{
			xFlipTexture = true;
			tex = -tex;
		}
		Tesselator::instance.color(shade * r * br, shade * g * br, shade * b * br);
		fn(tt, x, y, z, tex);
		xFlipTexture = false;
		changed = true;
	};
	float br = tt.getBrightness(*level, x, y - 1, z);
	if (tt.yy0 > 0.0) br = lightOwn;
	renderFace(Facing::DOWN, c10, br, [this](Tile &tile, int_t rx, int_t ry, int_t rz, int_t tex) { renderFaceUp(tile, rx, ry, rz, tex); });
	br = tt.getBrightness(*level, x, y + 1, z);
	if (tt.yy1 < 1.0) br = lightOwn;
	renderFace(Facing::UP, c11, br, [this](Tile &tile, int_t rx, int_t ry, int_t rz, int_t tex) { renderFaceDown(tile, rx, ry, rz, tex); });
	br = tt.getBrightness(*level, x, y, z - 1);
	if (tt.zz0 > 0.0) br = lightOwn;
	renderFace(Facing::NORTH, c2, br, [this](Tile &tile, int_t rx, int_t ry, int_t rz, int_t tex) { renderNorth(tile, rx, ry, rz, tex); });
	br = tt.getBrightness(*level, x, y, z + 1);
	if (tt.zz1 < 1.0) br = lightOwn;
	renderFace(Facing::SOUTH, c2, br, [this](Tile &tile, int_t rx, int_t ry, int_t rz, int_t tex) { renderSouth(tile, rx, ry, rz, tex); });
	br = tt.getBrightness(*level, x - 1, y, z);
	if (tt.xx0 > 0.0) br = lightOwn;
	renderFace(Facing::WEST, c3, br, [this](Tile &tile, int_t rx, int_t ry, int_t rz, int_t tex) { renderWest(tile, rx, ry, rz, tex); });
	br = tt.getBrightness(*level, x + 1, y, z);
	if (tt.xx1 < 1.0) br = lightOwn;
	renderFace(Facing::EAST, c3, br, [this](Tile &tile, int_t rx, int_t ry, int_t rz, int_t tex) { renderEast(tile, rx, ry, rz, tex); });
	return changed;
}

void TileRenderer::tesselateTorch(Tile &tt, double x, double y, double z, double xxa, double zza)
{
	Tesselator &t = Tesselator::instance;
	int_t tex = tt.getTexture(Facing::NORTH);
	if (fixedTexture >= 0)
		tex = fixedTexture;

	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	float u0 = xt / 256.0f;
	float u1 = (xt + 15.99f) / 256.0f;
	float v0 = yt / 256.0f;
	float v1 = (yt + 15.99f) / 256.0f;
	double uc0 = u0 + 0.02734375;
	double vc0 = v0 + 0.0234375;
	double uc1 = u0 + 0.03515625;
	double vc1 = v0 + 0.03125;
	x += 0.5;
	z += 0.5;
	double x0 = x - 0.5;
	double x1 = x + 0.5;
	double z0 = z - 0.5;
	double z1 = z + 0.5;
	double r = 0.0625;
	double h = 0.625;
	t.vertexUV(x + xxa * (1.0 - h) - r, y + h, z + zza * (1.0 - h) - r, uc0, vc0);
	t.vertexUV(x + xxa * (1.0 - h) - r, y + h, z + zza * (1.0 - h) + r, uc0, vc1);
	t.vertexUV(x + xxa * (1.0 - h) + r, y + h, z + zza * (1.0 - h) + r, uc1, vc1);
	t.vertexUV(x + xxa * (1.0 - h) + r, y + h, z + zza * (1.0 - h) - r, uc1, vc0);
	t.vertexUV(x - r, y + 1.0, z0, u0, v0);
	t.vertexUV(x - r + xxa, y + 0.0, z0 + zza, u0, v1);
	t.vertexUV(x - r + xxa, y + 0.0, z1 + zza, u1, v1);
	t.vertexUV(x - r, y + 1.0, z1, u1, v0);
	t.vertexUV(x + r, y + 1.0, z1, u0, v0);
	t.vertexUV(x + xxa + r, y + 0.0, z1 + zza, u0, v1);
	t.vertexUV(x + xxa + r, y + 0.0, z0 + zza, u1, v1);
	t.vertexUV(x + r, y + 1.0, z0, u1, v0);
	t.vertexUV(x0, y + 1.0, z + r, u0, v0);
	t.vertexUV(x0 + xxa, y + 0.0, z + r + zza, u0, v1);
	t.vertexUV(x1 + xxa, y + 0.0, z + r + zza, u1, v1);
	t.vertexUV(x1, y + 1.0, z + r, u1, v0);
	t.vertexUV(x1, y + 1.0, z - r, u0, v0);
	t.vertexUV(x1 + xxa, y + 0.0, z - r + zza, u0, v1);
	t.vertexUV(x0 + xxa, y + 0.0, z - r + zza, u1, v1);
	t.vertexUV(x0, y + 1.0, z - r, u1, v0);
}

bool TileRenderer::tesselateRepeaterInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	int_t data = level->getData(x, y, z);
	int_t orient = data & 3;
	int_t delay = (data & 12) >> 2;
	tesselateBlockInWorld(tt, x, y, z);

	Tesselator &t = Tesselator::instance;
	float br = tt.getBrightness(*level, x, y, z);
	if (Tile::lightEmission[tt.id] > 0)
		br = (br + 1.0f) * 0.5f;
	t.color(br, br, br);

	double torchY = -0.1875;
	double x0Off = 0.0;
	double z0Off = 0.0;
	double x1Off = 0.0;
	double z1Off = 0.0;
	double delayOffset = RepeaterTile::getDelayTorchOffset(delay);
	switch (orient)
	{
	case 0:
		z1Off = -0.3125;
		z0Off = delayOffset;
		break;
	case 1:
		x1Off = 0.3125;
		x0Off = -delayOffset;
		break;
	case 2:
		z1Off = 0.3125;
		z0Off = -delayOffset;
		break;
	case 3:
		x1Off = -0.3125;
		x0Off = delayOffset;
		break;
	}

	bool hadFixedTexture = fixedTexture >= 0;
	if (!hadFixedTexture)
		fixedTexture = tt.getTexture(Facing::DOWN, data);
	tesselateTorch(tt, static_cast<double>(x) + x0Off, static_cast<double>(y) + torchY, static_cast<double>(z) + z0Off, 0.0, 0.0);
	tesselateTorch(tt, static_cast<double>(x) + x1Off, static_cast<double>(y) + torchY, static_cast<double>(z) + z1Off, 0.0, 0.0);
	if (!hadFixedTexture)
		fixedTexture = -1;

	int_t tex = tt.getTexture(Facing::UP, data);
	if (fixedTexture >= 0)
		tex = fixedTexture;
	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	double u0 = static_cast<double>(xt) / 256.0;
	double u1 = (static_cast<double>(xt) + 15.99) / 256.0;
	double v0 = static_cast<double>(yt) / 256.0;
	double v1 = (static_cast<double>(yt) + 15.99) / 256.0;
	float topY = static_cast<float>(y) + 2.0f / 16.0f;
	float vx0 = static_cast<float>(x + 1);
	float vx1 = static_cast<float>(x + 1);
	float vx2 = static_cast<float>(x + 0);
	float vx3 = static_cast<float>(x + 0);
	float vz0 = static_cast<float>(z + 0);
	float vz1 = static_cast<float>(z + 1);
	float vz2 = static_cast<float>(z + 1);
	float vz3 = static_cast<float>(z + 0);
	if (orient == 2)
	{
		vx1 = static_cast<float>(x + 0);
		vx0 = vx1;
		vx3 = static_cast<float>(x + 1);
		vx2 = vx3;
		vz3 = static_cast<float>(z + 1);
		vz0 = vz3;
		vz2 = static_cast<float>(z + 0);
		vz1 = vz2;
	}
	else if (orient == 3)
	{
		vx3 = static_cast<float>(x + 0);
		vx0 = vx3;
		vx2 = static_cast<float>(x + 1);
		vx1 = vx2;
		vz1 = static_cast<float>(z + 0);
		vz0 = vz1;
		vz3 = static_cast<float>(z + 1);
		vz2 = vz3;
	}
	else if (orient == 1)
	{
		vx3 = static_cast<float>(x + 1);
		vx0 = vx3;
		vx2 = static_cast<float>(x + 0);
		vx1 = vx2;
		vz1 = static_cast<float>(z + 1);
		vz0 = vz1;
		vz3 = static_cast<float>(z + 0);
		vz2 = vz3;
	}

	t.vertexUV(vx3, topY, vz3, u0, v0);
	t.vertexUV(vx2, topY, vz2, u0, v1);
	t.vertexUV(vx1, topY, vz1, u1, v1);
	t.vertexUV(vx0, topY, vz0, u1, v0);
	return true;
}

bool TileRenderer::tesselateDustInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	Tesselator &t = Tesselator::instance;
	int_t data = level->getData(x, y, z);
	int_t tex = tt.getTexture(Facing::UP, data);
	if (fixedTexture >= 0)
		tex = fixedTexture;

	float br = tt.getBrightness(*level, x, y, z);
	float power = static_cast<float>(data) / 15.0f;
	float tintR = power * 0.6f + 0.4f;
	if (data == 0)
		tintR = 0.3f;
	float tintG = power * power * 0.7f - 0.5f;
	float tintB = power * power * 0.6f - 0.7f;
	if (tintG < 0.0f)
		tintG = 0.0f;
	if (tintB < 0.0f)
		tintB = 0.0f;

	t.color(br * tintR, br * tintG, br * tintB);
	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	double u0 = static_cast<double>(xt) / 256.0;
	double u1 = (static_cast<double>(xt) + 15.99) / 256.0;
	double v0 = static_cast<double>(yt) / 256.0;
	double v1 = (static_cast<double>(yt) + 15.99) / 256.0;
	bool west = RedStoneDustTile::isPowerProviderOrWire(*level, x - 1, y, z, 1) || (!level->isBlockNormalCube(x - 1, y, z) && RedStoneDustTile::isPowerProviderOrWire(*level, x - 1, y - 1, z, -1));
	bool east = RedStoneDustTile::isPowerProviderOrWire(*level, x + 1, y, z, 3) || (!level->isBlockNormalCube(x + 1, y, z) && RedStoneDustTile::isPowerProviderOrWire(*level, x + 1, y - 1, z, -1));
	bool north = RedStoneDustTile::isPowerProviderOrWire(*level, x, y, z - 1, 2) || (!level->isBlockNormalCube(x, y, z - 1) && RedStoneDustTile::isPowerProviderOrWire(*level, x, y - 1, z - 1, -1));
	bool south = RedStoneDustTile::isPowerProviderOrWire(*level, x, y, z + 1, 0) || (!level->isBlockNormalCube(x, y, z + 1) && RedStoneDustTile::isPowerProviderOrWire(*level, x, y - 1, z + 1, -1));
	if (!level->isBlockNormalCube(x, y + 1, z))
	{
		if (level->isBlockNormalCube(x - 1, y, z) && RedStoneDustTile::isPowerProviderOrWire(*level, x - 1, y + 1, z, -1)) west = true;
		if (level->isBlockNormalCube(x + 1, y, z) && RedStoneDustTile::isPowerProviderOrWire(*level, x + 1, y + 1, z, -1)) east = true;
		if (level->isBlockNormalCube(x, y, z - 1) && RedStoneDustTile::isPowerProviderOrWire(*level, x, y + 1, z - 1, -1)) north = true;
		if (level->isBlockNormalCube(x, y, z + 1) && RedStoneDustTile::isPowerProviderOrWire(*level, x, y + 1, z + 1, -1)) south = true;
	}

	float fx0 = static_cast<float>(x);
	float fx1 = static_cast<float>(x + 1);
	float fz0 = static_cast<float>(z);
	float fz1 = static_cast<float>(z + 1);
	int_t pic = 0;
	if ((west || east) && !north && !south) pic = 1;
	if ((north || south) && !east && !west) pic = 2;
	if (pic != 0)
	{
		u0 = static_cast<double>(xt + 16) / 256.0;
		u1 = (static_cast<double>(xt + 16) + 15.99) / 256.0;
		v0 = static_cast<double>(yt) / 256.0;
		v1 = (static_cast<double>(yt) + 15.99) / 256.0;
	}

	if (pic == 0)
	{
		if (east || north || south || west)
		{
			if (!west) { fx0 += 5.0f / 16.0f; u0 += 1.25 / 64.0; }
			if (!east) { fx1 -= 5.0f / 16.0f; u1 -= 1.25 / 64.0; }
			if (!north) { fz0 += 5.0f / 16.0f; v0 += 1.25 / 64.0; }
			if (!south) { fz1 -= 5.0f / 16.0f; v1 -= 1.25 / 64.0; }
		}
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz1, u1, v1);
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz0, u1, v0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz0, u0, v0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz1, u0, v1);
		t.color(br, br, br);
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz1, u1, v1 + 1.0 / 16.0);
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz0, u1, v0 + 1.0 / 16.0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz0, u0, v0 + 1.0 / 16.0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz1, u0, v1 + 1.0 / 16.0);
	}
	else if (pic == 1)
	{
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz1, u1, v1);
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz0, u1, v0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz0, u0, v0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz1, u0, v1);
		t.color(br, br, br);
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz1, u1, v1 + 1.0 / 16.0);
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz0, u1, v0 + 1.0 / 16.0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz0, u0, v0 + 1.0 / 16.0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz1, u0, v1 + 1.0 / 16.0);
	}
	else if (pic == 2)
	{
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz1, u1, v1);
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz0, u0, v1);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz0, u0, v0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz1, u1, v0);
		t.color(br, br, br);
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz1, u1, v1 + 1.0 / 16.0);
		t.vertexUV(fx1, static_cast<float>(y) + 0.015625f, fz0, u0, v1 + 1.0 / 16.0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz0, u0, v0 + 1.0 / 16.0);
		t.vertexUV(fx0, static_cast<float>(y) + 0.015625f, fz1, u1, v0 + 1.0 / 16.0);
	}

	if (!level->isBlockNormalCube(x, y + 1, z))
	{
		u0 = static_cast<double>(xt + 16) / 256.0;
		u1 = (static_cast<double>(xt + 16) + 15.99) / 256.0;
		v0 = static_cast<double>(yt) / 256.0;
		v1 = (static_cast<double>(yt) + 15.99) / 256.0;
		if (level->isBlockNormalCube(x - 1, y, z) && level->getTile(x - 1, y + 1, z) == tt.id)
		{
			t.color(br * tintR, br * tintG, br * tintB);
			t.vertexUV(static_cast<float>(x) + 0.015625f, static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<double>(z + 1), u1, v0);
			t.vertexUV(static_cast<float>(x) + 0.015625f, static_cast<double>(y), static_cast<double>(z + 1), u0, v0);
			t.vertexUV(static_cast<float>(x) + 0.015625f, static_cast<double>(y), static_cast<double>(z), u0, v1);
			t.vertexUV(static_cast<float>(x) + 0.015625f, static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<double>(z), u1, v1);
			t.color(br, br, br);
			t.vertexUV(static_cast<float>(x) + 0.015625f, static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<double>(z + 1), u1, v0 + 1.0 / 16.0);
			t.vertexUV(static_cast<float>(x) + 0.015625f, static_cast<double>(y), static_cast<double>(z + 1), u0, v0 + 1.0 / 16.0);
			t.vertexUV(static_cast<float>(x) + 0.015625f, static_cast<double>(y), static_cast<double>(z), u0, v1 + 1.0 / 16.0);
			t.vertexUV(static_cast<float>(x) + 0.015625f, static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<double>(z), u1, v1 + 1.0 / 16.0);
		}
		if (level->isBlockNormalCube(x + 1, y, z) && level->getTile(x + 1, y + 1, z) == tt.id)
		{
			t.color(br * tintR, br * tintG, br * tintB);
			t.vertexUV(static_cast<float>(x + 1) - 0.015625f, static_cast<double>(y), static_cast<double>(z + 1), u0, v1);
			t.vertexUV(static_cast<float>(x + 1) - 0.015625f, static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<double>(z + 1), u1, v1);
			t.vertexUV(static_cast<float>(x + 1) - 0.015625f, static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<double>(z), u1, v0);
			t.vertexUV(static_cast<float>(x + 1) - 0.015625f, static_cast<double>(y), static_cast<double>(z), u0, v0);
			t.color(br, br, br);
			t.vertexUV(static_cast<float>(x + 1) - 0.015625f, static_cast<double>(y), static_cast<double>(z + 1), u0, v1 + 1.0 / 16.0);
			t.vertexUV(static_cast<float>(x + 1) - 0.015625f, static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<double>(z + 1), u1, v1 + 1.0 / 16.0);
			t.vertexUV(static_cast<float>(x + 1) - 0.015625f, static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<double>(z), u1, v0 + 1.0 / 16.0);
			t.vertexUV(static_cast<float>(x + 1) - 0.015625f, static_cast<double>(y), static_cast<double>(z), u0, v0 + 1.0 / 16.0);
		}
		if (level->isBlockNormalCube(x, y, z - 1) && level->getTile(x, y + 1, z - 1) == tt.id)
		{
			t.color(br * tintR, br * tintG, br * tintB);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y), static_cast<float>(z) + 0.015625f, u0, v1);
			t.vertexUV(static_cast<double>(x + 1), static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<float>(z) + 0.015625f, u1, v1);
			t.vertexUV(static_cast<double>(x), static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<float>(z) + 0.015625f, u1, v0);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y), static_cast<float>(z) + 0.015625f, u0, v0);
			t.color(br, br, br);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y), static_cast<float>(z) + 0.015625f, u0, v1 + 1.0 / 16.0);
			t.vertexUV(static_cast<double>(x + 1), static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<float>(z) + 0.015625f, u1, v1 + 1.0 / 16.0);
			t.vertexUV(static_cast<double>(x), static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<float>(z) + 0.015625f, u1, v0 + 1.0 / 16.0);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y), static_cast<float>(z) + 0.015625f, u0, v0 + 1.0 / 16.0);
		}
		if (level->isBlockNormalCube(x, y, z + 1) && level->getTile(x, y + 1, z + 1) == tt.id)
		{
			t.color(br * tintR, br * tintG, br * tintB);
			t.vertexUV(static_cast<double>(x + 1), static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<float>(z + 1) - 0.015625f, u1, v0);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y), static_cast<float>(z + 1) - 0.015625f, u0, v0);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y), static_cast<float>(z + 1) - 0.015625f, u0, v1);
			t.vertexUV(static_cast<double>(x), static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<float>(z + 1) - 0.015625f, u1, v1);
			t.color(br, br, br);
			t.vertexUV(static_cast<double>(x + 1), static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<float>(z + 1) - 0.015625f, u1, v0 + 1.0 / 16.0);
			t.vertexUV(static_cast<double>(x + 1), static_cast<double>(y), static_cast<float>(z + 1) - 0.015625f, u0, v0 + 1.0 / 16.0);
			t.vertexUV(static_cast<double>(x), static_cast<double>(y), static_cast<float>(z + 1) - 0.015625f, u0, v1 + 1.0 / 16.0);
			t.vertexUV(static_cast<double>(x), static_cast<float>(y + 1) + 7.0f / 320.0f, static_cast<float>(z + 1) - 0.015625f, u1, v1 + 1.0 / 16.0);
		}
	}

	return true;
}

bool TileRenderer::tesselateLeverInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	int_t data = level->getData(x, y, z);
	int_t orient = data & 7;
	bool flipped = (data & 8) > 0;
	Tesselator &t = Tesselator::instance;
	bool hadFixedTexture = fixedTexture >= 0;
	if (!hadFixedTexture)
		fixedTexture = Tile::cobblestone.tex;

	float baseLong = 0.25f;
	float baseHalf = 3.0f / 16.0f;
	float baseDepth = 3.0f / 16.0f;
	if (orient == 5)
		tt.setShape(0.5f - baseHalf, 0.0f, 0.5f - baseLong, 0.5f + baseHalf, baseDepth, 0.5f + baseLong);
	else if (orient == 6)
		tt.setShape(0.5f - baseLong, 0.0f, 0.5f - baseHalf, 0.5f + baseLong, baseDepth, 0.5f + baseHalf);
	else if (orient == 4)
		tt.setShape(0.5f - baseHalf, 0.5f - baseLong, 1.0f - baseDepth, 0.5f + baseHalf, 0.5f + baseLong, 1.0f);
	else if (orient == 3)
		tt.setShape(0.5f - baseHalf, 0.5f - baseLong, 0.0f, 0.5f + baseHalf, 0.5f + baseLong, baseDepth);
	else if (orient == 2)
		tt.setShape(1.0f - baseDepth, 0.5f - baseLong, 0.5f - baseHalf, 1.0f, 0.5f + baseLong, 0.5f + baseHalf);
	else if (orient == 1)
		tt.setShape(0.0f, 0.5f - baseLong, 0.5f - baseHalf, baseDepth, 0.5f + baseLong, 0.5f + baseHalf);

	tesselateBlockInWorld(tt, x, y, z);
	if (!hadFixedTexture)
		fixedTexture = -1;

	float br = tt.getBrightness(*level, x, y, z);
	if (Tile::lightEmission[tt.id] > 0)
		br = 1.0f;
	t.color(br, br, br);
	int_t tex = tt.getTexture(Facing::DOWN);
	if (fixedTexture >= 0)
		tex = fixedTexture;

	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	float u0 = static_cast<float>(xt) / 256.0f;
	float u1 = (static_cast<float>(xt) + 15.99f) / 256.0f;
	float v0 = static_cast<float>(yt) / 256.0f;
	float v1 = (static_cast<float>(yt) + 15.99f) / 256.0f;
	Vec3 *corners[8];
	float xv = 1.0f / 16.0f;
	float zv = 1.0f / 16.0f;
	float yv = 10.0f / 16.0f;
	corners[0] = Vec3::newTemp(-xv, 0.0, -zv);
	corners[1] = Vec3::newTemp(xv, 0.0, -zv);
	corners[2] = Vec3::newTemp(xv, 0.0, zv);
	corners[3] = Vec3::newTemp(-xv, 0.0, zv);
	corners[4] = Vec3::newTemp(-xv, yv, -zv);
	corners[5] = Vec3::newTemp(xv, yv, -zv);
	corners[6] = Vec3::newTemp(xv, yv, zv);
	corners[7] = Vec3::newTemp(-xv, yv, zv);

	for (int_t i = 0; i < 8; ++i)
	{
		if (flipped)
		{
			corners[i]->z -= 1.0 / 16.0;
			corners[i]->xRot(Mth::PI * 2.0f / 9.0f);
		}
		else
		{
			corners[i]->z += 1.0 / 16.0;
			corners[i]->xRot(-Mth::PI * 2.0f / 9.0f);
		}

		if (orient == 6)
			corners[i]->yRot(Mth::PI * 0.5f);

		if (orient < 5)
		{
			corners[i]->y -= 0.375;
			corners[i]->xRot(Mth::PI * 0.5f);
			if (orient == 3) corners[i]->yRot(Mth::PI);
			if (orient == 2) corners[i]->yRot(Mth::PI * 0.5f);
			if (orient == 1) corners[i]->yRot(Mth::PI * -0.5f);
			corners[i]->x += static_cast<double>(x) + 0.5;
			corners[i]->y += static_cast<double>(y) + 0.5;
			corners[i]->z += static_cast<double>(z) + 0.5;
		}
		else
		{
			corners[i]->x += static_cast<double>(x) + 0.5;
			corners[i]->y += static_cast<double>(y) + 2.0 / 16.0;
			corners[i]->z += static_cast<double>(z) + 0.5;
		}
	}

	Vec3 *c0 = nullptr;
	Vec3 *c1 = nullptr;
	Vec3 *c2 = nullptr;
	Vec3 *c3 = nullptr;
	for (int_t face = 0; face < 6; ++face)
	{
		if (face == 0)
		{
			u0 = static_cast<float>(xt + 7) / 256.0f;
			u1 = (static_cast<float>(xt + 9) - 0.01f) / 256.0f;
			v0 = static_cast<float>(yt + 6) / 256.0f;
			v1 = (static_cast<float>(yt + 8) - 0.01f) / 256.0f;
		}
		else if (face == 2)
		{
			u0 = static_cast<float>(xt + 7) / 256.0f;
			u1 = (static_cast<float>(xt + 9) - 0.01f) / 256.0f;
			v0 = static_cast<float>(yt + 6) / 256.0f;
			v1 = (static_cast<float>(yt + 16) - 0.01f) / 256.0f;
		}

		if (face == 0) { c0 = corners[0]; c1 = corners[1]; c2 = corners[2]; c3 = corners[3]; }
		else if (face == 1) { c0 = corners[7]; c1 = corners[6]; c2 = corners[5]; c3 = corners[4]; }
		else if (face == 2) { c0 = corners[1]; c1 = corners[0]; c2 = corners[4]; c3 = corners[5]; }
		else if (face == 3) { c0 = corners[2]; c1 = corners[1]; c2 = corners[5]; c3 = corners[6]; }
		else if (face == 4) { c0 = corners[3]; c1 = corners[2]; c2 = corners[6]; c3 = corners[7]; }
		else { c0 = corners[0]; c1 = corners[3]; c2 = corners[7]; c3 = corners[4]; }

		t.vertexUV(c0->x, c0->y, c0->z, u0, v1);
		t.vertexUV(c1->x, c1->y, c1->z, u1, v1);
		t.vertexUV(c2->x, c2->y, c2->z, u1, v0);
		t.vertexUV(c3->x, c3->y, c3->z, u0, v0);
	}

	return true;
}
bool TileRenderer::tesselatePistonBaseInWorld(Tile &tt, int_t x, int_t y, int_t z, bool forceExtended, int_t forceData)
{
	int_t data = (forceData == -1) ? level->getData(x, y, z) : forceData;
	bool extended = forceExtended || PistonBaseTile::isPowered(data);
	int_t facing = PistonBaseTile::getDirection(data);

	const float thickness = 4.0f / 16.0f;

	float oldX0 = static_cast<float>(tt.xx0), oldY0 = static_cast<float>(tt.yy0), oldZ0 = static_cast<float>(tt.zz0);
	float oldX1 = static_cast<float>(tt.xx1), oldY1 = static_cast<float>(tt.yy1), oldZ1 = static_cast<float>(tt.zz1);

	if (extended)
	{
		switch (facing)
		{
			case 0:
				northFlip = FLIP_180; southFlip = FLIP_180;
				eastFlip = FLIP_180; westFlip = FLIP_180;
				tt.setShape(0.0f, thickness, 0.0f, 1.0f, 1.0f, 1.0f);
				break;
			case 1:
				tt.setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f - thickness, 1.0f);
				break;
			case 2:
				eastFlip = FLIP_CW; westFlip = FLIP_CCW;
				tt.setShape(0.0f, 0.0f, thickness, 1.0f, 1.0f, 1.0f);
				break;
			case 3:
				eastFlip = FLIP_CCW; westFlip = FLIP_CW;
				upFlip = FLIP_180; downFlip = FLIP_180;
				tt.setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f - thickness);
				break;
		case 4:
			northFlip = FLIP_CW; southFlip = FLIP_CCW;
			upFlip = FLIP_CCW; downFlip = FLIP_CW;
				tt.setShape(thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
				break;
		case 5:
			northFlip = FLIP_CCW; southFlip = FLIP_CW;
			upFlip = FLIP_CW; downFlip = FLIP_CCW;
				tt.setShape(0.0f, 0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f);
				break;
		}
		tesselateBlockInWorld(tt, x, y, z);
		northFlip = FLIP_NONE; southFlip = FLIP_NONE;
		eastFlip = FLIP_NONE; westFlip = FLIP_NONE;
		upFlip = FLIP_NONE; downFlip = FLIP_NONE;
	}
	else
	{
		switch (facing)
		{
			case 0:
				northFlip = FLIP_180; southFlip = FLIP_180;
				eastFlip = FLIP_180; westFlip = FLIP_180;
				break;
			case 1:
				break;
			case 2:
				eastFlip = FLIP_CW; westFlip = FLIP_CCW;
				break;
			case 3:
				eastFlip = FLIP_CCW; westFlip = FLIP_CW;
				upFlip = FLIP_180; downFlip = FLIP_180;
				break;
			case 4:
				northFlip = FLIP_CW; southFlip = FLIP_CCW;
				upFlip = FLIP_CCW; downFlip = FLIP_CW;
				break;
			case 5:
				northFlip = FLIP_CCW; southFlip = FLIP_CW;
				upFlip = FLIP_CW; downFlip = FLIP_CCW;
				break;
		}
		tesselateBlockInWorld(tt, x, y, z);
		northFlip = FLIP_NONE; southFlip = FLIP_NONE;
		eastFlip = FLIP_NONE; westFlip = FLIP_NONE;
		upFlip = FLIP_NONE; downFlip = FLIP_NONE;
	}

	tt.setShape(oldX0, oldY0, oldZ0, oldX1, oldY1, oldZ1);
	return true;
}

void TileRenderer::tesselatePistonBaseForceExtended(Tile &tile, int_t x, int_t y, int_t z, int_t forceData)
{
	noCulling = true;
	tesselatePistonBaseInWorld(tile, x, y, z, true, forceData);
	noCulling = false;
}

bool TileRenderer::tesselatePistonExtensionInWorld(Tile &tt, int_t x, int_t y, int_t z, bool fullArm, int_t forceData)
{
	int_t data = (forceData == -1) ? level->getData(x, y, z) : forceData;
	int_t facing = PistonExtensionTile::getDirection(data);

	const float thickness = 4.0f / 16.0f;
	const float leftEdge = 6.0f / 16.0f;
	const float rightEdge = 10.0f / 16.0f;
	float br = tt.getBrightness(*level, x, y, z);
	float armLength = fullArm ? 1.0f : 0.5f;
	float armLengthPixels = fullArm ? 16.0f : 8.0f;

	float oldX0 = static_cast<float>(tt.xx0), oldY0 = static_cast<float>(tt.yy0), oldZ0 = static_cast<float>(tt.zz0);
	float oldX1 = static_cast<float>(tt.xx1), oldY1 = static_cast<float>(tt.yy1), oldZ1 = static_cast<float>(tt.zz1);

	switch (facing)
	{
		case 0:
			northFlip = FLIP_180; southFlip = FLIP_180;
			eastFlip = FLIP_180; westFlip = FLIP_180;
			tt.setShape(0.0f, 0.0f, 0.0f, 1.0f, thickness, 1.0f);
			tesselateBlockInWorld(tt, x, y, z);
			renderPistonArmUpDown(x + leftEdge, x + rightEdge, y + thickness, y + thickness + armLength, z + rightEdge, z + rightEdge, br * 0.8f, armLengthPixels);
			renderPistonArmUpDown(x + rightEdge, x + leftEdge, y + thickness, y + thickness + armLength, z + leftEdge, z + leftEdge, br * 0.8f, armLengthPixels);
			renderPistonArmUpDown(x + leftEdge, x + leftEdge, y + thickness, y + thickness + armLength, z + leftEdge, z + rightEdge, br * 0.6f, armLengthPixels);
			renderPistonArmUpDown(x + rightEdge, x + rightEdge, y + thickness, y + thickness + armLength, z + rightEdge, z + leftEdge, br * 0.6f, armLengthPixels);
			break;
		case 1:
			tt.setShape(0.0f, 1.0f - thickness, 0.0f, 1.0f, 1.0f, 1.0f);
			tesselateBlockInWorld(tt, x, y, z);
			renderPistonArmUpDown(x + leftEdge, x + rightEdge, y - thickness + 1.0f - armLength, y - thickness + 1.0f, z + rightEdge, z + rightEdge, br * 0.8f, armLengthPixels);
			renderPistonArmUpDown(x + rightEdge, x + leftEdge, y - thickness + 1.0f - armLength, y - thickness + 1.0f, z + leftEdge, z + leftEdge, br * 0.8f, armLengthPixels);
			renderPistonArmUpDown(x + leftEdge, x + leftEdge, y - thickness + 1.0f - armLength, y - thickness + 1.0f, z + leftEdge, z + rightEdge, br * 0.6f, armLengthPixels);
			renderPistonArmUpDown(x + rightEdge, x + rightEdge, y - thickness + 1.0f - armLength, y - thickness + 1.0f, z + rightEdge, z + leftEdge, br * 0.6f, armLengthPixels);
			break;
		case 2:
			eastFlip = FLIP_CW; westFlip = FLIP_CCW;
			tt.setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, thickness);
			tesselateBlockInWorld(tt, x, y, z);
			renderPistonArmNorthSouth(x + leftEdge, x + leftEdge, y + rightEdge, y + leftEdge, z + thickness, z + thickness + armLength, br * 0.6f, armLengthPixels);
			renderPistonArmNorthSouth(x + rightEdge, x + rightEdge, y + leftEdge, y + rightEdge, z + thickness, z + thickness + armLength, br * 0.6f, armLengthPixels);
			renderPistonArmNorthSouth(x + leftEdge, x + rightEdge, y + leftEdge, y + leftEdge, z + thickness, z + thickness + armLength, br * 0.5f, armLengthPixels);
			renderPistonArmNorthSouth(x + rightEdge, x + leftEdge, y + rightEdge, y + rightEdge, z + thickness, z + thickness + armLength, br, armLengthPixels);
			break;
		case 3:
			eastFlip = FLIP_CCW; westFlip = FLIP_CW;
			upFlip = FLIP_180; downFlip = FLIP_180;
			tt.setShape(0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f, 1.0f);
			tesselateBlockInWorld(tt, x, y, z);
			renderPistonArmNorthSouth(x + leftEdge, x + leftEdge, y + rightEdge, y + leftEdge, z - thickness + 1.0f - armLength, z - thickness + 1.0f, br * 0.6f, armLengthPixels);
			renderPistonArmNorthSouth(x + rightEdge, x + rightEdge, y + leftEdge, y + rightEdge, z - thickness + 1.0f - armLength, z - thickness + 1.0f, br * 0.6f, armLengthPixels);
			renderPistonArmNorthSouth(x + leftEdge, x + rightEdge, y + leftEdge, y + leftEdge, z - thickness + 1.0f - armLength, z - thickness + 1.0f, br * 0.5f, armLengthPixels);
			renderPistonArmNorthSouth(x + rightEdge, x + leftEdge, y + rightEdge, y + rightEdge, z - thickness + 1.0f - armLength, z - thickness + 1.0f, br, armLengthPixels);
			break;
		case 4:
			northFlip = FLIP_CW; southFlip = FLIP_CCW;
			upFlip = FLIP_CCW; downFlip = FLIP_CW;
			tt.setShape(0.0f, 0.0f, 0.0f, thickness, 1.0f, 1.0f);
			tesselateBlockInWorld(tt, x, y, z);
			renderPistonArmEastWest(x + thickness, x + thickness + armLength, y + leftEdge, y + leftEdge, z + rightEdge, z + leftEdge, br * 0.5f, armLengthPixels);
			renderPistonArmEastWest(x + thickness, x + thickness + armLength, y + rightEdge, y + rightEdge, z + leftEdge, z + rightEdge, br, armLengthPixels);
			renderPistonArmEastWest(x + thickness, x + thickness + armLength, y + leftEdge, y + rightEdge, z + leftEdge, z + leftEdge, br * 0.6f, armLengthPixels);
			renderPistonArmEastWest(x + thickness, x + thickness + armLength, y + rightEdge, y + leftEdge, z + rightEdge, z + rightEdge, br * 0.6f, armLengthPixels);
			break;
		case 5:
			northFlip = FLIP_CCW; southFlip = FLIP_CW;
			upFlip = FLIP_CW; downFlip = FLIP_CCW;
			tt.setShape(1.0f - thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
			tesselateBlockInWorld(tt, x, y, z);
			renderPistonArmEastWest(x - thickness + 1.0f - armLength, x - thickness + 1.0f, y + leftEdge, y + leftEdge, z + rightEdge, z + leftEdge, br * 0.5f, armLengthPixels);
			renderPistonArmEastWest(x - thickness + 1.0f - armLength, x - thickness + 1.0f, y + rightEdge, y + rightEdge, z + leftEdge, z + rightEdge, br, armLengthPixels);
			renderPistonArmEastWest(x - thickness + 1.0f - armLength, x - thickness + 1.0f, y + leftEdge, y + rightEdge, z + leftEdge, z + leftEdge, br * 0.6f, armLengthPixels);
			renderPistonArmEastWest(x - thickness + 1.0f - armLength, x - thickness + 1.0f, y + rightEdge, y + leftEdge, z + rightEdge, z + rightEdge, br * 0.6f, armLengthPixels);
			break;
	}

	northFlip = FLIP_NONE; southFlip = FLIP_NONE;
	eastFlip = FLIP_NONE; westFlip = FLIP_NONE;
	upFlip = FLIP_NONE; downFlip = FLIP_NONE;
	tt.setShape(oldX0, oldY0, oldZ0, oldX1, oldY1, oldZ1);

	return true;
}

void TileRenderer::tesselatePistonArmNoCulling(Tile &tile, int_t x, int_t y, int_t z, bool fullArm, int_t forceData)
{
	noCulling = true;
	tesselatePistonExtensionInWorld(tile, x, y, z, fullArm, forceData);
	noCulling = false;
}

void TileRenderer::renderPistonArmUpDown(float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels)
{
	Tesselator &t = Tesselator::instance;

	int_t armTex = (fixedTexture >= 0) ? fixedTexture : 108;
	int_t xt = (armTex & 0xF) << 4;
	int_t yt = armTex & 0xF0;

	float u00 = xt / 256.0f;
	float v00 = yt / 256.0f;
	float u11 = (xt + armLengthPixels - 0.01f) / 256.0f;
	float v11 = (yt + 4.0f - 0.01f) / 256.0f;

	t.color(br, br, br);

	t.vertexUV(x0, y1, z0, u11, v00);
	t.vertexUV(x0, y0, z0, u00, v00);
	t.vertexUV(x1, y0, z1, u00, v11);
	t.vertexUV(x1, y1, z1, u11, v11);
}

void TileRenderer::renderPistonArmNorthSouth(float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels)
{
	Tesselator &t = Tesselator::instance;

	int_t armTex = (fixedTexture >= 0) ? fixedTexture : 108;
	int_t xt = (armTex & 0xF) << 4;
	int_t yt = armTex & 0xF0;

	float u00 = xt / 256.0f;
	float v00 = yt / 256.0f;
	float u11 = (xt + armLengthPixels - 0.01f) / 256.0f;
	float v11 = (yt + 4.0f - 0.01f) / 256.0f;

	t.color(br, br, br);

	t.vertexUV(x0, y0, z1, u11, v00);
	t.vertexUV(x0, y0, z0, u00, v00);
	t.vertexUV(x1, y1, z0, u00, v11);
	t.vertexUV(x1, y1, z1, u11, v11);
}

void TileRenderer::renderPistonArmEastWest(float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels)
{
	Tesselator &t = Tesselator::instance;

	int_t armTex = (fixedTexture >= 0) ? fixedTexture : 108;
	int_t xt = (armTex & 0xF) << 4;
	int_t yt = armTex & 0xF0;

	float u00 = xt / 256.0f;
	float v00 = yt / 256.0f;
	float u11 = (xt + armLengthPixels - 0.01f) / 256.0f;
	float v11 = (yt + 4.0f - 0.01f) / 256.0f;

	t.color(br, br, br);

	t.vertexUV(x1, y0, z0, u11, v00);
	t.vertexUV(x0, y0, z0, u00, v00);
	t.vertexUV(x0, y1, z1, u00, v11);
	t.vertexUV(x1, y1, z1, u11, v11);
}

void TileRenderer::renderAllFacesBlockInWorld(Tile &tile, int_t x, int_t y, int_t z)
{
	noCulling = true;
	tesselateBlockInWorld(tile, x, y, z);
	noCulling = false;
}

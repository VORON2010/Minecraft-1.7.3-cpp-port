#include "world/level/tile/LiquidTile.h"

#include "world/level/Level.h"
#include "world/level/LevelSource.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/material/GasMaterial.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/LadderTile.h"
#include "world/level/tile/ReedTile.h"
#include "world/level/tile/SignTile.h"

#include "util/Mth.h"
#include <algorithm>

LiquidTile::LiquidTile(int_t id, int_t tex, const Material &material) : TransparentTile(id, tex, material, false)
{
	setTicking(true);
}

AABB *LiquidTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	return nullptr;
}

void LiquidTile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
}

bool LiquidTile::isCubeShaped()
{
	return false;
}

Tile::Shape LiquidTile::getRenderShape()
{
	return SHAPE_WATER;
}

float LiquidTile::getBrightness(LevelSource &level, int_t x, int_t y, int_t z)
{
	float a = level.getBrightness(x, y, z);
	float b = level.getBrightness(x, y + 1, z);
	return a > b ? a : b;
}

bool LiquidTile::shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	const Material &adjacent = level.getMaterial(x, y, z);
	if (&adjacent == &material || &adjacent == &Material::ice())
		return false;
	if (face == Facing::UP)
		return true;
	return Tile::shouldRenderFace(level, x, y, z, face);
}

int_t LiquidTile::getTexture(Facing face)
{
	if (face == Facing::DOWN || face == Facing::UP)
		return tex;
	return tex + 1;
}

float LiquidTile::getHeight(int_t data)
{
	if (data >= 8)
		data = 0;
	return static_cast<float>(data + 1) / 9.0f;
}

double LiquidTile::getSlopeAngle(LevelSource &level, int_t x, int_t y, int_t z, const Material &material)
{
	Vec3 flow(0, 0, 0);
	bool gotFlow = false;
	if (&material == static_cast<const Material *>(&Material::water))
	{
		if (Tile::tiles[Tile::water.id] != nullptr)
		{
			flow = static_cast<LiquidTile *>(Tile::tiles[Tile::water.id])->getFlowVector(static_cast<Level &>(level), x, y, z);
			gotFlow = true;
		}
	}
	if (&material == static_cast<const Material *>(&Material::lava))
	{
		if (Tile::tiles[Tile::lava.id] != nullptr)
		{
			flow = static_cast<LiquidTile *>(Tile::tiles[Tile::lava.id])->getFlowVector(static_cast<Level &>(level), x, y, z);
			gotFlow = true;
		}
	}

	if (!gotFlow)
		return -1000.0;
	if (flow.x == 0.0 && flow.z == 0.0)
		return -1000.0;
	return std::atan2(flow.z, flow.x) - Mth::PI * 0.5;
}

bool LiquidTile::mayPick(int_t data, bool canPickLiquid)
{
	return canPickLiquid && data == 0;
}


int_t LiquidTile::getResource(int_t data, Random &random)
{
	return 0;
}

int_t LiquidTile::getResourceCount(Random &random)
{
	return 0;
}

int_t LiquidTile::getRenderLayer()
{
	return &material == static_cast<const Material *>(&Material::water) ? 1 : 0;
}

int_t LiquidTile::getTickDelay()
{
	return &material == static_cast<const Material *>(&Material::water) ? 5 : 30;
}

void LiquidTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (&material == static_cast<const Material *>(&Material::water) && random.nextInt(64) == 0)
	{
		int_t data = level.getData(x, y, z);
		if (data > 0 && data < 8)
			level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5, u"liquid.water", random.nextFloat() * 0.25f + 12.0f / 16.0f, random.nextFloat() + 0.5f);
	}

	if (&material == static_cast<const Material *>(&Material::lava))
	{
		const Material &aboveMaterial = level.getMaterial(x, y + 1, z);
		const Material &airMaterial = Material::air;
		if (&aboveMaterial == &airMaterial && !level.isSolidTile(x, y + 1, z) && random.nextInt(100) == 0)
		{
			double px = static_cast<double>(x) + random.nextDouble();
			double py = static_cast<double>(y) + yy1;
			double pz = static_cast<double>(z) + random.nextDouble();
			level.addParticle(u"lava", px, py, pz, 0.0, 0.0, 0.0);
		}
	}
}

void LiquidTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	updateLiquid(level, x, y, z);
}

void LiquidTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	updateLiquid(level, x, y, z);
}

int_t LiquidTile::getDepth(Level &level, int_t x, int_t y, int_t z) const
{
	return &level.getMaterial(x, y, z) != &material ? -1 : level.getData(x, y, z);
}

int_t LiquidTile::getRenderedDepth(LevelSource &level, int_t x, int_t y, int_t z) const
{
	if (&level.getMaterial(x, y, z) != &material)
		return -1;
	int_t data = level.getData(x, y, z);
	if (data >= 8)
		data = 0;
	return data;
}
int_t LiquidTile::getEffectiveFlowDepth(Level &level, int_t x, int_t y, int_t z){
	if (&level.getMaterial(x, y, z) != &material)
		return -1;
	int_t data = level.getData(x, y, z);
	if (data >= 8)
		data = 0;
	return data;
}

Vec3 LiquidTile::getFlowVector(Level &level, int_t x, int_t y, int_t z)
{
	Vec3 vec(0.0, 0.0, 0.0);
	int_t depth = getRenderedDepth(level, x, y, z);

	for (int_t dir = 0; dir < 4; ++dir)
	{
		int_t nx = x;
		int_t nz = z;
		if (dir == 0) nx = x - 1;
		if (dir == 1) nz = z - 1;
		if (dir == 2) nx = x + 1;
		if (dir == 3) nz = z + 1;

		int_t adjDepth = getRenderedDepth(level, nx, y, nz);
		if (adjDepth < 0)
		{
			if (!level.getMaterial(nx, y, nz).blocksMotion())
			{
				adjDepth = getRenderedDepth(level, nx, y - 1, nz);
				if (adjDepth >= 0)
				{
					int_t diff = adjDepth - (depth - 8);
					vec.x += static_cast<double>((nx - x) * diff);
					vec.z += static_cast<double>((nz - z) * diff);
				}
			}
		}
		else if (adjDepth >= 0)
		{
			int_t diff = adjDepth - depth;
			vec.x += static_cast<double>((nx - x) * diff);
			vec.z += static_cast<double>((nz - z) * diff);
		}
	}

	if (level.getData(x, y, z) >= 8)
	{
		bool hasOpening = false;
		if (hasOpening || shouldRenderFace(level, x, y, z - 1, Facing::NORTH)) hasOpening = true;
		if (hasOpening || shouldRenderFace(level, x, y, z + 1, Facing::SOUTH)) hasOpening = true;
		if (hasOpening || shouldRenderFace(level, x - 1, y, z, Facing::WEST)) hasOpening = true;
		if (hasOpening || shouldRenderFace(level, x + 1, y, z, Facing::EAST)) hasOpening = true;
		if (hasOpening || shouldRenderFace(level, x, y + 1, z - 1, Facing::NORTH)) hasOpening = true;
		if (hasOpening || shouldRenderFace(level, x, y + 1, z + 1, Facing::SOUTH)) hasOpening = true;
		if (hasOpening || shouldRenderFace(level, x - 1, y + 1, z, Facing::WEST)) hasOpening = true;
		if (hasOpening || shouldRenderFace(level, x + 1, y + 1, z, Facing::EAST)) hasOpening = true;
		if (hasOpening)
		{
			double len = Mth::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
			if (len > 0.0)
			{
				vec.x /= len;
				vec.y /= len;
				vec.z /= len;
			}
			vec.y -= 6.0;
		}
	}

	double len = Mth::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	if (len > 0.0)
	{
		vec.x /= len;
		vec.y /= len;
		vec.z /= len;
	}

	return vec;
}

void LiquidTile::velocityToAddToEntity(Level &level, int_t x, int_t y, int_t z, Entity &entity, Vec3 &vec)
{
	Vec3 flow = getFlowVector(level, x, y, z);
	vec.x += flow.x;
	vec.y += flow.y;
	vec.z += flow.z;
}

void LiquidTile::updateLiquid(Level &level, int_t x, int_t y, int_t z)
{
	if (level.getTile(x, y, z) != id || &material != static_cast<const Material *>(&Material::lava))
		return;

	bool touchingWater = false;
	if (touchingWater || &level.getMaterial(x, y, z - 1) == static_cast<const Material *>(&Material::water))
		touchingWater = true;
	if (touchingWater || &level.getMaterial(x, y, z + 1) == static_cast<const Material *>(&Material::water))
		touchingWater = true;
	if (touchingWater || &level.getMaterial(x - 1, y, z) == static_cast<const Material *>(&Material::water))
		touchingWater = true;
	if (touchingWater || &level.getMaterial(x + 1, y, z) == static_cast<const Material *>(&Material::water))
		touchingWater = true;
	if (touchingWater || &level.getMaterial(x, y + 1, z) == static_cast<const Material *>(&Material::water))
		touchingWater = true;

	if (!touchingWater)
		return;

	int_t data = level.getData(x, y, z);
	if (data == 0)
	{
		int_t hardenedId = Tile::tiles[49] != nullptr ? 49 : Tile::cobblestone.id;
		level.setTile(x, y, z, hardenedId);
	}
	else if (data <= 4)
	{
		level.setTile(x, y, z, Tile::cobblestone.id);
	}

	fizz(level, x, y, z);
}

void LiquidTile::fizz(Level &level, int_t x, int_t y, int_t z)
{
	level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5, u"random.fizz", 0.5f, 2.6f + (level.random.nextFloat() - level.random.nextFloat()) * 0.8f);
}

LiquidTileDynamic::LiquidTileDynamic(int_t id, int_t tex, const Material &material) : LiquidTile(id, tex, material)
{
}

void LiquidTileDynamic::setStatic(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	level.setTileAndDataNoUpdate(x, y, z, id + 1, data);
	level.setTilesDirty(x, y, z, x, y, z);
	level.sendTileUpdated(x, y, z);
}

void LiquidTileDynamic::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	int_t depth = getDepth(level, x, y, z);
	int_t depthStep = 1;
	if (&material == static_cast<const Material *>(&Material::lava) && !level.dimension->ultraWarm)
		depthStep = 2;

	bool shouldSetStatic = true;
	if (depth > 0)
	{
		int_t highest = -100;
		adjacentSourceCount = 0;
		highest = getHighestNeighborDepth(level, x - 1, y, z, highest);
		highest = getHighestNeighborDepth(level, x + 1, y, z, highest);
		highest = getHighestNeighborDepth(level, x, y, z - 1, highest);
		highest = getHighestNeighborDepth(level, x, y, z + 1, highest);

		int_t nextDepth = highest + depthStep;
		if (nextDepth >= 8 || highest < 0)
			nextDepth = -1;

		if (getDepth(level, x, y + 1, z) >= 0)
		{
			int_t aboveDepth = getDepth(level, x, y + 1, z);
			if (aboveDepth >= 8)
				nextDepth = aboveDepth;
			else
				nextDepth = aboveDepth + 8;
		}

		if (adjacentSourceCount >= 2 && &material == static_cast<const Material *>(&Material::water))
		{
			if (level.getMaterial(x, y - 1, z).isSolid())
				nextDepth = 0;
			else if (&level.getMaterial(x, y - 1, z) == &material && level.getData(x, y, z) == 0)
				nextDepth = 0;
		}

		if (&material == static_cast<const Material *>(&Material::lava) && depth < 8 && nextDepth < 8 && nextDepth > depth && random.nextInt(4) != 0)
		{
			nextDepth = depth;
			shouldSetStatic = false;
		}

		if (nextDepth != depth)
		{
			depth = nextDepth;
			if (nextDepth < 0)
			{
				level.setTile(x, y, z, 0);
			}
			else
			{
				level.setData(x, y, z, nextDepth);
				level.addToTickNextTick(x, y, z, id);
				level.updateNeighborsAt(x, y, z, id);
			}
		}
		else if (shouldSetStatic)
		{
			setStatic(level, x, y, z);
		}
	}
	else
	{
		setStatic(level, x, y, z);
	}

	if (canSpreadTo(level, x, y - 1, z))
	{
		if (depth >= 8)
			level.setTileAndData(x, y - 1, z, id, depth);
		else
			level.setTileAndData(x, y - 1, z, id, depth + 8);
	}
	else if (depth >= 0 && (depth == 0 || blocksLiquidFlow(level, x, y - 1, z)))
	{
		const std::array<bool, 4> &spread = getSpread(level, x, y, z);
		int_t spreadDepth = depth + depthStep;
		if (depth >= 8)
			spreadDepth = 1;
		if (spreadDepth >= 8)
			return;

		if (spread[0])
			trySpreadTo(level, x - 1, y, z, spreadDepth);
		if (spread[1])
			trySpreadTo(level, x + 1, y, z, spreadDepth);
		if (spread[2])
			trySpreadTo(level, x, y, z - 1, spreadDepth);
		if (spread[3])
			trySpreadTo(level, x, y, z + 1, spreadDepth);
	}
}

void LiquidTileDynamic::trySpreadTo(Level &level, int_t x, int_t y, int_t z, int_t depth)
{
	if (!canSpreadTo(level, x, y, z))
		return;

	int_t tile = level.getTile(x, y, z);
	if (tile > 0)
	{
		if (&material == static_cast<const Material *>(&Material::lava))
			fizz(level, x, y, z);
		else if (Tile::tiles[tile] != nullptr)
			Tile::tiles[tile]->spawnResources(level, x, y, z, level.getData(x, y, z));
	}

	level.setTileAndData(x, y, z, id, depth);
}

int_t LiquidTileDynamic::getSlopeDistance(Level &level, int_t x, int_t y, int_t z, int_t distance, int_t fromDirection)
{
	int_t best = 1000;
	for (int_t direction = 0; direction < 4; direction++)
	{
		if ((direction == 0 && fromDirection == 1) || (direction == 1 && fromDirection == 0) || (direction == 2 && fromDirection == 3) || (direction == 3 && fromDirection == 2))
			continue;

		int_t nx = x;
		int_t nz = z;
		if (direction == 0)
			nx = x - 1;
		if (direction == 1)
			nx = x + 1;
		if (direction == 2)
			nz = z - 1;
		if (direction == 3)
			nz = z + 1;

		if (!blocksLiquidFlow(level, nx, y, nz) && (&level.getMaterial(nx, y, nz) != &material || level.getData(nx, y, nz) != 0))
		{
			if (!blocksLiquidFlow(level, nx, y - 1, nz))
				return distance;

			if (distance < 4)
			{
				int_t slope = getSlopeDistance(level, nx, y, nz, distance + 1, direction);
				if (slope < best)
					best = slope;
			}
		}
	}
	return best;
}

const std::array<bool, 4> &LiquidTileDynamic::getSpread(Level &level, int_t x, int_t y, int_t z)
{
	for (int_t direction = 0; direction < 4; direction++)
	{
		spreadDistance[direction] = 1000;
		int_t nx = x;
		int_t nz = z;
		if (direction == 0)
			nx = x - 1;
		if (direction == 1)
			nx = x + 1;
		if (direction == 2)
			nz = z - 1;
		if (direction == 3)
			nz = z + 1;

		if (!blocksLiquidFlow(level, nx, y, nz) && (&level.getMaterial(nx, y, nz) != &material || level.getData(nx, y, nz) != 0))
		{
			if (!blocksLiquidFlow(level, nx, y - 1, nz))
				spreadDistance[direction] = 0;
			else
				spreadDistance[direction] = getSlopeDistance(level, nx, y, nz, 1, direction);
		}
	}

	int_t best = spreadDistance[0];
	for (int_t direction = 1; direction < 4; direction++)
		if (spreadDistance[direction] < best)
			best = spreadDistance[direction];

	for (int_t direction = 0; direction < 4; direction++)
		spreadResult[direction] = spreadDistance[direction] == best;

	return spreadResult;
}

bool LiquidTileDynamic::blocksLiquidFlow(Level &level, int_t x, int_t y, int_t z)
{
	int_t tile = level.getTile(x, y, z);
	
	if (tile == Tile::doorWood.id || tile == Tile::doorIron.id || tile == Tile::signPost.id || tile == Tile::ladder.id || tile == Tile::reed.id)
		return true;
	if (tile == 0)
		return false;
	Tile *target = Tile::tiles[tile];
	return target != nullptr && target->material.isSolid();
}

int_t LiquidTileDynamic::getHighestNeighborDepth(Level &level, int_t x, int_t y, int_t z, int_t currentDepth)
{
	int_t depth = getDepth(level, x, y, z);
	if (depth < 0)
		return currentDepth;

	if (depth == 0)
		adjacentSourceCount++;
	if (depth >= 8)
		depth = 0;

	return currentDepth >= 0 && depth >= currentDepth ? currentDepth : depth;
}

bool LiquidTileDynamic::canSpreadTo(Level &level, int_t x, int_t y, int_t z)
{
	const Material &target = level.getMaterial(x, y, z);
	if (&target == &material)
		return false;
	return &target == static_cast<const Material *>(&Material::lava) ? false : !blocksLiquidFlow(level, x, y, z);
}

void LiquidTileDynamic::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	LiquidTile::onPlace(level, x, y, z);
	if (level.getTile(x, y, z) == id)
		level.addToTickNextTick(x, y, z, id);
}

LiquidTileStatic::LiquidTileStatic(int_t id, int_t tex, const Material &material) : LiquidTile(id, tex, material)
{
	setTicking(false);
	if (&material == static_cast<const Material *>(&Material::lava))
		setTicking(true);
}

void LiquidTileStatic::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	LiquidTile::neighborChanged(level, x, y, z, tile);
	if (level.getTile(x, y, z) == id)
		setDynamic(level, x, y, z);
}

void LiquidTileStatic::setDynamic(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	level.noNeighborUpdate = true;
	level.setTileAndDataNoUpdate(x, y, z, id - 1, data);
	level.setTilesDirty(x, y, z, x, y, z);
	level.addToTickNextTick(x, y, z, id - 1);
	level.noNeighborUpdate = false;
}

void LiquidTileStatic::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (&material != static_cast<const Material *>(&Material::lava) || Tile::tiles[51] == nullptr)
		return;

	int_t attempts = random.nextInt(3);
	for (int_t i = 0; i < attempts; i++)
	{
		x += random.nextInt(3) - 1;
		y++;
		z += random.nextInt(3) - 1;

		int_t tile = level.getTile(x, y, z);
		if (tile == 0)
		{
			if (isFlammable(level, x - 1, y, z) || isFlammable(level, x + 1, y, z) || isFlammable(level, x, y, z - 1) || isFlammable(level, x, y, z + 1) || isFlammable(level, x, y - 1, z) || isFlammable(level, x, y + 1, z))
			{
				level.setTile(x, y, z, 51);
				return;
			}
		}
		else if (Tile::tiles[tile] != nullptr && Tile::tiles[tile]->material.blocksMotion())
		{
			return;
		}
	}
}

bool LiquidTileStatic::isFlammable(Level &level, int_t x, int_t y, int_t z)
{
	return level.getMaterial(x, y, z).isFlammable();
}
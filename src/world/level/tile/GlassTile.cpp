#include "world/level/tile/GlassTile.h"

#include "java/Random.h"

GlassTile::GlassTile(int_t id, int_t tex, const Material &material, bool allowSame) : TransparentTile(id, tex, material, allowSame)
{
}

int_t GlassTile::getResourceCount(Random &random)
{
	return 0;
}
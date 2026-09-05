#include "world/level/tile/BookshelfTile.h"

#include "java/Random.h"

BookshelfTile::BookshelfTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
}

int_t BookshelfTile::getTexture(Facing face, int_t data)
{
	if (face == Facing::UP || face == Facing::DOWN)
		return 4; 

	return tex;
}

int_t BookshelfTile::getResourceCount(Random &random)
{
	return 0;
}
#pragma once

#include "world/level/tile/Tile.h"

class BookshelfTile : public Tile
{
public:
	BookshelfTile(int_t id, int_t tex, const Material &material);

	int_t getTexture(Facing face, int_t data) override;
	int_t getResourceCount(Random &random) override;
};
#include "world/level/tile/HugeMushroomTile.h"

#include "world/level/tile/MushroomTile.h"

HugeMushroomTile::HugeMushroomTile(int_t id, int_t tex, int_t variant) : Tile(id, tex, Material::wood), variant(variant)
{
	updateCachedProperties();
}

int_t HugeMushroomTile::getTexture(Facing face, int_t data)
{
	int_t f = static_cast<int_t>(face);
	int_t cap = tex - 16 - variant;
	if (data == 10 && f > 1)
		return tex - 1;
	if (data >= 1 && data <= 9 && f == 1)
		return cap;
	if (data >= 1 && data <= 3 && f == 2)
		return cap;
	if (data >= 7 && data <= 9 && f == 3)
		return cap;
	if ((data == 1 || data == 4 || data == 7) && f == 4)
		return cap;
	if ((data == 3 || data == 6 || data == 9) && f == 5)
		return cap;
	return tex;
}

int_t HugeMushroomTile::getResourceCount(Random &random)
{
	int_t count = random.nextInt(10) - 7;
	return count < 0 ? 0 : count;
}

int_t HugeMushroomTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Tile::brownMushroom.id + variant;
}

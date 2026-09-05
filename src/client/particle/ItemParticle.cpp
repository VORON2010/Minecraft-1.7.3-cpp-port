#include "client/particle/ItemParticle.h"

#include "client/renderer/Tesselator.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"

ItemParticle::ItemParticle(Level &level, double x, double y, double z, Item &item)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	tex = item.getIcon(ItemInstance(item.getShiftedIndex(), 1, 0));
	rCol = gCol = bCol = 1.0f;
	gravity = 1.0f;
	size /= 2.0f;
}

int_t ItemParticle::getParticleTexture() const
{
	return 2;
}

void ItemParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float u0 = (tex % 16 + uo / 4.0f) / 16.0f;
	float u1 = u0 + 0.015609375f;
	float v0 = (tex / 16 + vo / 4.0f) / 16.0f;
	float v1 = v0 + 0.015609375f;
	float r = 0.1f * size;
	float x = (float)(xo + (this->x - xo) * a - xOff);
	float y = (float)(yo + (this->y - yo) * a - yOff);
	float z = (float)(zo + (this->z - zo) * a - zOff);
	float br = getBrightness(a);
	t.color(br * rCol, br * gCol, br * bCol);
	t.vertexUV(x - xa * r - xa2 * r, y - ya * r, z - za * r - za2 * r, u0, v1);
	t.vertexUV(x - xa * r + xa2 * r, y + ya * r, z - za * r + za2 * r, u0, v0);
	t.vertexUV(x + xa * r + xa2 * r, y + ya * r, z + za * r + za2 * r, u1, v0);
	t.vertexUV(x + xa * r - xa2 * r, y - ya * r, z + za * r - za2 * r, u1, v1);
}

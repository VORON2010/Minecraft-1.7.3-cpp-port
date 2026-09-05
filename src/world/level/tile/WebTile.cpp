#include "world/level/tile/WebTile.h"

#include "world/entity/Entity.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "java/Random.h"

WebTile::WebTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
	updateCachedProperties();
}

bool WebTile::isCubeShaped()
{
	return false;
}

Tile::Shape WebTile::getRenderShape()
{
	return SHAPE_CROSS_TEXTURE;
}

bool WebTile::isSolidRender()
{
	return false;
}

AABB *WebTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	return nullptr;
}

void WebTile::entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	entity.isInWeb = true;
}

int_t WebTile::getResource(int_t data, Random &random)
{
	return Items::silk->getShiftedIndex();
}
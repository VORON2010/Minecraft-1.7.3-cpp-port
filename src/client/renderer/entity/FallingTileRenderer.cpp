#include "client/renderer/entity/FallingTileRenderer.h"

#include "client/renderer/Tesselator.h"
#include "pc/OpenGL.h"
#include "util/Mth.h"
#include "world/entity/item/FallingTile.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"

FallingTileRenderer::FallingTileRenderer(EntityRenderDispatcher &entityRenderDispatcher) : EntityRenderer(entityRenderDispatcher)
{
	shadowRadius = 0.5f;
}

void FallingTileRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	FallingTile &fallingTile = static_cast<FallingTile &>(entity);
	Tile *tile = Tile::tiles[fallingTile.tile];
	if (tile == nullptr)
		return;

	int_t xTile = Mth::floor(fallingTile.x);
	int_t yTile = Mth::floor(fallingTile.y);
	int_t zTile = Mth::floor(fallingTile.z);
	Level &level = fallingTile.getLevel();

	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	bindTexture(u"/terrain.png");
	glDisable(GL_LIGHTING);
	Tesselator::instance.begin();
	tileRenderer = TileRenderer(&level);
	Tesselator::instance.offset(-static_cast<double>(xTile) - 0.5, -static_cast<double>(yTile) - 0.5, -static_cast<double>(zTile) - 0.5);
	tileRenderer.tesselateInWorld(*tile, xTile, yTile, zTile);
	Tesselator::instance.offset(0.0, 0.0, 0.0);
	Tesselator::instance.end();
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

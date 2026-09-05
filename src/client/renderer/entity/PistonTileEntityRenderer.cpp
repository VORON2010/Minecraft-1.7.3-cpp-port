#include "client/renderer/entity/PistonTileEntityRenderer.h"

#include "client/renderer/TileRenderer.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/Textures.h"
#include "world/level/tile/entity/PistonTileEntity.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/PistonBaseTile.h"
#include "world/level/tile/PistonExtensionTile.h"

#include "OpenGL.h"

void PistonTileEntityRenderer::setLevel(LevelSource *level)
{
	delete tileRenderer;
	tileRenderer = new TileRenderer(level);
}

void PistonTileEntityRenderer::render(PistonTileEntity &entity, double x, double y, double z, float partialTick)
{
	if (entity.getProgress(partialTick) >= 1.0f)
		return;

	Tile *block = Tile::tiles[entity.getStoredBlockID()];
	if (block == nullptr)
		return;

	if (tileRenderer == nullptr && entity.level != nullptr)
		tileRenderer = new TileRenderer(entity.level.get());
	if (tileRenderer == nullptr)
		return;

	float offX = entity.getOffsetX(partialTick);
	float offY = entity.getOffsetY(partialTick);
	float offZ = entity.getOffsetZ(partialTick);

	if (textures != nullptr)
		textures->bind(textures->loadTexture(u"/terrain.png"));

	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	Tesselator &t = Tesselator::instance;
	t.begin();

	
	t.offset(x - entity.x + offX, y - entity.y + offY, z - entity.z + offZ);

	if (block->id == Tile::pistonExtension.id && entity.getProgress(partialTick) < 0.5f)
	{
		
		tileRenderer->tesselatePistonArmNoCulling(*block, entity.x, entity.y, entity.z, false, entity.getBlockMetadata());
	}
	else if (entity.shouldRenderHead() && !entity.isExtending())
	{
		
		PistonExtensionTile *ext = dynamic_cast<PistonExtensionTile *>(&Tile::pistonExtension);
		if (ext != nullptr)
		{
			int_t headTex = (entity.getStoredBlockID() == Tile::pistonStickyBase.id) ? 106 : 107;
			ext->setHeadTexture(headTex);
			tileRenderer->tesselatePistonArmNoCulling(*ext, entity.x, entity.y, entity.z, entity.getProgress(partialTick) < 0.5f, entity.getDirection());
			ext->clearHeadTexture();
		}
		
		t.offset(x - entity.x, y - entity.y, z - entity.z);
		tileRenderer->tesselatePistonBaseForceExtended(*block, entity.x, entity.y, entity.z, entity.getBlockMetadata());
	}
	else
	{
		
		tileRenderer->tesselateInWorldNoCulling(*block, entity.x, entity.y, entity.z);
	}

	t.offset(0.0, 0.0, 0.0);
	t.end();

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}

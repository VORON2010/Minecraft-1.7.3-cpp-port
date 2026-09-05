#pragma once

#include "client/renderer/Textures.h"
#include "java/Type.h"

class LevelSource;
class PistonTileEntity;
class TileRenderer;

class PistonTileEntityRenderer
{
public:
	PistonTileEntityRenderer(Textures *textures = nullptr) : textures(textures) {}

	void render(PistonTileEntity &entity, double x, double y, double z, float partialTick);
	void setLevel(LevelSource *level);

private:
	TileRenderer *tileRenderer = nullptr;
	Textures *textures = nullptr;
};

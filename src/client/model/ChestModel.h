#pragma once

#include "client/model/ModelPart.h"

// 1.4.7 ChestModel (item/chest.png, 64x64) and LargeChestModel (item/largechest.png, 128x64)
class ChestModel
{
public:
	ModelPart lid, base, lock;

	explicit ChestModel(bool large);
	void render();
};

class ChestTileEntity;
class Textures;

// 1.4.7 ChestRenderer
namespace ChestRenderer
{
	// x, y, z relative to the camera; a chest with no level is drawn as a single chest facing south
	void render(ChestTileEntity *chest, int_t data, double x, double y, double z, float a, Textures &textures);
	// ChestItemRenderHelper: the model drawn as an item
	void renderItem();
	// the texture manager the item helper binds with (set once by the level renderer)
	void setTextures(Textures &textures);
}

#pragma once

#include <memory>

#include "client/renderer/MapItemRenderer.h"
#include "client/renderer/TileRenderer.h"
#include "world/item/ItemInstance.h"

#include "java/Type.h"

class Minecraft;
class Textures;

namespace HeldItemRenderer
{
	void render(Textures &textures, TileRenderer &tileRenderer, ItemInstance &item, float brightness);
}
class ItemInHandRenderer
{
private:
	Minecraft &mc;
	ItemInstance selectedItem;
	std::shared_ptr<const ItemInstanceReference> selectedItemReference;

	float height = 0.0f;
	float oHeight = 0.0f;
	TileRenderer tileRenderer;

	int_t lastSlot = -1;

	std::unique_ptr<MapItemRenderer> mapItemRenderer;

	void renderItem(ItemInstance &item, float brightness);
	void renderMapFirstPerson(float a, float h);

public:
	ItemInHandRenderer(Minecraft &mc);

	void render(float a);
	void renderScreenEffect(float a);
	void renderTex(float a, int_t tex);
	void renderWater(float a);
	void renderFire(float a);

	void tick();
	void itemPlaced();
	void itemUsed();
};

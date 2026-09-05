#pragma once

#include "client/renderer/entity/EntityRenderer.h"
#include "java/Random.h"

class Font;
class Textures;
class ItemInstance;

class ItemRenderer : public EntityRenderer
{
public:
	bool renderWithColor = true;

	ItemRenderer(EntityRenderDispatcher &entityRenderDispatcher);

	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
	void renderGuiItem(Font &font, Textures &textures, ItemInstance &item, int_t x, int_t y);
	void renderGuiItemDecorations(Font &font, Textures &textures, ItemInstance &item, int_t x, int_t y);

private:
	TileRenderer tileRenderer;
	Random random;

	void blit(int_t x, int_t y, int_t sx, int_t sy, int_t w, int_t h);
};

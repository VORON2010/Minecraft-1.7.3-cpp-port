#include "client/renderer/entity/SheepRenderer.h"

#include "OpenGL.h"
#include "client/model/SheepFurModel.h"
#include "client/model/SheepModel.h"
#include "client/renderer/Textures.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/animal/Sheep.h"

namespace
{
	const float FLEECE_COLORS[16][3] = {
		{1.0f, 1.0f, 1.0f}, {0.95f, 0.7f, 0.2f}, {0.9f, 0.5f, 0.85f}, {0.6f, 0.7f, 0.95f},
		{0.9f, 0.9f, 0.2f}, {0.5f, 0.8f, 0.1f}, {0.95f, 0.7f, 0.8f}, {0.3f, 0.3f, 0.3f},
		{0.6f, 0.6f, 0.6f}, {0.3f, 0.6f, 0.7f}, {0.7f, 0.4f, 0.9f}, {0.2f, 0.4f, 0.8f},
		{0.5f, 0.4f, 0.3f}, {0.4f, 0.5f, 0.2f}, {0.8f, 0.3f, 0.3f}, {0.1f, 0.1f, 0.1f}
	};
}

SheepRenderer::SheepRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<SheepModel>(), 0.7f)
{
	setArmor(std::make_shared<SheepFurModel>());
}

bool SheepRenderer::prepareArmor(Mob &mobBase, int_t layer, float a)
{
	Sheep &sheep = static_cast<Sheep &>(mobBase);
	if (layer != 0 || sheep.isSheared() || entityRenderDispatcher.textures == nullptr)
		return false;
	entityRenderDispatcher.textures->bind(entityRenderDispatcher.textures->loadTexture(u"/mob/sheep_fur.png"));
	float brightness = sheep.getBrightness(a);
	const float *color = FLEECE_COLORS[sheep.getFleeceColor() & 15];
	glColor3f(brightness * color[0], brightness * color[1], brightness * color[2]);
	return true;
}

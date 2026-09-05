#include "client/renderer/entity/HumanoidMobRenderer.h"

#include <memory>

#include "client/model/HumanoidModel.h"
#include "client/renderer/Textures.h"
#include "client/renderer/TileRenderer.h"
#include "client/renderer/ItemInHandRenderer.h"
#include "world/entity/Mob.h"
#include "world/item/Item.h"
#include "world/level/tile/Tile.h"
#include "OpenGL.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"

HumanoidMobRenderer::HumanoidMobRenderer(EntityRenderDispatcher &entityRenderDispatcher, bool zombieArms, bool hasHeldItem)
	: MobRenderer(entityRenderDispatcher, std::make_shared<HumanoidModel>(), 0.5f), zombieArms(zombieArms), hasHeldItem(hasHeldItem)
{
	humanoidModel = std::static_pointer_cast<HumanoidModel>(model);
}

void HumanoidMobRenderer::setModel(const std::shared_ptr<Model> &model)
{
	MobRenderer::setModel(model);
	humanoidModel = std::static_pointer_cast<HumanoidModel>(model);
}

void HumanoidMobRenderer::additionalRendering(Mob &mob, float a)
{
	(void)a;
	humanoidModel->holdingRightHand = hasHeldItem;
	humanoidModel->zombieArms = zombieArms;
	if (!hasHeldItem || entityRenderDispatcher.textures == nullptr)
		return;

	ItemInstance *item = mob.getCarriedItem();
	if (item == nullptr || item->isEmpty())
		return;

	glPushMatrix();
	humanoidModel->arm0.translateTo(1.0f / 16.0f);
	glTranslatef(-1.0f / 16.0f, 7.0f / 16.0f, 1.0f / 16.0f);

	Tile *tile = item->itemID >= 0 && item->itemID < static_cast<int_t>(Tile::tiles.size()) ? Tile::tiles[item->itemID] : nullptr;
	if (tile != nullptr && TileRenderer::canRender(tile->getRenderShape()))
	{
		float s = 0.5f;
		glTranslatef(0.0f, 0.1875f, -0.3125f);
		s *= 0.75f;
		glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glScalef(s, -s, s);
	}
	else if (item->getItem() && item->getItem()->isFull3D())
	{
		float s = 0.625f;
		if (item->getItem()->shouldRotateAroundWhenRendering())
		{
			glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
			glTranslatef(0.0f, -0.125f, 0.0f);
		}
		glTranslatef(0.0f, 0.1875f, 0.0f);
		glScalef(s, -s, s);
		glRotatef(-100.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
	}
	else
	{
		float s = 0.375f;
		glTranslatef(0.25f, 0.1875f, -0.1875f);
		glScalef(s, s, s);
		glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(20.0f, 0.0f, 0.0f, 1.0f);
	}

	TileRenderer tileRenderer(false, false);
	HeldItemRenderer::render(*entityRenderDispatcher.textures, tileRenderer, *item,
		mob.getBrightness(1.0f));
	glPopMatrix();
}

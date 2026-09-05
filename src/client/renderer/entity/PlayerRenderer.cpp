#include "client/renderer/entity/PlayerRenderer.h"

#include "client/renderer/Textures.h"
#include "client/renderer/ItemInHandRenderer.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/ItemArmor.h"
#include "world/item/Items.h"
#include "util/Mth.h"

#include <cmath>

#include "OpenGL.h"

namespace
{
	const jstring armorFilenamePrefix[] = {u"cloth", u"chain", u"iron", u"diamond", u"gold"};
}

PlayerRenderer::PlayerRenderer(EntityRenderDispatcher &entityRenderDispatcher) : MobRenderer(entityRenderDispatcher, Util::make_shared<HumanoidModel>(0.0f), 0.5f)
{
	humanoidModel = std::static_pointer_cast<HumanoidModel>(model);

	armorParts1 = Util::make_shared<HumanoidModel>(1.0f);
	armorParts2 = Util::make_shared<HumanoidModel>(0.5f);
}

void PlayerRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	Player &mob = reinterpret_cast<Player &>(entity);

	humanoidModel->holdingRightHand = mob.inventory.getSelected() != nullptr;
	humanoidModel->sneaking = mob.isSneaking();
	armorParts1->holdingRightHand = humanoidModel->holdingRightHand;
	armorParts2->holdingRightHand = humanoidModel->holdingRightHand;
	armorParts1->sneaking = humanoidModel->sneaking;
	armorParts2->sneaking = humanoidModel->sneaking;

	double yp = y - mob.heightOffset;
	if (mob.isSneaking() && entityRenderDispatcher.player.get() != &mob)
		yp -= 0.125;

	if (mob.isAlive() && mob.sleeping)
	{
		MobRenderer::render(mob, x + mob.bedViewX, yp + mob.bedViewY, z + mob.bedViewZ, rot, a);
	}
	else
	{
		MobRenderer::render(mob, x, yp, z, rot, a);
	}
	renderName(mob, x, yp, z);

	humanoidModel->holdingRightHand = false;
	humanoidModel->sneaking = false;
	armorParts1->holdingRightHand = false;
	armorParts2->holdingRightHand = false;
	armorParts1->sneaking = false;
	armorParts2->sneaking = false;
}


void PlayerRenderer::setupRotations(Mob &mobBase, float bob, float bodyRot, float a)
{
	Player &mob = static_cast<Player &>(mobBase);
	if (mob.isAlive() && mob.sleeping)
	{
		glRotatef(mob.getBedOrientationInDegrees(), 0.0f, 1.0f, 0.0f);
		glRotatef(getFlipDegrees(mob), 0.0f, 0.0f, 1.0f);
		glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
	}
	else
	{
		MobRenderer::setupRotations(mob, bob, bodyRot, a);
	}
}

bool PlayerRenderer::prepareArmor(Mob &mobBase, int_t layer, float a)
{
	(void)a;
	Player &mob = static_cast<Player &>(mobBase);
	ItemInstance &stack = mob.inventory.armorInventory[3 - layer];
	if (stack.isEmpty())
		return false;

	Item *item = stack.getItem();
	auto *armorItem = dynamic_cast<ItemArmor *>(item);
	if (armorItem == nullptr || entityRenderDispatcher.textures == nullptr)
		return false;

	auto parts = layer == 2 ? armorParts2 : armorParts1;
	armor = parts;
	parts->holdingRightHand = humanoidModel->holdingRightHand;
	parts->holdingLeftHand = humanoidModel->holdingLeftHand;
	parts->sneaking = humanoidModel->sneaking;
	parts->attackTime = humanoidModel->attackTime;
	parts->riding = humanoidModel->riding;
	parts->head.visible = layer == 0;
	parts->hair.visible = layer == 0;
	parts->body.visible = layer == 1 || layer == 2;
	parts->arm0.visible = layer == 1;
	parts->arm1.visible = layer == 1;
	parts->leg0.visible = layer == 2 || layer == 3;
	parts->leg1.visible = layer == 2 || layer == 3;
	entityRenderDispatcher.textures->bind(entityRenderDispatcher.textures->loadTexture(
		u"/armor/" + armorFilenamePrefix[armorItem->renderIndex] + u"_" + (layer == 2 ? u"2" : u"1") + u".png"));
	return true;
}


void PlayerRenderer::scale(Mob &mob, float a)
{
	float s = 0.9375f;
	glScalef(s, s, s);
}


void PlayerRenderer::additionalRendering(Mob &mobBase, float a)
{
	Player &mob = static_cast<Player &>(mobBase);
	Textures *textures = entityRenderDispatcher.textures;
	if (textures == nullptr)
		return;
	TileRenderer tileRenderer(false, false);

	ItemInstance &headItem = mob.inventory.armorInventory[3];
	if (!headItem.isEmpty() && headItem.getItem()->getShiftedIndex() < 256)
	{
		glPushMatrix();
		humanoidModel->head.translateTo(1.0f / 16.0f);
		Tile *tile = Tile::tiles[headItem.itemID];
		if (TileRenderer::canRender(tile->getRenderShape()))
		{
			float s = 0.625f;
			glTranslatef(0.0f, -0.25f, 0.0f);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glScalef(s, -s, s);
		}
		HeldItemRenderer::render(*textures, tileRenderer, headItem, mob.getBrightness(1.0f));
		glPopMatrix();
	}

	if (mob.name == u"deadmau5" && !mob.customTextureUrl.empty())
	{
		int_t skinId = textures->loadHttpTexture(mob.customTextureUrl);
		if (skinId >= 0)
		{
			textures->bind(skinId);
			for (int_t ear = 0; ear < 2; ++ear)
			{
				float headYaw = mob.yRotO + (mob.yRot - mob.yRotO) * a - (mob.yBodyRotO + (mob.yBodyRot - mob.yBodyRotO) * a);
				float headPitch = mob.xRotO + (mob.xRot - mob.xRotO) * a;
				glPushMatrix();
				glRotatef(headYaw, 0.0f, 1.0f, 0.0f);
				glRotatef(headPitch, 1.0f, 0.0f, 0.0f);
				glTranslatef(6.0f / 16.0f * (ear * 2 - 1), 0.0f, 0.0f);
				glTranslatef(0.0f, -6.0f / 16.0f, 0.0f);
				glRotatef(-headPitch, 1.0f, 0.0f, 0.0f);
				glRotatef(-headYaw, 0.0f, 1.0f, 0.0f);
				float s = 4.0f / 3.0f;
				glScalef(s, s, s);
				humanoidModel->renderEars(1.0f / 16.0f);
				glPopMatrix();
			}
		}
	}

	if (!mob.cloakTexture.empty())
	{
		int_t cloakId = textures->loadHttpTexture(mob.cloakTexture);
		if (cloakId >= 0)
		{
			textures->bind(cloakId);
			glPushMatrix();
			glTranslatef(0.0f, 0.0f, 2.0f / 16.0f);

			double cloakX = mob.xCloakO + (mob.xCloak - mob.xCloakO) * a - (mob.xo + (mob.x - mob.xo) * a);
			double cloakY = mob.yCloakO + (mob.yCloak - mob.yCloakO) * a - (mob.yo + (mob.y - mob.yo) * a);
			double cloakZ = mob.zCloakO + (mob.zCloak - mob.zCloakO) * a - (mob.zo + (mob.z - mob.zo) * a);
			float bodyRot = mob.yBodyRotO + (mob.yBodyRot - mob.yBodyRotO) * a;
			double sinRot = std::sin(bodyRot * Mth::PI / 180.0f);
			double cosRot = -std::cos(bodyRot * Mth::PI / 180.0f);

			float pitch = static_cast<float>(cloakY * 10.0);
			if (pitch < -6.0f) pitch = -6.0f;
			if (pitch > 32.0f) pitch = 32.0f;
			float sway = static_cast<float>((cloakX * sinRot + cloakZ * cosRot) * 100.0);
			float twist = static_cast<float>((cloakX * cosRot - cloakZ * sinRot) * 100.0);
			if (sway < 0.0f) sway = 0.0f;
			float walkBob = mob.oBob + (mob.bob - mob.oBob) * a;
			pitch += std::sin((mob.walkDistO + (mob.walkDist - mob.walkDistO) * a) * 6.0f) * 32.0f * walkBob;
			if (mob.isSneaking()) pitch += 25.0f;

			glRotatef(6.0f + sway / 2.0f + pitch, 1.0f, 0.0f, 0.0f);
			glRotatef(twist / 2.0f, 0.0f, 0.0f, 1.0f);
			glRotatef(-twist / 2.0f, 0.0f, 1.0f, 0.0f);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			humanoidModel->renderCloak(1.0f / 16.0f);
			glPopMatrix();
		}
	}

	ItemInstance *item = mob.inventory.getSelected();
	if (item == nullptr)
		return;
	ItemInstance fishingStick;

	glPushMatrix();
	humanoidModel->arm0.translateTo(1.0f / 16.0f);
	glTranslatef(-1.0f / 16.0f, 7.0f / 16.0f, 1.0f / 16.0f);
	if (mob.fishEntity != nullptr)
	{
		fishingStick = ItemInstance(Items::stick->getShiftedIndex());
		item = &fishingStick;
	}

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

	HeldItemRenderer::render(*textures, tileRenderer, *item, mob.getBrightness(1.0f));
	glPopMatrix();
}


void PlayerRenderer::renderName(Player &player, double x, double y, double z)
{
	if (entityRenderDispatcher.options == nullptr || entityRenderDispatcher.options->hideGui ||
		entityRenderDispatcher.player == nullptr || entityRenderDispatcher.player.get() == &player)
		return;

	float distance = player.distanceTo(*entityRenderDispatcher.player);
	float maxDistance = player.isSneaking() ? 32.0f : 64.0f;
	if (distance >= maxDistance)
		return;

	if (!player.isSneaking())
	{
		if (player.sleeping)
			renderLivingLabel(player, player.name, x, y - 1.5, z, 64);
		else
			renderLivingLabel(player, player.name, x, y, z, 64);
		return;
	}

	Font &font = getFont();
	float scale = (1.0f / 60.0f) * 1.6f;
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + 2.3f, static_cast<float>(z));
	glNormal3f(0.0f, 1.0f, 0.0f);
	glRotatef(-entityRenderDispatcher.playerRotY, 0.0f, 1.0f, 0.0f);
	glRotatef(entityRenderDispatcher.playerRotX, 1.0f, 0.0f, 0.0f);
	glScalef(-scale, -scale, scale);
	glDisable(GL_LIGHTING);
	glTranslatef(0.0f, 0.25f / scale, 0.0f);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Tesselator &t = Tesselator::instance;
	glDisable(GL_TEXTURE_2D);
	t.begin();
	int_t halfWidth = font.width(player.name) / 2;
	t.color(0.0f, 0.0f, 0.0f, 0.25f);
	t.vertex(-halfWidth - 1, -1.0, 0.0);
	t.vertex(-halfWidth - 1, 8.0, 0.0);
	t.vertex(halfWidth + 1, 8.0, 0.0);
	t.vertex(halfWidth + 1, -1.0, 0.0);
	t.end();
	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	font.draw(player.name, -font.width(player.name) / 2, 0, 553648127);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();
}


void PlayerRenderer::renderLivingLabel(Player &player, const jstring &name, double x, double y, double z, int_t maxDistance)
{
	if (entityRenderDispatcher.player == nullptr || player.distanceTo(*entityRenderDispatcher.player) > maxDistance)
		return;

	Font &font = getFont();
	float scale = (1.0f / 60.0f) * 1.6f;
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + 2.3f, static_cast<float>(z));
	glNormal3f(0.0f, 1.0f, 0.0f);
	glRotatef(-entityRenderDispatcher.playerRotY, 0.0f, 1.0f, 0.0f);
	glRotatef(entityRenderDispatcher.playerRotX, 1.0f, 0.0f, 0.0f);
	glScalef(-scale, -scale, scale);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int_t yOffset = name == u"deadmau5" ? -10 : 0;
	Tesselator &t = Tesselator::instance;
	glDisable(GL_TEXTURE_2D);
	t.begin();
	int_t halfWidth = font.width(name) / 2;
	t.color(0.0f, 0.0f, 0.0f, 0.25f);
	t.vertex(-halfWidth - 1, -1 + yOffset, 0.0);
	t.vertex(-halfWidth - 1, 8 + yOffset, 0.0);
	t.vertex(halfWidth + 1, 8 + yOffset, 0.0);
	t.vertex(halfWidth + 1, -1 + yOffset, 0.0);
	t.end();
	glEnable(GL_TEXTURE_2D);
	font.draw(name, -font.width(name) / 2, yOffset, 553648127);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	font.draw(name, -font.width(name) / 2, yOffset, -1);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();
}

void PlayerRenderer::renderHand()
{
	humanoidModel->attackTime = 0.0f;
	humanoidModel->setupAnim(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / 16.0f);
	humanoidModel->arm0.render(1.0f / 16.0f);
}

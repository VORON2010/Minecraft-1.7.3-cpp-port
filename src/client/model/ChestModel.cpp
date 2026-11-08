#include "client/model/ChestModel.h"

#include <ctime>

#include "OpenGL.h"
#include "client/renderer/Textures.h"
#include "util/Mth.h"
#include "world/level/tile/entity/ChestTileEntity.h"

ChestModel::ChestModel(bool large)
{
	int_t texW = large ? 128 : 64;
	int_t width = large ? 30 : 14;
	lid.setTexSize(texW, 64);
	lid.addBox(0, 0, 0.0f, -5.0f, -14.0f, width, 5, 14);
	lid.setPos(1.0f, 7.0f, 15.0f);
	lock.setTexSize(texW, 64);
	lock.addBox(0, 0, -1.0f, -2.0f, -15.0f, 2, 4, 1);
	lock.setPos(large ? 16.0f : 8.0f, 7.0f, 15.0f);
	base.setTexSize(texW, 64);
	base.addBox(0, 19, 0.0f, 0.0f, 0.0f, width, 10, 14);
	base.setPos(1.0f, 6.0f, 1.0f);
}

void ChestModel::render()
{
	lock.xRot = lid.xRot;
	lid.render(0.0625f);
	lock.render(0.0625f);
	base.render(0.0625f);
}

namespace ChestRenderer
{

static bool isChristmas()
{
	std::time_t now = std::time(nullptr);
	std::tm *tm = std::localtime(&now);
	return tm != nullptr && tm->tm_mon + 1 == 12 && tm->tm_mday >= 24 && tm->tm_mday <= 26;
}

void render(ChestTileEntity *chest, int_t data, double x, double y, double z, float a, Textures &textures)
{
	static ChestModel single(false);
	static ChestModel large(true);
	static const bool christmas = isChristmas();

	if (chest != nullptr)
	{
		chest->updateNeighbors();
		if (chest->northNeighbor != nullptr || chest->westNeighbor != nullptr)
			return; // the other half draws the large chest
	}

	bool isLarge = chest != nullptr && (chest->eastNeighbor != nullptr || chest->southNeighbor != nullptr);
	ChestModel &model = isLarge ? large : single;
	if (isLarge)
		glBindTexture(GL_TEXTURE_2D, textures.loadTexture(christmas ? u"/item/largexmaschest.png" : u"/item/largechest.png"));
	else
		glBindTexture(GL_TEXTURE_2D, textures.loadTexture(christmas ? u"/item/xmaschest.png" : u"/item/chest.png"));

	glPushMatrix();
	glEnable(GL_RESCALE_NORMAL);
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + 1.0f, static_cast<float>(z) + 1.0f);
	glScalef(1.0f, -1.0f, -1.0f);
	glTranslatef(0.5f, 0.5f, 0.5f);
	int_t rot = 0;
	if (data == 2) rot = 180;
	if (data == 3) rot = 0;
	if (data == 4) rot = 90;
	if (data == 5) rot = -90;
	if (data == 2 && chest != nullptr && chest->eastNeighbor != nullptr)
		glTranslatef(1.0f, 0.0f, 0.0f);
	if (data == 5 && chest != nullptr && chest->southNeighbor != nullptr)
		glTranslatef(0.0f, 0.0f, -1.0f);
	glRotatef(static_cast<float>(rot), 0.0f, 1.0f, 0.0f);
	glTranslatef(-0.5f, -0.5f, -0.5f);

	float open = 0.0f;
	if (chest != nullptr)
	{
		open = chest->oOpenness + (chest->openness - chest->oOpenness) * a;
		for (ChestTileEntity *n : {chest->northNeighbor, chest->westNeighbor})
			if (n != nullptr)
				open = std::max(open, n->oOpenness + (n->openness - n->oOpenness) * a);
	}
	open = 1.0f - open;
	open = 1.0f - open * open * open;
	model.lid.xRot = -(open * Mth::PI / 2.0f);
	model.render();
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
}

static Textures *itemTextures = nullptr;

void setTextures(Textures &textures)
{
	itemTextures = &textures;
}

void renderItem()
{
	if (itemTextures != nullptr)
		render(nullptr, 0, 0.0, 0.0, 0.0, 0.0f, *itemTextures);
}

}

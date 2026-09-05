#include "client/renderer/SignRenderer.h"

#include "client/gui/Font.h"
#include "client/model/SignModel.h"
#include "client/renderer/Textures.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/SignTileEntity.h"

#include "OpenGL.h"

namespace
{
	SignModel signModel;
}

void SignRenderer::renderSign(SignTileEntity &sign, double x, double y, double z, float scale, Font &font, Textures &textures)
{
	(void)scale;
	int_t tileId = sign.level != nullptr ? sign.level->getTile(sign.x, sign.y, sign.z) : 0;
	glPushMatrix();
	float signScale = 2.0f / 3.0f;
	float rot;
	if (tileId == 63)
	{
		glTranslatef(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 12.0f / 16.0f * signScale, static_cast<float>(z) + 0.5f);
		rot = static_cast<float>(sign.getData() * 360) / 16.0f;
		glRotatef(-rot, 0.0f, 1.0f, 0.0f);
		signModel.stick.visible = true;
	}
	else
	{
		int_t data = sign.getData();
		rot = 0.0f;
		if (data == 2)
			rot = 180.0f;
		if (data == 4)
			rot = 90.0f;
		if (data == 5)
			rot = -90.0f;
		glTranslatef(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 12.0f / 16.0f * signScale, static_cast<float>(z) + 0.5f);
		glRotatef(-rot, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, -(5.0f / 16.0f), -(7.0f / 16.0f));
		signModel.stick.visible = false;
	}

	glBindTexture(GL_TEXTURE_2D, textures.loadTexture(u"/item/sign.png"));
	glPushMatrix();
	glScalef(signScale, -signScale, -signScale);
	signModel.render();
	glPopMatrix();
	float textScale = (1.0f / 60.0f) * signScale;
	glTranslatef(0.0f, 0.5f * signScale, 0.07f * signScale);
	glScalef(textScale, -textScale, textScale);
	glNormal3f(0.0f, 0.0f, -1.0f * textScale);
	glDepthMask(GL_FALSE);
	int_t textColor = 0;

	for (int_t i = 0; i < static_cast<int_t>(sign.signText.size()); ++i)
	{
		jstring line = sign.signText[i];
		if (i == sign.lineBeingEdited)
		{
			line = u"> " + line + u" <";
			font.draw(line, -font.width(line) / 2, i * 10 - static_cast<int_t>(sign.signText.size()) * 5, textColor);
		}
		else
		{
			font.draw(line, -font.width(line) / 2, i * 10 - static_cast<int_t>(sign.signText.size()) * 5, textColor);
		}
	}

	glDepthMask(GL_TRUE);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();
}

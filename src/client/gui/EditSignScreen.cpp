#include "client/gui/EditSignScreen.h"

#include <utility>

#include "client/Minecraft.h"
#include "client/gui/Font.h"
#include "client/renderer/SignRenderer.h"
#include "client/renderer/Textures.h"
#include "network/NetClientHandler.h"
#include "network/PacketWorld.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/SignTileEntity.h"
#include "SharedConstants.h"
#include "lwjgl/Keyboard.h"

#include "OpenGL.h"

EditSignScreen::EditSignScreen(Minecraft &minecraft, std::shared_ptr<SignTileEntity> sign) : Screen(minecraft), sign(sign)
{
}

void EditSignScreen::init()
{
	buttons.clear();
	lwjgl::Keyboard::enableRepeatEvents(true);
	buttons.push_back(Util::make_shared<Button>(0, width / 2 - 100, height / 4 + 120, u"Done"));
}

void EditSignScreen::removed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
	if (minecraft.isOnline())
	{
		auto signLines = std::shared_ptr<const std::array<jstring, 4>>(sign, &sign->signText);
		minecraft.connection->addToSendQueue(std::make_unique<Packet130UpdateSign>(
			sign->x, sign->y, sign->z, std::move(signLines)));
	}
}

void EditSignScreen::tick()
{
	updateCounter++;
}

void EditSignScreen::render(int_t xm, int_t ym, float a)
{
	renderBackground(0);
	drawCenteredString(font, u"Edit sign message:", width / 2, 40, 0xFFFFFF);

	glPushMatrix();
	glTranslatef(static_cast<float>(width / 2), 0.0f, 50.0f);
	float s = 93.75f;
	glScalef(-s, -s, -s);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

	int_t tileId = (sign->level != nullptr) ? sign->level->getTile(sign->x, sign->y, sign->z) : 0;
	bool isPost = (tileId == 63);
	if (isPost)
	{
		float rot = static_cast<float>(sign->getData() * 360) / 16.0f;
		glRotatef(rot, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, -1.0625f, 0.0f);
	}
	else
	{
		int_t data = sign->getData();
		float rot = 0.0f;
		if (data == 2)
			rot = 180.0f;
		else if (data == 4)
			rot = 90.0f;
		else if (data == 5)
			rot = -90.0f;
		glRotatef(rot, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, -1.0625f, 0.0f);
	}

	if (updateCounter / 6 % 2 == 0)
		sign->lineBeingEdited = editLine;
	else
		sign->lineBeingEdited = -1;

	SignRenderer::renderSign(*sign, -0.5, -0.75, -0.5, 0.0f, font, minecraft.textures);
	sign->lineBeingEdited = -1;

	glPopMatrix();
	Screen::render(xm, ym, a);
}

void EditSignScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == 200) 
		editLine = (editLine - 1) & 3;
	if (eventKey == 208 || eventKey == 28) 
		editLine = (editLine + 1) & 3;
	if (eventKey == 14 && sign->signText[editLine].length() > 0) 
		sign->signText[editLine] = sign->signText[editLine].substr(0, sign->signText[editLine].length() - 1);
	if (SharedConstants::acceptableLetters.find(eventCharacter) != jstring::npos && sign->signText[editLine].length() < 15)
		sign->signText[editLine] += eventCharacter;
}

void EditSignScreen::buttonClicked(Button &button)
{
	if (button.id == 0)
	{
		sign->setChanged();
		minecraft.setScreen(nullptr);
	}
}

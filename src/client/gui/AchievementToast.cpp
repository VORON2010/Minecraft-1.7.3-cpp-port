#include "client/gui/AchievementToast.h"

#include "OpenGL.h"
#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gui/ScreenSizeCalculator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "java/System.h"
#include "world/stats/Achievement.h"
#include "world/stats/StatCollector.h"

AchievementToast::AchievementToast(Minecraft &minecraft) : minecraft(minecraft)
{
}

void AchievementToast::queueTakenAchievement(Achievement &achievement)
{
	title = StatCollector::translate(u"achievement.get");
	description = achievement.statName;
	startTime = System::currentTimeMillis();
	this->achievement = &achievement;
	information = false;
}

void AchievementToast::queueAchievementInformation(Achievement &achievement)
{
	title = achievement.statName;
	description = achievement.getDescription();
	startTime = System::currentTimeMillis() - 2500;
	this->achievement = &achievement;
	information = true;
}

void AchievementToast::updateWindowScale()
{
	glViewport(0, 0, minecraft.width, minecraft.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	ScreenSizeCalculator size(minecraft.options, minecraft.width, minecraft.height);
	windowWidth = size.getWidth();
	windowHeight = size.getHeight();
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, windowWidth, windowHeight, 0.0, 1000.0, 3000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -2000.0f);
}

void AchievementToast::render()
{
	if (achievement == nullptr || startTime == 0)
		return;

	double progress = (System::currentTimeMillis() - startTime) / 3000.0;
	if (!information && (progress < 0.0 || progress > 1.0))
	{
		startTime = 0;
		return;
	}

	updateWindowScale();
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	double offset = progress * 2.0;
	if (offset > 1.0)
		offset = 2.0 - offset;
	offset *= 4.0;
	offset = 1.0 - offset;
	if (offset < 0.0)
		offset = 0.0;
	offset *= offset;
	offset *= offset;

	int_t x = windowWidth - 160;
	int_t y = -static_cast<int_t>(offset * 36.0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/achievement/bg.png"));
	glDisable(GL_LIGHTING);
	blit(x, y, 96, 202, 160, 32);
	if (information)
		minecraft.font->drawWordWrap(description, x + 30, y + 7, 120, -1);
	else
	{
		minecraft.font->draw(title, x + 30, y + 7, -256);
		minecraft.font->draw(description, x + 30, y + 18, -1);
	}

	glPushMatrix();
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);
	ItemInstance item = achievement->item;
	itemRenderer.renderGuiItem(*minecraft.font, minecraft.textures, item, x + 8, y + 8);
	glDisable(GL_LIGHTING);
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
}

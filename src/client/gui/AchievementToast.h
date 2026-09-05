#pragma once

#include "client/gui/GuiComponent.h"
#include "java/String.h"
#include "java/Type.h"

class Achievement;
class Minecraft;

class AchievementToast : public GuiComponent
{
private:
	Minecraft &minecraft;
	int_t windowWidth = 0;
	int_t windowHeight = 0;
	jstring title;
	jstring description;
	Achievement *achievement = nullptr;
	long_t startTime = 0;
	bool information = false;

	void updateWindowScale();

public:
	explicit AchievementToast(Minecraft &minecraft);

	void queueTakenAchievement(Achievement &achievement);
	void queueAchievementInformation(Achievement &achievement);
	void render();
};

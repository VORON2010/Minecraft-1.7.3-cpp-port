#pragma once

#include "client/gui/GuiComponent.h"

#include "java/Random.h"

class Minecraft;

class Gui : public GuiComponent
{
private:

	Random random;
	int_t tickCount = 0;

	jstring nowPlayingString = u"";
	int_t nowPlayingTime = 0;
	bool animateNowPlayingColor = false;
	float prevVignetteBrightness = 1.0f;

	Minecraft &minecraft;
	void renderSlot(int_t slot, int_t x, int_t y, float a);
	void renderPumpkinBlur(int_t width, int_t height);
	void renderVignette(float brightness, int_t width, int_t height);
	void renderPortalOverlay(float portalTime, int_t width, int_t height);
public:
	float progress = 0.0f;
	float tbr = 1.0f;

	Gui(Minecraft &minecraft);
	void render(float a, bool inScreen, int_t xm, int_t ym);

	void tick();
	void setRecordPlayingMessage(const jstring &name);
};

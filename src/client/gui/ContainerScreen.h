#pragma once

#include "client/gui/Screen.h"

class ItemInstance;

class ContainerScreen : public Screen
{
protected:
	explicit ContainerScreen(Minecraft &minecraft);
	static bool shouldClose(bool alive, bool removed);
	static jstring getTooltipName(const ItemInstance &stack);

public:
	void tick() override;
};

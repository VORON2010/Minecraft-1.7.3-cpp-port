#pragma once

#include "client/gui/Screen.h"

class ItemInstance;

class ContainerScreen : public Screen
{
protected:
	explicit ContainerScreen(Minecraft &minecraft);
	static bool shouldClose(bool alive, bool removed);

public:
	static jstring getTooltipName(const ItemInstance &stack);
	static std::vector<jstring> getTooltipLines(const ItemInstance &stack, const Options *options);
	void tick() override;
};

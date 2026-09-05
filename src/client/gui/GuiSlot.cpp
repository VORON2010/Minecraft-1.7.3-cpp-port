#include "client/gui/GuiSlot.h"

#include "client/Minecraft.h"
#include "client/renderer/Tesselator.h"

#include "java/System.h"

#include "lwjgl/Mouse.h"

#include "OpenGL.h"

GuiSlot::GuiSlot(Minecraft &minecraft, int_t width, int_t height, int_t top, int_t bottom, int_t slotHeight)
	: minecraft(minecraft), width(width), height(height), top(top), bottom(bottom), right(width), left(0), posZ(slotHeight)
{

}

void GuiSlot::setRenderSelection(bool renderSelection)
{
	this->renderSelection = renderSelection;
}

void GuiSlot::setHeader(bool hasHeader, int_t headerPadding)
{
	this->hasHeader = hasHeader;
	this->headerPadding = headerPadding;
	if (!this->hasHeader)
		this->headerPadding = 0;
}

int_t GuiSlot::getSlotIndex(int_t x, int_t y)
{
	int_t leftEdge = width / 2 - 110;
	int_t rightEdge = width / 2 + 110;
	int_t relativeY = y - top - headerPadding + static_cast<int_t>(amountScrolled) - 4;
	int_t index = relativeY / posZ;
	return x >= leftEdge && x <= rightEdge && index >= 0 && relativeY >= 0 && index < getSize() ? index : -1;
}

void GuiSlot::registerScrollButtons(int_t scrollUpButtonID, int_t scrollDownButtonID)
{
	this->scrollUpButtonID = scrollUpButtonID;
	this->scrollDownButtonID = scrollDownButtonID;
}

void GuiSlot::bindAmountScrolled()
{
	int_t maxScroll = getContentHeight() - (bottom - top - 4);
	if (maxScroll < 0)
		maxScroll /= 2;

	if (amountScrolled < 0.0f)
		amountScrolled = 0.0f;
	if (amountScrolled > static_cast<float>(maxScroll))
		amountScrolled = static_cast<float>(maxScroll);
}

void GuiSlot::actionPerformed(Button &button)
{
	if (!button.active)
		return;

	if (button.id == scrollUpButtonID)
	{
		amountScrolled -= static_cast<float>(posZ * 2 / 3);
		initialClickY = -2.0f;
		bindAmountScrolled();
	}
	else if (button.id == scrollDownButtonID)
	{
		amountScrolled += static_cast<float>(posZ * 2 / 3);
		initialClickY = -2.0f;
		bindAmountScrolled();
	}
}

void GuiSlot::mouseScrolled(int_t scrollAmount)
{
	if (scrollAmount == 0)
		return;

	int_t step = posZ / 2;
	if (step < 1)
		step = 1;

	amountScrolled -= static_cast<float>(scrollAmount > 0 ? step : -step);
	bindAmountScrolled();
}

int_t GuiSlot::getContentHeight()
{
	return getSize() * posZ + headerPadding;
}

void GuiSlot::drawHeader(int_t x, int_t y, Tesselator &t)
{

}

void GuiSlot::headerClicked(int_t x, int_t y)
{

}

void GuiSlot::drawOverlay(int_t mouseX, int_t mouseY)
{

}

void GuiSlot::drawScreen(int_t mouseX, int_t mouseY, float partialTick)
{
	drawBackground();

	int_t size = getSize();
	int_t scrollBarLeft = width / 2 + 124;
	int_t scrollBarRight = scrollBarLeft + 6;

	if (lwjgl::Mouse::isButtonDown(0))
	{
		if (initialClickY == -1.0f)
		{
			bool clickedInside = true;
			if (mouseY >= top && mouseY <= bottom)
			{
				int_t rowLeft = width / 2 - 110;
				int_t rowRight = width / 2 + 110;
				int_t relativeY = mouseY - top - headerPadding + static_cast<int_t>(amountScrolled) - 4;
				int_t index = relativeY / posZ;
				if (mouseX >= rowLeft && mouseX <= rowRight && index >= 0 && relativeY >= 0 && index < size)
				{
					bool doubleClick = index == selectedElement && System::currentTimeMillis() - lastClicked < 250L;
					elementClicked(index, doubleClick);
					selectedElement = index;
					lastClicked = System::currentTimeMillis();
				}
				else if (mouseX >= rowLeft && mouseX <= rowRight && relativeY < 0)
				{
					headerClicked(mouseX - rowLeft, mouseY - top + static_cast<int_t>(amountScrolled) - 4);
					clickedInside = false;
				}

				if (mouseX >= scrollBarLeft && mouseX <= scrollBarRight)
				{
					scrollMultiplier = -1.0f;
					int_t maxScroll = getContentHeight() - (bottom - top - 4);
					if (maxScroll < 1)
						maxScroll = 1;

					int_t contentHeight = getContentHeight();
					if (contentHeight < 1)
						contentHeight = 1;

					int_t thumbHeight = (bottom - top) * (bottom - top) / contentHeight;
					if (thumbHeight < 32)
						thumbHeight = 32;
					if (thumbHeight > bottom - top - 8)
						thumbHeight = bottom - top - 8;

					scrollMultiplier /= static_cast<float>(bottom - top - thumbHeight) / static_cast<float>(maxScroll);
				}
				else
				{
					scrollMultiplier = 1.0f;
				}

				if (clickedInside)
					initialClickY = static_cast<float>(mouseY);
				else
					initialClickY = -2.0f;
			}
			else
			{
				initialClickY = -2.0f;
			}
		}
		else if (initialClickY >= 0.0f)
		{
			amountScrolled -= (static_cast<float>(mouseY) - initialClickY) * scrollMultiplier;
			initialClickY = static_cast<float>(mouseY);
		}
	}
	else
	{
		initialClickY = -1.0f;
	}

	bindAmountScrolled();
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);

	Tesselator &t = Tesselator::instance;
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/gui/background.png"));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	float backgroundScale = 32.0f;
	t.begin();
	t.color(0x202020);
	t.vertexUV(left, bottom, 0.0, left / backgroundScale, (bottom + static_cast<int_t>(amountScrolled)) / backgroundScale);
	t.vertexUV(right, bottom, 0.0, right / backgroundScale, (bottom + static_cast<int_t>(amountScrolled)) / backgroundScale);
	t.vertexUV(right, top, 0.0, right / backgroundScale, (top + static_cast<int_t>(amountScrolled)) / backgroundScale);
	t.vertexUV(left, top, 0.0, left / backgroundScale, (top + static_cast<int_t>(amountScrolled)) / backgroundScale);
	t.end();

	int_t rowX = width / 2 - 92 - 16;
	int_t rowY = top + 4 - static_cast<int_t>(amountScrolled);
	if (hasHeader)
		drawHeader(rowX, rowY, t);

	for (int_t index = 0; index < size; index++)
	{
		int_t slotY = rowY + index * posZ + headerPadding;
		int_t slotHeight = posZ - 4;
		if (slotY <= bottom && slotY + slotHeight >= top)
		{
			if (renderSelection && isSelected(index))
			{
				int_t selectedLeft = width / 2 - 110;
				int_t selectedRight = width / 2 + 110;
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glDisable(GL_TEXTURE_2D);
				t.begin();
				t.color(0x808080);
				t.vertex(selectedLeft, slotY + slotHeight + 2, 0.0);
				t.vertex(selectedRight, slotY + slotHeight + 2, 0.0);
				t.vertex(selectedRight, slotY - 2, 0.0);
				t.vertex(selectedLeft, slotY - 2, 0.0);
				t.color(0x000000);
				t.vertex(selectedLeft + 1, slotY + slotHeight + 1, 0.0);
				t.vertex(selectedRight - 1, slotY + slotHeight + 1, 0.0);
				t.vertex(selectedRight - 1, slotY - 1, 0.0);
				t.vertex(selectedLeft + 1, slotY - 1, 0.0);
				t.end();
				glEnable(GL_TEXTURE_2D);
			}

			drawSlot(index, rowX, slotY, slotHeight, t);
		}
	}

	glDisable(GL_DEPTH_TEST);
	overlayBackground(0, top, 255, 255);
	overlayBackground(bottom, height, 255, 255);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_TEXTURE_2D);

	t.begin();
	t.color(0x000000, 0);
	t.vertexUV(left, top + 4, 0.0, 0.0, 1.0);
	t.vertexUV(right, top + 4, 0.0, 1.0, 1.0);
	t.color(0x000000, 255);
	t.vertexUV(right, top, 0.0, 1.0, 0.0);
	t.vertexUV(left, top, 0.0, 0.0, 0.0);
	t.end();

	t.begin();
	t.color(0x000000, 255);
	t.vertexUV(left, bottom, 0.0, 0.0, 1.0);
	t.vertexUV(right, bottom, 0.0, 1.0, 1.0);
	t.color(0x000000, 0);
	t.vertexUV(right, bottom - 4, 0.0, 1.0, 0.0);
	t.vertexUV(left, bottom - 4, 0.0, 0.0, 0.0);
	t.end();

	int_t maxScroll = getContentHeight() - (bottom - top - 4);
	if (maxScroll > 0)
	{
		int_t thumbHeight = (bottom - top) * (bottom - top) / getContentHeight();
		if (thumbHeight < 32)
			thumbHeight = 32;
		if (thumbHeight > bottom - top - 8)
			thumbHeight = bottom - top - 8;

		int_t thumbY = static_cast<int_t>(amountScrolled) * (bottom - top - thumbHeight) / maxScroll + top;
		if (thumbY < top)
			thumbY = top;

		t.begin();
		t.color(0x000000, 255);
		t.vertex(scrollBarLeft, bottom, 0.0);
		t.vertex(scrollBarRight, bottom, 0.0);
		t.vertex(scrollBarRight, top, 0.0);
		t.vertex(scrollBarLeft, top, 0.0);
		t.end();

		t.begin();
		t.color(0x808080, 255);
		t.vertex(scrollBarLeft, thumbY + thumbHeight, 0.0);
		t.vertex(scrollBarRight, thumbY + thumbHeight, 0.0);
		t.vertex(scrollBarRight, thumbY, 0.0);
		t.vertex(scrollBarLeft, thumbY, 0.0);
		t.end();

		t.begin();
		t.color(0xC0C0C0, 255);
		t.vertex(scrollBarLeft, thumbY + thumbHeight - 1, 0.0);
		t.vertex(scrollBarRight - 1, thumbY + thumbHeight - 1, 0.0);
		t.vertex(scrollBarRight - 1, thumbY, 0.0);
		t.vertex(scrollBarLeft, thumbY, 0.0);
		t.end();
	}

	drawOverlay(mouseX, mouseY);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
	glEnable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
}

void GuiSlot::overlayBackground(int_t y0, int_t y1, int_t alphaTop, int_t alphaBottom)
{
	Tesselator &t = Tesselator::instance;
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/gui/background.png"));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	float backgroundScale = 32.0f;
	t.begin();
	t.color(0x404040, alphaBottom);
	t.vertexUV(0.0, y1, 0.0, 0.0, y1 / backgroundScale);
	t.vertexUV(width, y1, 0.0, width / backgroundScale, y1 / backgroundScale);
	t.color(0x404040, alphaTop);
	t.vertexUV(width, y0, 0.0, width / backgroundScale, y0 / backgroundScale);
	t.vertexUV(0.0, y0, 0.0, 0.0, y0 / backgroundScale);
	t.end();
}

#include "client/renderer/OffsettedRenderList.h"

#include "client/renderer/Chunk.h"

#include "OpenGL.h"

void OffsettedRenderList::init(int_t x, int_t y, int_t z, int_t layer, double xOff, double yOff, double zOff)
{
	inited = true;
	chunks.clear();
	this->x = x;
	this->y = y;
	this->z = z;
	this->layer = layer;

	this->xOff = xOff;
	this->yOff = yOff;
	this->zOff = zOff;
}

bool OffsettedRenderList::isAt(int_t x, int_t y, int_t z)
{
	if (!inited) return false;
	return this->x == x && this->y == y && this->z == z;
}

void OffsettedRenderList::add(Chunk *chunk)
{
	chunks.push_back(chunk);
	if (chunks.size() == 0x10000) render();
}

void OffsettedRenderList::render()
{
	if (!inited) return;
	if (!rendered)
		rendered = true;

	if (!chunks.empty())
	{
		glPushMatrix();
		glTranslatef(static_cast<float>(x - xOff), static_cast<float>(y - yOff), static_cast<float>(z - zOff));
		for (Chunk *chunk : chunks)
			chunk->draw(layer);
		glPopMatrix();
	}
}

void OffsettedRenderList::clear()
{
	inited = false;
	rendered = false;
	chunks.clear();
}

#include "client/renderer/texturefx/TextureFX.h"

#include "client/renderer/texturefx/TileSize.h"

TextureFX::TextureFX(int_t iconIndex) : imageData(TileSize::numBytes)
{
	this->iconIndex = iconIndex;
}

void TextureFX::onTick()
{
}

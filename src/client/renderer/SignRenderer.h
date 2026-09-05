#pragma once

#include <memory>

#include "java/Type.h"

class SignTileEntity;
class Font;
class Textures;

class SignRenderer
{
public:
	static void renderSign(SignTileEntity &sign, double x, double y, double z, float scale, Font &font, Textures &textures);
};
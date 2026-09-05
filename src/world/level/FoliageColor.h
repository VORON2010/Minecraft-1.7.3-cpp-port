#pragma once

#include "java/Type.h"

class BufferedImage;

namespace FoliageColor
{

int_t get(double x, double y);
int_t getEvergreenColor();
int_t getBirchColor();

int_t getDefaultColor();
void setImage(BufferedImage image);

}

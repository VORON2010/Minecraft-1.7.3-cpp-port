#pragma once

#include "java/File.h"
#include "java/String.h"

namespace ScreenShotHelper
{

jstring saveScreenshot(const File &workingDir, int_t width, int_t height);

}

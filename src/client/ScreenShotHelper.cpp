#include "client/ScreenShotHelper.h"

#include <ctime>
#include <string>
#include <vector>

#include "OpenGL.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace ScreenShotHelper
{

jstring saveScreenshot(const File &workingDir, int_t width, int_t height)
{
	try
	{
		std::unique_ptr<File> dir(File::open(workingDir, u"screenshots"));
		dir->mkdir();

		std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 3);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

		std::time_t now = std::time(nullptr);
		std::tm *tmv = std::localtime(&now);
		char stamp[32] = {};
		if (tmv != nullptr)
			std::strftime(stamp, sizeof(stamp), "%Y-%m-%d_%H.%M.%S", tmv);
		jstring base = String::fromUTF8(stamp);

		for (int_t attempt = 1; ; ++attempt)
		{
			jstring fileName = base + (attempt == 1 ? u"" : u"_" + String::fromUTF8(std::to_string(attempt))) + u".png";
			std::unique_ptr<File> file(File::open(*dir, fileName));
			if (file->exists())
				continue;

			
			std::vector<unsigned char> flipped(pixels.size());
			size_t stride = static_cast<size_t>(width) * 3;
			for (int_t y = 0; y < height; ++y)
				std::copy(pixels.begin() + static_cast<size_t>(height - 1 - y) * stride,
					pixels.begin() + static_cast<size_t>(height - y) * stride,
					flipped.begin() + static_cast<size_t>(y) * stride);

			if (!stbi_write_png(String::toUTF8(file->toString()).c_str(), width, height, 3, flipped.data(), static_cast<int>(stride)))
				return u"Failed to save: could not write " + fileName;

			return u"Saved screenshot as " + fileName;
		}
	}
	catch (const std::exception &e)
	{
		return u"Failed to save: " + String::fromUTF8(e.what());
	}
	catch (...)
	{
		return u"Failed to save: unknown error";
	}
}

}

#include "client/renderer/Textures.h"
#include "client/renderer/texturefx/TextureFX.h"

#include <cassert>
#include <algorithm>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <thread>

#include "client/Minecraft.h"
#include "client/Options.h"
#include "client/renderer/texturefx/CustomAnimation.h"
#include "client/renderer/texturefx/TextureCompassFX.h"
#include "client/renderer/texturefx/TextureFlamesFX.h"
#include "client/renderer/texturefx/TextureLavaFlowFX.h"
#include "client/renderer/texturefx/TextureLavaFX.h"
#include "client/renderer/texturefx/TexturePortalFX.h"
#include "client/renderer/texturefx/TextureWatchFX.h"
#include "client/renderer/texturefx/TextureWaterFlowFX.h"
#include "client/renderer/texturefx/TextureWaterFX.h"
#include "client/renderer/texturefx/TileSize.h"
#include "client/skins/FileTexturePack.h"
#include "client/skins/TexturePackRepository.h"
#include "java/File.h"
#include "world/level/FoliageColor.h"
#include "world/level/GrassColor.h"

#include "OpenGL.h"
#include "util/HttpGet.h"
#include "stb_image.h"

struct HttpTexture
{
	std::mutex mutex;
	BufferedImage loadedImage;
	int_t count = 1;
	int_t id = -1;
	bool hasLoadedImage = false;
	bool isUploaded = false;
	
	bool downloadRequested = false;
};

namespace
{
const std::pair<jstring, int_t> EXPECTED_COLUMNS[] =
{
	{ u"/terrain.png", 16 },
	{ u"/gui/items.png", 16 },
	{ u"/misc/dial.png", 1 },
	{ u"/custom_lava_still.png", 1 },
	{ u"/custom_lava_flowing.png", 1 },
	{ u"/custom_water_still.png", 1 },
	{ u"/custom_water_flowing.png", 1 },
	{ u"/custom_fire_n_s.png", 1 },
	{ u"/custom_fire_e_w.png", 1 },
	{ u"/custom_portal.png", 1 }
};

struct ParsedHttpUrl
{
	std::string host;
	std::string path;
	int port = 80;
	bool valid = false;
};

constexpr const char *BETACRAFT_PROXY_HOST = "betacraft.uk";
constexpr int BETACRAFT_PROXY_PORT = 11705;

bool usesBetacraftTextureProxy(const ParsedHttpUrl &parsed)
{
	if (parsed.host != "s3.amazonaws.com")
	{
		return false;
	}

	return parsed.path.rfind("/MinecraftSkins/", 0) == 0 || parsed.path.rfind("/MinecraftCloaks/", 0) == 0;
}

std::string makeHttpTextureCacheName(const ParsedHttpUrl &parsed)
{
	std::string name = parsed.host + parsed.path;
	for (char &c : name)
	{
		bool isAlphaNum = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
		if (!isAlphaNum && c != '.' && c != '-' && c != '_')
		{
			c = '_';
		}
	}
	return name;
}

File *openHttpTextureCacheFile(const ParsedHttpUrl &parsed)
{
	std::unique_ptr<File> workDir(File::openWorkingDirectory(u".mcbetacpp"));
	if (workDir == nullptr)
	{
		return nullptr;
	}

	std::unique_ptr<File> cacheDir(File::open(*workDir, u"http-textures"));
	if (cacheDir == nullptr)
	{
		return nullptr;
	}

	if (!cacheDir->exists() && !cacheDir->mkdirs())
	{
		return nullptr;
	}

	return File::open(*cacheDir, String::fromUTF8(makeHttpTextureCacheName(parsed)));
}

BufferedImage loadHttpTextureFromCache(const ParsedHttpUrl &parsed)
{
	std::unique_ptr<File> cacheFile(openHttpTextureCacheFile(parsed));
	if (cacheFile == nullptr || !cacheFile->exists() || !cacheFile->isFile())
	{
		return {};
	}

	std::unique_ptr<std::istream> is(cacheFile->toStreamIn());
	if (!is)
	{
		return {};
	}

	try
	{
		return BufferedImage::ImageIO_read(*is);
	}
	catch (...)
	{
		return {};
	}
}

bool storeHttpTextureCache(const ParsedHttpUrl &parsed, const std::vector<byte_t> &body)
{
	std::unique_ptr<File> cacheFile(openHttpTextureCacheFile(parsed));
	if (cacheFile == nullptr)
	{
		return false;
	}

	std::unique_ptr<std::ostream> os(cacheFile->toStreamOut());
	if (!os)
	{
		return false;
	}

	os->write(reinterpret_cast<const char *>(body.data()), static_cast<std::streamsize>(body.size()));
	return os->good();
}

ParsedHttpUrl parseHttpUrl(const jstring &url)
{
	const std::string urlUtf8 = String::toUTF8(url);
	const std::string prefix = "http://";
	if (urlUtf8.compare(0, prefix.size(), prefix) != 0)
	{
		return {};
	}

	ParsedHttpUrl parsed;
	std::string hostAndPath = urlUtf8.substr(prefix.size());
	size_t pathPos = hostAndPath.find('/');
	if (pathPos == std::string::npos)
	{
		return {};
	}

	parsed.host = hostAndPath.substr(0, pathPos);
	parsed.path = hostAndPath.substr(pathPos);

	size_t portPos = parsed.host.find(':');
	if (portPos != std::string::npos)
	{
		try
		{
			parsed.port = std::stoi(parsed.host.substr(portPos + 1));
		}
		catch (...)
		{
			return {};
		}

		parsed.host = parsed.host.substr(0, portPos);
	}

	if (parsed.host.empty() || parsed.path.empty())
	{
		return {};
	}

	parsed.valid = true;
	return parsed;
}

BufferedImage readImageFromMemory(const std::vector<byte_t> &data)
{
	int w = 0;
	int h = 0;
	int comp = 0;
	stbi_uc *rawData = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(data.data()), static_cast<int>(data.size()), &w, &h, &comp, 0);
	if (rawData == nullptr)
	{
		return {};
	}

	std::unique_ptr<unsigned char[]> pixels = std::make_unique<unsigned char[]>(w * h * 4);

	if (comp == 1)
	{
		for (int i = 0; i < w * h; i++)
		{
			pixels[i * 4 + 0] = rawData[i];
			pixels[i * 4 + 1] = rawData[i];
			pixels[i * 4 + 2] = rawData[i];
			pixels[i * 4 + 3] = 255;
		}
	}
	else if (comp == 2)
	{
		for (int i = 0; i < w * h; i++)
		{
			pixels[i * 4 + 0] = rawData[i * 2 + 0];
			pixels[i * 4 + 1] = rawData[i * 2 + 0];
			pixels[i * 4 + 2] = rawData[i * 2 + 0];
			pixels[i * 4 + 3] = rawData[i * 2 + 1];
		}
	}
	else if (comp == 3)
	{
		for (int i = 0; i < w * h; i++)
		{
			pixels[i * 4 + 0] = rawData[i * 3 + 0];
			pixels[i * 4 + 1] = rawData[i * 3 + 1];
			pixels[i * 4 + 2] = rawData[i * 3 + 2];
			pixels[i * 4 + 3] = 255;
		}
	}
	else if (comp == 4)
	{
		for (int i = 0; i < w * h; i++)
		{
			pixels[i * 4 + 0] = rawData[i * 4 + 0];
			pixels[i * 4 + 1] = rawData[i * 4 + 1];
			pixels[i * 4 + 2] = rawData[i * 4 + 2];
			pixels[i * 4 + 3] = rawData[i * 4 + 3];
		}
	}
	else
	{
		stbi_image_free(rawData);
		return {};
	}

	stbi_image_free(rawData);
	return BufferedImage(w, h, std::move(pixels));
}

bool hasAlpha(const unsigned char *pixels, int_t imgWidth, int_t x0, int_t y0, int_t x1, int_t y1)
{
	for (int_t x = x0; x < x1; x++)
	{
		for (int_t y = y0; y < y1; y++)
		{
			if (pixels[(y * imgWidth + x) * 4 + 3] < 128)
			{
				return true;
			}
		}
	}

	return false;
}

void setForceAlpha(unsigned char *pixels, int_t imgWidth, int_t x0, int_t y0, int_t x1, int_t y1)
{
	if (hasAlpha(pixels, imgWidth, x0, y0, x1, y1))
	{
		return;
	}

	for (int_t x = x0; x < x1; x++)
	{
		for (int_t y = y0; y < y1; y++)
		{
			pixels[(y * imgWidth + x) * 4 + 3] = 0;
		}
	}
}

void setNoAlpha(unsigned char *pixels, int_t imgWidth, int_t x0, int_t y0, int_t x1, int_t y1)
{
	for (int_t x = x0; x < x1; x++)
	{
		for (int_t y = y0; y < y1; y++)
		{
			pixels[(y * imgWidth + x) * 4 + 3] = 255;
		}
	}
}

BufferedImage processMobSkin(BufferedImage &in)
{
	if (in.getWidth() == 0 || in.getHeight() == 0)
	{
		return {};
	}

	BufferedImage out(64, 32);
	unsigned char *dstPixels = const_cast<unsigned char *>(out.getRawPixels());
	const unsigned char *srcPixels = in.getRawPixels();
	int_t srcW = in.getWidth();
	int_t srcH = in.getHeight();

	for (int_t y = 0; y < srcH && y < 32; y++)
	{
		for (int_t x = 0; x < srcW && x < 64; x++)
		{
			int_t srcIdx = (y * srcW + x) * 4;
			int_t dstIdx = (y * 64 + x) * 4;
			dstPixels[dstIdx + 0] = srcPixels[srcIdx + 0];
			dstPixels[dstIdx + 1] = srcPixels[srcIdx + 1];
			dstPixels[dstIdx + 2] = srcPixels[srcIdx + 2];
			dstPixels[dstIdx + 3] = srcPixels[srcIdx + 3];
		}
	}

	setNoAlpha(dstPixels, 64, 0, 0, 32, 16);
	setForceAlpha(dstPixels, 64, 32, 0, 64, 32);
	setNoAlpha(dstPixels, 64, 0, 16, 64, 32);

	return out;
}

void downloadHttpTexture(const std::shared_ptr<HttpTexture> &texture, const jstring &url)
{
	const std::string urlUtf8 = String::toUTF8(url);
	try
	{
		ParsedHttpUrl parsed = parseHttpUrl(url);
		if (!parsed.valid)
		{
			std::cerr << "[HTTP Texture] Invalid URL: " << urlUtf8 << std::endl;
			return;
		}

		BufferedImage image = loadHttpTextureFromCache(parsed);
		if (image.getWidth() > 0 && image.getHeight() > 0)
		{
			std::cerr << "[HTTP Texture] Cache hit " << urlUtf8 << " size=" << image.getWidth() << "x" << image.getHeight() << std::endl;
		}
		else
		{
			const bool useBetacraftProxy = usesBetacraftTextureProxy(parsed);
			std::cerr << "[HTTP Texture] Downloading " << urlUtf8
				<< " host=" << parsed.host
				<< " port=" << parsed.port
				<< " path=" << parsed.path;
			if (useBetacraftProxy)
			{
				std::cerr << " proxyHost=" << BETACRAFT_PROXY_HOST << " proxyPort=" << BETACRAFT_PROXY_PORT;
			}
			std::cerr << std::endl;

			HttpGet::Response response = useBetacraftProxy
				? HttpGet::fetch(urlUtf8, 30, BETACRAFT_PROXY_HOST, BETACRAFT_PROXY_PORT)
				: HttpGet::fetch(urlUtf8);
			if (!response.error.empty())
			{
				std::cerr << "[HTTP Texture] Request failed: " << urlUtf8 << " (" << response.error << ")" << std::endl;
				return;
			}

			std::cerr << "[HTTP Texture] Response " << urlUtf8 << " status=" << response.status
				<< " bytes=" << response.body.size() << std::endl;
			if (!response.ok())
			{
				return;
			}

			image = readImageFromMemory(response.body);
			if (image.getWidth() == 0 || image.getHeight() == 0)
			{
				std::cerr << "[HTTP Texture] Decode failed: " << urlUtf8 << std::endl;
				return;
			}

			if (storeHttpTextureCache(parsed, response.body))
			{
				std::cerr << "[HTTP Texture] Cached " << urlUtf8 << std::endl;
			}
		}

		image = processMobSkin(image);

		std::lock_guard<std::mutex> lock(texture->mutex);
		if (image.getWidth() == 0 || image.getHeight() == 0)
		{
			std::cerr << "[HTTP Texture] Decode failed: " << urlUtf8 << std::endl;
			return;
		}

		std::cerr << "[HTTP Texture] Decoded " << urlUtf8 << " size=" << image.getWidth() << "x" << image.getHeight() << std::endl;
		texture->loadedImage = std::move(image);
		texture->hasLoadedImage = true;
	}
	catch (const std::exception &e)
	{
		std::cerr << "[HTTP Texture] Exception for " << urlUtf8 << ": " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "[HTTP Texture] Unknown exception for " << urlUtf8 << std::endl;
	}
}
}

Textures::Textures(TexturePackRepository &skins, Options &options, Minecraft &minecraft)
	: skins(skins), options(options), minecraft(minecraft)
{

}

Textures::~Textures()
{
	{
		std::lock_guard<std::mutex> lock(httpMutex);
		stopHttp = true;
		httpJobs.clear();
	}
	httpReady.notify_one();
	if (httpWorker.joinable())
		httpWorker.join();
}

void Textures::queueHttpTexture(const jstring &url, const std::shared_ptr<HttpTexture> &texture)
{
	if (texture->downloadRequested)
		return;
	{
		std::lock_guard<std::mutex> lock(httpMutex);
		
		if (stopHttp || httpJobs.size() >= 16)
			return;
		if (!httpWorker.joinable())
			httpWorker = std::thread(&Textures::downloadHttpTextures, this);
		httpJobs.emplace_back(url, texture);
		texture->downloadRequested = true;
	}
	httpReady.notify_one();
}

void Textures::downloadHttpTextures()
{
	for (;;)
	{
		std::pair<jstring, std::weak_ptr<HttpTexture>> job;
		{
			std::unique_lock<std::mutex> lock(httpMutex);
			httpReady.wait(lock, [this] { return stopHttp || !httpJobs.empty(); });
			if (stopHttp)
				return;
			job = std::move(httpJobs.front());
			httpJobs.pop_front();
		}
		std::shared_ptr<HttpTexture> texture = job.second.lock();
		if (texture != nullptr)
			downloadHttpTexture(texture, job.first);
	}
}

BufferedImage Textures::readResourceImage(const jstring &resourceName, bool resize)
{
	TexturePack *skin = skins.selected;
	if (skin == nullptr)
		throw std::runtime_error("No selected texture pack");

	std::unique_ptr<std::istream> input(skin->getResource(resourceName));
	if (input == nullptr)
		throw std::runtime_error("Missing texture resource " + String::toUTF8(resourceName));
	BufferedImage image = readImage(*input);

	if (resize)
	{
		for (const auto &entry : EXPECTED_COLUMNS)
		{
			if (entry.first == resourceName && image.getWidth() != entry.second * TileSize::size)
				return resizeImage(image, entry.second * TileSize::size);
		}
	}

	return image;
}

BufferedImage Textures::getResourceImage(const jstring &resourceName)
{
	return readResourceImage(resourceName, true);
}

bool Textures::hasResource(const jstring &resourceName)
{
	try
	{
		TexturePack *skin = skins.selected;
		std::unique_ptr<std::istream> input(skin == nullptr ? nullptr : skin->getResource(resourceName));
		return input != nullptr;
	}
	catch (...)
	{
		return false;
	}
}

int_t Textures::calculateTileSize()
{
	int_t size = 0;
	for (const auto &entry : EXPECTED_COLUMNS)
	{
		try
		{
			BufferedImage image = readResourceImage(entry.first, false);
			size = std::max(size, image.getWidth() / entry.second);
		}
		catch (...)
		{
		}
	}
	return size > 0 ? size : 16;
}

void Textures::setTileSize()
{
	TileSize::set(calculateTileSize());
	reloadAll();
	refreshTextureFX();
	refreshColorizers();
}

int_t Textures::loadTexture(const jstring &resourceName)
{
	auto it = idMap.find(resourceName);
	if (it != idMap.end())
	{
		return it->second;
	}

	MemoryTracker::genTextures(ib);
	int_t i = ib[0];

	if (!resourceName.compare(0, 2, u"##"))
	{
		BufferedImage img = getResourceImage(resourceName.substr(2));
		img = makeStrip(img);
		loadTexture(img, i);
	}
	else if (!resourceName.compare(0, 7, u"%clamp%"))
	{
		BufferedImage img = getResourceImage(resourceName.substr(7));
		clamp = true;
		loadTexture(img, i);
		clamp = false;
	}
	else if (!resourceName.compare(0, 6, u"%blur%"))
	{
		BufferedImage img = getResourceImage(resourceName.substr(6));
		blur = true;
		loadTexture(img, i);
		blur = false;
	}
	else
	{
		BufferedImage img = getResourceImage(resourceName);
		loadTexture(img, i);
	}

	idMap.emplace(resourceName, i);
	return i;
}

BufferedImage Textures::makeStrip(BufferedImage &source)
{
	int_t cols = source.getWidth() / TileSize::size;

	BufferedImage out(TileSize::size, source.getHeight() * cols);

	std::vector<unsigned char> column(TileSize::size * source.getHeight() * 4);
	for (int_t i = 0; i < cols; i++)
	{
		
		source.getRGB(i * TileSize::size, 0, TileSize::size, source.getHeight(), column.data());
		out.setRGB(0, i * source.getHeight(), TileSize::size, source.getHeight(), column.data());
	}

	return out;
}

int_t Textures::getTexture(BufferedImage &img)
{
	MemoryTracker::genTextures(Textures::ib);
	loadTexture(img, ib[0]);
	loadedImages.emplace(ib[0], std::move(img));
	return ib[0];
}

void Textures::loadTexture(BufferedImage &img, int_t id)
{
	glBindTexture(GL_TEXTURE_2D, id);

	if (MIPMAP)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (blur)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	if (clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	int_t w = img.getWidth();
	int_t h = img.getHeight();

	const unsigned char *rawPixels = img.getRawPixels();
	std::unique_ptr<unsigned char[]> newPixels(new unsigned char[w * h * 4]);

	const unsigned char *rawPixels_p = rawPixels;
	unsigned char *newPixels_p = newPixels.get();

	for (int_t i = 0; i < w * h; i++, rawPixels_p += 4, newPixels_p += 4)
	{
		int_t a = rawPixels_p[3];
		int_t r = rawPixels_p[0];
		int_t g = rawPixels_p[1];
		int_t b = rawPixels_p[2];

		if (options.anaglyph3d)
		{
			int_t rr = (r * 30 + g * 59 + b * 11) / 100;
			int_t gg = (r * 30 + g * 70) / 100;
			int_t bb = (r * 30 + b * 70) / 100;

			r = rr;
			g = gg;
			b = bb;
		}

		newPixels_p[0] = r;
		newPixels_p[1] = g;
		newPixels_p[2] = b;
		newPixels_p[3] = a;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, newPixels.get());

	if (MIPMAP)
	{
		std::unique_ptr<int_t[]> mipmapPixels(new int_t[w * h * 4]);
		int_t *inPixels = (int_t*)newPixels.get();

		for (int level = 1; level <= 4; level++)
		{
			int_t ow = w >> (level - 1);

			int_t ww = w >> level;
			int_t hh = h >> level;

			for (int_t x = 0; x < ww; x++)
			{
				for (int_t y = 0; y < hh; y++)
				{
					int_t c0 = inPixels[x * 2 + 0 + (y * 2 + 0) * ow];
					int_t c1 = inPixels[x * 2 + 1 + (y * 2 + 0) * ow];
					int_t c2 = inPixels[x * 2 + 1 + (y * 2 + 1) * ow];
					int_t c3 = inPixels[x * 2 + 0 + (y * 2 + 1) * ow];
					int_t col = crispBlend(crispBlend(c0, c1), crispBlend(c3, c2));
					mipmapPixels[x + y * ww] = col;
				}
			}

			glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, ww, hh, 0, GL_RGBA, GL_UNSIGNED_BYTE, mipmapPixels.get());
		}
	}
}

void Textures::releaseTexture(int_t id)
{
	loadedImages.erase(id);

	ib[0] = id;
	glDeleteTextures(1, reinterpret_cast<GLuint*>(ib.data()));
}

int_t Textures::loadHttpTexture(const jstring &url, const jstring *backup)
{
	auto it = httpTextures.find(url);
	if (it != httpTextures.end())
	{
		queueHttpTexture(url, it->second);
		HttpTexture *texture = it->second.get();
		std::lock_guard<std::mutex> lock(texture->mutex);
		if (texture->hasLoadedImage && !texture->isUploaded)
		{
			if (texture->id < 0)
			{
				texture->id = getTexture(texture->loadedImage);
			}
			else
			{
				loadTexture(texture->loadedImage, texture->id);
			}
			texture->isUploaded = true;
			std::cerr << "[HTTP Texture] Uploaded " << String::toUTF8(url) << " to GL id=" << texture->id << std::endl;
		}

		if (texture->id >= 0)
		{
			return texture->id;
		}
	}

	if (backup != nullptr)
	{
		return loadTexture(*backup);
	}

	return -1;
}

int_t Textures::loadHttpTexture(const jstring &url)
{
	return loadHttpTexture(url, nullptr);
}

BufferedImage Textures::resizeImage(BufferedImage &source, int_t width)
{
	int_t sourceWidth = source.getWidth();
	int_t sourceHeight = source.getHeight();
	int_t height = sourceHeight * width / sourceWidth;
	std::unique_ptr<unsigned char[]> pixels = std::make_unique<unsigned char[]>(width * height * 4);
	const unsigned char *sourcePixels = source.getRawPixels();
	for (int_t y = 0; y < height; ++y)
	{
		int_t sourceY = y * sourceHeight / height;
		for (int_t x = 0; x < width; ++x)
		{
			int_t sourceX = x * sourceWidth / width;
			std::memcpy(pixels.get() + (x + y * width) * 4,
				sourcePixels + (sourceX + sourceY * sourceWidth) * 4, 4);
		}
	}
	return BufferedImage(width, height, std::move(pixels));
}

void Textures::obtainHttpTexture(const jstring &url)
{
	addHttpTexture(url);
}

HttpTexture *Textures::addHttpTexture(const jstring &url)
{
	
	if (minecraft.unattended)
		return nullptr;
	auto it = httpTextures.find(url);
	if (it != httpTextures.end())
	{
		it->second->count++;
		queueHttpTexture(url, it->second);
		std::cerr << "[HTTP Texture] Reusing cached URL " << String::toUTF8(url) << " refCount=" << it->second->count << std::endl;
		return it->second.get();
	}

	std::cerr << "[HTTP Texture] Queueing download for " << String::toUTF8(url) << std::endl;
	auto texture = Util::make_shared<HttpTexture>();
	HttpTexture *ptr = texture.get();
	httpTextures.emplace(url, texture);
	queueHttpTexture(url, texture);
	return ptr;
}

void Textures::removeHttpTexture(const jstring &url)
{
	auto it = httpTextures.find(url);
	if (it == httpTextures.end())
	{
		return;
	}

	HttpTexture *texture = it->second.get();
	texture->count--;
	std::cerr << "[HTTP Texture] Release URL " << String::toUTF8(url) << " refCount=" << texture->count << std::endl;
	if (texture->count == 0)
	{
		if (texture->id >= 0)
		{
			releaseTexture(texture->id);
		}
		httpTextures.erase(it);
	}
}

void Textures::registerTextureFX(std::unique_ptr<TextureFX> fx)
{
	fx->onTick();
	textureList.push_back(std::move(fx));
}

void Textures::refreshTextureFX()
{
	std::vector<std::unique_ptr<TextureFX>> saved;
	for (std::unique_ptr<TextureFX> &fx : textureList)
	{
		if (dynamic_cast<TextureCompassFX *>(fx.get()) == nullptr &&
			dynamic_cast<TextureWatchFX *>(fx.get()) == nullptr &&
			dynamic_cast<TextureLavaFX *>(fx.get()) == nullptr &&
			dynamic_cast<TextureLavaFlowFX *>(fx.get()) == nullptr &&
			dynamic_cast<TextureWaterFX *>(fx.get()) == nullptr &&
			dynamic_cast<TextureWaterFlowFX *>(fx.get()) == nullptr &&
			dynamic_cast<TextureFlamesFX *>(fx.get()) == nullptr &&
			dynamic_cast<TexturePortalFX *>(fx.get()) == nullptr &&
			dynamic_cast<CustomAnimation *>(fx.get()) == nullptr)
		{
			if (fx->imageData.size() != static_cast<size_t>(TileSize::numBytes))
				fx->imageData.assign(TileSize::numBytes, 0);
			saved.push_back(std::move(fx));
		}
	}
	textureList.clear();

	textureList.push_back(std::make_unique<TextureCompassFX>(minecraft));
	textureList.push_back(std::make_unique<TextureWatchFX>(minecraft));

	bool customPack = dynamic_cast<FileTexturePack *>(skins.selected) != nullptr;
	if (customPack && hasResource(u"/custom_lava_still.png"))
		textureList.push_back(std::make_unique<CustomAnimation>(*this, 14 * 16 + 13, 0, 1, u"lava_still", -1, -1));
	else
		textureList.push_back(std::make_unique<TextureLavaFX>());
	if (customPack && hasResource(u"/custom_lava_flowing.png"))
		textureList.push_back(std::make_unique<CustomAnimation>(*this, 14 * 16 + 14, 0, 2, u"lava_flowing", 3, 6));
	else
		textureList.push_back(std::make_unique<TextureLavaFlowFX>());

	if (customPack && hasResource(u"/custom_water_still.png"))
		textureList.push_back(std::make_unique<CustomAnimation>(*this, 12 * 16 + 13, 0, 1, u"water_still", -1, -1));
	else
		textureList.push_back(std::make_unique<TextureWaterFX>());
	if (customPack && hasResource(u"/custom_water_flowing.png"))
		textureList.push_back(std::make_unique<CustomAnimation>(*this, 12 * 16 + 14, 0, 2, u"water_flowing", 0, 0));
	else
		textureList.push_back(std::make_unique<TextureWaterFlowFX>());

	if (customPack && hasResource(u"/custom_fire_e_w.png") && hasResource(u"/custom_fire_n_s.png"))
	{
		textureList.push_back(std::make_unique<CustomAnimation>(*this, 1 * 16 + 15 + 16, 0, 1, u"fire_n_s", 2, 4));
		textureList.push_back(std::make_unique<CustomAnimation>(*this, 1 * 16 + 15, 0, 1, u"fire_e_w", 2, 4));
	}
	else
	{
		textureList.push_back(std::make_unique<TextureFlamesFX>(0));
		textureList.push_back(std::make_unique<TextureFlamesFX>(1));
	}

	if (customPack && hasResource(u"/custom_portal.png"))
		textureList.push_back(std::make_unique<CustomAnimation>(*this, 14, 0, 1, u"portal", -1, -1));
	else
		textureList.push_back(std::make_unique<TexturePortalFX>(14));

	for (std::unique_ptr<TextureFX> &fx : saved)
		textureList.push_back(std::move(fx));
	for (std::unique_ptr<TextureFX> &fx : textureList)
		fx->onTick();
}

void Textures::refreshColorizers()
{
	auto refresh = [this](const jstring &resourceName, void (*setter)(BufferedImage))
	{
		try
		{
			BufferedImage source = getResourceImage(resourceName);
			if (source.getWidth() < 256 || source.getHeight() < 256)
				throw std::runtime_error("Colorizer image is too small");
			std::unique_ptr<unsigned char[]> pixels = std::make_unique<unsigned char[]>(256 * 256 * 4);
			source.getRGB(0, 0, 256, 256, pixels.get());
			setter(BufferedImage(256, 256, std::move(pixels)));
		}
		catch (...)
		{
		}
	};

	refresh(u"/misc/grasscolor.png", GrassColor::setImage);
	refresh(u"/misc/foliagecolor.png", FoliageColor::setImage);
}

void Textures::tick()
{
	for (auto &fx : textureList)
	{
		fx->anaglyphEnabled = options.anaglyph3d;
		fx->onTick();

		if (fx->tileImage == 0)
			glBindTexture(GL_TEXTURE_2D, loadTexture(u"/terrain.png"));
		else if (fx->tileImage == 1)
			glBindTexture(GL_TEXTURE_2D, loadTexture(u"/gui/items.png"));
		for (int_t tx = 0; tx < fx->tileSize; ++tx)
		{
			for (int_t ty = 0; ty < fx->tileSize; ++ty)
			{
				glTexSubImage2D(GL_TEXTURE_2D, 0,
					(fx->iconIndex % 16) * TileSize::size + tx * TileSize::size,
					(fx->iconIndex / 16) * TileSize::size + ty * TileSize::size,
					TileSize::size, TileSize::size, GL_RGBA, GL_UNSIGNED_BYTE, fx->imageData.data());
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D, loadTexture(u"/terrain.png"));
}

int_t Textures::smoothBlend(int_t c0, int_t c1)
{
	int_t a0 = (c0 & 0xFF000000) >> 24 & 0xFF;
	int_t a1 = (c1 & 0xFF000000) >> 24 & 0xFF;
	return (((a0 + a1) >> 1) << 24) + (((c0 & 0xFEFEFE) + (c1 & 0xFEFEFE)) >> 1);
}

int_t Textures::crispBlend(int_t c0, int_t c1)
{
	int_t a0 = ((c0 & 0xFF000000) >> 24) & 0xFF;
	int_t a1 = ((c1 & 0xFF000000) >> 24) & 0xFF;

	int_t a = 255;
	if (a0 + a1 == 0)
	{
		a0 = 1;
		a1 = 1;
		a = 0;
	}

	int_t r0 = ((c0 >> 16) & 0xFF) * a0;
	int_t g0 = ((c0 >> 8) & 0xFF) * a0;
	int_t b0 = (c0 & 0xFF) * a0;

	int_t r1 = ((c1 >> 16) & 0xFF) * a1;
	int_t g1 = ((c1 >> 8) & 0xFF) * a1;
	int_t b1 = (c1 & 0xFF) * a1;

	int_t r = (r0 + r1) / (a0 + a1);
	int_t g = (g0 + g1) / (a0 + a1);
	int_t b = (b0 + b1) / (a0 + a1);

	return (a << 24) | (r << 16) | (g << 8) | b;
}

void Textures::reloadAll()
{
	
	for (auto &entry : loadedImages)
	{
		loadTexture(entry.second, entry.first);
	}

	
	for (auto &entry : idMap)
	{
		const jstring &name = entry.first;

		BufferedImage image;

		if (!name.compare(0, 2, u"##"))
		{
			image = getResourceImage(name.substr(2));
			image = makeStrip(image);
		}
		else if (!name.compare(0, 7, u"%clamp%"))
		{
			clamp = true;
			image = getResourceImage(name.substr(7));
		}
		else if (!name.compare(0, 6, u"%blur%"))
		{
			blur = true;
			image = getResourceImage(name.substr(6));
		}
		else
		{
			image = getResourceImage(name);
		}

		int_t id = entry.second;
		loadTexture(image, id);
		blur = false;
		clamp = false;
	}
}

BufferedImage Textures::readImage(std::istream &in)
{
	return BufferedImage::ImageIO_read(in);
}

void Textures::bind(int_t id)
{
	if (id < 0) return;
	glBindTexture(GL_TEXTURE_2D, id);
}

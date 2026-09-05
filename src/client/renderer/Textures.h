#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>

#include "java/Type.h"
#include "java/String.h"

#include "client/MemoryTracker.h"
#include "java/BufferedImage.h"

#include "client/renderer/texturefx/TextureFX.h"

class TexturePackRepository;
class Options;
class Minecraft;
struct HttpTexture;

class Textures
{
private:
	std::vector<std::unique_ptr<TextureFX>> textureList;

	std::unordered_map<jstring, int_t> idMap;
	std::unordered_map<jstring, std::shared_ptr<HttpTexture>> httpTextures;
	std::deque<std::pair<jstring, std::weak_ptr<HttpTexture>>> httpJobs;
	std::mutex httpMutex;
	std::condition_variable httpReady;
	std::thread httpWorker;
	bool stopHttp = false;
public:
	static constexpr bool MIPMAP = false;

private:
	std::unordered_map<int_t, BufferedImage> loadedImages;

	std::vector<int_t> ib = MemoryTracker::createIntBuffer(1);
	std::vector<byte_t> pixels = MemoryTracker::createByteBuffer(0x100000);

	TexturePackRepository &skins;
	Options &options;
	Minecraft &minecraft;

	bool clamp = false;
	bool blur = false;

public:
	Textures(TexturePackRepository &skins, Options &options, Minecraft &minecraft);
	~Textures();

	int_t loadTexture(const jstring &resourceName);
	BufferedImage getResourceImage(const jstring &resourceName);
	bool hasResource(const jstring &resourceName);
	void setTileSize();

private:
	BufferedImage makeStrip(BufferedImage &source);
	BufferedImage resizeImage(BufferedImage &source, int_t width);
	BufferedImage readResourceImage(const jstring &resourceName, bool resize);
	int_t calculateTileSize();
	void refreshTextureFX();
	void refreshColorizers();
	HttpTexture *addHttpTexture(const jstring &url);
	void queueHttpTexture(const jstring &url, const std::shared_ptr<HttpTexture> &texture);
	void downloadHttpTextures();
public:
	int_t getTexture(BufferedImage &img);
	void loadTexture(BufferedImage &img, int_t id);
	void releaseTexture(int_t id);
	int_t loadHttpTexture(const jstring &url, const jstring *backup);
	int_t loadHttpTexture(const jstring &url);
	void obtainHttpTexture(const jstring &url);
	void removeHttpTexture(const jstring &url);

	void registerTextureFX(std::unique_ptr<TextureFX> fx);
	void tick();

private:
	int_t smoothBlend(int_t c0, int_t c1);
	int_t crispBlend(int_t c0, int_t c1);

public:
	void reloadAll();

private:
	BufferedImage readImage(std::istream &in);

public:
	void bind(int_t id);
};

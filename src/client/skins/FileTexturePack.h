#pragma once

#include <memory>

#include "client/skins/TexturePack.h"
#include "java/BufferedImage.h"

class File;
class ZipArchive;

class FileTexturePack : public TexturePack
{
private:
	std::unique_ptr<File> file;
	std::unique_ptr<ZipArchive> archive;
	int_t texture = -1;
	std::unique_ptr<BufferedImage> icon;

public:
	explicit FileTexturePack(std::unique_ptr<File> file);
	~FileTexturePack() override;

	void select() override;
	void deselect() override;
	void load(Minecraft &minecraft) override;
	void unload(Minecraft &minecraft) override;
	void bindTexture(Minecraft &minecraft) override;
	std::istream *getResource(const jstring &name) override;
};

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "client/skins/DefaultTexturePack.h"
#include "java/String.h"

class File;
class FileTexturePack;
class Minecraft;
class TexturePack;

class TexturePackRepository
{
private:
	DefaultTexturePack defaultTexturePack;
	std::vector<TexturePack *> texturePacks;
	std::unordered_map<jstring, std::unique_ptr<FileTexturePack>> cachedTexturePacks;
	Minecraft &minecraft;
	std::unique_ptr<File> texturePackDir;
	jstring currentTexturePack;

public:
	TexturePack *selected = nullptr;

	TexturePackRepository(Minecraft &minecraft);
	~TexturePackRepository();

	void updateList();
	void updateListAndSelect();
	const std::vector<TexturePack *> &getTexturePacks() const;
	bool setSelected(TexturePack *texturePack);
};

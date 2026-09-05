#include "client/skins/TexturePackRepository.h"

#include <algorithm>
#include <unordered_set>

#include "client/Minecraft.h"
#include "client/skins/FileTexturePack.h"
#include "java/File.h"

TexturePackRepository::TexturePackRepository(Minecraft &minecraft) : minecraft(minecraft)
{
	selected = &defaultTexturePack;
	currentTexturePack = defaultTexturePack.name;
}

TexturePackRepository::~TexturePackRepository()
{
}

void TexturePackRepository::updateList()
{
	if (texturePackDir == nullptr)
	{
		texturePackDir.reset(File::open(*minecraft.getWorkingDirectory(), u"texturepacks"));
		if (!texturePackDir->exists())
			texturePackDir->mkdirs();
	}

	std::vector<TexturePack *> available;
	std::unordered_set<jstring> retained;
	selected = nullptr;
	available.push_back(&defaultTexturePack);

	if (texturePackDir->exists() && texturePackDir->isDirectory())
	{
		for (std::unique_ptr<File> &file : texturePackDir->listFiles())
		{
			jstring lowerName = file->getName();
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), [](char16_t c)
			{
				return c >= u'A' && c <= u'Z' ? static_cast<char16_t>(c - u'A' + u'a') : c;
			});
			if (!file->isFile() || lowerName.size() < 4 || lowerName.substr(lowerName.size() - 4) != u".zip")
				continue;

			jstring key = file->getName() + u":" + String::toString(file->length()) + u":" + String::toString(file->lastModified());
			auto cached = cachedTexturePacks.find(key);
			if (cached == cachedTexturePacks.end())
			{
				auto texturePack = std::make_unique<FileTexturePack>(std::move(file));
				texturePack->id = key;
				texturePack->load(minecraft);
				cached = cachedTexturePacks.emplace(key, std::move(texturePack)).first;
			}

			FileTexturePack *texturePack = cached->second.get();
			retained.insert(key);
			if (texturePack->name == currentTexturePack)
				selected = texturePack;
			available.push_back(texturePack);
		}
	}

	if (selected == nullptr)
		selected = &defaultTexturePack;

	for (auto it = cachedTexturePacks.begin(); it != cachedTexturePacks.end();)
	{
		if (retained.find(it->first) == retained.end())
		{
			it->second->unload(minecraft);
			it = cachedTexturePacks.erase(it);
		}
		else
		{
			++it;
		}
	}

	texturePacks = std::move(available);
}

void TexturePackRepository::updateListAndSelect()
{
	currentTexturePack = minecraft.options.skin;
	updateList();
	selected->select();
}

const std::vector<TexturePack *> &TexturePackRepository::getTexturePacks() const
{
	return texturePacks;
}

bool TexturePackRepository::setSelected(TexturePack *texturePack)
{
	if (texturePack == nullptr)
		texturePack = &defaultTexturePack;
	if (selected == texturePack)
		return false;

	if (selected != nullptr)
		selected->deselect();

	currentTexturePack = texturePack->name;
	selected = texturePack;
	minecraft.options.skin = currentTexturePack;
	minecraft.options.save();
	selected->select();
	minecraft.textures.setTileSize();
	if (minecraft.font != nullptr)
		minecraft.font->initialize(minecraft.options, u"/font/default.png", minecraft.textures);
	return true;
}

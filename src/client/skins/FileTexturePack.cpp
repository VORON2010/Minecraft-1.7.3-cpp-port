#include "client/skins/FileTexturePack.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "client/Minecraft.h"
#include "java/File.h"

#include "OpenGL.h"

extern "C"
{
#include "ioapi.h"
#include "unzip.h"
}

class ZipArchive
{
private:
	struct MemoryFile
	{
		std::vector<byte_t> bytes;
		size_t position = 0;
		int error = 0;
	};

	MemoryFile memory;
	unzFile handle = nullptr;

	static voidpf open(voidpf opaque, const void *, int)
	{
		MemoryFile *memory = static_cast<MemoryFile *>(opaque);
		memory->position = 0;
		memory->error = 0;
		return memory;
	}

	static uLong read(voidpf, voidpf stream, void *buffer, uLong size)
	{
		MemoryFile *memory = static_cast<MemoryFile *>(stream);
		size_t count = std::min(static_cast<size_t>(size), memory->bytes.size() - memory->position);
		if (count > 0)
			std::memcpy(buffer, memory->bytes.data() + memory->position, count);
		memory->position += count;
		return static_cast<uLong>(count);
	}

	static uLong write(voidpf, voidpf, const void *, uLong)
	{
		return 0;
	}

	static ZPOS64_T tell(voidpf, voidpf stream)
	{
		return static_cast<ZPOS64_T>(static_cast<MemoryFile *>(stream)->position);
	}

	static long seek(voidpf, voidpf stream, ZPOS64_T offset, int origin)
	{
		MemoryFile *memory = static_cast<MemoryFile *>(stream);
		std::int64_t base;
		if (origin == ZLIB_FILEFUNC_SEEK_CUR)
			base = static_cast<std::int64_t>(memory->position);
		else if (origin == ZLIB_FILEFUNC_SEEK_END)
			base = static_cast<std::int64_t>(memory->bytes.size());
		else if (origin == ZLIB_FILEFUNC_SEEK_SET)
			base = 0;
		else
			return -1;

		std::int64_t position = base + static_cast<std::int64_t>(offset);
		if (position < 0 || static_cast<std::uint64_t>(position) > memory->bytes.size())
		{
			memory->error = 1;
			return -1;
		}

		memory->position = static_cast<size_t>(position);
		return 0;
	}

	static int close(voidpf, voidpf)
	{
		return 0;
	}

	static int error(voidpf, voidpf stream)
	{
		return static_cast<MemoryFile *>(stream)->error;
	}

public:
	explicit ZipArchive(const File &file)
	{
		std::unique_ptr<std::istream> input(file.toStreamIn());
		if (input == nullptr)
			throw std::runtime_error("Failed to open texture pack");
		memory.bytes.assign(std::istreambuf_iterator<char>(*input), std::istreambuf_iterator<char>());

		zlib_filefunc64_def callbacks = {};
		callbacks.zopen64_file = open;
		callbacks.zread_file = read;
		callbacks.zwrite_file = write;
		callbacks.ztell64_file = tell;
		callbacks.zseek64_file = seek;
		callbacks.zclose_file = close;
		callbacks.zerror_file = error;
		callbacks.opaque = &memory;
		handle = unzOpen2_64(nullptr, &callbacks);
		if (handle == nullptr)
			throw std::runtime_error("Invalid texture pack ZIP");
	}

	~ZipArchive()
	{
		if (handle != nullptr)
			unzClose(handle);
	}

	bool get(const std::string &name, std::string &contents)
	{
		if (unzLocateFile(handle, name.c_str(), 1) != UNZ_OK)
			return false;

		unz_file_info64 info = {};
		if (unzGetCurrentFileInfo64(handle, &info, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK)
			return false;
		if (unzOpenCurrentFile(handle) != UNZ_OK)
			return false;

		contents.resize(static_cast<size_t>(info.uncompressed_size));
		size_t position = 0;
		while (position < contents.size())
		{
			unsigned int remaining = static_cast<unsigned int>(std::min<size_t>(contents.size() - position, 65536));
			int count = unzReadCurrentFile(handle, &contents[position], remaining);
			if (count <= 0)
			{
				unzCloseCurrentFile(handle);
				return false;
			}
			position += static_cast<size_t>(count);
		}

		return unzCloseCurrentFile(handle) == UNZ_OK;
	}
};

namespace
{
	jstring truncate(const std::string &line)
	{
		jstring result = String::fromUTF8(line);
		if (result.size() > 34)
			result.resize(34);
		return result;
	}
}

FileTexturePack::FileTexturePack(std::unique_ptr<File> file) : file(std::move(file))
{
	name = this->file->getName();
}

FileTexturePack::~FileTexturePack()
{
}

void FileTexturePack::select()
{
	try
	{
		archive = std::make_unique<ZipArchive>(*file);
	}
	catch (...)
	{
		archive.reset();
	}
}

void FileTexturePack::deselect()
{
	archive.reset();
}

void FileTexturePack::load(Minecraft &minecraft)
{
	try
	{
		ZipArchive archive(*file);
		std::string contents;
		if (archive.get("pack.txt", contents))
		{
			std::istringstream input(contents);
			std::string line;
			if (std::getline(input, line))
			{
				if (!line.empty() && line.back() == '\r')
					line.pop_back();
				desc1 = truncate(line);
			}
			if (std::getline(input, line))
			{
				if (!line.empty() && line.back() == '\r')
					line.pop_back();
				desc2 = truncate(line);
			}
		}

		if (archive.get("pack.png", contents))
		{
			try
			{
				std::istringstream input(contents, std::ios::binary);
				icon = std::make_unique<BufferedImage>(BufferedImage::ImageIO_read(input));
			}
			catch (...)
			{
			}
		}
	}
	catch (...)
	{
	}
}

void FileTexturePack::unload(Minecraft &minecraft)
{
	if (icon != nullptr && texture >= 0)
		minecraft.textures.releaseTexture(texture);
	deselect();
}

void FileTexturePack::bindTexture(Minecraft &minecraft)
{
	if (icon != nullptr && texture < 0)
		texture = minecraft.textures.getTexture(*icon);
	if (icon != nullptr)
		minecraft.textures.bind(texture);
	else
		glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/gui/unknown_pack.png"));
}

std::istream *FileTexturePack::getResource(const jstring &name)
{
	try
	{
		std::string contents;
		jstring entryName = !name.empty() && name.front() == u'/' ? name.substr(1) : name;
		if (archive != nullptr && archive->get(String::toUTF8(entryName), contents))
			return new std::istringstream(contents, std::ios::binary);
	}
	catch (...)
	{
	}
	return TexturePack::getResource(name);
}

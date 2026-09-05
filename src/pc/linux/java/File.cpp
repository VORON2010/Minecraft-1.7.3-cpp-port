#include "java/File.h"

#include <queue>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cerrno>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif


static std::string ToPath(const jstring &path)
{
	std::string result;
	for (char c : String::toUTF8(path))
	{
		if (c != '/' || result.empty() || result.back() != '/')
			result.push_back(c);
	}
	if (result.size() > 1 && result.back() == '/')
		result.pop_back();
	return result;
}

static jstring FromPath(const std::string &path)
{
	return String::fromUTF8(path);
}

class File_Impl : public File
{
private:
	std::string u8path;

public:
	File_Impl(const jstring &path)
	{
		u8path = ToPath(path);
		this->path = FromPath(u8path);
		if (this->path.find(u'\0') != jstring::npos)
			u8path.clear();
	}

	virtual ~File_Impl()
	{

	}

	virtual bool createNewFile() const override
	{
		int fd = ::open(u8path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
		if (fd < 0)
			return false;
		::close(fd);
		return true;
	
	}

	bool remove() const override
	{
		if (isDirectory())
			return ::rmdir(u8path.c_str()) == 0;
		else
			return ::unlink(u8path.c_str()) == 0;
	}

	bool renameTo(const File &dest) const override
	{
		std::string destination = ToPath(dest.toString());
		if (destination.find('\0') != std::string::npos)
			return false;
		return ::rename(u8path.c_str(), destination.c_str()) == 0;
	}

	bool exists() const override
	{
		struct ::stat buffer;
		return ::stat(u8path.c_str(), &buffer) == 0;
	}

	bool isDirectory() const override
	{
		struct ::stat buffer;
		if (::stat(u8path.c_str(), &buffer) != 0)
			return false;
		return S_ISDIR(buffer.st_mode);
	}

	bool isFile() const override
	{
		struct ::stat buffer;
		if (::stat(u8path.c_str(), &buffer) != 0)
			return false;
		return S_ISREG(buffer.st_mode);
	}

	long_t lastModified() const override
	{
		struct ::stat buffer;
		if (::stat(u8path.c_str(), &buffer) != 0)
			return false;
		
#if defined(__APPLE__)
		return buffer.st_mtimespec.tv_sec * 1000LL + buffer.st_mtimespec.tv_nsec / 1000000LL;
#elif defined(st_mtime)
		return buffer.st_mtim.tv_sec * 1000LL + buffer.st_mtim.tv_nsec / 1000000LL;
#else
		return buffer.st_mtime * 1000LL;
#endif
	}

	long_t length() const override
	{
		struct ::stat buffer;
		if (::stat(u8path.c_str(), &buffer) != 0)
			return false;
		return buffer.st_size;
	}

	std::vector<std::unique_ptr<File>> listFiles() const override
	{
		std::vector<std::unique_ptr<File>> files;

		if (!isDirectory())
			return files;

		::DIR *dir = ::opendir(u8path.c_str());
		if (!dir)
			return files;

		struct dirent *entry;
		while ((entry = readdir(dir)) != nullptr)
		{
			
			if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
				continue;

			std::string child_path = u8path + "/" + entry->d_name;
			files.push_back(std::make_unique<File_Impl>(FromPath(child_path)));
		}

		::closedir(dir);
		return files;
	}

	File *getParentFile() const override
	{
		size_t npos = path.find_last_of(u'/');
		if (npos == jstring::npos || path == u"/")
			return nullptr;
		return new File_Impl(path.substr(0, npos == 0 ? 1 : npos));
	}

	bool mkdir() const override
	{
		return ::mkdir(u8path.c_str(), 0777) == 0;
	}

	std::istream *toStreamIn() const override
	{
		auto is = std::make_unique<std::ifstream>(u8path, std::ios::binary);
		if (!is->is_open() || !is->good())
			return nullptr;
		return is.release();
	}

	std::ostream *toStreamOut() const override
	{
		auto os = std::make_unique<std::ofstream>(u8path, std::ios::binary);
		if (!os->is_open() || !os->good())
			return nullptr;
		return os.release();
	}

	friend File *File::open(const jstring &path);
	friend File *File::open(const File &parent, const jstring &child);
};

File *File::open(const jstring &path)
{
	return new File_Impl(path);
}

File *File::open(const File &parent, const jstring &child)
{
	jstring new_path = parent.path + u'/' + child;
	return new File_Impl(new_path);
}

File *File::openResourceDirectory()
{
	
	std::vector<char> path(PATH_MAX);
#ifdef __APPLE__
	uint32_t length = static_cast<uint32_t>(path.size());
	if (_NSGetExecutablePath(path.data(), &length) != 0)
	{
		path.resize(length);
		if (_NSGetExecutablePath(path.data(), &length) != 0)
			throw std::runtime_error("Failed to get executable path");
	}
#else
	for (;;)
	{
		ssize_t length = ::readlink("/proc/self/exe", path.data(), path.size() - 1);
		if (length < 0)
			throw std::runtime_error("Failed to get executable path");
		if (static_cast<size_t>(length) < path.size() - 1)
		{
			path[static_cast<size_t>(length)] = '\0';
			break;
		}
		path.resize(path.size() * 2);
	}
#endif
	jstring u16str = FromPath(path.data());

	
	size_t pos = u16str.find_last_of(u'/');
	if (pos == std::string::npos)
		return new File_Impl(u"");

	
	return new File_Impl(u16str.substr(0, pos) + u"/resource");
}

File *File::openWorkingDirectory(const jstring &name)
{
	
	const char *path = ::getenv("HOME");
	if (path == nullptr)
		return new File_Impl(u"");

	
	jstring u16str = FromPath(path);

	return new File_Impl(u16str + u"/" + name);
}

jstring File::getName() const
{
	size_t pos = path.find_last_of(u'/');
	return pos == jstring::npos ? path : path.substr(pos + 1);
}

jstring File::toURL() const
{
	if (path.find(u'\0') != jstring::npos)
		throw std::runtime_error("java.net.MalformedURLException: Invalid file path");
	jstring absolute = path;
	if (absolute.empty() || absolute.front() != u'/')
	{
		std::vector<char> current(PATH_MAX);
		while (::getcwd(current.data(), current.size()) == nullptr)
		{
			if (errno != ERANGE)
				throw std::runtime_error("Failed to get working directory");
			current.resize(current.size() * 2);
		}
		absolute = FromPath(current.data());
		if (!path.empty())
			absolute += (absolute.back() == u'/' ? u"" : u"/") + path;
	}
	if (isDirectory() && absolute.back() != u'/')
		absolute.push_back(u'/');
	return u"file:" + absolute;
}

#include "java/File.h"

#include <stack>
#include <string>
#include <iostream>


jstring File::toString() const
{
	return path;
}


bool File::mkdirs() const
{
	if (exists())
		return false;
	if (mkdir())
		return true;

	
	std::stack<std::unique_ptr<File>> back;

	jstring current = path;
	while (!current.empty())
	{
		std::unique_ptr<File> fp(File::open(current));
		if (fp->isDirectory())
			break;

#ifdef _WIN32
		size_t npos = current.find_last_of(u"/\\");
#else
		size_t npos = current.find_last_of(u'/');
#endif
		back.emplace(std::move(fp));
		if (npos == jstring::npos)
			break;
		current = current.substr(0, npos);
	}

	if (back.empty())
		return false;

	
	while (!back.empty())
	{
		auto fp = back.top().get();
		if (!fp->mkdir())
			return false;
		back.pop();
	}

	return true;
}

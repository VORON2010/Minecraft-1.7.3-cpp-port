#include "world/stats/AchievementMap.h"

#include <unordered_map>

#include "java/Resource.h"

namespace
{
	std::unordered_map<int_t, jstring> &guidMap()
	{
		static std::unordered_map<int_t, jstring> result;
		static bool loaded = false;
		if (loaded)
			return result;

		loaded = true;
		std::unique_ptr<std::istream> stream(Resource::getResource(u"/achievement/map.txt"));
		std::string line;
		while (stream != nullptr && std::getline(*stream, line))
		{
			size_t comma = line.find(',');
			if (comma == std::string::npos)
				continue;
			if (!line.empty() && line.back() == '\r')
				line.pop_back();
			result.emplace(std::stoi(line.substr(0, comma)), String::fromUTF8(line.substr(comma + 1)));
		}
		return result;
	}
}

jstring AchievementMap::getGuid(int_t statId)
{
	auto &map = guidMap();
	auto it = map.find(statId);
	return it == map.end() ? u"" : it->second;
}

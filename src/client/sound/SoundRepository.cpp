#include "client/sound/SoundRepository.h"

jstring SoundRepository::poolName(const jstring &name) const
{
	jstring baseName = name;
	size_t dotPos = baseName.find(u'.');
	if (dotPos != jstring::npos)
		baseName = baseName.substr(0, dotPos);

	
	if (trimDigits)
	{
		while (!baseName.empty() && baseName.back() >= u'0' && baseName.back() <= u'9')
			baseName = baseName.substr(0, baseName.length() - 1);
	}

	
	for (size_t i = 0; i < baseName.length(); ++i)
	{
		if (baseName[i] == u'/')
			baseName[i] = u'.';
	}

	return baseName;
}

Sound SoundRepository::add(const jstring &name, const std::string &filePath)
{
	jstring baseName = poolName(name);
	primary.insert(baseName);

	Sound sound(name, filePath);
	urls[baseName].push_back(sound);
	all.push_back(sound);
	count++;

	return sound;
}

// 1.5.2 loads only sound3; older sound/newsound files stay as a fallback
// for names that sound3 does not have
bool SoundRepository::addFallback(const jstring &name, const std::string &filePath)
{
	jstring baseName = poolName(name);
	if (primary.count(baseName))
		return false;
	Sound sound(name, filePath);
	urls[baseName].push_back(sound);
	all.push_back(sound);
	count++;
	return true;
}

Sound *SoundRepository::get(const jstring &name)
{
	auto it = urls.find(name);
	if (it == urls.end() || it->second.empty())
		return nullptr;

	return &it->second[random.nextInt((int_t)it->second.size())];
}

Sound *SoundRepository::any()
{
	if (all.empty())
		return nullptr;

	return &all[random.nextInt((int_t)all.size())];
}

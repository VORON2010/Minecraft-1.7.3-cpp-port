#include "world/stats/StatCollector.h"

#include "client/locale/Language.h"

jstring StatCollector::translate(const jstring &key)
{
	return Language::getInstance().getElement(key);
}

jstring StatCollector::translate(const jstring &key, const jstring &argument)
{
	jstring value = translate(key);
	size_t position = value.find(u"%1$s");
	if (position == jstring::npos)
		position = value.find(u"%s");
	if (position != jstring::npos)
		value.replace(position, value[position + 1] == u'1' ? 4 : 2, argument);
	return value;
}

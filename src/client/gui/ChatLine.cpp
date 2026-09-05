#include "client/gui/ChatLine.h"

#include "client/gui/Font.h"

std::vector<jstring> ChatLine::split(Font &font, const jstring &text, int_t maxWidth)
{
	std::vector<jstring> lines;
	jstring remaining = text;
	while (font.width(remaining) > maxWidth)
	{
		int_t splitPos = 1;
		while (splitPos < static_cast<int_t>(remaining.length()) && font.width(remaining.substr(0, splitPos + 1)) <= maxWidth)
			splitPos++;
		lines.push_back(remaining.substr(0, splitPos));
		remaining = remaining.substr(splitPos);
	}
	lines.push_back(remaining);
	return lines;
}

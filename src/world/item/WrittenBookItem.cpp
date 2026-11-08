#include "world/item/WrittenBookItem.h"

#include "nbt/CompoundTag.h"
#include "world/item/ItemInstance.h"

jstring WrittenBookItem::getHoverName(const ItemInstance &stack) const
{
	if (stack.otherTag != nullptr && stack.otherTag->contains(u"title"))
		return stack.otherTag->getString(u"title");
	return u"";
}

void WrittenBookItem::appendHoverText(const ItemInstance &stack, std::vector<jstring> &lines) const
{
	if (stack.otherTag != nullptr && stack.otherTag->contains(u"author"))
		lines.push_back(u"\u00a77by " + stack.otherTag->getString(u"author"));
}

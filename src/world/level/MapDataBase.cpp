#include "world/level/MapDataBase.h"

MapDataBase::MapDataBase(const jstring &id) : id(id)
{
}

void MapDataBase::markDirty()
{
	setDirty(true);
}

void MapDataBase::setDirty(bool dirty)
{
	this->dirty = dirty;
}

bool MapDataBase::isDirty() const
{
	return dirty;
}

#pragma once

#include "java/String.h"
#include "java/Type.h"

class CompoundTag;

class MapDataBase
{
public:
	jstring id;

	MapDataBase(const jstring &id);
	virtual ~MapDataBase() {}

	virtual void readFromNBT(CompoundTag &tag) = 0;
	virtual void writeToNBT(CompoundTag &tag) = 0;

	void markDirty();
	void setDirty(bool dirty);
	bool isDirty() const;

private:
	bool dirty = false;
};

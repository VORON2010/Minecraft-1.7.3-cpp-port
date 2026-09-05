#pragma once

#include "world/stats/StatBase.h"

class StatCrafting : public StatBase
{
private:
	const int_t itemId;

public:
	StatCrafting(int_t id, const jstring &name, int_t itemId);
	int_t getItemId() const;
};

#pragma once

#include "world/stats/StatBase.h"

class StatBasic : public StatBase
{
public:
	StatBasic(int_t id, const jstring &name);
	StatBasic(int_t id, const jstring &name, const StatFormatter &formatter);

	StatBase &registerStat() override;
};

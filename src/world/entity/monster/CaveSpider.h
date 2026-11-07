#pragma once

#include "world/entity/monster/Spider.h"

// b1.8 cave spider: small spider from abandoned mineshafts, poisons on normal and hard
class CaveSpider : public Spider
{
public:
	CaveSpider(Level &level);
	jstring getEncodeId() const override { return u"CaveSpider"; }

protected:
	void checkHurtTarget(Entity &entity, float distance) override;
};

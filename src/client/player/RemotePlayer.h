#pragma once

#include "world/entity/player/Player.h"

class RemotePlayer : public Player
{
private:
	int_t interpolationSteps = 0;
	double interpolationX = 0.0;
	double interpolationY = 0.0;
	double interpolationZ = 0.0;
	double interpolationYRot = 0.0;
	double interpolationXRot = 0.0;

protected:
	void resetHeight() override;

public:
	float fallTime = 0.0f;

	RemotePlayer(Level &level, const jstring &name);

	bool hurt(Entity *source, int_t dmg) override;
	void lerpTo(double x, double y, double z, float yRot, float xRot, int_t steps) override;
	void tick() override;
	float getShadowHeightOffs() override;
	void aiStep() override;
	void setEquippedSlot(int_t slot, int_t itemId, int_t auxValue) override;
	void animateRespawn() override;
};

#include "client/player/RemotePlayer.h"

#include "util/Mth.h"

RemotePlayer::RemotePlayer(Level &level, const jstring &name) : Player(level)
{
	this->name = name;
	heightOffset = 0.0f;
	footSize = 0.0f;
	if (!name.empty())
		customTextureUrl = u"http://s3.amazonaws.com/MinecraftSkins/" + name + u".png";

	noPhysics = true;
	bedViewY = 0.25f;
	viewScale = 10.0;
}

void RemotePlayer::resetHeight()
{
	heightOffset = 0.0f;
}

bool RemotePlayer::hurt(Entity *source, int_t dmg)
{
	(void)source;
	(void)dmg;
	return true;
}

void RemotePlayer::lerpTo(double x, double y, double z, float yRot, float xRot, int_t steps)
{
	interpolationX = x;
	interpolationY = y;
	interpolationZ = z;
	interpolationYRot = yRot;
	interpolationXRot = xRot;
	interpolationSteps = steps;
}

void RemotePlayer::tick()
{
	bedViewY = 0.0f;
	Player::tick();
	walkAnimSpeedO = walkAnimSpeed;
	double xDelta = x - xo;
	double zDelta = z - zo;
	float speed = Mth::sqrt(xDelta * xDelta + zDelta * zDelta) * 4.0f;
	if (speed > 1.0f)
		speed = 1.0f;

	walkAnimSpeed += (speed - walkAnimSpeed) * 0.4f;
	walkAnimPos += walkAnimSpeed;
}

float RemotePlayer::getShadowHeightOffs()
{
	return 0.0f;
}

void RemotePlayer::aiStep()
{
	Player::updateAi();
	if (interpolationSteps > 0)
	{
		double targetX = x + (interpolationX - x) / interpolationSteps;
		double targetY = y + (interpolationY - y) / interpolationSteps;
		double targetZ = z + (interpolationZ - z) / interpolationSteps;
		double yRotDelta = interpolationYRot - yRot;

		while (yRotDelta < -180.0)
			yRotDelta += 360.0;
		while (yRotDelta >= 180.0)
			yRotDelta -= 360.0;

		yRot = static_cast<float>(yRot + yRotDelta / interpolationSteps);
		xRot = static_cast<float>(xRot + (interpolationXRot - xRot) / interpolationSteps);
		--interpolationSteps;
		setPos(targetX, targetY, targetZ);
		setRot(yRot, xRot);
	}

	oBob = bob;
	float targetBob = Mth::sqrt(xd * xd + zd * zd);
	float targetTilt = static_cast<float>(std::atan(-yd * 0.2f)) * 15.0f;
	if (targetBob > 0.1f)
		targetBob = 0.1f;

	if (!onGround || health <= 0)
		targetBob = 0.0f;
	if (onGround || health <= 0)
		targetTilt = 0.0f;

	bob += (targetBob - bob) * 0.4f;
	tilt += (targetTilt - tilt) * 0.8f;
}

void RemotePlayer::setEquippedSlot(int_t slot, int_t itemId, int_t auxValue)
{
	ItemInstance item;
	if (itemId >= 0)
		item = ItemInstance(itemId, 1, auxValue);

	if (slot == 0)
		inventory.mainInventory.at(inventory.currentItem) = item;
	else
		inventory.armorInventory.at(slot - 1) = item;
}

void RemotePlayer::animateRespawn()
{
}

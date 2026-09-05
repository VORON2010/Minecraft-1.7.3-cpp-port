#include "world/entity/animal/Squid.h"

#include <cmath>

#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"
#include "util/Mth.h"

Squid::Squid(Level &level) : Animal(level)
{
	textureName = u"/mob/squid.png";
	setSize(0.95f, 0.95f);
	health = 10;
	tentacleSpeed = 1.0f / (random.nextFloat() + 1.0f) * 0.2f;
}

bool Squid::isInWater()
{
	return level.handleMaterialAcceleration(*bb.grow(0.0, -0.6f, 0.0), Material::water, *this);
}

void Squid::aiStep()
{
	Animal::aiStep();
	xBodyRotO = xBodyRot;
	zBodyRotO = zBodyRot;
	oldTentacleMovement = tentacleMovement;
	oldTentacleAngle = tentacleAngle;
	tentacleMovement += tentacleSpeed;
	if (tentacleMovement > Mth::PI * 2.0f)
	{
		tentacleMovement -= Mth::PI * 2.0f;
		if (random.nextInt(10) == 0)
			tentacleSpeed = 1.0f / (random.nextFloat() + 1.0f) * 0.2f;
	}

	if (isInWater())
	{
		if (tentacleMovement < Mth::PI)
		{
			float cycle = tentacleMovement / Mth::PI;
			tentacleAngle = Mth::sin(cycle * cycle * Mth::PI) * Mth::PI * 0.25f;
			if (cycle > 0.75f)
			{
				speed = 1.0f;
				rotateSpeed = 1.0f;
			}
			else
			{
				rotateSpeed *= 0.8f;
			}
		}
		else
		{
			tentacleAngle = 0.0f;
			speed *= 0.9f;
			rotateSpeed *= 0.99f;
		}

		if (!interpolateOnly)
		{
			xd = tx * speed;
			yd = ty * speed;
			zd = tz * speed;
		}

		float horizontal = Mth::sqrt((float)(xd * xd + zd * zd));
		yBodyRot += (-((float)std::atan2(xd, zd)) * Mth::RADDEG - yBodyRot) * 0.1f;
		yRot = yBodyRot;
		zBodyRot += Mth::PI * rotateSpeed * 1.5f;
		xBodyRot += (-((float)std::atan2(horizontal, yd)) * Mth::RADDEG - xBodyRot) * 0.1f;
	}
	else
	{
		tentacleAngle = Mth::abs(Mth::sin(tentacleMovement)) * Mth::PI * 0.25f;
		if (!interpolateOnly)
		{
			xd = 0.0;
			yd -= 0.08;
			yd *= 0.98f;
			zd = 0.0;
		}
		xBodyRot += (-90.0f - xBodyRot) * 0.02f;
	}
}

void Squid::travel(float x, float z)
{
	(void)x;
	(void)z;
	move(xd, yd, zd);
}

jstring Squid::getAmbientSound()
{
	return {};
}

jstring Squid::getHurtSound()
{
	return {};
}

jstring Squid::getDeathSound()
{
	return {};
}

float Squid::getSoundVolume()
{
	return 0.4f;
}

int_t Squid::getDeathLoot()
{
	return 0;
}

void Squid::dropDeathLoot()
{
	int_t count = random.nextInt(3) + 1;
	for (int_t i = 0; i < count; ++i)
		spawnAtLocation(ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 0), 0.0f);
}

void Squid::updateAi()
{
	if (random.nextInt(50) == 0 || !wasInWater || (tx == 0.0f && ty == 0.0f && tz == 0.0f))
	{
		float angle = random.nextFloat() * Mth::PI * 2.0f;
		tx = Mth::cos(angle) * 0.2f;
		ty = -0.1f + random.nextFloat() * 0.2f;
		tz = Mth::sin(angle) * 0.2f;
	}
}

bool Squid::isWaterMob()
{
	return true;
}

bool Squid::canSpawn()
{
	return level.isUnobstructed(bb);
}

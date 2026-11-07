#include "world/entity/EyeOfEnderSignal.h"

#include <cmath>

#include "util/Mth.h"
#include "world/entity/item/EntityItem.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

EyeOfEnderSignal::EyeOfEnderSignal(Level &level) : Entity(level)
{
	setSize(0.25f, 0.25f);
}

EyeOfEnderSignal::EyeOfEnderSignal(Level &level, double x, double y, double z) : Entity(level)
{
	life = 0;
	setSize(0.25f, 0.25f);
	setPos(x, y, z);
	heightOffset = 0.0f;
}

bool EyeOfEnderSignal::shouldRenderAtSqrDistance(double distance)
{
	double size = bb.getSize() * 4.0;
	size *= 64.0;
	return distance < size * size;
}

void EyeOfEnderSignal::signalTo(double tx, int_t ty, double tz)
{
	double dx = tx - x;
	double dz = tz - z;
	float dist = Mth::sqrt(dx * dx + dz * dz);
	if (dist > 12.0f)
	{
		targetX = x + dx / dist * 12.0;
		targetZ = z + dz / dist * 12.0;
		targetY = y + 8.0;
	}
	else
	{
		targetX = tx;
		targetY = ty;
		targetZ = tz;
	}
	life = 0;
	surviveAfterDeath = random.nextInt(5) > 0;
}

void EyeOfEnderSignal::lerpMotion(double nxd, double nyd, double nzd)
{
	xd = nxd;
	yd = nyd;
	zd = nzd;
	if (xRotO == 0.0f && yRotO == 0.0f)
	{
		float horizontal = Mth::sqrt(nxd * nxd + nzd * nzd);
		yRotO = yRot = static_cast<float>(std::atan2(nxd, nzd) * 180.0 / Mth::PI);
		xRotO = xRot = static_cast<float>(std::atan2(nyd, static_cast<double>(horizontal)) * 180.0 / Mth::PI);
	}
}

void EyeOfEnderSignal::tick()
{
	xo = x;
	yo = y;
	zo = z;
	Entity::tick();
	x += xd;
	y += yd;
	z += zd;
	float horizontal = Mth::sqrt(xd * xd + zd * zd);
	yRot = static_cast<float>(std::atan2(xd, zd) * 180.0 / Mth::PI);
	xRot = static_cast<float>(std::atan2(yd, static_cast<double>(horizontal)) * 180.0 / Mth::PI);
	while (xRot - xRotO < -180.0f)
		xRotO -= 360.0f;
	while (xRot - xRotO >= 180.0f)
		xRotO += 360.0f;
	while (yRot - yRotO < -180.0f)
		yRotO -= 360.0f;
	while (yRot - yRotO >= 180.0f)
		yRotO += 360.0f;
	xRot = xRotO + (xRot - xRotO) * 0.2f;
	yRot = yRotO + (yRot - yRotO) * 0.2f;

	if (!level.isOnline)
	{
		double dx = targetX - x;
		double dz = targetZ - z;
		float dist = static_cast<float>(std::sqrt(dx * dx + dz * dz));
		float angle = static_cast<float>(std::atan2(dz, dx));
		double speed = horizontal + (dist - horizontal) * 0.0025;
		if (dist < 1.0f)
		{
			speed *= 0.8;
			yd *= 0.8;
		}
		xd = std::cos(static_cast<double>(angle)) * speed;
		zd = std::sin(static_cast<double>(angle)) * speed;
		if (y < targetY)
			yd = yd + (1.0 - yd) * 0.015f;
		else
			yd = yd + (-1.0 - yd) * 0.015f;
	}

	float tail = 0.25f;
	if (isInWater())
	{
		for (int_t i = 0; i < 4; i++)
			level.addParticle(u"bubble", x - xd * tail, y - yd * tail, z - zd * tail, xd, yd, zd);
	}
	else
	{
		level.addParticle(u"portal", x - xd * tail + random.nextDouble() * 0.6 - 0.3, y - yd * tail - 0.5, z - zd * tail + random.nextDouble() * 0.6 - 0.3, xd, yd, zd);
	}

	if (!level.isOnline)
	{
		setPos(x, y, z);
		life++;
		if (life > 80)
		{
			remove();
			if (surviveAfterDeath)
				level.addEntity(std::make_shared<EntityItem>(level, x, y, z, ItemInstance(Items::eyeOfEnder->getShiftedIndex(), 1, 0)));
			else
				level.levelEvent(2003, static_cast<int_t>(std::round(x)), static_cast<int_t>(std::round(y)), static_cast<int_t>(std::round(z)), 0);
		}
	}
}

float EyeOfEnderSignal::getBrightness(float a)
{
	(void)a;
	return 1.0f;
}

void EyeOfEnderSignal::addAdditionalSaveData(CompoundTag &tag)
{
	(void)tag;
}

void EyeOfEnderSignal::readAdditionalSaveData(CompoundTag &tag)
{
	(void)tag;
}

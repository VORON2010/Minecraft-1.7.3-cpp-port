#include "world/entity/projectile/ThrownEnderpearl.h"

#include "world/level/Level.h"
#include "world/phys/HitResult.h"

void ThrownEnderpearl::onHit(HitResult &hit)
{
	std::shared_ptr<Entity> thrower = owner.lock();
	if (hit.entity != nullptr)
		hit.entity->hurt(thrower.get(), 0);
	for (int_t i = 0; i < 32; i++)
		level.addParticle(u"portal", x, y + random.nextDouble() * 2.0, z, random.nextGaussian(), 0.0, random.nextGaussian());
	if (!level.isOnline)
	{
		if (thrower != nullptr)
		{
			thrower->moveTo(x, y, z, thrower->yRot, thrower->xRot);
			thrower->fallDistance = 0.0f;
			thrower->hurt(nullptr, 5);
		}
		remove();
	}
}

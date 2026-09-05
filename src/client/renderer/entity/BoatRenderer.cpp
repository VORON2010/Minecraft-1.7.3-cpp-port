#include "client/renderer/entity/BoatRenderer.h"

#include <cmath>

#include "OpenGL.h"
#include "util/Mth.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/item/EntityBoat.h"

BoatRenderer::BoatRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: EntityRenderer(entityRenderDispatcher)
{
	shadowRadius = 0.5f;
}

void BoatRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	EntityBoat &boat = static_cast<EntityBoat &>(entity);
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	glRotatef(180.0f - rot, 0.0f, 1.0f, 0.0f);

	float hurtTime = static_cast<float>(boat.boatTimeSinceHit) - a;
	float damage = static_cast<float>(boat.boatCurrentDamage) - a;
	if (damage < 0.0f)
		damage = 0.0f;
	if (hurtTime > 0.0f)
		glRotatef(Mth::sin(hurtTime) * hurtTime * damage / 10.0f * boat.boatRockDirection, 1.0f, 0.0f, 0.0f);

	bindTexture(u"/terrain.png");
	float ss = 0.75f;
	glScalef(ss, ss, ss);
	glScalef(1.0f / ss, 1.0f / ss, 1.0f / ss);
	bindTexture(u"/item/boat.png");
	glScalef(-1.0f, -1.0f, 1.0f);
	modelBoat.render(0.0f, 0.0f, -0.1f, 0.0f, 0.0f, 1.0f / 16.0f);
	glPopMatrix();
}

#include "client/renderer/entity/MinecartRenderer.h"

#include <cmath>

#include "OpenGL.h"
#include "util/Mth.h"
#include "world/phys/Vec3.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/item/EntityMinecart.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/ChestTile.h"

MinecartRenderer::MinecartRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: EntityRenderer(entityRenderDispatcher), tileRenderer(false, false)
{
	shadowRadius = 0.5f;
}

void MinecartRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	EntityMinecart &minecart = static_cast<EntityMinecart &>(entity);
	glPushMatrix();

	double interpX = minecart.xOld + (minecart.x - minecart.xOld) * a;
	double interpY = minecart.yOld + (minecart.y - minecart.yOld) * a;
	double interpZ = minecart.zOld + (minecart.z - minecart.zOld) * a;
	double offset = 0.3;
	Vec3 *center = minecart.getPosOnTrack(interpX, interpY, interpZ);
	float pitch = minecart.xRotO + (minecart.xRot - minecart.xRotO) * a;
	if (center != nullptr)
	{
		Vec3 *ahead = minecart.getPosOffs(interpX, interpY, interpZ, offset);
		Vec3 *behind = minecart.getPosOffs(interpX, interpY, interpZ, -offset);
		if (ahead == nullptr)
			ahead = center;
		if (behind == nullptr)
			behind = center;

		x += center->x - interpX;
		y += (ahead->y + behind->y) * 0.5 - interpY;
		z += center->z - interpZ;
		Vec3 *dir = behind->add(-ahead->x, -ahead->y, -ahead->z);
		if (dir->length() != 0.0)
		{
			dir = dir->normalize();
			rot = static_cast<float>(std::atan2(dir->z, dir->x) * 180.0 / Mth::PI);
			pitch = static_cast<float>(std::atan(dir->y) * 73.0);
		}
	}

	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	glRotatef(180.0f - rot, 0.0f, 1.0f, 0.0f);
	glRotatef(-pitch, 0.0f, 0.0f, 1.0f);

	float hurtTime = static_cast<float>(minecart.minecartTimeSinceHit) - a;
	float damage = static_cast<float>(minecart.minecartCurrentDamage) - a;
	if (damage < 0.0f)
		damage = 0.0f;
	if (hurtTime > 0.0f)
		glRotatef(Mth::sin(hurtTime) * hurtTime * damage / 10.0f * minecart.minecartRockDirection, 1.0f, 0.0f, 0.0f);

	if (minecart.minecartType == EntityMinecart::TYPE_CHEST || minecart.minecartType == EntityMinecart::TYPE_FURNACE)
	{
		bindTexture(u"/terrain.png");
		float cargoScale = 12.0f / 16.0f;
		glScalef(cargoScale, cargoScale, cargoScale);
		glTranslatef(0.0f, 5.0f / 16.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		if (minecart.minecartType == EntityMinecart::TYPE_CHEST)
			tileRenderer.renderGuiTile(static_cast<Tile &>(Tile::chest), 0);
		else
			tileRenderer.renderGuiTile(static_cast<Tile &>(Tile::furnace), 0);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, -(5.0f / 16.0f), 0.0f);
		glScalef(1.0f / cargoScale, 1.0f / cargoScale, 1.0f / cargoScale);
	}

	bindTexture(u"/item/cart.png");
	glScalef(-1.0f, -1.0f, 1.0f);
	modelMinecart.render(0.0f, 0.0f, -0.1f, 0.0f, 0.0f, 1.0f / 16.0f);
	glPopMatrix();
}

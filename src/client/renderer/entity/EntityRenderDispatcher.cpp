#include "client/renderer/entity/EntityRenderDispatcher.h"

#include "client/renderer/entity/ArrowRenderer.h"
#include "client/renderer/entity/ThrownItemRenderer.h"
#include "client/renderer/entity/BoatRenderer.h"
#include "client/renderer/entity/ChickenRenderer.h"
#include "client/renderer/entity/CreeperRenderer.h"
#include "client/renderer/entity/FallingTileRenderer.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "client/renderer/entity/MinecartRenderer.h"
#include "client/renderer/entity/PigRenderer.h"
#include "client/renderer/entity/PlayerRenderer.h"
#include "client/renderer/entity/SheepRenderer.h"
#include "client/renderer/entity/SpiderRenderer.h"
#include "client/renderer/entity/SquidRenderer.h"
#include "client/renderer/entity/SlimeRenderer.h"
#include "client/renderer/entity/WolfRenderer.h"
#include "client/renderer/entity/GiantRenderer.h"
#include "client/renderer/entity/FireballRenderer.h"
#include "client/renderer/entity/FishingHookRenderer.h"
#include "client/renderer/entity/PaintingRenderer.h"
#include "client/renderer/entity/LightningBoltRenderer.h"
#include "client/renderer/entity/GenericEntityRenderer.h"
#include "client/renderer/entity/GhastRenderer.h"
#include "client/renderer/entity/TNTPrimedRenderer.h"
#include "client/renderer/entity/MobRenderer.h"
#include "client/renderer/entity/HumanoidMobRenderer.h"
#include "client/model/CowModel.h"
#include "client/model/HumanoidModel.h"
#include "client/model/SkeletonModel.h"
#include "world/entity/animal/Chicken.h"
#include "world/entity/animal/Cow.h"
#include "world/entity/animal/Pig.h"
#include "world/entity/animal/Sheep.h"
#include "world/entity/animal/Squid.h"
#include "world/entity/animal/Wolf.h"
#include "world/entity/item/EntityBoat.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/item/FallingTile.h"
#include "world/entity/item/EntityMinecart.h"
#include "world/entity/item/EntityPainting.h"
#include "world/entity/monster/Monster.h"
#include "world/entity/monster/Creeper.h"
#include "world/entity/monster/Spider.h"
#include "world/entity/monster/Zombie.h"
#include "world/entity/monster/Skeleton.h"
#include "world/entity/monster/PigZombie.h"
#include "world/entity/monster/Slime.h"
#include "world/entity/monster/Giant.h"
#include "world/entity/monster/Ghast.h"
#include "world/entity/projectile/EntityFireball.h"
#include "world/entity/projectile/EntityArrow.h"
#include "world/entity/projectile/EntitySnowball.h"
#include "world/entity/projectile/EntityThrownEgg.h"
#include "world/entity/projectile/EntityFish.h"
#include "world/entity/PrimedTNT.h"
#include "world/entity/EntityLightningBolt.h"
#include "world/entity/player/Player.h"
#include "OpenGL.h"


FallingTileRenderer &EntityRenderDispatcher::getFallingTileRenderer()
{
	static FallingTileRenderer renderer(instance);
	return renderer;
}


ItemRenderer &EntityRenderDispatcher::getItemRenderer()
{
	static ItemRenderer renderer(instance);
	return renderer;
}

ArrowRenderer &EntityRenderDispatcher::getArrowRenderer()
{
	static ArrowRenderer renderer(instance);
	return renderer;
}

FireballRenderer &EntityRenderDispatcher::getFireballRenderer()
{
	static FireballRenderer renderer(instance);
	return renderer;
}

EntityRenderer &EntityRenderDispatcher::getSnowballRenderer()
{
	static ThrownItemRenderer renderer(instance, 14);
	return renderer;
}

EntityRenderer &EntityRenderDispatcher::getThrownEggRenderer()
{
	static ThrownItemRenderer renderer(instance, 12);
	return renderer;
}

FishingHookRenderer &EntityRenderDispatcher::getFishingHookRenderer()
{
	static FishingHookRenderer renderer(instance);
	return renderer;
}

PaintingRenderer &EntityRenderDispatcher::getPaintingRenderer()
{
	static PaintingRenderer renderer(instance);
	return renderer;
}

LightningBoltRenderer &EntityRenderDispatcher::getLightningBoltRenderer()
{
	static LightningBoltRenderer renderer(instance);
	return renderer;
}

GenericEntityRenderer &EntityRenderDispatcher::getGenericEntityRenderer()
{
	static GenericEntityRenderer renderer(instance);
	return renderer;
}

MinecartRenderer &EntityRenderDispatcher::getMinecartRenderer()
{
	static MinecartRenderer renderer(instance);
	return renderer;
}

BoatRenderer &EntityRenderDispatcher::getBoatRenderer()
{
	static BoatRenderer renderer(instance);
	return renderer;
}

TNTPrimedRenderer &EntityRenderDispatcher::getTNTPrimedRenderer()
{
	static TNTPrimedRenderer renderer(instance);
	return renderer;
}

ChickenRenderer &EntityRenderDispatcher::getChickenRenderer()
{
	static ChickenRenderer renderer(instance);
	return renderer;
}

PigRenderer &EntityRenderDispatcher::getPigRenderer()
{
	static PigRenderer renderer(instance);
	return renderer;
}

SheepRenderer &EntityRenderDispatcher::getSheepRenderer()
{
	static SheepRenderer renderer(instance);
	return renderer;
}

SpiderRenderer &EntityRenderDispatcher::getSpiderRenderer()
{
	static SpiderRenderer renderer(instance);
	return renderer;
}

CreeperRenderer &EntityRenderDispatcher::getCreeperRenderer()
{
	static CreeperRenderer renderer(instance);
	return renderer;
}

SquidRenderer &EntityRenderDispatcher::getSquidRenderer()
{
	static SquidRenderer renderer(instance);
	return renderer;
}

SlimeRenderer &EntityRenderDispatcher::getSlimeRenderer()
{
	static SlimeRenderer renderer(instance);
	return renderer;
}

WolfRenderer &EntityRenderDispatcher::getWolfRenderer()
{
	static WolfRenderer renderer(instance);
	return renderer;
}

GiantRenderer &EntityRenderDispatcher::getGiantRenderer()
{
	static GiantRenderer renderer(instance);
	return renderer;
}

GhastRenderer &EntityRenderDispatcher::getGhastRenderer()
{
	static GhastRenderer renderer(instance);
	return renderer;
}

HumanoidMobRenderer &EntityRenderDispatcher::getMonsterRenderer()
{
	static HumanoidMobRenderer renderer(instance, false, false);
	return renderer;
}

MobRenderer &EntityRenderDispatcher::getCowRenderer()
	{
		static MobRenderer renderer(instance, std::make_shared<CowModel>(), 0.7f);
		return renderer;
	}

HumanoidMobRenderer &EntityRenderDispatcher::getZombieRenderer()
{
	static HumanoidMobRenderer renderer(instance, true, false);
	return renderer;
}

HumanoidMobRenderer &EntityRenderDispatcher::getSkeletonRenderer()
	{
		static HumanoidMobRenderer renderer(instance, true, true);
		static bool initialized = false;
		if (!initialized)
		{
			renderer.setModel(std::make_shared<SkeletonModel>());
			initialized = true;
		}
		return renderer;
	}

HumanoidMobRenderer &EntityRenderDispatcher::getPigZombieRenderer()
	{
		static HumanoidMobRenderer renderer(instance, true, true);
		return renderer;
	}


EntityRenderDispatcher EntityRenderDispatcher::instance;

double EntityRenderDispatcher::xOff = 0.0;
double EntityRenderDispatcher::yOff = 0.0;
double EntityRenderDispatcher::zOff = 0.0;

PlayerRenderer EntityRenderDispatcher::playerRenderer = PlayerRenderer(EntityRenderDispatcher::instance);

void EntityRenderDispatcher::prepare(std::shared_ptr<Level> level, Textures &textures, Font &font, std::shared_ptr<Player> player, Options &options, float a)
{
	this->level = level;
	this->textures = &textures;
	this->options = &options;
	this->player = player;
	this->font = &font;

	playerRotY = player->yRotO + (player->yRot - player->yRotO) * a;
	playerRotX = player->xRotO + (player->xRot - player->xRotO) * a;
	xPlayer = player->xOld + (player->x - player->xOld) * a;
	yPlayer = player->yOld + (player->y - player->yOld) * a;
	zPlayer = player->zOld + (player->z - player->zOld) * a;
}

void EntityRenderDispatcher::render(Entity &entity, float a)
{
	double x = entity.xOld + (entity.x - entity.xOld) * a;
	double y = entity.yOld + (entity.y - entity.yOld) * a;
	double z = entity.zOld + (entity.z - entity.zOld) * a;
	float rot = entity.yRotO + (entity.yRot - entity.yRotO) * a;

	float brightness = entity.getBrightness(a);
	glColor3f(brightness, brightness, brightness);
	render(entity, x - xOff, y - yOff, z - zOff, rot, a);
}

void EntityRenderDispatcher::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	if (dynamic_cast<FallingTile *>(&entity) != nullptr)
	{
		FallingTileRenderer &fallingTileRenderer = getFallingTileRenderer();
		fallingTileRenderer.render(entity, x, y, z, rot, a);
		fallingTileRenderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<EntityArrow *>(&entity) != nullptr)
	{
		ArrowRenderer &renderer = getArrowRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<EntityFireball *>(&entity) != nullptr)
	{
		FireballRenderer &renderer = getFireballRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}


	if (dynamic_cast<EntitySnowball *>(&entity) != nullptr)
	{
		EntityRenderer &renderer = getSnowballRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<EntityThrownEgg *>(&entity) != nullptr)
	{
		EntityRenderer &renderer = getThrownEggRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<EntityFish *>(&entity) != nullptr)
	{
		FishingHookRenderer &renderer = getFishingHookRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<EntityPainting *>(&entity) != nullptr)
	{
		PaintingRenderer &renderer = getPaintingRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<EntityLightningBolt *>(&entity) != nullptr)
	{
		LightningBoltRenderer &renderer = getLightningBoltRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<EntityItem *>(&entity) != nullptr)
	{
		ItemRenderer &itemRenderer = getItemRenderer();
		itemRenderer.render(entity, x, y, z, rot, a);
		itemRenderer.postRender(entity, x, y, z, rot, a);
		return;
	}
	if (dynamic_cast<EntityMinecart *>(&entity) != nullptr)
	{
		MinecartRenderer &minecartRenderer = getMinecartRenderer();
		minecartRenderer.render(entity, x, y, z, rot, a);
		minecartRenderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<EntityBoat *>(&entity) != nullptr)
	{
		BoatRenderer &boatRenderer = getBoatRenderer();
		boatRenderer.render(entity, x, y, z, rot, a);
		boatRenderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Ghast *>(&entity) != nullptr)
	{
		GhastRenderer &renderer = getGhastRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}


	if (dynamic_cast<Creeper *>(&entity) != nullptr)
	{
		CreeperRenderer &renderer = getCreeperRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Spider *>(&entity) != nullptr)
	{
		SpiderRenderer &renderer = getSpiderRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Slime *>(&entity) != nullptr)
	{
		SlimeRenderer &renderer = getSlimeRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}


	if (dynamic_cast<Giant *>(&entity) != nullptr)
	{
		GiantRenderer &renderer = getGiantRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}


	if (dynamic_cast<PigZombie *>(&entity) != nullptr)
	{
		HumanoidMobRenderer &renderer = getPigZombieRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Zombie *>(&entity) != nullptr)
	{
		HumanoidMobRenderer &renderer = getZombieRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Skeleton *>(&entity) != nullptr)
	{
		HumanoidMobRenderer &renderer = getSkeletonRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}
	
	if (dynamic_cast<Chicken *>(&entity) != nullptr)
	{
		ChickenRenderer &renderer = getChickenRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Sheep *>(&entity) != nullptr)
	{
		SheepRenderer &renderer = getSheepRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Wolf *>(&entity) != nullptr)
	{
		WolfRenderer &renderer = getWolfRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}


	if (dynamic_cast<Pig *>(&entity) != nullptr)
	{
		PigRenderer &renderer = getPigRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Cow *>(&entity) != nullptr)
	{
		MobRenderer &renderer = getCowRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Squid *>(&entity) != nullptr)
	{
		SquidRenderer &renderer = getSquidRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Monster *>(&entity) != nullptr)
	{
		HumanoidMobRenderer &renderer = getMonsterRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}
	
	if (dynamic_cast<PrimedTNT *>(&entity) != nullptr)
	{
		TNTPrimedRenderer &tntRenderer = getTNTPrimedRenderer();
		tntRenderer.render(entity, x, y, z, rot, a);
		tntRenderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Player *>(&entity) != nullptr)
	{
		playerRenderer.render(entity, x, y, z, rot, a);
		playerRenderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	if (dynamic_cast<Mob *>(&entity) != nullptr)
	{
		HumanoidMobRenderer &renderer = getMonsterRenderer();
		renderer.render(entity, x, y, z, rot, a);
		renderer.postRender(entity, x, y, z, rot, a);
		return;
	}

	GenericEntityRenderer &renderer = getGenericEntityRenderer();
	renderer.render(entity, x, y, z, rot, a);
	renderer.postRender(entity, x, y, z, rot, a);
}

void EntityRenderDispatcher::setLevel(std::shared_ptr<Level> level)
{
	this->level = level;
}

double EntityRenderDispatcher::distanceToSqr(double x, double y, double z)
{
	double dx = x - xPlayer;
	double dy = y - yPlayer;
	double dz = z - zPlayer;
	return dx * dx + dy * dy + dz * dz;
}

Font &EntityRenderDispatcher::getFont()
{
	return *font;
}

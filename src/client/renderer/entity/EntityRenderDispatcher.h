#pragma once

#include "world/level/Level.h"

#include "client/Options.h"
#include "client/renderer/Textures.h"
#include "client/gui/Font.h"

class HumanoidMobRenderer;
class PlayerRenderer;
class ArrowRenderer;
class FireballRenderer;
class EntityRenderer;
class ChickenRenderer;
class PigRenderer;
class SheepRenderer;
class SpiderRenderer;
class CreeperRenderer;
class SquidRenderer;
class SlimeRenderer;
class WolfRenderer;
class GiantRenderer;
class GhastRenderer;
class FallingTileRenderer;
class MinecartRenderer;
class BoatRenderer;
class ItemRenderer;
class TNTPrimedRenderer;
class MobRenderer;
class FishingHookRenderer;
class PaintingRenderer;
class LightningBoltRenderer;
class GenericEntityRenderer;

class EntityRenderDispatcher
{
public:
	static EntityRenderDispatcher instance;

	static PlayerRenderer playerRenderer;

private:
	static FallingTileRenderer &getFallingTileRenderer();
	static ItemRenderer &getItemRenderer();
	static MinecartRenderer &getMinecartRenderer();
	static BoatRenderer &getBoatRenderer();
	static TNTPrimedRenderer &getTNTPrimedRenderer();
	static ArrowRenderer &getArrowRenderer();
	static FireballRenderer &getFireballRenderer();
	static EntityRenderer &getSnowballRenderer();
	static EntityRenderer &getThrownEggRenderer();
	static FishingHookRenderer &getFishingHookRenderer();
	static PaintingRenderer &getPaintingRenderer();
	static LightningBoltRenderer &getLightningBoltRenderer();
	static GenericEntityRenderer &getGenericEntityRenderer();
	static ChickenRenderer &getChickenRenderer();
	static PigRenderer &getPigRenderer();
	static SheepRenderer &getSheepRenderer();
	static SpiderRenderer &getSpiderRenderer();
	static CreeperRenderer &getCreeperRenderer();
	static SquidRenderer &getSquidRenderer();
	static SlimeRenderer &getSlimeRenderer();
	static WolfRenderer &getWolfRenderer();
	static GiantRenderer &getGiantRenderer();
	static GhastRenderer &getGhastRenderer();
	static MobRenderer &getCowRenderer();
	static HumanoidMobRenderer &getZombieRenderer();
	static HumanoidMobRenderer &getSkeletonRenderer();
	static HumanoidMobRenderer &getPigZombieRenderer();
	static HumanoidMobRenderer &getMonsterRenderer();
	Font *font = nullptr;
public:
	static double xOff;
	static double yOff;
	static double zOff;

	Textures *textures = nullptr;

	std::shared_ptr<Level> level = nullptr;

	std::shared_ptr<Player> player = nullptr;
	float playerRotY = 0.0f, playerRotX = 0.0f;

	Options *options = nullptr;

	double xPlayer = 0.0, yPlayer = 0.0, zPlayer = 0.0;

	void prepare(std::shared_ptr<Level> level, Textures &textures, Font &font, std::shared_ptr<Player> player, Options &options, float a);
	void render(Entity &entity, float a);
	void render(Entity &entity, double x, double y, double z, float rot, float a);

	void setLevel(std::shared_ptr<Level> level);

	double distanceToSqr(double x, double y, double z);

	Font &getFont();
};

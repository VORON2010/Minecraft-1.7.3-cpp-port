#pragma once

#include <string>
#include <array>

#include "Facing.h"
#include "locale/Descriptive.h"


#include "world/level/tile/StepSound.h"
#include "world/level/material/Material.h"
#include "world/phys/AABB.h"

#include "java/Type.h"
#include "java/Random.h"
#include "java/String.h"
class SpongeTile;
class GlassTile;
class WebTile;
class ClothTile;
class BookshelfTile;
class SnowBlockTile;
class FenceTile;
class SoulSandTile;
class GlowStoneTile;
class StairTile;
class LadderTile;
class TrapDoorTile;
class DoorTile;
class NoteTile;
class JukeboxTile;
class DispenserTile;
class SignTile;
class OreTile;
class RedstoneOreTile;

class Level;
class LevelSource;
class Entity;
class Player;

class StoneTile;
class GrassTile;
class DirtTile;
class WoodTile;
class SandTile;
class GravelTile;
class TreeTile;
class LeafTile;
class FlowerTile;
class CropsTile;
class FarmlandTile;
class TallGrassTile;
class DeadBushTile;
class MushroomTile;
class ReedTile;
class CactusTile;
class TorchTile;
class FireTile;
class SaplingTile;
class PumpkinTile;
class SnowTile;
class IceTile;
class BedTile;
class WorkbenchTile;
class SlabTile;
class FurnaceTile;
class RedStoneDustTile;
class NotGateTile;
class LeverTile;
class ButtonTile;
class PressurePlateTile;
class RepeaterTile;
class LockedChestTile;
class ChestTile;

class TNTTile;
class MobSpawnerTile;
class CakeTile;
class RailTile;
class DetectorRailTile;
class PortalTile;
class TransparentTile;
class LiquidTile;
class PistonBaseTile;
class PistonExtensionTile;
class PistonMovingTile;
class Tile 
{
private:
	static const jstring TILE_DESCRIPTION_PREFIX;

public:
	enum Shape
	{
		SHAPE_INVISIBLE = -1,
		SHAPE_BLOCK,
		SHAPE_CROSS_TEXTURE,
		SHAPE_TORCH,
		SHAPE_FIRE,
		SHAPE_WATER,
		SHAPE_RED_DUST,
		SHAPE_ROWS,
		SHAPE_DOOR,
		SHAPE_LADDER,
		SHAPE_RAIL,
		SHAPE_STAIRS,
		SHAPE_FENCE,
		SHAPE_LEVER,
		SHAPE_CACTUS,
		SHAPE_BED = 14,
		SHAPE_REPEATER = 15,
		SHAPE_PISTON_BASE = 16,
		SHAPE_PISTON_EXTENSION = 17,
	};
	
	static std::array<Tile *, 256> tiles;

	static std::array<bool, 256> shouldTick;
	static std::array<bool, 256> solid;
	static std::array<bool, 256> isEntityTile;
	static std::array<int_t, 256> lightBlock;
	static std::array<bool, 256> translucent;
	static std::array<int_t, 256> lightEmission;
	static std::array<bool, 256> notifyRenderOnDataChange;


	
	static StepSound soundPowderFootstep;
	static StepSound soundWoodFootstep;
	static StepSound soundGravelFootstep;
	static StepSound soundGrassFootstep;
	static StepSound soundStoneFootstep;
	static StepSound soundMetalFootstep;
	static StepSoundStone soundGlassFootstep;
	static StepSound soundClothFootstep;
	static StepSoundSand soundSandFootstep;
	
	static StoneTile rock;
	static GrassTile grass;
	static DirtTile dirt;
	static WoodTile wood;

	static SandTile sand;
	static GravelTile gravel;

	static TreeTile treeTrunk;
	static LeafTile leaves;
	static TallGrassTile tallGrass;
	static DeadBushTile deadBush;
	static FlowerTile flower;
	static FlowerTile rose;
	static MushroomTile brownMushroom;
	static MushroomTile redMushroom;

	static Tile cobblestone;
	static Tile bedrock;
	static LiquidTile &water;
	static LiquidTile &calmWater;
	static LiquidTile &lava;
	static LiquidTile &calmLava;
	static Tile goldOre;
	static Tile ironOre;
	static OreTile coalOre;
	static OreTile lapisOre;
	static Tile &sandstone;
	static DispenserTile dispenser;
	static NoteTile noteBlock;
	static RailTile railPowered;
	static DetectorRailTile railDetector;
	static Tile mossyCobblestone;
	static Tile obsidian;
	static BedTile bed;
	static SlabTile slabDouble;
	static SlabTile slabSingle;
	static WorkbenchTile workBench;
	static CropsTile crops;
	static FarmlandTile farmland;
	static FurnaceTile furnace;
	static FurnaceTile furnaceLit;
	static RailTile rail;
	static OreTile diamondOre;
	static RedstoneOreTile redstoneOre;
	static RedstoneOreTile redstoneOreGlowing;
	static SnowTile snow;
	static IceTile ice;
	static CactusTile cactus;
	static Tile &clay;
	static ReedTile reed;
	static JukeboxTile jukebox;
	static PumpkinTile pumpkin;
	static TorchTile torch;
	static FireTile fire;
	static SaplingTile sapling;

	
	static SpongeTile sponge;
	static GlassTile glass;
	static Tile lapisBlock;
	static WebTile cobweb;
	static ClothTile wool;
	static Tile goldBlock;
	static Tile ironBlock;
	static Tile brick;
	static BookshelfTile bookshelf;
	static Tile diamondBlock;
	static SnowBlockTile snowBlock;
	static FenceTile fence;
	static Tile netherrack;
	static SoulSandTile soulSand;
	static GlowStoneTile glowstone;
	static PortalTile portal;
	static PumpkinTile jackOLantern;
	static TNTTile tnt;
	static MobSpawnerTile mobSpawner;
	static CakeTile cake;

	
	static DoorTile doorWood;
	static LadderTile ladder;
	static StairTile stairsWood;
	static StairTile stairsStone;
	static ChestTile chest;
	static LockedChestTile lockedChest;
	static TrapDoorTile trapdoor;
	static DoorTile doorIron;
	static SignTile signPost;
	static SignTile signWall;
	
	static RedStoneDustTile redstoneWire;
	static LeverTile lever;
	static PressurePlateTile pressurePlateStone;
	static PressurePlateTile pressurePlateWood;
	static NotGateTile torchRedstoneIdle;
	static NotGateTile torchRedstoneActive;
	static ButtonTile buttonStone;
	static RepeaterTile repeaterIdle;
	static RepeaterTile repeaterActive;
	static PistonBaseTile pistonBase;
	static PistonBaseTile pistonStickyBase;
	static PistonExtensionTile pistonExtension;
	static PistonMovingTile pistonMoving;
	static void initTiles();

public:
	int_t tex = 0;

	int_t id = 0;

protected:
	float destroySpeed = 0.0f;
	float explosionResistance = 0.0f;

public:
	float getDestroyTime() const { return destroySpeed; }

public:
	StepSound *soundType = nullptr;

	double xx0 = 0.0;
	double yy0 = 0.0;
	double zz0 = 0.0;
	
	double xx1 = 0.0;
	double yy1 = 0.0;
	double zz1 = 0.0;

	float gravity = 1.0f;

	const Material &material;

	float friction = 0.6f;

public:
	Tile(int_t id, const Material &material);
	Tile(int_t id, int_t tex, const Material &material);

	virtual ~Tile() {}

	Tile &setLightBlock(int_t lightBlock);
	Tile &setLightEmission(int_t lightEmission);
	Tile &setExplodeable(float resistance);
	Tile &setNotifyRenderOnDataChange();

public:
	virtual bool isCubeShaped();
	virtual Shape getRenderShape();

protected:
	Tile &setDestroyTime(float time);
	Tile &setSoundType(StepSound &sound);
	void setTicking(bool ticking);
	void updateCachedProperties();

public:
	void setShape(float x0, float y0, float z0, float x1, float y1, float z1);

	virtual float getBrightness(LevelSource &level, int_t x, int_t y, int_t z);

	static bool isFaceVisible(LevelSource &level, int_t x, int_t y, int_t z, Facing face);
	virtual bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face);

	virtual int_t getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face);
	virtual int_t getTexture(Facing face, int_t data);
	virtual int_t getTexture(Facing face);

	virtual AABB *getTileAABB(Level &level, int_t x, int_t y, int_t z);

	virtual void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList);
	virtual AABB *getAABB(Level &level, int_t x, int_t y, int_t z);

	virtual bool isSolidRender();

	virtual bool mayPick(int_t data, bool canPickLiquid);
	virtual bool mayPick();

	virtual void tick(Level &level, int_t x, int_t y, int_t z, Random &random);
	virtual void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random);
	virtual void destroy(Level &level, int_t x, int_t y, int_t z, int_t data);
	virtual void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile);
	virtual void addLights(Level &level, int_t x, int_t y, int_t z);
	
	virtual int_t getTickDelay();

	virtual void onPlace(Level &level, int_t x, int_t y, int_t z);
	virtual void onRemove(Level &level, int_t x, int_t y, int_t z);

	virtual int_t getResourceCount(Random &random);
	virtual int_t getResource(int_t data, Random &random);

	float getDestroyProgress(Player &player);

	void spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data);
	virtual void spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance);

	virtual int_t getSpawnResourcesAuxValue(int_t data);

	virtual HitResult clip(Level &level, int_t x, int_t y, int_t z, Vec3 &from, Vec3 &to);

private:
	virtual bool containsX(Vec3 *vec);
	virtual bool containsY(Vec3 *vec);
	virtual bool containsZ(Vec3 *vec);

public:
	virtual int_t getRenderLayer();

	virtual void stepOn(Level &level, int_t x, int_t y, int_t z, Entity &entity);
	virtual void setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face);
	virtual void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player);
	virtual void prepareRender(Level &level, int_t x, int_t y, int_t z);
	virtual void attack(Level &level, int_t x, int_t y, int_t z, Player &player);
	virtual bool use(Level &level, int_t x, int_t y, int_t z, Player &player);

	virtual void updateShape(LevelSource &level, int_t x, int_t y, int_t z);
	virtual int_t getColor(LevelSource &level, int_t x, int_t y, int_t z);

	virtual void entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity);

	virtual void updateDefaultShape();
	virtual bool mayPlace(Level &level, int_t x, int_t y, int_t z);

	virtual bool getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir);
	virtual bool isSignalSource() { return false; }
	virtual bool getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir);
	virtual int_t getItemColor(int_t data);

	jstring descriptionId;
	Tile &setDescriptionId(const jstring &id);

	virtual void harvestBlock(Level &level, Player &player, int_t x, int_t y, int_t z, int_t data);
	virtual void playerDestroy(Level &level, int_t x, int_t y, int_t z, int_t data);
	virtual void onBlockDestroyedByExplosion(Level &level, int_t x, int_t y, int_t z) {}
	virtual float getExplosionResistance(Entity *entity) { (void)entity; return explosionResistance / 5.0f; }
	virtual void playBlock(Level &level, int_t x, int_t y, int_t z, int_t type, int_t data);
	virtual int_t getMobilityFlag() const;
};

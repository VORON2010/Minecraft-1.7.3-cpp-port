#pragma once

#include "world/level/LevelSource.h"
#include "world/level/tile/Tile.h"

#include "java/Type.h"

class TileRenderer
{
private:
	LevelSource *level = nullptr;
	int_t fixedTexture = -1;

	bool xFlipTexture = false;
	bool noCulling = false;
	bool ambientOcclusion = false;
	bool fancyGrass = false;
	bool enableAO = false;
	float colorR0 = 1.0f, colorG0 = 1.0f, colorB0 = 1.0f;
	float colorR1 = 1.0f, colorG1 = 1.0f, colorB1 = 1.0f;
	float colorR2 = 1.0f, colorG2 = 1.0f, colorB2 = 1.0f;
	enum Flip
	{
		FLIP_NONE,
		FLIP_CW,
		FLIP_CCW,
		FLIP_180
	};

	int_t northFlip = FLIP_NONE;
	int_t southFlip = FLIP_NONE;
	int_t eastFlip = FLIP_NONE;
	int_t westFlip = FLIP_NONE;
	int_t upFlip = FLIP_NONE;
	int_t downFlip = FLIP_NONE;
	float colorR3 = 1.0f, colorG3 = 1.0f, colorB3 = 1.0f;

public:
	TileRenderer(LevelSource *levelSource, bool ambientOcclusion = false, bool fancyGrass = false);
	TileRenderer(bool ambientOcclusion = false, bool fancyGrass = false);

	void tesselateInWorld(Tile &tile, int_t x, int_t y, int_t z, int_t fixedTexture);
	void tesselateInWorldNoCulling(Tile &tile, int_t x, int_t y, int_t z);
	bool tesselateInWorld(Tile &tt, int_t x, int_t y, int_t z);

	bool tesselateBlockInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateBlockInWorld(Tile &tt, int_t x, int_t y, int_t z, float r, float g, float b);
	bool tesselateCrossTextureInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateCactusInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateLiquidInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateTorchInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateStairsInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateFenceInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateFireInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateLadderInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateRailInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateDoorInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateDustInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateRepeaterInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselateLeverInWorld(Tile &tt, int_t x, int_t y, int_t z);
	void tesselateTorch(Tile &tt, double x, double y, double z, double xxa, double zza);

	float getLiquidHeight(int_t x, int_t y, int_t z, const Material &material);

	void renderFaceUp(Tile &tt, double x, double y, double z, int_t tex);
	void renderFaceDown(Tile &tt, double x, double y, double z, int_t tex);
	void renderNorth(Tile &tt, double x, double y, double z, int_t tex);
	void renderSouth(Tile &tt, double x, double y, double z, int_t tex);
	void renderWest(Tile &tt, double x, double y, double z, int_t tex);
	void renderEast(Tile &tt, double x, double y, double z, int_t tex);

	void renderCube(Tile &tile, float alpha);
	bool tesselateBedInWorld(Tile &tt, int_t x, int_t y, int_t z);
	bool tesselatePistonBaseInWorld(Tile &tile, int_t x, int_t y, int_t z, bool forceExtended = false, int_t forceData = -1);
	void tesselatePistonBaseForceExtended(Tile &tile, int_t x, int_t y, int_t z, int_t forceData = -1);
	bool tesselatePistonExtensionInWorld(Tile &tile, int_t x, int_t y, int_t z, bool fullArm = true, int_t forceData = -1);
	void tesselatePistonArmNoCulling(Tile &tile, int_t x, int_t y, int_t z, bool fullArm, int_t forceData = -1);
	void renderPistonArmUpDown(float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels);
	void renderPistonArmNorthSouth(float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels);
	void renderPistonArmEastWest(float x0, float x1, float y0, float y1, float z0, float z1, float br, float armLengthPixels);
	void renderAllFacesBlockInWorld(Tile &tile, int_t x, int_t y, int_t z);
	void renderTile(Tile &tile, int_t data);
	void renderGuiTile(Tile &tile, int_t data);
	static bool canRender(int_t renderShape);
};

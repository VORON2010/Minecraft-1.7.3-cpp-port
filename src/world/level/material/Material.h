#pragma once

#include "java/Type.h"

class MapColor;
class DecorationMaterial;
class GasMaterial;
class LiquidMaterial;

class Material
{
public:
	static GasMaterial air;
	static Material dirt;
	static Material grassMaterial;
	static Material wood;
	static Material stone;
	static Material sand;
	static Material clay;
	static LiquidMaterial water;
	static LiquidMaterial lava;
	static Material sponge;
	static Material cloth;
	static Material glass;
	static Material iron;
	static Material builtSnow;
	static Material tnt;
	static Material web;
	static GasMaterial fire;
	static Material piston;
	static Material leaves;
	static DecorationMaterial portal;
	static Material cakeMaterial;

	static Material &plants();
	static Material &cactus();
	static Material &pumpkin();
	static Material &snow();
	static Material &ice();
	static Material &circuits();

private:
	bool flammableFlag = false;
	bool harvestableFlag = true;
	int_t mobilityFlag = 0;
	bool translucentFlag = false;
	bool groundCoverFlag = false;

public:
	const MapColor *mapColor = nullptr;

	virtual ~Material() {}

	virtual bool isLiquid() const;
	virtual bool letsWaterThrough() const;
	virtual bool isSolid() const;
	virtual bool blocksLight() const;
	virtual bool blocksMotion() const;
	virtual bool isHarvestable() const;
	int_t getMobilityFlag() const;
	bool isSolidBlocking() const;
	bool isGroundCover() const;

private:
	Material &flammable();
	Material &noHarvest();
	Material &setTranslucent();
protected:
	Material &setGroundCover();
	Material &setNoPushMobility();
	Material &setImmovableMobility();

public:
	virtual bool isFlammable() const;

	Material &setMapColor(MapColor &color);

	friend class Tile;
};

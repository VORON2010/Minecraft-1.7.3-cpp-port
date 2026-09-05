#include "world/level/material/Material.h"

#include "world/level/material/MapColor.h"
#include "world/level/material/GasMaterial.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/material/DecorationMaterial.h"

GasMaterial Material::air;
static struct AirMaterialInit { AirMaterialInit() { Material::air.setMapColor(MapColor::airColor); } } airMaterialInit;

Material Material::dirt = Material().setMapColor(MapColor::dirtColor);
Material Material::grassMaterial = Material().setMapColor(MapColor::grassColor);
Material Material::wood = Material().flammable().setMapColor(MapColor::woodColor);
Material Material::stone = Material().noHarvest().setMapColor(MapColor::stoneColor);
Material Material::sand = Material().setMapColor(MapColor::sandColor);
Material Material::clay = Material().setMapColor(MapColor::clayColor);
LiquidMaterial Material::water = LiquidMaterial(MapColor::waterColor);
LiquidMaterial Material::lava = LiquidMaterial(MapColor::tntColor);

Material Material::sponge = Material().setMapColor(MapColor::clothColor);
Material Material::cloth = Material().flammable().setMapColor(MapColor::clothColor);
Material Material::glass = Material().setTranslucent().setMapColor(MapColor::airColor);
Material Material::iron = Material().noHarvest().setMapColor(MapColor::ironColor);
Material Material::builtSnow = Material().noHarvest().setMapColor(MapColor::snowColor);
Material Material::tnt = Material().flammable().setTranslucent().setMapColor(MapColor::tntColor);
Material Material::web = Material().noHarvest().setNoPushMobility().setMapColor(MapColor::clothColor);
GasMaterial Material::fire = [] {
	GasMaterial material;
	material.setNoPushMobility().setMapColor(MapColor::airColor);
	return material;
}();
Material Material::piston = Material().setImmovableMobility().setMapColor(MapColor::stoneColor);
Material Material::leaves = Material().flammable().setTranslucent().setNoPushMobility().setMapColor(MapColor::foliageColor);
DecorationMaterial Material::portal = [] {
	DecorationMaterial material(false, false, false, true);
	material.setImmovableMobility().setMapColor(MapColor::airColor);
	return material;
}();
Material Material::cakeMaterial = Material().setNoPushMobility().setMapColor(MapColor::airColor);
Material &Material::plants()
{
	static DecorationMaterial material(false, false, false, true);
	material.setNoPushMobility().setMapColor(MapColor::foliageColor);
	return material;
}

Material &Material::cactus()
{
	static Material material = Material().setTranslucent().setNoPushMobility().setMapColor(MapColor::foliageColor);
	return material;
}

Material &Material::pumpkin()
{
	static DecorationMaterial material(true, true, true, false);
	material.setNoPushMobility().setMapColor(MapColor::foliageColor);
	return material;
}

Material &Material::snow()
{
	static DecorationMaterial material(false, false, false, true);
	static bool initialized = (material.noHarvest().setGroundCover().setTranslucent(), true);
	material.setNoPushMobility().setMapColor(MapColor::snowColor);
	(void)initialized;
	return material;
}

Material &Material::ice()
{
	static Material material = Material().setTranslucent().setMapColor(MapColor::iceColor);
	return material;
}

Material &Material::circuits()
{
	static DecorationMaterial material(false, false, false, true);
	material.setNoPushMobility().setMapColor(MapColor::airColor);
	return material;
}

bool Material::isLiquid() const
{
	return false;
}

bool Material::letsWaterThrough() const
{
	return (!isLiquid() && !isSolid());
}

bool Material::isSolid() const
{
	return true;
}

bool Material::blocksLight() const
{
	return true;
}

bool Material::blocksMotion() const
{
	return true;
}

bool Material::isHarvestable() const
{
	return harvestableFlag;
}

Material &Material::flammable()
{
	flammableFlag = true;
	return *this;
}

Material &Material::noHarvest()
{
	harvestableFlag = false;
	return *this;
}

int_t Material::getMobilityFlag() const
{
	return mobilityFlag;
}

Material &Material::setNoPushMobility()
{
	mobilityFlag = 1;
	return *this;
}

Material &Material::setImmovableMobility()
{
	mobilityFlag = 2;
	return *this;
}

bool Material::isFlammable() const
{
	return flammableFlag;
}

Material &Material::setMapColor(MapColor &color)
{
	mapColor = &color;
	return *this;
}

Material &Material::setTranslucent()
{
	translucentFlag = true;
	return *this;
}

Material &Material::setGroundCover()
{
	groundCoverFlag = true;
	return *this;
}

bool Material::isSolidBlocking() const
{
	return !translucentFlag && blocksMotion();
}

bool Material::isGroundCover() const
{
	return groundCoverFlag;
}

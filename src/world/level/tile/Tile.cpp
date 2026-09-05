#include "world/level/tile/Tile.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/SlabTile.h"
#include "world/level/tile/TNTTile.h"

#include <stdexcept>

#include "world/level/Level.h"
#include "world/level/LevelSource.h"
#include "world/entity/Entity.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/stats/StatList.h"


std::array<Tile *, 256> Tile::tiles;

std::array<bool, 256> Tile::shouldTick = {};
std::array<bool, 256> Tile::solid = {};
std::array<bool, 256> Tile::isEntityTile = {};
std::array<int_t, 256> Tile::lightBlock = {};
std::array<bool, 256> Tile::translucent = {};
std::array<int_t, 256> Tile::lightEmission = {};
std::array<bool, 256> Tile::notifyRenderOnDataChange = {};


StepSound Tile::soundPowderFootstep(u"stone", 1.0f, 1.0f);
StepSound Tile::soundWoodFootstep(u"wood", 1.0f, 1.0f);
StepSound Tile::soundGravelFootstep(u"gravel", 1.0f, 1.0f);
StepSound Tile::soundGrassFootstep(u"grass", 1.0f, 1.0f);
StepSound Tile::soundStoneFootstep(u"stone", 1.0f, 1.0f);
StepSound Tile::soundMetalFootstep(u"stone", 1.0f, 1.5f);
StepSoundStone Tile::soundGlassFootstep(u"stone", 1.0f, 1.0f);
StepSound Tile::soundClothFootstep(u"cloth", 1.0f, 1.0f);
StepSoundSand Tile::soundSandFootstep(u"sand", 1.0f, 1.0f);


#include "world/level/tile/StoneTile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/WoodTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/SandStoneTile.h"
#include "world/level/tile/GravelTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/LeafTile.h"
#include "world/level/tile/FlowerTile.h"
#include "world/level/tile/CropsTile.h"
#include "world/level/tile/FarmlandTile.h"
#include "world/level/tile/TallGrassTile.h"
#include "world/level/tile/DeadBushTile.h"
#include "world/level/tile/MushroomTile.h"
#include "world/level/tile/ReedTile.h"
#include "world/level/tile/CactusTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/IceTile.h"
#include "world/level/tile/BedTile.h"
#include "world/level/tile/WorkbenchTile.h"
#include "world/level/tile/TransparentTile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/OreTile.h"
#include "world/level/tile/RedstoneOreTile.h"
#include "world/level/tile/TorchTile.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/SaplingTile.h"
#include "world/level/tile/SpongeTile.h"
#include "world/level/tile/GlassTile.h"
#include "world/level/tile/WebTile.h"
#include "world/level/tile/ClothTile.h"
#include "world/level/tile/ClayTile.h"
#include "world/level/tile/BookshelfTile.h"
#include "world/level/tile/SnowBlockTile.h"
#include "world/level/tile/FenceTile.h"
#include "world/level/tile/SoulSandTile.h"
#include "world/level/tile/GlowStoneTile.h"
#include "world/level/tile/PortalTile.h"
#include "world/level/tile/StairTile.h"
#include "world/level/tile/LadderTile.h"
#include "world/level/tile/TrapDoorTile.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/NoteTile.h"
#include "world/level/tile/JukeboxTile.h"
#include "world/level/tile/DispenserTile.h"
#include "world/level/tile/SignTile.h"
#include "world/level/tile/RedStoneDustTile.h"
#include "world/level/tile/NotGateTile.h"
#include "world/level/tile/LeverTile.h"
#include "world/level/tile/ButtonTile.h"
#include "world/level/tile/PressurePlateTile.h"
#include "world/level/tile/RepeaterTile.h"
#include "world/level/tile/RailTile.h"
#include "world/level/tile/DetectorRailTile.h"
#include "world/level/tile/ChestTile.h"
#include "world/level/tile/LockedChestTile.h"
#include "world/level/tile/PistonBaseTile.h"
#include "world/level/tile/PistonExtensionTile.h"
#include "world/level/tile/PistonMovingTile.h"
#include "world/level/tile/MobSpawnerTile.h"
#include "world/level/tile/CakeTile.h"
StoneTile Tile::rock = StoneTile(1, 1);
GrassTile Tile::grass = GrassTile(2);
DirtTile Tile::dirt = DirtTile(3, 2);
WoodTile Tile::wood = WoodTile(5, 4);

SandTile Tile::sand = SandTile(12, 18);
GravelTile Tile::gravel = GravelTile(13, 19);

TreeTile Tile::treeTrunk = TreeTile(17);
LeafTile Tile::leaves = LeafTile(18, 52);
TallGrassTile Tile::tallGrass = TallGrassTile(31, 39);
DeadBushTile Tile::deadBush = DeadBushTile(32, 55);
FlowerTile Tile::flower = FlowerTile(37, 13);
FlowerTile Tile::rose = FlowerTile(38, 12);
MushroomTile Tile::brownMushroom = MushroomTile(39, 29);
MushroomTile Tile::redMushroom = MushroomTile(40, 28);

Tile Tile::cobblestone = Tile(4, 16, Material::stone);
Tile Tile::bedrock = Tile(7, 17, Material::stone);
static LiquidTileDynamic waterTile(8, 205, Material::water);
LiquidTile &Tile::water = waterTile;
static LiquidTileStatic calmWaterTile(9, 205, Material::water);
LiquidTile &Tile::calmWater = calmWaterTile;
static LiquidTileDynamic lavaTile(10, 237, Material::lava);
LiquidTile &Tile::lava = lavaTile;
static LiquidTileStatic calmLavaTile(11, 237, Material::lava);
LiquidTile &Tile::calmLava = calmLavaTile;
Tile Tile::goldOre = Tile(14, 32, Material::stone);
Tile Tile::ironOre = Tile(15, 33, Material::stone);
OreTile Tile::coalOre = OreTile(16, 34);
OreTile Tile::lapisOre = OreTile(21, 160);
static SandStoneTile sandstoneTile(24, 192);
Tile &Tile::sandstone = sandstoneTile;
DispenserTile Tile::dispenser = DispenserTile(23, 45, Material::stone);
NoteTile Tile::noteBlock = NoteTile(25, 74, Material::wood);
BedTile Tile::bed = BedTile(26, 134);
RailTile Tile::railPowered = RailTile(27, 179, true);
DetectorRailTile Tile::railDetector = DetectorRailTile(28, 195);
SlabTile Tile::slabDouble = SlabTile(43, true);
SlabTile Tile::slabSingle = SlabTile(44, false);
Tile Tile::mossyCobblestone = Tile(48, 36, Material::stone);
Tile Tile::obsidian = Tile(49, 37, Material::stone);
OreTile Tile::diamondOre = OreTile(56, 50);
WorkbenchTile Tile::workBench = WorkbenchTile(58);
CropsTile Tile::crops = CropsTile(59, 88);
FarmlandTile Tile::farmland = FarmlandTile(60);
FurnaceTile Tile::furnace = FurnaceTile(61, false);
FurnaceTile Tile::furnaceLit = FurnaceTile(62, true);
RailTile Tile::rail = RailTile(66, 128, false);
RedstoneOreTile Tile::redstoneOre = RedstoneOreTile(73, 51, false);
RedstoneOreTile Tile::redstoneOreGlowing = RedstoneOreTile(74, 51, true);
SnowTile Tile::snow = SnowTile(78, 66);
IceTile Tile::ice = IceTile(79, 67);
CactusTile Tile::cactus = CactusTile(81, 70);
static ClayTile clayTile(82, 72);
Tile &Tile::clay = clayTile;
ReedTile Tile::reed = ReedTile(83, 73);
JukeboxTile Tile::jukebox = JukeboxTile(84, 74, Material::wood);
PumpkinTile Tile::pumpkin = PumpkinTile(86, 102, false);
TorchTile Tile::torch = TorchTile(50, 80);
FireTile Tile::fire = FireTile(51, 31);
SaplingTile Tile::sapling = SaplingTile(6, 15);

SpongeTile Tile::sponge = SpongeTile(19, 48, Material::sponge);
GlassTile Tile::glass = GlassTile(20, 49, Material::glass, false);
Tile Tile::lapisBlock = Tile(22, 144, Material::stone);
WebTile Tile::cobweb = WebTile(30, 11, Material::web);
ClothTile Tile::wool = ClothTile(35, 64, Material::cloth);
Tile Tile::goldBlock = Tile(41, 23, Material::iron);
Tile Tile::ironBlock = Tile(42, 22, Material::iron);
Tile Tile::brick = Tile(45, 7, Material::stone);
BookshelfTile Tile::bookshelf = BookshelfTile(47, 35, Material::wood);
Tile Tile::diamondBlock = Tile(57, 24, Material::iron);
SnowBlockTile Tile::snowBlock = SnowBlockTile(80, 66, Material::builtSnow);
FenceTile Tile::fence = FenceTile(85, 4, Material::wood);
Tile Tile::netherrack = Tile(87, 103, Material::stone);
SoulSandTile Tile::soulSand = SoulSandTile(88, 104, Material::sand);
GlowStoneTile Tile::glowstone = GlowStoneTile(89, 105, Material::stone);
PortalTile Tile::portal = PortalTile(90, 14);
PumpkinTile Tile::jackOLantern = PumpkinTile(91, 102, true);
TNTTile Tile::tnt = TNTTile(46, 8);
MobSpawnerTile Tile::mobSpawner = MobSpawnerTile(52, 65);
CakeTile Tile::cake = CakeTile(92, 121);
RepeaterTile Tile::repeaterIdle = RepeaterTile(93, false);
RepeaterTile Tile::repeaterActive = RepeaterTile(94, true);
LockedChestTile Tile::lockedChest = LockedChestTile(95);
PistonBaseTile Tile::pistonStickyBase = PistonBaseTile(29, 106, true);
PistonBaseTile Tile::pistonBase = PistonBaseTile(33, 107, false);
PistonExtensionTile Tile::pistonExtension = PistonExtensionTile(34, 107);
PistonMovingTile Tile::pistonMoving = PistonMovingTile(36);

DoorTile Tile::doorWood = DoorTile(64, 97, Material::wood, false);
LadderTile Tile::ladder = LadderTile(65, 83);
StairTile Tile::stairsWood = StairTile(53, Tile::wood);
StairTile Tile::stairsStone = StairTile(67, Tile::cobblestone);
ChestTile Tile::chest = ChestTile(54);
TrapDoorTile Tile::trapdoor = TrapDoorTile(96, 84, Material::wood);
DoorTile Tile::doorIron = DoorTile(71, 98, Material::iron, true);
SignTile Tile::signPost = SignTile(63, true);
SignTile Tile::signWall = SignTile(68, false);

RedStoneDustTile Tile::redstoneWire = RedStoneDustTile(55, 164);
LeverTile Tile::lever = LeverTile(69, 96);
PressurePlateTile Tile::pressurePlateStone = PressurePlateTile(70, 1, PressurePlateTile::Sensitivity::MOBS, Material::stone);
PressurePlateTile Tile::pressurePlateWood = PressurePlateTile(72, 4, PressurePlateTile::Sensitivity::EVERYTHING, Material::wood);
NotGateTile Tile::torchRedstoneIdle = NotGateTile(75, 115, false);
NotGateTile Tile::torchRedstoneActive = NotGateTile(76, 99, true);
ButtonTile Tile::buttonStone = ButtonTile(77, 1);
void Tile::initTiles()
{
	rock.setDestroyTime(1.5f).setExplodeable(10.0f).setSoundType(soundStoneFootstep);
	grass.setDestroyTime(0.6f).setSoundType(soundGrassFootstep);
	dirt.setDestroyTime(0.5f).setSoundType(soundGravelFootstep);
	wood.setDestroyTime(2.0f).setExplodeable(5.0f).setSoundType(soundWoodFootstep);

	sand.setDestroyTime(0.5f).setSoundType(soundSandFootstep);
	gravel.setDestroyTime(0.6f).setSoundType(soundGravelFootstep);

	treeTrunk.setDestroyTime(2.0f).setSoundType(soundWoodFootstep);
	leaves.setDestroyTime(0.2f).setLightBlock(1).setSoundType(soundGrassFootstep);
	tallGrass.setDestroyTime(0.0f).setSoundType(soundGrassFootstep);
	deadBush.setDestroyTime(0.0f).setSoundType(soundGrassFootstep);
	flower.setDestroyTime(0.0f).setSoundType(soundGrassFootstep);
	rose.setDestroyTime(0.0f).setSoundType(soundGrassFootstep);
	brownMushroom.setDestroyTime(0.0f).setLightEmission(1).setSoundType(soundGrassFootstep);
	redMushroom.setDestroyTime(0.0f).setSoundType(soundGrassFootstep);
	bedrock.setDestroyTime(-1.0f).setExplodeable(6000000.0f).setSoundType(soundStoneFootstep);
	
	water.setDestroyTime(100.0f);
	calmWater.setDestroyTime(100.0f);
	lava.setDestroyTime(0.0f);
	calmLava.setDestroyTime(100.0f);
	reed.setDestroyTime(0.0f).setSoundType(soundGrassFootstep);
	pumpkin.setDestroyTime(1.0f).setSoundType(soundWoodFootstep);
	cactus.setDestroyTime(0.4f).setSoundType(soundClothFootstep);

	cobblestone.setDestroyTime(2.0f).setExplodeable(10.0f).setSoundType(soundStoneFootstep);
	sandstone.setDestroyTime(0.8f).setSoundType(soundStoneFootstep);
	noteBlock.setDestroyTime(0.8f);
	bed.setDestroyTime(0.2f).setSoundType(soundStoneFootstep);
	slabDouble.setDestroyTime(2.0f).setExplodeable(10.0f).setSoundType(soundStoneFootstep);
	slabSingle.setDestroyTime(2.0f).setExplodeable(10.0f).setSoundType(soundStoneFootstep);
	mossyCobblestone.setDestroyTime(2.0f).setExplodeable(10.0f).setSoundType(soundStoneFootstep);
	dispenser.setDestroyTime(3.5f).setSoundType(soundStoneFootstep);
	obsidian.setDestroyTime(10.0f).setExplodeable(2000.0f).setSoundType(soundStoneFootstep);
	workBench.setDestroyTime(2.5f).setSoundType(soundWoodFootstep);
	crops.setDestroyTime(0.0f).setSoundType(soundGrassFootstep);
	farmland.setDestroyTime(0.6f).setSoundType(soundGravelFootstep);
	furnace.setDestroyTime(3.5f).setSoundType(soundStoneFootstep);
	furnaceLit.setDestroyTime(3.5f).setSoundType(soundStoneFootstep).setLightEmission(13);

	goldOre.setDestroyTime(3.0f).setExplodeable(5.0f).setSoundType(soundStoneFootstep);
	ironOre.setDestroyTime(3.0f).setExplodeable(5.0f).setSoundType(soundStoneFootstep);
	coalOre.setDestroyTime(3.0f).setExplodeable(5.0f).setSoundType(soundStoneFootstep);
	lapisOre.setDestroyTime(3.0f).setExplodeable(5.0f).setSoundType(soundStoneFootstep);
	diamondOre.setDestroyTime(3.0f).setExplodeable(5.0f).setSoundType(soundStoneFootstep);
	jukebox.setDestroyTime(2.0f).setExplodeable(10.0f).setSoundType(soundStoneFootstep);
	redstoneOre.setDestroyTime(3.0f).setExplodeable(5.0f).setSoundType(soundStoneFootstep);
	redstoneOreGlowing.setDestroyTime(3.0f).setExplodeable(5.0f).setSoundType(soundStoneFootstep);
	snow.setDestroyTime(0.1f).setSoundType(soundClothFootstep);
	ice.setDestroyTime(0.5f).setLightBlock(3).setSoundType(soundGlassFootstep);
	torch.setDestroyTime(0.0f).setLightEmission(14).setSoundType(soundWoodFootstep);
	fire.setDestroyTime(0.0f).setLightEmission(15).setSoundType(soundWoodFootstep);
	sapling.setDestroyTime(0.0f).setSoundType(soundGrassFootstep);
	clay.setDestroyTime(0.6f).setSoundType(soundGravelFootstep);

	sponge.setDestroyTime(0.6f).setSoundType(soundGrassFootstep);
	glass.setDestroyTime(0.3f).setSoundType(soundGlassFootstep);
	lapisBlock.setDestroyTime(3.0f).setExplodeable(5.0f).setSoundType(soundStoneFootstep);
	cobweb.setDestroyTime(4.0f).setLightBlock(1).setSoundType(soundPowderFootstep);
	wool.setDestroyTime(0.8f).setSoundType(soundClothFootstep);
	goldBlock.setDestroyTime(3.0f).setExplodeable(10.0f).setSoundType(soundMetalFootstep);
	ironBlock.setDestroyTime(5.0f).setExplodeable(10.0f).setSoundType(soundMetalFootstep);
	brick.setDestroyTime(2.0f).setExplodeable(10.0f).setSoundType(soundStoneFootstep);
	bookshelf.setDestroyTime(1.5f).setSoundType(soundWoodFootstep);
	diamondBlock.setDestroyTime(5.0f).setExplodeable(10.0f).setSoundType(soundMetalFootstep);
	snowBlock.setDestroyTime(0.2f).setSoundType(soundClothFootstep);
	fence.setDestroyTime(2.0f).setExplodeable(5.0f).setSoundType(soundWoodFootstep);
	netherrack.setDestroyTime(0.4f).setSoundType(soundStoneFootstep);
	soulSand.setDestroyTime(0.5f).setSoundType(soundSandFootstep);
	glowstone.setDestroyTime(0.3f).setSoundType(soundGlassFootstep).setLightEmission(15);
	portal.setDestroyTime(-1.0f).setLightEmission(11).setSoundType(soundGlassFootstep);
	jackOLantern.setDestroyTime(1.0f).setSoundType(soundWoodFootstep).setLightEmission(15);
	tnt.setDestroyTime(0.0f).setSoundType(soundGrassFootstep);
	mobSpawner.setDestroyTime(5.0f).setSoundType(soundMetalFootstep);
	cake.setDestroyTime(0.5f).setSoundType(soundClothFootstep);
	repeaterIdle.setDestroyTime(0.0f).setSoundType(soundWoodFootstep);
	repeaterActive.setDestroyTime(0.0f).setLightEmission(9).setSoundType(soundWoodFootstep);
	pistonStickyBase.setDestroyTime(0.5f).setSoundType(soundStoneFootstep);
	pistonBase.setDestroyTime(0.5f).setSoundType(soundStoneFootstep);
	pistonExtension.setDestroyTime(0.5f).setSoundType(soundStoneFootstep);
	pistonMoving.setDestroyTime(-1.0f);

	doorWood.setDestroyTime(3.0f).setSoundType(soundWoodFootstep);
	ladder.setDestroyTime(0.4f).setSoundType(soundWoodFootstep);
	stairsWood.setDestroyTime(2.0f).setExplodeable(5.0f).setLightBlock(255).setSoundType(soundWoodFootstep);
	stairsStone.setDestroyTime(2.0f).setExplodeable(10.0f).setLightBlock(255).setSoundType(soundStoneFootstep);
	chest.setDestroyTime(2.5f).setSoundType(soundWoodFootstep);
	lockedChest.setDestroyTime(0.0f).setLightEmission(15).setSoundType(soundWoodFootstep);
	trapdoor.setDestroyTime(3.0f).setSoundType(soundWoodFootstep);
	doorIron.setDestroyTime(5.0f).setSoundType(soundMetalFootstep);
	signPost.setDestroyTime(1.0f).setSoundType(soundWoodFootstep);
	signWall.setDestroyTime(1.0f).setSoundType(soundWoodFootstep);
	redstoneWire.setDestroyTime(0.0f).setSoundType(soundPowderFootstep);
	lever.setDestroyTime(0.5f).setSoundType(soundWoodFootstep);
	pressurePlateStone.setDestroyTime(0.5f).setSoundType(soundStoneFootstep);
	pressurePlateWood.setDestroyTime(0.5f).setSoundType(soundWoodFootstep);
	torchRedstoneIdle.setDestroyTime(0.0f).setLightEmission(0).setSoundType(soundWoodFootstep);
	torchRedstoneActive.setDestroyTime(0.0f).setLightEmission(7).setSoundType(soundWoodFootstep);
	buttonStone.setDestroyTime(0.5f).setSoundType(soundStoneFootstep);

	railPowered.setDestroyTime(0.7f).setSoundType(soundMetalFootstep);
	railDetector.setDestroyTime(0.7f).setSoundType(soundMetalFootstep);
	rail.setDestroyTime(0.7f).setSoundType(soundMetalFootstep);

	Tile::lightBlock[8] = 3;
	Tile::lightBlock[9] = 3;
	Tile::lightBlock[10] = 255;
	Tile::lightBlock[11] = 255;

	Tile::lightEmission[10] = 15;
	Tile::lightEmission[11] = 15;

	
	rock.setDescriptionId(u"tile.stone");
	grass.setDescriptionId(u"tile.grass");
	dirt.setDescriptionId(u"tile.dirt");
	cobblestone.setDescriptionId(u"tile.stonebrick");
	wood.setDescriptionId(u"tile.wood");
	sapling.setDescriptionId(u"tile.sapling");
	bedrock.setDescriptionId(u"tile.bedrock");
	dispenser.setDescriptionId(u"tile.dispenser");
	sand.setDescriptionId(u"tile.sand");
	gravel.setDescriptionId(u"tile.gravel");
	goldOre.setDescriptionId(u"tile.oreGold");
	ironOre.setDescriptionId(u"tile.oreIron");
	coalOre.setDescriptionId(u"tile.oreCoal");
	treeTrunk.setDescriptionId(u"tile.log");
	leaves.setDescriptionId(u"tile.leaves");
	lapisOre.setDescriptionId(u"tile.oreLapis");
	noteBlock.setDescriptionId(u"tile.musicBlock");
	bed.setDescriptionId(u"tile.bed");
	railPowered.setDescriptionId(u"tile.goldenRail");
	railDetector.setDescriptionId(u"tile.detectorRail");
	rail.setDescriptionId(u"tile.rail");
	sandstone.setDescriptionId(u"tile.sandStone");
	tallGrass.setDescriptionId(u"tile.grass");
	deadBush.setDescriptionId(u"tile.deadbush");
	flower.setDescriptionId(u"tile.flower");
	rose.setDescriptionId(u"tile.rose");
	brownMushroom.setDescriptionId(u"tile.mushroom");
	redMushroom.setDescriptionId(u"tile.mushroom");
	slabDouble.setDescriptionId(u"tile.stoneSlab.stone");
	slabSingle.setDescriptionId(u"tile.stoneSlab.stone");
	mossyCobblestone.setDescriptionId(u"tile.stoneMoss");
	obsidian.setDescriptionId(u"tile.obsidian");
	diamondOre.setDescriptionId(u"tile.oreDiamond");
	workBench.setDescriptionId(u"tile.workbench");
	crops.setDescriptionId(u"tile.crops");
	farmland.setDescriptionId(u"tile.farmland");
	furnace.setDescriptionId(u"tile.furnace");
	furnaceLit.setDescriptionId(u"tile.furnace");
	redstoneOre.setDescriptionId(u"tile.oreRedstone");
	redstoneOreGlowing.setDescriptionId(u"tile.oreRedstone");
	snow.setDescriptionId(u"tile.snow");
	ice.setDescriptionId(u"tile.ice");
	cactus.setDescriptionId(u"tile.cactus");
	clay.setDescriptionId(u"tile.clay");
	reed.setDescriptionId(u"tile.reeds");
	pumpkin.setDescriptionId(u"tile.pumpkin");
	torch.setDescriptionId(u"tile.torch");
	fire.setDescriptionId(u"tile.fire");
	jukebox.setDescriptionId(u"tile.jukebox");
	tnt.setDescriptionId(u"tile.tnt");

	sponge.setDescriptionId(u"tile.sponge");
	glass.setDescriptionId(u"tile.glass");
	lapisBlock.setDescriptionId(u"tile.blockLapis");
	cobweb.setDescriptionId(u"tile.web");
	wool.setDescriptionId(u"tile.cloth");
	goldBlock.setDescriptionId(u"tile.blockGold");
	ironBlock.setDescriptionId(u"tile.blockIron");
	brick.setDescriptionId(u"tile.brick");
	bookshelf.setDescriptionId(u"tile.bookshelf");
	diamondBlock.setDescriptionId(u"tile.blockDiamond");
	snowBlock.setDescriptionId(u"tile.snow");
	fence.setDescriptionId(u"tile.fence");
	doorWood.setDescriptionId(u"tile.doorWood");
	ladder.setDescriptionId(u"tile.ladder");
	stairsWood.setDescriptionId(u"tile.stairsWood");
	stairsStone.setDescriptionId(u"tile.stairsStone");
	chest.setDescriptionId(u"tile.chest");
	lockedChest.setDescriptionId(u"tile.lockedchest");
	trapdoor.setDescriptionId(u"tile.trapdoor");
	doorIron.setDescriptionId(u"tile.doorIron");
	netherrack.setDescriptionId(u"tile.hellrock");
	soulSand.setDescriptionId(u"tile.hellsand");
	glowstone.setDescriptionId(u"tile.lightgem");
	portal.setDescriptionId(u"tile.portal");
	jackOLantern.setDescriptionId(u"tile.litpumpkin");
	mobSpawner.setDescriptionId(u"tile.mobSpawner");
	cake.setDescriptionId(u"tile.cake");
	signPost.setDescriptionId(u"tile.sign");
	signWall.setDescriptionId(u"tile.sign");
	repeaterIdle.setDescriptionId(u"tile.diode");
	repeaterActive.setDescriptionId(u"tile.diode");
	pistonBase.setDescriptionId(u"tile.pistonBase");
	pistonStickyBase.setDescriptionId(u"tile.pistonStickyBase");
	redstoneWire.setDescriptionId(u"tile.redstoneDust");
	lever.setDescriptionId(u"tile.lever");
	pressurePlateStone.setDescriptionId(u"tile.pressurePlate");
	pressurePlateWood.setDescriptionId(u"tile.pressurePlate");
	torchRedstoneIdle.setDescriptionId(u"tile.notGate");
	torchRedstoneActive.setDescriptionId(u"tile.notGate");
	buttonStone.setDescriptionId(u"tile.button");
	
	for (Tile *tile : tiles)
		if (tile != nullptr)
			translucent[tile->id] = !tile->material.blocksLight();
	translucent[0] = true;

	wood.setNotifyRenderOnDataChange();
	sapling.setNotifyRenderOnDataChange();
	water.setNotifyRenderOnDataChange();
	calmWater.setNotifyRenderOnDataChange();
	lava.setNotifyRenderOnDataChange();
	calmLava.setNotifyRenderOnDataChange();
	treeTrunk.setNotifyRenderOnDataChange();
	leaves.setNotifyRenderOnDataChange();
	dispenser.setNotifyRenderOnDataChange();
	noteBlock.setNotifyRenderOnDataChange();
	bed.setNotifyRenderOnDataChange();
	railPowered.setNotifyRenderOnDataChange();
	railDetector.setNotifyRenderOnDataChange();
	pistonStickyBase.setNotifyRenderOnDataChange();
	pistonBase.setNotifyRenderOnDataChange();
	pistonExtension.setNotifyRenderOnDataChange();
	wool.setNotifyRenderOnDataChange();
	torch.setNotifyRenderOnDataChange();
	fire.setNotifyRenderOnDataChange();
	stairsWood.setNotifyRenderOnDataChange();
	chest.setNotifyRenderOnDataChange();
	redstoneWire.setNotifyRenderOnDataChange();
	crops.setNotifyRenderOnDataChange();
	furnace.setNotifyRenderOnDataChange();
	furnaceLit.setNotifyRenderOnDataChange();
	signPost.setNotifyRenderOnDataChange();
	doorWood.setNotifyRenderOnDataChange();
	ladder.setNotifyRenderOnDataChange();
	rail.setNotifyRenderOnDataChange();
	stairsStone.setNotifyRenderOnDataChange();
	signWall.setNotifyRenderOnDataChange();
	lever.setNotifyRenderOnDataChange();
	pressurePlateStone.setNotifyRenderOnDataChange();
	doorIron.setNotifyRenderOnDataChange();
	pressurePlateWood.setNotifyRenderOnDataChange();
	redstoneOre.setNotifyRenderOnDataChange();
	redstoneOreGlowing.setNotifyRenderOnDataChange();
	torchRedstoneIdle.setNotifyRenderOnDataChange();
	torchRedstoneActive.setNotifyRenderOnDataChange();
	buttonStone.setNotifyRenderOnDataChange();
	jukebox.setNotifyRenderOnDataChange();
	fence.setNotifyRenderOnDataChange();
	pumpkin.setNotifyRenderOnDataChange();
	jackOLantern.setNotifyRenderOnDataChange();
	cake.setNotifyRenderOnDataChange();
	repeaterIdle.setNotifyRenderOnDataChange();
	repeaterActive.setNotifyRenderOnDataChange();
	lockedChest.setNotifyRenderOnDataChange();
	trapdoor.setNotifyRenderOnDataChange();

}

Tile &Tile::setNotifyRenderOnDataChange()
{
	notifyRenderOnDataChange[id] = true;
	return *this;
}


const jstring Tile::TILE_DESCRIPTION_PREFIX = u"tile.";

Tile &Tile::setDescriptionId(const jstring &id)
{
	descriptionId = id;
	return *this;
}

Tile::Tile(int_t id, const Material &material) : material(material)
{
	if (tiles.at(id) != nullptr)
		throw std::runtime_error("Slot " + std::to_string(id) + " is already occupied");
	tiles[id] = this;

	this->id = id;
	
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	
	updateCachedProperties();
	isEntityTile[id] = false;
	soundType = &soundPowderFootstep;
}

Tile::Tile(int_t id, int_t tex, const Material &material) : Tile(id, material)
{
	this->tex = tex;
}

Tile &Tile::setLightBlock(int_t lightBlock)
{
	Tile::lightBlock[id] = lightBlock;
	return *this;
}

Tile &Tile::setLightEmission(int_t lightEmission)
{
	Tile::lightEmission[id] = lightEmission;
	return *this;
}

Tile &Tile::setExplodeable(float resistance)
{
	explosionResistance = resistance * 3.0f;
	return *this;
}


bool Tile::isCubeShaped()
{
	return true;
}

Tile::Shape Tile::getRenderShape()
{
	return SHAPE_BLOCK;
}

Tile &Tile::setDestroyTime(float time)
{
	destroySpeed = time;
	
	if (explosionResistance < time * 5.0f)
		explosionResistance = time * 5.0f;
	return *this;
}

Tile &Tile::setSoundType(StepSound &sound)
{
	soundType = &sound;
	return *this;
}

void Tile::setTicking(bool ticking)

{
	shouldTick[id] = ticking;
}

void Tile::updateCachedProperties()
{
	solid[id] = isSolidRender();
	lightBlock[id] = isSolidRender() ? 255 : 0;
}

void Tile::setShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
	xx0 = x0;
	yy0 = y0;
	zz0 = z0;
	xx1 = x1;
	yy1 = y1;
	zz1 = z1;
}

float Tile::getBrightness(LevelSource &level, int_t x, int_t y, int_t z)
{
	return level.getBrightness(x, y, z);
}

bool Tile::isFaceVisible(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	if (face == Facing::DOWN)
		y--;
	if (face == Facing::UP)
		y++;
	if (face == Facing::NORTH)
		z--;
	if (face == Facing::SOUTH)
		z++;
	if (face == Facing::WEST)
		x--;
	if (face == Facing::EAST)
		x++;
	return !level.isSolidTile(x, y, z);
}

bool Tile::shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	if (face == Facing::DOWN && yy0 > 0.0)
		return true;
	else if (face == Facing::UP && yy1 < 1.0)
		return true;
	else if (face == Facing::NORTH && zz0 > 0.0)
		return true;
	else if (face == Facing::SOUTH && zz1 < 1.0)
		return true;
	else if (face == Facing::WEST && xx0 > 0.0)
		return true;
	else if (face == Facing::EAST && xx1 < 1.0)
		return true;
	else
		return !level.isSolidTile(x, y, z);
}

int_t Tile::getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	return getTexture(face, level.getData(x, y, z));
}

int_t Tile::getTexture(Facing face, int_t data)
{
	return getTexture(face);
}

int_t Tile::getTexture(Facing face)
{
	return tex;
}

AABB *Tile::getTileAABB(Level &level, int_t x, int_t y, int_t z)
{
	return AABB::newTemp(x + xx0, y + yy0, z + zz0, x + xx1, y + yy1, z + zz1);
}

void Tile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
	AABB *aabb = getAABB(level, x, y, z);
	if (aabb != nullptr && aabb->intersects(bb))
		aabbList.push_back(aabb);
}

AABB *Tile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	return AABB::newTemp(x + xx0, y + yy0, z + zz0, x + xx1, y + yy1, z + zz1);
}

bool Tile::isSolidRender()
{
	return true;
}

bool Tile::mayPick(int_t data, bool canPickLiquid)
{
	return mayPick();
}

bool Tile::mayPick()
{
	return true;
}

void Tile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{

}

void Tile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{

}

void Tile::destroy(Level &level, int_t x, int_t y, int_t z, int_t data)
{

}

void Tile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{

}

void Tile::addLights(Level &level, int_t x, int_t y, int_t z)
{

}

int_t Tile::getTickDelay()
{
	return 10;
}

void Tile::onPlace(Level &level, int_t x, int_t y, int_t z)
{

}

void Tile::onRemove(Level &level, int_t x, int_t y, int_t z)
{

}

int_t Tile::getResourceCount(Random &random)
{
	return 1;
}

int_t Tile::getResource(int_t data, Random &random)
{
	return id;
}

float Tile::getDestroyProgress(Player &player)
{
	if (destroySpeed < 0.0f)
		return 0.0f;

	if (!player.canDestroy(*this))
		return 1.0f / destroySpeed / 100.0f;

	return player.getDestroySpeed(*this) / destroySpeed / 30.0f;
}

void Tile::spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data)
{
	spawnResources(level, x, y, z, data, 1.0f);
}

void Tile::spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance)
{
	if (level.isOnline)
		return;

	int_t resourceCount = getResourceCount(level.random);
	for (int_t i = 0; i < resourceCount; ++i)
	{
		if (level.random.nextFloat() > chance)
			continue;

		int_t resource = getResource(data, level.random);
		if (resource <= 0)
			continue;

		float spread = 0.7f;
		double xo = static_cast<double>(level.random.nextFloat() * spread) + static_cast<double>(1.0f - spread) * 0.5;
		double yo = static_cast<double>(level.random.nextFloat() * spread) + static_cast<double>(1.0f - spread) * 0.5;
		double zo = static_cast<double>(level.random.nextFloat() * spread) + static_cast<double>(1.0f - spread) * 0.5;

		ItemInstance stack(resource, 1, getSpawnResourcesAuxValue(data));
		auto entity = std::make_shared<EntityItem>(level, x + xo, y + yo, z + zo, stack);
		entity->throwTime = 10;
		level.addEntity(entity);
	}
}

int_t Tile::getSpawnResourcesAuxValue(int_t data)
{
	return 0;
}

HitResult Tile::clip(Level &level, int_t x, int_t y, int_t z, Vec3 &from, Vec3 &to)
{
	updateShape(level, x, y, z);

	Vec3 *localFrom = from.add(-x, -y, -z);
	Vec3 *localTo = to.add(-x, -y, -z);
	Vec3 *cxx0 = localFrom->clipX(*localTo, xx0);
	Vec3 *cxx1 = localFrom->clipX(*localTo, xx1);
	Vec3 *cyy0 = localFrom->clipY(*localTo, yy0);
	Vec3 *cyy1 = localFrom->clipY(*localTo, yy1);
	Vec3 *czz0 = localFrom->clipZ(*localTo, zz0);
	Vec3 *czz1 = localFrom->clipZ(*localTo, zz1);

	if (!containsX(cxx0)) cxx0 = nullptr;
	if (!containsX(cxx1)) cxx1 = nullptr;
	if (!containsY(cyy0)) cyy0 = nullptr;
	if (!containsY(cyy1)) cyy1 = nullptr;
	if (!containsZ(czz0)) czz0 = nullptr;
	if (!containsZ(czz1)) czz1 = nullptr;

	Vec3 *pick = nullptr;
	if (cxx0 != nullptr && (pick == nullptr || localFrom->distanceTo(*cxx0) < localFrom->distanceTo(*pick))) pick = cxx0;
	if (cxx1 != nullptr && (pick == nullptr || localFrom->distanceTo(*cxx1) < localFrom->distanceTo(*pick))) pick = cxx1;
	if (cyy0 != nullptr && (pick == nullptr || localFrom->distanceTo(*cyy0) < localFrom->distanceTo(*pick))) pick = cyy0;
	if (cyy1 != nullptr && (pick == nullptr || localFrom->distanceTo(*cyy1) < localFrom->distanceTo(*pick))) pick = cyy1;
	if (czz0 != nullptr && (pick == nullptr || localFrom->distanceTo(*czz0) < localFrom->distanceTo(*pick))) pick = czz0;
	if (czz1 != nullptr && (pick == nullptr || localFrom->distanceTo(*czz1) < localFrom->distanceTo(*pick))) pick = czz1;
	if (pick == nullptr)
		return HitResult();

	Facing face = Facing::NONE;
	if (pick == cxx0)
		face = Facing::WEST;
	if (pick == cxx1)
		face = Facing::EAST;
	if (pick == cyy0)
		face = Facing::DOWN;
	if (pick == cyy1)
		face = Facing::UP;
	if (pick == czz0)
		face = Facing::NORTH;
	if (pick == czz1)
		face = Facing::SOUTH;

	return HitResult(x, y, z, face, *pick->add(x, y, z));
}

bool Tile::containsX(Vec3 *vec)
{
	if (vec == nullptr)
		return false;
	return vec->y >= yy0 && vec->y <= yy1 && vec->z >= zz0 && vec->z <= zz1;
}

bool Tile::containsY(Vec3 *vec)
{
	if (vec == nullptr)
		return false;
	return vec->x >= xx0 && vec->x <= xx1 && vec->z >= zz0 && vec->z <= zz1;
}

bool Tile::containsZ(Vec3 *vec)
{
	if (vec == nullptr)
		return false;
	return vec->x >= xx0 && vec->x <= xx1 && vec->y >= yy0 && vec->y <= yy1;
}

int_t Tile::getRenderLayer()
{
	return 0;
}

void Tile::stepOn(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{

}

void Tile::setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	(void)face;
}

void Tile::setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	(void)player;
}

void Tile::prepareRender(Level &level, int_t x, int_t y, int_t z)
{

}

void Tile::attack(Level &level, int_t x, int_t y, int_t z, Player &player)
{

}

bool Tile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	(void)player;
	return false;
}

void Tile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{

}

int_t Tile::getColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	return 0xFFFFFF;
}

int_t Tile::getItemColor(int_t data)
{
	return 0xFFFFFF;
}

void Tile::entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{

}

void Tile::updateDefaultShape()
{

}

bool Tile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	return true;
}

void Tile::playerDestroy(Level &, int_t, int_t, int_t, int_t)
{
}

void Tile::harvestBlock(Level &level, Player &player, int_t x, int_t y, int_t z, int_t data)
{
	if (StatBase *stat = StatList::mineBlockStats[id])
		player.addStat(*stat, 1);
	spawnResources(level, x, y, z, data);
}



bool Tile::getSignal(Level &, int_t, int_t, int_t, int_t)
{
	return false;
}

bool Tile::getDirectSignal(Level &, int_t, int_t, int_t, int_t)
{
	return false;
}

void Tile::playBlock(Level &, int_t, int_t, int_t, int_t, int_t)
{
}

int_t Tile::getMobilityFlag() const
{
	return material.getMobilityFlag();
}

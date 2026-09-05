#include "SPCCommand.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "client/Minecraft.h"
#include "client/gamemode/CreativeMode.h"
#include "client/gamemode/SurvivalMode.h"
#include "client/gui/ChatLine.h"
#include "client/gui/Font.h"
#include "client/locale/Language.h"
#include "java/String.h"
#include "java/System.h"
#include "util/Mth.h"
#include "world/entity/EntityIO.h"
#include "world/entity/animal/Chicken.h"
#include "world/entity/animal/Cow.h"
#include "world/entity/animal/Pig.h"
#include "world/entity/animal/Sheep.h"
#include "world/entity/animal/Squid.h"
#include "world/entity/animal/Wolf.h"
#include "world/entity/monster/Creeper.h"
#include "world/entity/monster/Spider.h"
#include "world/entity/monster/PigZombie.h"
#include "world/entity/monster/Skeleton.h"
#include "world/entity/monster/Monster.h"
#include "world/entity/monster/Slime.h"
#include "world/entity/monster/Giant.h"
#include "world/entity/monster/Ghast.h"
#include "world/entity/monster/Zombie.h"
#include "world/entity/Mob.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/item/Item.h"
#include "world/item/ItemArmor.h"
#include "world/item/ItemInstance.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/phys/Vec3.h"

std::vector<SPCCommand::ChatMessage> SPCCommand::messages;

int_t SPCCommand::guiTickCount = 0;
Font *SPCCommand::messageFont = nullptr;

bool SPCCommand::flying = false;
bool SPCCommand::noclip = false;
bool SPCCommand::damageEnabled = true;
bool SPCCommand::fallDamage = true;
bool SPCCommand::fireDamage = true;
bool SPCCommand::waterDamage = true;
bool SPCCommand::instantMine = false;
bool SPCCommand::instantKill = false;
bool SPCCommand::freezeMobs = false;
bool SPCCommand::infiniteItems = false;
bool SPCCommand::keepItems = false;
bool SPCCommand::blockDrops = true;
bool SPCCommand::itemDamage = true;
bool SPCCommand::mobDamage = true;
bool SPCCommand::waterMovement = true;
bool SPCCommand::outputEnabled = true;
bool SPCCommand::longerLegs = false;
double SPCCommand::speed = 1.0;
double SPCCommand::gravity = 1.0;
double SPCCommand::reachDistance = 4.0;
double SPCCommand::superPunch = 1.0;
float SPCCommand::flySpeed = 1.0f;
bool SPCCommand::monstersSpawn = true;
bool SPCCommand::animalSpawn = true;

std::map<jstring, SPCCommand::Waypoint> SPCCommand::waypoints = {};

double SPCCommand::prevX = 0;
double SPCCommand::prevY = 0;
double SPCCommand::prevZ = 0;
bool SPCCommand::hasPrevPos = false;

bool SPCCommand::weWandEnabled = false;
int_t SPCCommand::wePos1X = 0, SPCCommand::wePos1Y = 0, SPCCommand::wePos1Z = 0;
int_t SPCCommand::wePos2X = 0, SPCCommand::wePos2Y = 0, SPCCommand::wePos2Z = 0;
bool SPCCommand::weHasPos1 = false, SPCCommand::weHasPos2 = false;

jstring SPCCommand::lastCommand = u"";

namespace
{
constexpr size_t MAX_MESSAGES = 50;
constexpr int_t MAX_MESSAGE_WIDTH = 320;
double gTimeSpeed = 1.0;

jstring lowerAscii(const jstring &value)
{
	jstring out = value;
	for (char16_t &ch : out)
	{
		if (ch >= u'A' && ch <= u'Z')
			ch = static_cast<char16_t>(ch - u'A' + u'a');
	}
	return out;
}

bool startsWith(const jstring &value, const jstring &prefix)
{
	return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

jstring normalizeName(const jstring &value)
{
	jstring lowered = lowerAscii(value);
	if (startsWith(lowered, u"item."))
		lowered = lowered.substr(5);
	else if (startsWith(lowered, u"tile."))
		lowered = lowered.substr(5);

	jstring out;
	out.reserve(lowered.size());
	for (char16_t ch : lowered)
	{
		if ((ch >= u'a' && ch <= u'z') || (ch >= u'0' && ch <= u'9'))
			out.push_back(ch);
	}
	return out;
}

std::vector<jstring> splitSpaces(const jstring &value)
{
	std::vector<jstring> parts;
	jstring current;
	for (char16_t ch : value)
	{
		if (ch == u' ' || ch == u'\t' || ch == u'\r' || ch == u'\n')
		{
			if (!current.empty())
			{
				parts.push_back(current);
				current.clear();
			}
		}
		else
		{
			current.push_back(ch);
		}
	}
	if (!current.empty())
		parts.push_back(current);
	return parts;
}

jstring joinParts(const std::vector<jstring> &parts, size_t start)
{
	jstring out;
	for (size_t i = start; i < parts.size(); ++i)
	{
		if (!out.empty())
			out.push_back(u' ');
		out += parts[i];
	}
	return out;
}

bool parseLong(const jstring &text, long_t &out)
{
	try
	{
		size_t used = 0;
		std::string utf8 = String::toUTF8(text);
		out = static_cast<long_t>(std::stoll(utf8, &used, 10));
		return used == utf8.size();
	}
	catch (...)
	{
		return false;
	}
}

bool parseDouble(const jstring &text, double &out)
{
	try
	{
		size_t used = 0;
		std::string utf8 = String::toUTF8(text);
		out = std::stod(utf8, &used);
		return used == utf8.size();
	}
	catch (...)
	{
		return false;
	}
}

void pushMessage(const jstring &text)
{
	std::vector<jstring> lines = SPCCommand::messageFont == nullptr
		? std::vector<jstring>{text}
		: ChatLine::split(*SPCCommand::messageFont, text, MAX_MESSAGE_WIDTH);
	for (const jstring &line : lines)
		SPCCommand::messages.push_back({line, SPCCommand::guiTickCount});
	if (SPCCommand::messages.size() > MAX_MESSAGES)
		SPCCommand::messages.erase(SPCCommand::messages.begin(), SPCCommand::messages.begin() + static_cast<long long>(SPCCommand::messages.size() - MAX_MESSAGES));
}

void addAlias(std::map<jstring, int_t> &aliases, const jstring &name, int_t id)
{
	jstring normalized = normalizeName(name);
	if (!normalized.empty())
		aliases[normalized] = id;
}

void addItemAliases(std::map<jstring, int_t> &aliases, Item *item, std::initializer_list<const char16_t *> extra = {})
{
	if (item == nullptr)
		return;
	addAlias(aliases, item->getDescriptionId(), item->getShiftedIndex());
	for (const char16_t *name : extra)
		addAlias(aliases, jstring(name), item->getShiftedIndex());
}

void addTileAliases(std::map<jstring, int_t> &aliases, int_t tileId, std::initializer_list<const char16_t *> names)
{
	for (const char16_t *name : names)
		addAlias(aliases, jstring(name), tileId);
}

const std::map<jstring, int_t> &itemAliases()
{
	static std::map<jstring, int_t> aliases;
	static bool initialized = false;
	if (initialized)
		return aliases;
	initialized = true;

	
	
	Language &language = Language::getInstance();
	for (Item *item : Item::items)
	{
		if (item == nullptr || item->getDescriptionId().empty())
			continue;
		addAlias(aliases, item->getDescriptionId(), item->getShiftedIndex());
		jstring displayName = language.getElementName(item->getDescriptionId());
		if (!displayName.empty())
			addAlias(aliases, displayName, item->getShiftedIndex());
	}

	for (int_t tileId = 1; tileId < static_cast<int_t>(Tile::tiles.size()) && tileId < 256; ++tileId)
	{
		Tile *tile = Tile::tiles[tileId];
		if (tile == nullptr || tile->descriptionId.empty())
			continue;
		jstring descAlias = normalizeName(tile->descriptionId);
		if (!descAlias.empty() && aliases.find(descAlias) == aliases.end())
			aliases[descAlias] = tileId;
		jstring displayName = normalizeName(language.getElementName(tile->descriptionId));
		if (!displayName.empty() && aliases.find(displayName) == aliases.end())
			aliases[displayName] = tileId;
	}

	addItemAliases(aliases, Items::flintAndSteel, {u"flintsteel", u"flintandsteel"});
	addItemAliases(aliases, Items::ingotIron, {u"iron", u"ironingot"});
	addItemAliases(aliases, Items::ingotGold, {u"gold", u"goldingot"});
	addItemAliases(aliases, Items::stick, {u"sticks"});
	addItemAliases(aliases, Items::seeds, {u"seed"});
	addItemAliases(aliases, Items::wheat);
	addItemAliases(aliases, Items::bread);
	addItemAliases(aliases, Items::reed, {u"reeds", u"sugarcane"});
	addItemAliases(aliases, Items::coal);
	addItemAliases(aliases, Items::diamond);
	addItemAliases(aliases, Items::redstone, {u"reddust"});
	addItemAliases(aliases, Items::dyePowder, {u"dye", u"lapislazuli"});
	addItemAliases(aliases, Items::flint);
	addItemAliases(aliases, Items::leather);
	addItemAliases(aliases, Items::silk, {u"string"});
	addItemAliases(aliases, Items::feather);
	addItemAliases(aliases, Items::gunpowder, {u"sulphur"});
	addItemAliases(aliases, Items::bowlEmpty, {u"bowl"});
	addItemAliases(aliases, Items::brick, {u"brickitem"});
	addItemAliases(aliases, Items::clayItem, {u"clay"});
	addItemAliases(aliases, Items::paper);
	addItemAliases(aliases, Items::book);
	addItemAliases(aliases, Items::sugar);
	addItemAliases(aliases, Items::compass);
	addItemAliases(aliases, Items::clock, {u"watch"});

	addItemAliases(aliases, Items::swordWood, {u"woodsword", u"woodensword"});
	addItemAliases(aliases, Items::shovelWood, {u"woodshovel", u"woodenspade"});
	addItemAliases(aliases, Items::pickaxeWood, {u"woodpickaxe", u"woodenpickaxe"});
	addItemAliases(aliases, Items::axeWood, {u"woodaxe", u"woodenaxe"});
	addItemAliases(aliases, Items::hoeWood, {u"woodhoe", u"woodenhoe"});
	addItemAliases(aliases, Items::swordStone, {u"stonesword"});
	addItemAliases(aliases, Items::shovelStone, {u"stoneshovel", u"stonespade"});
	addItemAliases(aliases, Items::pickaxeStone, {u"stonepickaxe"});
	addItemAliases(aliases, Items::axeStone, {u"stoneaxe"});
	addItemAliases(aliases, Items::hoeStone, {u"stonehoe"});
	addItemAliases(aliases, Items::swordIron, {u"ironsword"});
	addItemAliases(aliases, Items::shovelIron, {u"ironshovel", u"ironspade"});
	addItemAliases(aliases, Items::pickaxeIron, {u"ironpickaxe"});
	addItemAliases(aliases, Items::axeIron, {u"ironaxe"});
	addItemAliases(aliases, Items::hoeIron, {u"ironhoe"});
	addItemAliases(aliases, Items::swordDiamond, {u"diamondsword"});
	addItemAliases(aliases, Items::shovelDiamond, {u"diamondshovel", u"diamondspade"});
	addItemAliases(aliases, Items::pickaxeDiamond, {u"diamondpickaxe"});
	addItemAliases(aliases, Items::axeDiamond, {u"diamondaxe"});
	addItemAliases(aliases, Items::hoeDiamond, {u"diamondhoe"});
	addItemAliases(aliases, Items::swordGold, {u"goldsword", u"goldensword"});
	addItemAliases(aliases, Items::shovelGold, {u"goldshovel", u"goldenspade"});
	addItemAliases(aliases, Items::pickaxeGold, {u"goldpickaxe", u"goldenpickaxe"});
	addItemAliases(aliases, Items::axeGold, {u"goldaxe", u"goldenaxe"});
	addItemAliases(aliases, Items::hoeGold, {u"goldhoe", u"goldenhoe"});

	addItemAliases(aliases, Items::apple);
	addItemAliases(aliases, Items::appleGold, {u"goldapple", u"goldenapple"});
	addItemAliases(aliases, Items::bow);
	addItemAliases(aliases, Items::arrow, {u"arrows"});
	addItemAliases(aliases, Items::cookie);
	addItemAliases(aliases, Items::porkchopRaw, {u"pork", u"rawpork", u"porkchop"});
	addItemAliases(aliases, Items::porkchopCooked, {u"cookedpork", u"grilledpork", u"cookedporkchop"});
	addItemAliases(aliases, Items::fishRaw, {u"fish", u"rawfish"});
	addItemAliases(aliases, Items::fishCooked, {u"cookedfish"});
	addItemAliases(aliases, Items::bowlSoup, {u"soup", u"mushroomsoup", u"mushroomstew", u"stew"});
	addItemAliases(aliases, Items::cake);
	addItemAliases(aliases, Items::egg, {u"eggs"});
	addItemAliases(aliases, Items::painting);
	addItemAliases(aliases, Items::sign);
	addItemAliases(aliases, Items::doorWood, {u"door", u"woodendoor", u"wooddoor"});
	addItemAliases(aliases, Items::doorIron, {u"irondoor"});
	addItemAliases(aliases, Items::bed);
	addItemAliases(aliases, Items::map);
	addItemAliases(aliases, Items::minecart, {u"cart"});
	addItemAliases(aliases, Items::minecartPowered, {u"poweredminecart", u"furnacecart", u"furnaceminecart"});
	addItemAliases(aliases, Items::minecartChest, {u"chestcart", u"chestminecart", u"storageminecart"});
	addItemAliases(aliases, Items::boat);
	addItemAliases(aliases, Items::saddle);
	addItemAliases(aliases, Items::fishingRod, {u"rod", u"fishingpole"});
	addItemAliases(aliases, Items::shears, {u"shear"});
	addItemAliases(aliases, Items::snowball, {u"snowballs"});
	addItemAliases(aliases, Items::slimeball, {u"slime"});
	addItemAliases(aliases, Items::bone, {u"bones"});
	addItemAliases(aliases, Items::glowstoneDust, {u"glowdust", u"lightstonedust"});
	addItemAliases(aliases, Items::redstoneRepeater, {u"repeater", u"diode"});
	addItemAliases(aliases, Items::bucketEmpty, {u"bucket"});
	addItemAliases(aliases, Items::bucketWater, {u"waterbucket"});
	addItemAliases(aliases, Items::bucketLava, {u"lavabucket"});
	addItemAliases(aliases, Items::bucketMilk, {u"milkbucket", u"milk"});
	addItemAliases(aliases, Items::record13, {u"record", u"gold13record", u"goldrecord"});
	addItemAliases(aliases, Items::recordCat, {u"catrecord", u"greenrecord"});

	addItemAliases(aliases, Items::helmetLeather, {u"leatherhelmet", u"leathercap"});
	addItemAliases(aliases, Items::plateLeather, {u"leatherchestplate", u"leathertunic"});
	addItemAliases(aliases, Items::legsLeather, {u"leatherleggings", u"leatherpants"});
	addItemAliases(aliases, Items::bootsLeather, {u"leatherboots"});
	addItemAliases(aliases, Items::helmetChain, {u"chainhelmet", u"chainmailhelmet"});
	addItemAliases(aliases, Items::plateChain, {u"chainchestplate", u"chainmailchestplate"});
	addItemAliases(aliases, Items::legsChain, {u"chainleggings", u"chainmailleggings"});
	addItemAliases(aliases, Items::bootsChain, {u"chainboots", u"chainmailboots"});
	addItemAliases(aliases, Items::helmetIron, {u"ironhelmet"});
	addItemAliases(aliases, Items::plateIron, {u"ironchestplate", u"ironplate"});
	addItemAliases(aliases, Items::legsIron, {u"ironleggings", u"ironpants"});
	addItemAliases(aliases, Items::bootsIron, {u"ironboots"});
	addItemAliases(aliases, Items::helmetDiamond, {u"diamondhelmet"});
	addItemAliases(aliases, Items::plateDiamond, {u"diamondchestplate", u"diamondplate"});
	addItemAliases(aliases, Items::legsDiamond, {u"diamondleggings", u"diamondpants"});
	addItemAliases(aliases, Items::bootsDiamond, {u"diamondboots"});
	addItemAliases(aliases, Items::helmetGold, {u"goldhelmet", u"goldenhelmet"});
	addItemAliases(aliases, Items::plateGold, {u"goldchestplate", u"goldenchestplate"});
	addItemAliases(aliases, Items::legsGold, {u"goldleggings", u"goldenleggings"});
	addItemAliases(aliases, Items::bootsGold, {u"goldboots", u"goldenboots"});

	addTileAliases(aliases, 1, {u"stone"});
	addTileAliases(aliases, 2, {u"grass"});
	addTileAliases(aliases, 3, {u"dirt"});
	addTileAliases(aliases, 5, {u"wood", u"planks", u"plank"});
	addTileAliases(aliases, 12, {u"sand"});
	addTileAliases(aliases, 13, {u"gravel"});
	addTileAliases(aliases, 17, {u"log", u"tree", u"trunk"});
	addTileAliases(aliases, 18, {u"leaf", u"leaves"});
	addTileAliases(aliases, 37, {u"yellowflower", u"flower"});
	addTileAliases(aliases, 38, {u"rose", u"redflower"});
	addTileAliases(aliases, 39, {u"brownmushroom"});
	addTileAliases(aliases, 40, {u"redmushroom"});
	addTileAliases(aliases, 4, {u"cobble", u"cobblestone"});
	addTileAliases(aliases, 7, {u"bedrock"});
	addTileAliases(aliases, 8, {u"water"});
	addTileAliases(aliases, 10, {u"lava"});
	addTileAliases(aliases, 14, {u"goldore"});
	addTileAliases(aliases, 15, {u"ironore"});
	addTileAliases(aliases, 16, {u"coalore"});
	addTileAliases(aliases, 21, {u"lapisore"});
	addTileAliases(aliases, 24, {u"sandstone"});
	addTileAliases(aliases, 48, {u"mossycobblestone", u"mossstone"});
	addTileAliases(aliases, 49, {u"obsidian"});
	addTileAliases(aliases, 43, {u"doubleslab"});
	addTileAliases(aliases, 44, {u"slab", u"halfslab"});
	addTileAliases(aliases, 58, {u"workbench", u"craftingtable"});
	addTileAliases(aliases, 59, {u"crop", u"crops", u"wheatcrop"});
	addTileAliases(aliases, 60, {u"farmland", u"soil"});
	addTileAliases(aliases, 61, {u"furnace"});
	addTileAliases(aliases, 56, {u"diamondore"});
	addTileAliases(aliases, 73, {u"redstoneore"});
	addTileAliases(aliases, 78, {u"snow"});
	addTileAliases(aliases, 79, {u"ice"});
	addTileAliases(aliases, 81, {u"cactus"});
	addTileAliases(aliases, 82, {u"clayblock"});
	addTileAliases(aliases, 83, {u"reedblock"});
	addTileAliases(aliases, 86, {u"pumpkin"});
	addTileAliases(aliases, 50, {u"torch"});
	addTileAliases(aliases, 6, {u"sapling"});
	addTileAliases(aliases, 19, {u"sponge"});
	addTileAliases(aliases, 20, {u"glass"});
	addTileAliases(aliases, 22, {u"lapisblock", u"lapislazuliblock"});
	addTileAliases(aliases, 23, {u"dispenser"});
	addTileAliases(aliases, 25, {u"noteblock", u"note"});
	addTileAliases(aliases, 27, {u"poweredrail", u"goldenrail", u"boosterrail", u"booster"});
	addTileAliases(aliases, 28, {u"detectorrail", u"detector"});
	addTileAliases(aliases, 29, {u"stickypiston"});
	addTileAliases(aliases, 30, {u"web", u"cobweb"});
	addTileAliases(aliases, 31, {u"tallgrass", u"longgrass"});
	addTileAliases(aliases, 32, {u"deadbush", u"deadshrub"});
	addTileAliases(aliases, 33, {u"piston"});
	addTileAliases(aliases, 35, {u"wool", u"cloth", u"whitewool"});
	addTileAliases(aliases, 41, {u"goldblock", u"blockofgold"});
	addTileAliases(aliases, 42, {u"ironblock", u"blockofiron"});
	addTileAliases(aliases, 45, {u"brick", u"bricks", u"brickblock"});
	addTileAliases(aliases, 46, {u"tnt"});
	addTileAliases(aliases, 47, {u"bookshelf", u"bookcase"});
	addTileAliases(aliases, 52, {u"mobspawner", u"spawner"});
	addTileAliases(aliases, 53, {u"woodstairs", u"woodenstairs", u"stairs"});
	addTileAliases(aliases, 54, {u"chest"});
	addTileAliases(aliases, 57, {u"diamondblock", u"blockofdiamond"});
	addTileAliases(aliases, 65, {u"ladder"});
	addTileAliases(aliases, 66, {u"rail", u"rails", u"track", u"tracks", u"minecarttrack"});
	addTileAliases(aliases, 67, {u"cobblestairs", u"cobblestonestairs", u"stonestairs"});
	addTileAliases(aliases, 69, {u"lever", u"switch"});
	addTileAliases(aliases, 70, {u"stoneplate", u"stonepressureplate", u"pressureplate"});
	addTileAliases(aliases, 72, {u"woodplate", u"woodenpressureplate", u"woodpressureplate"});
	addTileAliases(aliases, 76, {u"redstonetorch", u"redtorch"});
	addTileAliases(aliases, 77, {u"button", u"stonebutton"});
	addTileAliases(aliases, 80, {u"snowblock"});
	addTileAliases(aliases, 84, {u"jukebox"});
	addTileAliases(aliases, 85, {u"fence", u"woodfence", u"woodenfence"});
	addTileAliases(aliases, 87, {u"netherrack", u"netherstone", u"hellrock", u"bloodstone"});
	addTileAliases(aliases, 88, {u"soulsand", u"slowsand"});
	addTileAliases(aliases, 89, {u"glowstone", u"lightstone", u"glowstoneblock"});
	addTileAliases(aliases, 91, {u"jackolantern", u"pumpkinlantern"});
	addTileAliases(aliases, 95, {u"lockedchest", u"aprilchest"});
	addTileAliases(aliases, 96, {u"trapdoor", u"hatch"});
	return aliases;
}

int_t resolveItemId(const jstring &name)
{
	long_t numeric = 0;
	if (parseLong(name, numeric))
	{
		if (numeric >= 0 && numeric < 256 && Tile::tiles[static_cast<size_t>(numeric)] != nullptr)
			return static_cast<int_t>(numeric);
		if (numeric >= 0 && numeric < static_cast<long_t>(Item::items.size()) && Item::items[static_cast<size_t>(numeric)] != nullptr)
			return static_cast<int_t>(numeric);
	}

	auto it = itemAliases().find(normalizeName(name));
	if (it != itemAliases().end())
		return it->second;
	return -1;
}

jstring describeId(int_t id)
{
	if (id >= 0 && id < 256 && Tile::tiles[static_cast<size_t>(id)] != nullptr)
	{
		for (const auto &entry : itemAliases())
		{
			if (entry.second == id)
				return entry.first;
		}
		return u"tile " + String::toString(id);
	}
	if (id >= 0 && id < static_cast<int_t>(Item::items.size()) && Item::items[static_cast<size_t>(id)] != nullptr)
	{
		const jstring &desc = Item::items[static_cast<size_t>(id)]->getDescriptionId();
		if (!desc.empty())
			return desc;
	}
	return u"id " + String::toString(id);
}

bool requirePlayer(Minecraft &mc)
{
	if (mc.player != nullptr)
		return true;
	SPCCommand::sendError(u"No player loaded");
	return false;
}

bool requireLevel(Minecraft &mc)
{
	if (mc.level != nullptr)
		return true;
	SPCCommand::sendError(u"No level loaded");
	return false;
}

bool matches(const jstring &cmd, std::initializer_list<const char16_t *> names)
{
	for (const char16_t *name : names)
	{
		if (cmd == name)
			return true;
	}
	return false;
}

std::shared_ptr<Entity> createSpawnEntity(Level &level, const jstring &name)
{
	jstring key = normalizeName(name);
	if (key == u"random" || key == u"r")
	{
		static const std::vector<const char16_t *> randomNames = {
			u"pig", u"sheep", u"cow", u"chicken", u"squid", u"wolf", u"zombie", u"skeleton", u"spider", u"creeper", u"slime", u"pigzombie"
		};
		key = jstring(randomNames[static_cast<size_t>(level.random.nextInt(static_cast<int_t>(randomNames.size())))]);
	}
	if (key == u"pig") return std::make_shared<Pig>(level);
	if (key == u"sheep") return std::make_shared<Sheep>(level);
	if (key == u"cow") return std::make_shared<Cow>(level);
	if (key == u"chicken") return std::make_shared<Chicken>(level);
	if (key == u"squid") return std::make_shared<Squid>(level);
	if (key == u"wolf") return std::make_shared<Wolf>(level);
	if (key == u"zombie") return std::make_shared<Zombie>(level);
	if (key == u"monster") return std::make_shared<Monster>(level);
	if (key == u"giant") return std::make_shared<Giant>(level);
	if (key == u"skeleton") return std::make_shared<Skeleton>(level);
	if (key == u"ghast") return std::make_shared<Ghast>(level);
	if (key == u"spider") return std::make_shared<Spider>(level);
	if (key == u"creeper") return std::make_shared<Creeper>(level);
	if (key == u"chargedcreeper" || key == u"poweredcreeper")
	{
		std::shared_ptr<Creeper> creeper = std::make_shared<Creeper>(level);
		creeper->setPowered(true);
		return creeper;
	}
	if (key == u"slime") return std::make_shared<Slime>(level);
	if (key == u"pigzombie" || key == u"zombiepigman") return std::make_shared<PigZombie>(level);
	return nullptr;
}


void announceToggle(const jstring &label, bool enabled)
{
	SPCCommand::addMessage(u"\u00a77" + label + (enabled ? u" enabled" : u" disabled"));
}
}

void SPCCommand::setCurrentPos(Minecraft &mc)
{
	if (mc.player == nullptr)
		return;
	prevX = mc.player->x;
	prevY = mc.player->y;
	prevZ = mc.player->z;
	hasPrevPos = true;
}

void SPCCommand::addMessage(const jstring &text)
{
	if (!outputEnabled)
		return;
	pushMessage(text);
}

void SPCCommand::addChatMessage(const jstring &text)
{
	pushMessage(text);
}

void SPCCommand::sendError(const jstring &text)
{
	addMessage(u"\u00a74" + text);
}

void SPCCommand::setMessageFont(Font *font)
{
	messageFont = font;
}

void SPCCommand::execute(Minecraft &mc, const jstring &input)
{
	if (input.empty())
		return;

	jstring commandLine = input;
	if (!commandLine.empty() && commandLine[0] == u'/')
		commandLine.erase(commandLine.begin());

	auto parts = splitSpaces(commandLine);
	if (parts.empty())
		return;

	jstring cmd = lowerAscii(parts[0]);
	lastCommand = commandLine;

	if (cmd == u"gamemode" || cmd == u"gm")
	{
		if (parts.size() < 2)
		{
			sendError(u"Usage: gamemode <0|1>");
			return;
		}
		if (parts[1] == u"1" || parts[1] == u"creative")
		{
			mc.gameMode = std::make_shared<CreativeMode>(mc);
			if (mc.player) {
				mc.gameMode->initPlayer(mc.player);
				int_t basicBlocks[] = { 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 20, 24, 35, 41, 42, 45, 46, 47, 48, 49, 50, 53, 54, 57, 58, 61, 65, 87, 88, 89, 91 };
				for (int i = 0; i < 33 && i < 36; i++) {
					mc.player->inventory.setItem(i, ItemInstance(basicBlocks[i], 1, 0));
				}
				addMessage(u"Game mode updated to Creative");
			}
		}
		else if (parts[1] == u"0" || parts[1] == u"survival")
		{
			mc.gameMode = std::make_shared<SurvivalMode>(mc);
			if (mc.player) {
				mc.gameMode->initPlayer(mc.player);
				addMessage(u"Game mode updated to Survival");
			}
		}
		else
		{
			sendError(u"Invalid game mode. Use 0 for Survival, 1 for Creative.");
		}
		return;
	}

	if (cmd == u"/admin" || cmd == u"//admin" || cmd == u"admin")
	{
		if (parts.size() < 2)
		{
			sendError(u"Usage: /admin <player>");
			return;
		}
		// If we are in multiplayer, we could send an op command.
		// For now, let's just add it as a local alias/message.
		mc.player->sendChatMessage(u"/op " + parts[1]);
		addMessage(u"Granted admin to " + parts[1]);
		return;
	}

	if (cmd == u"/time" || cmd == u"//time" || cmd == u"time")
	{
		if (parts.size() < 2)
		{
			sendError(u"Usage: /time <day|night|set> [value]");
			return;
		}
		if (parts[1] == u"day") {
			mc.level->setTime(0);
			addMessage(u"Time set to Day");
		} else if (parts[1] == u"night") {
			mc.level->setTime(14000);
			addMessage(u"Time set to Night");
		} else if (parts[1] == u"set" && parts.size() >= 3) {
			long_t timeVal = 0;
			if (parseLong(parts[2], timeVal)) {
				mc.level->setTime(timeVal);
				addMessage(u"Time set to " + parts[2]);
			}
		}
		return;
	}

	if (cmd == u"/weather" || cmd == u"//weather" || cmd == u"weather")
	{
		if (parts.size() < 2)
		{
			sendError(u"Usage: /weather <clear|rain|thunder>");
			return;
		}
		if (parts[1] == u"clear") {
			mc.level->setWeather(false, false);
			addMessage(u"Weather set to Clear");
		} else if (parts[1] == u"rain") {
			mc.level->setWeather(true, false);
			addMessage(u"Weather set to Rain");
		} else if (parts[1] == u"thunder") {
			mc.level->setWeather(true, true);
			addMessage(u"Weather set to Thunder");
		}
		return;
	}

	if (cmd == u"tpr1")
	{
		weWandEnabled = !weWandEnabled;
		if (weWandEnabled)
		{
			if (mc.player) mc.player->inventory.setItem(mc.player->inventory.currentItem, ItemInstance(271, 1, 0)); // Wooden Axe
			addMessage(u"Wand enabled! Left/Right click blocks with Wooden Axe.");
		}
		else
		{
			addMessage(u"Wand disabled.");
		}
		return;
	}

	if (cmd == u"/set" || cmd == u"/fil" || cmd == u"/fill" || cmd == u"//set" || cmd == u"//fil" || cmd == u"//fill")
	{
		if (!weHasPos1 || !weHasPos2)
		{
			sendError(u"Make a selection first.");
			return;
		}
		if (parts.size() < 2)
		{
			sendError(u"Usage: //set <block_id>");
			return;
		}
		int_t id = resolveItemId(parts[1]);
		int_t minX = std::min(wePos1X, wePos2X);
		int_t maxX = std::max(wePos1X, wePos2X);
		int_t minY = std::min(wePos1Y, wePos2Y);
		int_t maxY = std::max(wePos1Y, wePos2Y);
		int_t minZ = std::min(wePos1Z, wePos2Z);
		int_t maxZ = std::max(wePos1Z, wePos2Z);
		int_t count = 0;
		for (int_t x = minX; x <= maxX; x++) {
			for (int_t y = minY; y <= maxY; y++) {
				for (int_t z = minZ; z <= maxZ; z++) {
					mc.level->setTile(x, y, z, id);
					count++;
				}
			}
		}
		addMessage(u"Set " + String::fromUTF8(std::to_string(count)) + u" blocks.");
		return;
	}

	if (cmd == u"/replace" || cmd == u"//replace")
	{
		if (!weHasPos1 || !weHasPos2)
		{
			sendError(u"Make a selection first.");
			return;
		}
		if (parts.size() < 3)
		{
			sendError(u"Usage: //replace <from_id> <to_id>");
			return;
		}
		int_t fromId = resolveItemId(parts[1]);
		int_t toId = resolveItemId(parts[2]);
		int_t minX = std::min(wePos1X, wePos2X);
		int_t maxX = std::max(wePos1X, wePos2X);
		int_t minY = std::min(wePos1Y, wePos2Y);
		int_t maxY = std::max(wePos1Y, wePos2Y);
		int_t minZ = std::min(wePos1Z, wePos2Z);
		int_t maxZ = std::max(wePos1Z, wePos2Z);
		int_t count = 0;
		for (int_t x = minX; x <= maxX; x++) {
			for (int_t y = minY; y <= maxY; y++) {
				for (int_t z = minZ; z <= maxZ; z++) {
					if (mc.level->getTile(x, y, z) == fromId) {
						mc.level->setTile(x, y, z, toId);
						count++;
					}
				}
			}
		}
		addMessage(u"Replaced " + String::fromUTF8(std::to_string(count)) + u" blocks.");
		return;
	}

	if (matches(cmd, {u"give", u"i", u"item"}))
	{
		if (!requirePlayer(mc))
			return;
		if (parts.size() < 2)
		{
			sendError(u"Usage: give <item> [count]");
			return;
		}
		int_t id = resolveItemId(parts[1]);
		if (id < 0)
		{
			sendError(u"Unknown item: " + parts[1]);
			return;
		}
		long_t count = 1;
		if (parts.size() >= 3 && !parseLong(parts[2], count))
		{
			sendError(u"Invalid count: " + parts[2]);
			return;
		}
		if (count <= 0)
		{
			sendError(u"Count must be positive");
			return;
		}
		ItemInstance stack(id, static_cast<int_t>(count));
		if (!mc.player->inventory.add(stack))
		{
			sendError(u"Inventory full");
			return;
		}
		addMessage(u"\u00a77Given " + String::toString(count) + u" of " + describeId(id));
		return;
	}

	if (matches(cmd, {u"tele", u"teleport", u"t"}))
	{
		if (!requirePlayer(mc))
			return;
		if (parts.size() < 4)
		{
			sendError(u"Usage: tele <x> <y> <z>");
			return;
		}
		double x = 0.0, y = 0.0, z = 0.0;
		if (!parseDouble(parts[1], x) || !parseDouble(parts[2], y) || !parseDouble(parts[3], z))
		{
			sendError(u"Invalid coordinates");
			return;
		}
		setCurrentPos(mc);
		mc.player->setPos(x, y, z);
		addMessage(u"\u00a77Teleported to " + String::toString(x) + u", " + String::toString(y) + u", " + String::toString(z));
		return;
	}

	if (matches(cmd, {u"pos", u"p"}))
	{
		if (!requirePlayer(mc))
			return;
		addMessage(u"\u00a77Position: " + String::toString(mc.player->x) + u", " + String::toString(mc.player->y) + u", " + String::toString(mc.player->z));
		return;
	}

	if (matches(cmd, {u"set", u"s"}))
	{
		if (!requirePlayer(mc))
			return;
		if (parts.size() < 2)
		{
			sendError(u"Usage: set <name>");
			return;
		}
		jstring name = lowerAscii(parts[1]);
		waypoints[name] = {mc.player->x, mc.player->y, mc.player->z};
		addMessage(u"\u00a77Waypoint set: " + name);
		return;
	}

	if (cmd == u"goto")
	{
		if (!requirePlayer(mc))
			return;
		if (parts.size() < 2)
		{
			sendError(u"Usage: goto <name>");
			return;
		}
		auto it = waypoints.find(lowerAscii(parts[1]));
		if (it == waypoints.end())
		{
			sendError(u"Unknown waypoint: " + parts[1]);
			return;
		}
		setCurrentPos(mc);
		mc.player->setPos(it->second.x, it->second.y, it->second.z);
		addMessage(u"\u00a77Warped to " + parts[1]);
		return;
	}

	if (cmd == u"rem")
	{
		if (parts.size() < 2)
		{
			sendError(u"Usage: rem <name>");
			return;
		}
		auto it = waypoints.find(lowerAscii(parts[1]));
		if (it == waypoints.end())
		{
			sendError(u"Unknown waypoint: " + parts[1]);
			return;
		}
		waypoints.erase(it);
		addMessage(u"\u00a77Removed waypoint: " + parts[1]);
		return;
	}

	if (matches(cmd, {u"listwaypoints", u"l"}))
	{
		if (waypoints.empty())
		{
			addMessage(u"\u00a77No waypoints set");
			return;
		}
		addMessage(u"\u00a76Waypoints:");
		for (const auto &entry : waypoints)
			addMessage(u"\u00a7e" + entry.first + u"\u00a77 = " + String::toString(entry.second.x) + u", " + String::toString(entry.second.y) + u", " + String::toString(entry.second.z));
		return;
	}

	if (cmd == u"home")
	{
		if (!requirePlayer(mc) || !requireLevel(mc))
			return;
		setCurrentPos(mc);
		mc.player->setPos(mc.level->xSpawn + 0.5, mc.level->ySpawn + 1.0, mc.level->zSpawn + 0.5);
		addMessage(u"\u00a77Teleported home");
		return;
	}

	if (cmd == u"return")
	{
		if (!requirePlayer(mc))
			return;
		if (!hasPrevPos)
		{
			sendError(u"No previous position saved");
			return;
		}
		double x = prevX;
		double y = prevY;
		double z = prevZ;
		setCurrentPos(mc);
		mc.player->setPos(x, y, z);
		addMessage(u"\u00a77Returned to previous position");
		return;
	}

	if (cmd == u"kill")
	{
		if (!requirePlayer(mc))
			return;
		mc.player->health = 0;
		mc.player->die(nullptr);
		addMessage(u"\u00a77Killed player");
		return;
	}

	if (cmd == u"heal")
	{
		if (!requirePlayer(mc))
			return;
		long_t amount = Player::MAX_HEALTH;
		if (parts.size() >= 2)
		{
			if (!parseLong(parts[1], amount))
			{
				sendError(u"Invalid heal amount: " + parts[1]);
				return;
			}
		}
		if (amount <= 0)
		{
			sendError(u"Heal amount must be positive");
			return;
		}
		mc.player->heal(static_cast<int_t>(amount));
		addMessage(u"\u00a77Healed " + String::toString(amount) + u" health");
		return;
	}

	if (cmd == u"hurt")
	{
		if (!requirePlayer(mc))
			return;
		if (parts.size() < 2)
		{
			sendError(u"Usage: hurt <amount>");
			return;
		}
		long_t amount = 0;
		if (!parseLong(parts[1], amount) || amount <= 0)
		{
			sendError(u"Invalid hurt amount: " + parts[1]);
			return;
		}
		mc.player->hurt(nullptr, static_cast<int_t>(amount));
		addMessage(u"\u00a77Applied " + String::toString(amount) + u" damage");
		return;
	}

	if (cmd == u"health")
	{
		if (!requirePlayer(mc))
			return;
		if (parts.size() < 2)
		{
			addMessage(u"\u00a77Health: " + String::toString(mc.player->health));
			return;
		}
		jstring mode = lowerAscii(parts[1]);
		if (mode == u"max")
			mc.player->health = Player::MAX_HEALTH;
		else if (mode == u"min")
			mc.player->health = 1;
		else if (mode == u"inf" || mode == u"infinite")
			mc.player->health = 32767;
		else
		{
			long_t amount = 0;
			if (!parseLong(parts[1], amount))
			{
				sendError(u"Usage: health <max|min|infinite|value>");
				return;
			}
			mc.player->health = static_cast<int_t>(amount);
		}
		addMessage(u"\u00a77Health set to " + String::toString(mc.player->health));
		return;
	}

	if (cmd == u"fly")
	{
		if (!requirePlayer(mc))
			return;
		if (parts.size() >= 2)
		{
			double newSpeed = 0.0;
			if (!parseDouble(parts[1], newSpeed) || newSpeed <= 0.0)
			{
				sendError(u"Invalid fly speed: " + parts[1]);
				return;
			}
			flySpeed = static_cast<float>(newSpeed);
		}
		flying = !flying;
		mc.player->noPhysics = flying || noclip;
		announceToggle(u"Fly", flying);
		return;
	}

	if (cmd == u"noclip")
	{
		if (!requirePlayer(mc))
			return;
		noclip = !noclip;
		mc.player->noPhysics = flying || noclip;
		announceToggle(u"Noclip", noclip);
		return;
	}

	if (cmd == u"weather")
	{
		if (!requireLevel(mc))
			return;
		if (mc.isOnline())
		{
			sendError(u"weather is singleplayer only");
			return;
		}
		if (parts.size() == 1)
		{
			jstring state = mc.level->isRaining()
				? (mc.level->isThundering() ? jstring(u"thunderstorm") : jstring(u"rain"))
				: jstring(u"clear");
			addMessage(u"00a77Weather: " + state);
			return;
		}
		jstring mode = lowerAscii(parts[1]);
		if (matches(mode, {u"clear", u"off", u"sun"}))
		{
			mc.level->setWeather(false, false);
			addMessage(u"00a77Weather cleared");
			return;
		}
		if (matches(mode, {u"rain", u"on"}))
		{
			mc.level->setWeather(true, false);
			addMessage(u"00a77Rain started");
			return;
		}
		if (matches(mode, {u"thunder", u"storm", u"lightning"}))
		{
			mc.level->setWeather(true, true);
			addMessage(u"00a77Thunderstorm started");
			return;
		}
		sendError(u"Usage: weather [clear|rain|thunder]");
		return;
	}

	if (cmd == u"time")
	{
		if (!requireLevel(mc))
			return;
		if (parts.size() == 1)
		{
			addMessage(u"\u00a77Time: " + String::toString(mc.level->time) + u" (day " + String::toString(mc.level->time / Level::TICKS_PER_DAY) + u")");
			return;
		}
		jstring mode = lowerAscii(parts[1]);
		if (mode == u"day")
		{
			long_t day = mc.level->time / Level::TICKS_PER_DAY;
			mc.level->setTime(day * Level::TICKS_PER_DAY);
			addMessage(u"\u00a77Time set to day");
			return;
		}
		if (mode == u"night")
		{
			long_t day = mc.level->time / Level::TICKS_PER_DAY;
			mc.level->setTime(day * Level::TICKS_PER_DAY + 13000);
			addMessage(u"\u00a77Time set to night");
			return;
		}
		if (mode == u"set")
		{
			if (parts.size() < 3)
			{
				sendError(u"Usage: time set <ticks|day|hour|minute> <value>");
				return;
			}
			if (parts.size() == 3)
			{
				long_t ticks = 0;
				if (!parseLong(parts[2], ticks))
				{
					sendError(u"Invalid time value: " + parts[2]);
					return;
				}
				mc.level->setTime(ticks);
				addMessage(u"\u00a77Time set to " + String::toString(ticks));
				return;
			}
			jstring unit = lowerAscii(parts[2]);
			long_t value = 0;
			if (!parseLong(parts[3], value))
			{
				sendError(u"Invalid time value: " + parts[3]);
				return;
			}
			long_t day = mc.level->time / Level::TICKS_PER_DAY;
			if (unit == u"day")
				mc.level->setTime(value * Level::TICKS_PER_DAY);
			else if (unit == u"hour")
				mc.level->setTime(day * Level::TICKS_PER_DAY + value * 1000);
			else if (unit == u"minute")
				mc.level->setTime(day * Level::TICKS_PER_DAY + (value * Level::TICKS_PER_DAY) / 1440);
			else
			{
				sendError(u"Unknown time unit: " + parts[2]);
				return;
			}
			addMessage(u"\u00a77Time updated");
			return;
		}
		if (mode == u"add")
		{
			if (parts.size() < 3)
			{
				sendError(u"Usage: time add <ticks>");
				return;
			}
			long_t ticks = 0;
			if (!parseLong(parts[2], ticks))
			{
				sendError(u"Invalid time value: " + parts[2]);
				return;
			}
			mc.level->setTime(mc.level->time + ticks);
			addMessage(u"\u00a77Added " + String::toString(ticks) + u" ticks");
			return;
		}
		if (mode == u"get")
		{
			long_t day = mc.level->time / Level::TICKS_PER_DAY;
			long_t ticks = mc.level->time % Level::TICKS_PER_DAY;
			addMessage(u"\u00a77Day " + String::toString(day) + u", tick " + String::toString(ticks));
			return;
		}
		if (mode == u"speed")
		{
			if (parts.size() < 3)
			{
				addMessage(u"\u00a77Time speed: " + String::toString(gTimeSpeed));
				return;
			}
			double value = 0.0;
			if (!parseDouble(parts[2], value) || value <= 0.0)
			{
				sendError(u"Invalid time speed: " + parts[2]);
				return;
			}
			gTimeSpeed = value;
			addMessage(u"\u00a77Time speed set to " + String::toString(gTimeSpeed) + u" (not yet hooked into tick rate)");
			return;
		}
		sendError(u"Usage: time [day|night|set|add|get|speed]");
		return;
	}

	if (cmd == u"spawn")
	{
		if (!requirePlayer(mc) || !requireLevel(mc))
			return;
		if (parts.size() < 2)
		{
			sendError(u"Usage: spawn <creature> [count]");
			return;
		}
		long_t count = 1;
		if (parts.size() >= 3 && (!parseLong(parts[2], count) || count <= 0))
		{
			sendError(u"Invalid count: " + parts[2]);
			return;
		}
		int_t spawned = 0;
		for (long_t i = 0; i < count; ++i)
		{
			std::shared_ptr<Entity> entity = createSpawnEntity(*mc.level, parts[1]);
			if (entity == nullptr)
			{
				sendError(u"Invalid mob: " + parts[1]);
				return;
			}
			double spawnX = mc.player->x + static_cast<double>(mc.level->random.nextInt(5));
			double spawnY = mc.player->y;
			double spawnZ = mc.player->z + static_cast<double>(mc.level->random.nextInt(5));
			entity->moveTo(spawnX, spawnY, spawnZ, mc.player->yRot, 0.0f);
			if (mc.level->addEntity(entity))
				++spawned;
		}
		addMessage(u"\u00a77Spawned " + String::toString(spawned) + u" " + lowerAscii(parts[1]) + u"(s)");
		return;
	}
	

	if (cmd == u"spawncontrol")
	{
		if (!requireLevel(mc))
			return;
		if (parts.size() < 2)
		{
			sendError(u"Usage: spawncontrol <all|animals|monsters>");
			return;
		}
		jstring mode = lowerAscii(parts[1]);
		if (mode == u"all")
		{
			animalSpawn = !animalSpawn;
			monstersSpawn = animalSpawn;
		}
		else if (mode == u"animal" || mode == u"animals" || mode == u"friendly")
		{
			animalSpawn = !animalSpawn;
		}
		else if (mode == u"monster" || mode == u"monsters" || mode == u"agressive" || mode == u"aggressive")
		{
			monstersSpawn = !monstersSpawn;
		}
		else
		{
			sendError(u"Usage: spawncontrol <all|animals|monsters>");
			return;
		}
		mc.level->setSpawnSettings(monstersSpawn, animalSpawn);
		addMessage(u"\u00a77Friendly mobs will " + jstring(animalSpawn ? u"now spawn" : u"not spawn") + u". Aggressive mobs will " + jstring(monstersSpawn ? u"now spawn" : u"not spawn") + u".");
		return;
	}
	

	if (matches(cmd, {u"setspawn", u"spawnpoint"}))
	{
		if (!requireLevel(mc))
			return;
		if (parts.size() >= 4)
		{
			long_t x = 0, y = 0, z = 0;
			if (!parseLong(parts[1], x) || !parseLong(parts[2], y) || !parseLong(parts[3], z))
			{
				sendError(u"Invalid coordinates");
				return;
			}
			mc.level->xSpawn = static_cast<int_t>(x);
			mc.level->ySpawn = static_cast<int_t>(y);
			mc.level->zSpawn = static_cast<int_t>(z);
			addMessage(u"\u00a77Spawn point set to " + String::toString(x) + u", " + String::toString(y) + u", " + String::toString(z));
			return;
		}
		if (!requirePlayer(mc))
			return;
		mc.level->xSpawn = Mth::floor(mc.player->x);
		mc.level->ySpawn = Mth::floor(mc.player->y);
		mc.level->zSpawn = Mth::floor(mc.player->z);
		addMessage(u"\u00a77Spawn point set to current position");
		return;
	}

	if (cmd == u"seed")
	{
		if (!requireLevel(mc))
			return;
		addMessage(u"\u00a77Seed: " + String::toString(mc.level->seed));
		return;
	}

	if (matches(cmd, {u"difficulty", u"diff"}))
	{
		if (!requireLevel(mc))
			return;
		if (parts.size() < 2)
		{
			addMessage(u"\u00a77Difficulty: " + String::toString(mc.level->difficulty));
			return;
		}
		long_t difficulty = 0;
		if (!parseLong(parts[1], difficulty) || difficulty < 0 || difficulty > 3)
		{
			sendError(u"Difficulty must be 0-3");
			return;
		}
		mc.level->difficulty = static_cast<int_t>(difficulty);
		addMessage(u"\u00a77Difficulty set to " + String::toString(difficulty));
		return;
	}

	if (cmd == u"instantmine")
	{
		instantMine = !instantMine;
		if (mc.gameMode != nullptr)
			mc.gameMode->instaBuild = instantMine;
		announceToggle(u"Instant mine", instantMine);
		return;
	}

	if (matches(cmd, {u"longer", u"longerlegs"}))
	{
		if (!requirePlayer(mc))
			return;
		longerLegs = !longerLegs;
		mc.player->footSize = longerLegs ? 1.0f : 0.5f;
		announceToggle(u"Longer legs", longerLegs);
		return;
	}

	if (matches(cmd, {u"extinguish", u"ext"}))
	{
		if (!requirePlayer(mc))
			return;
		mc.player->onFire = 0;
		addMessage(u"\u00a77Extinguished player");
		return;
	}

	if (cmd == u"clear")
	{
		messages.clear();
		return;
	}

	if (cmd == u"msg")
	{
		if (parts.size() < 2)
		{
			sendError(u"Usage: msg <text>");
			return;
		}
		addMessage(joinParts(parts, 1));
		return;
	}

	if (matches(cmd, {u"repair"}))
	{
		if (!requirePlayer(mc))
			return;
		bool all = parts.size() >= 2 && lowerAscii(parts[1]) == u"all";
		int_t repaired = 0;
		for (ItemInstance &stack : mc.player->inventory.mainInventory)
		{
			if (stack.isEmpty())
				continue;
			if (!all && &stack != mc.player->inventory.getSelected())
				continue;
			if (stack.getMaxDamage() > 0 && stack.itemDamage != 0)
			{
				stack.itemDamage = 0;
				++repaired;
			}
		}
		addMessage(u"\u00a77Repaired " + String::toString(repaired) + u" item(s)");
		return;
	}

	if (cmd == u"destroy")
	{
		if (!requirePlayer(mc))
			return;
		bool all = parts.size() >= 2 && lowerAscii(parts[1]) == u"all";
		int_t destroyed = 0;
		for (ItemInstance &stack : mc.player->inventory.mainInventory)
		{
			if (stack.isEmpty())
				continue;
			if (!all && &stack != mc.player->inventory.getSelected())
				continue;
			stack = ItemInstance();
			++destroyed;
		}
		addMessage(u"\u00a77Destroyed " + String::toString(destroyed) + u" item stack(s)");
		return;
	}

	if (matches(cmd, {u"duplicate", u"dupe"}))
	{
		if (!requirePlayer(mc))
			return;
		ItemInstance *selected = mc.player->inventory.getSelected();
		if (selected == nullptr || selected->isEmpty())
		{
			sendError(u"No selected item to duplicate");
			return;
		}
		ItemInstance copy = *selected;
		if (!mc.player->inventory.add(copy))
		{
			sendError(u"Inventory full");
			return;
		}
		addMessage(u"\u00a77Duplicated selected stack");
		return;
	}

	if (cmd == u"refill")
	{
		if (!requirePlayer(mc))
			return;
		bool all = parts.size() >= 2 && lowerAscii(parts[1]) == u"all";
		int_t refilled = 0;
		for (ItemInstance &stack : mc.player->inventory.mainInventory)
		{
			if (stack.isEmpty())
				continue;
			if (!all && &stack != mc.player->inventory.getSelected())
				continue;
			int_t maxStack = stack.getMaxStackSize();
			if (stack.stackSize < maxStack)
			{
				stack.stackSize = maxStack;
				++refilled;
			}
		}
		addMessage(u"\u00a77Refilled " + String::toString(refilled) + u" stack(s)");
		return;
	}

	if (cmd == u"itemname")
	{
		if (!requirePlayer(mc))
			return;
		ItemInstance *selected = mc.player->inventory.getSelected();
		if (selected == nullptr || selected->isEmpty())
		{
			sendError(u"No selected item");
			return;
		}
		addMessage(u"\u00a77Selected item: " + describeId(selected->itemID) + u" (" + String::toString(selected->itemID) + u")");
		return;
	}

	if (cmd == u"itemstack")
	{
		if (!requirePlayer(mc))
			return;
		if (parts.size() < 2)
		{
			sendError(u"Usage: itemstack <item> [stacks]");
			return;
		}
		int_t id = resolveItemId(parts[1]);
		if (id < 0)
		{
			sendError(u"Unknown item: " + parts[1]);
			return;
		}
		long_t stacks = 1;
		if (parts.size() >= 3 && !parseLong(parts[2], stacks))
		{
			sendError(u"Invalid stack count: " + parts[2]);
			return;
		}
		ItemInstance proto(id, 1);
		int_t maxStack = proto.getMaxStackSize();
		int_t given = 0;
		for (long_t i = 0; i < stacks; ++i)
		{
			ItemInstance stack(id, maxStack);
			if (!mc.player->inventory.add(stack))
				break;
			++given;
		}
		addMessage(u"\u00a77Given " + String::toString(given) + u" stack(s) of " + describeId(id));
		return;
	}

	if (cmd == u"search")
	{
		if (parts.size() < 2)
		{
			sendError(u"Usage: search <text>");
			return;
		}
		jstring needle = normalizeName(parts[1]);
		int_t shown = 0;
		for (const auto &entry : itemAliases())
		{
			if (entry.first.find(needle) == jstring::npos)
				continue;
			addMessage(u"\u00a7e" + entry.first + u"\u00a77 -> " + String::toString(entry.second));
			if (++shown >= 15)
				break;
		}
		if (shown == 0)
			sendError(u"No matches for: " + parts[1]);
		return;
	}

	if (cmd == u"ascend")
	{
		if (!requirePlayer(mc) || !requireLevel(mc))
			return;
		int_t x = Mth::floor(mc.player->x);
		int_t z = Mth::floor(mc.player->z);
		int_t startY = Mth::floor(mc.player->y);
		for (int_t y = startY + 1; y < Level::DEPTH - 2; ++y)
		{
			if (mc.level->isSolidTile(x, y, z) && !mc.level->isSolidTile(x, y + 1, z) && !mc.level->isSolidTile(x, y + 2, z))
			{
				setCurrentPos(mc);
				mc.player->setPos(x + 0.5, y + 1.0, z + 0.5);
				addMessage(u"\u00a77Ascended to next platform");
				return;
			}
		}
		sendError(u"No platform found above");
		return;
	}

	if (cmd == u"descend")
	{
		if (!requirePlayer(mc) || !requireLevel(mc))
			return;
		int_t x = Mth::floor(mc.player->x);
		int_t z = Mth::floor(mc.player->z);
		int_t startY = Mth::floor(mc.player->y);
		for (int_t y = startY - 1; y >= 1; --y)
		{
			if (mc.level->isSolidTile(x, y, z) && !mc.level->isSolidTile(x, y + 1, z) && !mc.level->isSolidTile(x, y + 2, z))
			{
				setCurrentPos(mc);
				mc.player->setPos(x + 0.5, y + 1.0, z + 0.5);
				addMessage(u"\u00a77Descended to lower platform");
				return;
			}
		}
		sendError(u"No platform found below");
		return;
	}

	if (cmd == u"jump")
	{
		if (!requirePlayer(mc))
			return;
		HitResult hit = mc.player->pick(500.0f, 1.0f);
		if (hit.type != HitResult::Type::TILE)
		{
			sendError(u"No block targeted");
			return;
		}
		setCurrentPos(mc);
		mc.player->setPos(hit.x + 0.5, hit.y + 1.0, hit.z + 0.5);
		addMessage(u"\u00a77Jumped to target block");
		return;
	}

	if (cmd == u"removedrops")
	{
		if (!requirePlayer(mc) || !requireLevel(mc))
			return;
		AABB box(mc.player->x - 32.0, mc.player->y - 32.0, mc.player->z - 32.0, mc.player->x + 32.0, mc.player->y + 32.0, mc.player->z + 32.0);
		auto nearby = mc.level->getEntities(mc.player.get(), box);
		std::vector<std::shared_ptr<Entity>> toRemove;
		for (const auto &entity : nearby)
		{
			if (dynamic_cast<EntityItem *>(entity.get()) != nullptr)
				toRemove.push_back(entity);
		}
		for (const auto &entity : toRemove)
			mc.level->removeEntity(entity);
		addMessage(u"\u00a77Removed " + String::toString(static_cast<int_t>(toRemove.size())) + u" dropped item entities");
		return;
	}

	if (cmd == u"platform")
	{
		if (!requirePlayer(mc) || !requireLevel(mc))
			return;
		int_t x = Mth::floor(mc.player->x);
		int_t y = Mth::floor(mc.player->y) - 1;
		int_t z = Mth::floor(mc.player->z);
		mc.level->setTile(x, y, z, Tile::cobblestone.id);
		addMessage(u"\u00a77Placed cobblestone platform beneath player (glass not implemented yet)");
		return;
	}

	if (cmd == u"output")
	{
		bool newState = !outputEnabled;
		outputEnabled = true;
		pushMessage(u"\u00a77Output " + jstring(newState ? u"enabled" : u"disabled"));
		outputEnabled = newState;
		return;
	}

	if (cmd == u"reset")
	{
		flying = false;
		noclip = false;
		damageEnabled = true;
		fallDamage = true;
		fireDamage = true;
		waterDamage = true;
		instantMine = false;
		instantKill = false;
		freezeMobs = false;
		infiniteItems = false;
		keepItems = false;
		blockDrops = true;
		itemDamage = true;
		mobDamage = true;
		waterMovement = true;
		longerLegs = false;
		speed = 1.0;
		gravity = 1.0;
		reachDistance = 4.0;
		superPunch = 1.0;
		flySpeed = 1.0f;
		gTimeSpeed = 1.0;
		if (mc.player != nullptr)
		{
			mc.player->noPhysics = false;
			mc.player->footSize = 0.5f;
		}
		if (mc.gameMode != nullptr)
			mc.gameMode->instaBuild = false;
		addMessage(u"\u00a77SPC settings reset to defaults");
		return;
	}

	if (matches(cmd, {u"damage", u"drops", u"itemdamage", u"mobdamage", u"watermovement", u"waterdamage", u"instantkill", u"freeze", u"infiniteitems"}))
	{
		if (cmd == u"damage")
		{
			damageEnabled = !damageEnabled;
			announceToggle(u"Damage", damageEnabled);
			addMessage(u"\u00a76Note: damage toggle state is stored, but global damage hooks are not wired yet");
		}
		else if (cmd == u"drops")
		{
			blockDrops = !blockDrops;
			announceToggle(u"Block drops", blockDrops);
			addMessage(u"\u00a76Note: drop suppression is not wired into block breaking yet");
		}
		else if (cmd == u"itemdamage")
		{
			itemDamage = !itemDamage;
			announceToggle(u"Item damage", itemDamage);
			addMessage(u"\u00a76Note: durability suppression is not wired yet");
		}
		else if (cmd == u"mobdamage")
		{
			mobDamage = !mobDamage;
			announceToggle(u"Mob damage", mobDamage);
			addMessage(u"\u00a76Note: there are no mob-specific damage hooks yet");
		}
		else if (cmd == u"watermovement")
		{
			waterMovement = !waterMovement;
			announceToggle(u"Water movement", waterMovement);
			addMessage(u"\u00a76Note: water push toggling is not wired yet");
		}
		else if (cmd == u"waterdamage")
		{
			waterDamage = !waterDamage;
			announceToggle(u"Water damage", waterDamage);
			addMessage(u"\u00a76Note: water damage is not implemented in the engine yet");
		}
		else if (cmd == u"instantkill")
		{
			instantKill = !instantKill;
			announceToggle(u"Instant kill", instantKill);
			addMessage(u"\u00a76Note: attack hook for instant kill is not wired yet");
		}
		else if (cmd == u"freeze")
		{
			freezeMobs = !freezeMobs;
			announceToggle(u"Freeze mobs", freezeMobs);
			addMessage(u"\u00a76Note: mob AI freeze hook is not wired yet");
		}
		else if (cmd == u"infiniteitems")
		{
			infiniteItems = !infiniteItems;
			announceToggle(u"Infinite items", infiniteItems);
			addMessage(u"\u00a76Note: inventory consumption suppression is not wired yet");
		}
		return;
	}

	if (matches(cmd, {u"reach", u"setspeed", u"setjump"}))
	{
		if (parts.size() < 2)
		{
			sendError(u"Usage: " + cmd + u" <value>");
			return;
		}
		double value = 0.0;
		if (!parseDouble(parts[1], value))
		{
			sendError(u"Invalid value: " + parts[1]);
			return;
		}
		if (cmd == u"reach")
		{
			reachDistance = value;
			addMessage(u"\u00a77Reach set to " + String::toString(reachDistance) + u" (not yet hooked into pick range)");
		}
		else if (cmd == u"setspeed")
		{
			speed = value;
			addMessage(u"\u00a77Move speed set to " + String::toString(speed) + u" (not yet hooked into movement)");
		}
		else
		{
			gravity = value;
			addMessage(u"\u00a77Jump/gravity scale set to " + String::toString(gravity) + u" (not yet hooked into jump logic)");
		}
		return;
	}

	if (matches(cmd, {u"help", u"h", u"phelp"}))
	{
		addMessage(u"\u00a76--- SPC Commands ---");
		addMessage(u"\u00a7e give/i/item, tele/t, pos/p, set/s, goto, rem, listwaypoints/l");
		addMessage(u"\u00a7e home, return, heal, health, hurt, kill, seed, time, weather, difficulty");
		addMessage(u"\u00a7e fly, noclip, instantmine, longer, extinguish/ext, setspawn/spawnpoint");
		addMessage(u"\u00a7e repair, destroy, duplicate/dupe, refill, itemname, itemstack, search");
		addMessage(u"\u00a7e ascend, descend, jump, removedrops, platform, reset, output");
		addMessage(u"\u00a76Many remaining SPC names are registered as stubs and report when unimplemented.");
		return;
	}

	if (matches(cmd, {
		u"achievement", u"alias", u"atlantis", u"biome", u"bind", u"bindid", u"bring", u"cannon",
		u"chest", u"clearwater", u"clone", u"config", u"confuse", u"confusesuicide", u"cyclepainting",
		u"defuse", u"effect", u"enchant", u"ender", u"entity", u"excavate", u"explode", u"falldistance",
		u"fog", u"global", u"grow", u"instantplant", u"instantgrow", u"killnpc", u"kit", u"leash",
		u"light", u"macro", u"maxstack", u"move", u"music", u"nozzle", u"path", u"portal", u"redstone",
		u"refkill", u"remote", u"ride", u"shrink", u"sl", u"sprint", u"stacksize", u"startup",
		u"superheat", u"superpunch", u"temp", u"timeschedule", u"toggleedit", u"toggledropgive",
		u"togglepickaxe", u"world"}))
	{
		sendError(u"Not implemented yet: " + cmd);
		return;
	}

	sendError(u"Command not found - " + parts[0]);
}

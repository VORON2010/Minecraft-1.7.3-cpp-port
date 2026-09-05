#include "world/item/Items.h"

#include "world/item/Item.h"
#include "world/item/ItemAxe.h"
#include "world/item/ItemHoe.h"
#include "world/item/ItemMap.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/ItemSeeds.h"
#include "world/item/ItemSpade.h"
#include "world/item/ItemSword.h"
#include "world/item/ItemArmor.h"
#include "world/item/ItemFood.h"
#include "world/item/ItemSlab.h"
#include "world/item/ItemDye.h"
#include "world/item/ItemDoor.h"
#include "world/item/ItemFlintAndSteel.h"
#include "world/item/ItemBow.h"
#include "world/item/RecordItem.h"
#include "world/item/ItemSign.h"
#include "world/item/ItemRedStone.h"
#include "world/item/ItemRepeater.h"
#include "world/item/ItemMinecart.h"
#include "world/item/ItemShears.h"
#include "world/item/ItemSaddle.h"
#include "world/item/ItemBed.h"
#include "world/item/ItemBoat.h"
#include "world/item/ItemSoup.h"
#include "world/item/ItemBucket.h"
#include "world/item/ItemCake.h"
#include "world/item/ItemCoal.h"
#include "world/item/ItemSnowball.h"
#include "world/item/ItemEgg.h"
#include "world/item/ItemFishingRod.h"
#include "world/item/ItemPainting.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/CakeTile.h"
#include "world/level/tile/SaplingTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/LeafTile.h"
#include "world/level/tile/ClothTile.h"

namespace Items
{
	Item *flintAndSteel = nullptr;
	Item *apple = nullptr;
	Item *bow = nullptr;
	Item *arrow = nullptr;
	Item *ingotIron = nullptr;
	Item *ingotGold = nullptr;
	Item *swordWood = nullptr;
	Item *shovelWood = nullptr;
	ItemPickaxe *pickaxeWood = nullptr;
	Item *axeWood = nullptr;
	Item *stick = nullptr;
	Item *hoeWood = nullptr;
	Item *swordStone = nullptr;
	Item *shovelStone = nullptr;
	ItemPickaxe *pickaxeStone = nullptr;
	Item *axeStone = nullptr;
	Item *hoeStone = nullptr;
	Item *seeds = nullptr;
	Item *wheat = nullptr;
	Item *bread = nullptr;
	Item *cookie = nullptr;
	Item *porkchopRaw = nullptr;
	Item *porkchopCooked = nullptr;
	Item *painting = nullptr;
	Item *appleGold = nullptr;
	Item *reed = nullptr;
	Item *doorWood = nullptr;
	Item *doorIron = nullptr;
	Item *minecart = nullptr;
	Item *minecartPowered = nullptr;
	Item *minecartChest = nullptr;
	Item *boat = nullptr;
	Item *bed = nullptr;
	Item *map = nullptr;
	Item *cake = nullptr;
	Item *egg = nullptr;
	Item *saddle = nullptr;
	Item *compass = nullptr;
	Item *fishingRod = nullptr;
	Item *clock = nullptr;
	Item *record13 = nullptr;
	Item *recordCat = nullptr;
	Item *coal = nullptr;
	Item *diamond = nullptr;
	Item *redstone = nullptr;
	Item *dyePowder = nullptr;
	Item *flint = nullptr;
	Item *leather = nullptr;
	Item *silk = nullptr;
	Item *feather = nullptr;
	Item *gunpowder = nullptr;
	Item *bowlEmpty = nullptr;
	Item *bowlSoup = nullptr;
	Item *bucketEmpty = nullptr;
	Item *bucketWater = nullptr;
	Item *bucketLava = nullptr;
	Item *bucketMilk = nullptr;
	Item *brick = nullptr;
	Item *clayItem = nullptr;
	Item *paper = nullptr;
	Item *book = nullptr;
	Item *sugar = nullptr;
	Item *glowstoneDust = nullptr;
	Item *fishRaw = nullptr;
	Item *fishCooked = nullptr;
	Item *bone = nullptr;
	Item *sign = nullptr;
	Item *redstoneRepeater = nullptr;
	Item *snowball = nullptr;
	Item *slimeball = nullptr;
	Item *shears = nullptr;
	Item *swordIron = nullptr;
	Item *shovelIron = nullptr;
	ItemPickaxe *pickaxeIron = nullptr;
	Item *axeIron = nullptr;
	Item *hoeIron = nullptr;

	Item *swordDiamond = nullptr;
	Item *shovelDiamond = nullptr;
	ItemPickaxe *pickaxeDiamond = nullptr;
	Item *axeDiamond = nullptr;
	Item *hoeDiamond = nullptr;

	Item *swordGold = nullptr;
	Item *shovelGold = nullptr;
	ItemPickaxe *pickaxeGold = nullptr;
	Item *axeGold = nullptr;
	Item *hoeGold = nullptr;

	ItemArmor *helmetLeather = nullptr;
	ItemArmor *plateLeather = nullptr;
	ItemArmor *legsLeather = nullptr;
	ItemArmor *bootsLeather = nullptr;
	ItemArmor *helmetChain = nullptr;
	ItemArmor *plateChain = nullptr;
	ItemArmor *legsChain = nullptr;
	ItemArmor *bootsChain = nullptr;
	ItemArmor *helmetIron = nullptr;
	ItemArmor *plateIron = nullptr;
	ItemArmor *legsIron = nullptr;
	ItemArmor *bootsIron = nullptr;
	ItemArmor *helmetDiamond = nullptr;
	ItemArmor *plateDiamond = nullptr;
	ItemArmor *legsDiamond = nullptr;
	ItemArmor *bootsDiamond = nullptr;
	ItemArmor *helmetGold = nullptr;
	ItemArmor *plateGold = nullptr;
	ItemArmor *legsGold = nullptr;
	ItemArmor *bootsGold = nullptr;

	void initItems()
	{
		static bool initialized = false;
		if (initialized)
			return;
		initialized = true;

		
		for (int_t id : {Tile::sapling.id, Tile::treeTrunk.id, Tile::leaves.id, Tile::wool.id})
			Item::subtypeItems[id] = true;

		flintAndSteel = new ItemFlintAndSteel(3);

		apple = new ItemFood(4, 4, false);
		apple->setIconIndex(10).setDescriptionId(u"item.apple");

		bow = new ItemBow(5);
		bow->setIconIndex(21).setDescriptionId(u"item.bow");

		arrow = new Item(6);
		arrow->setIconIndex(37).setDescriptionId(u"item.arrow");

		ingotIron = new Item(9);
		ingotIron->setIconIndex(23).setDescriptionId(u"item.ingotIron");
		ingotGold = new Item(10);
		ingotGold->setIconIndex(39).setDescriptionId(u"item.ingotGold");


		swordWood = new ItemSword(12, ToolMaterialType::WOOD);
		swordWood->setIconIndex(64).setDescriptionId(u"item.swordWood");

		shovelWood = new ItemSpade(13, ToolMaterialType::WOOD);
		shovelWood->setIconIndex(80).setDescriptionId(u"item.shovelWood");

		pickaxeWood = new ItemPickaxe(14, ToolMaterialType::WOOD);
		pickaxeWood->setIconIndex(96).setDescriptionId(u"item.pickaxeWood");

		axeWood = new ItemAxe(15, ToolMaterialType::WOOD);
		axeWood->setIconIndex(112).setDescriptionId(u"item.hatchetWood");

		swordStone = new ItemSword(16, ToolMaterialType::STONE);
		swordStone->setIconIndex(65).setDescriptionId(u"item.swordStone");

		shovelStone = new ItemSpade(17, ToolMaterialType::STONE);
		shovelStone->setIconIndex(81).setDescriptionId(u"item.shovelStone");

		pickaxeStone = new ItemPickaxe(18, ToolMaterialType::STONE);
		pickaxeStone->setIconIndex(97).setDescriptionId(u"item.pickaxeStone");

		axeStone = new ItemAxe(19, ToolMaterialType::STONE);
		axeStone->setIconIndex(113).setDescriptionId(u"item.hatchetStone");

		
		swordIron = new ItemSword(11, ToolMaterialType::IRON);
		swordIron->setIconIndex(66).setDescriptionId(u"item.swordIron");

		shovelIron = new ItemSpade(0, ToolMaterialType::IRON);
		shovelIron->setIconIndex(82).setDescriptionId(u"item.shovelIron");

		pickaxeIron = new ItemPickaxe(1, ToolMaterialType::IRON);
		pickaxeIron->setIconIndex(98).setDescriptionId(u"item.pickaxeIron");

		axeIron = new ItemAxe(2, ToolMaterialType::IRON);
		axeIron->setIconIndex(114).setDescriptionId(u"item.hatchetIron");

		hoeIron = new ItemHoe(36, ToolMaterialType::IRON);
		hoeIron->setIconIndex(130).setDescriptionId(u"item.hoeIron");

		
		swordDiamond = new ItemSword(20, ToolMaterialType::DIAMOND);
		swordDiamond->setIconIndex(67).setDescriptionId(u"item.swordDiamond");

		shovelDiamond = new ItemSpade(21, ToolMaterialType::DIAMOND);
		shovelDiamond->setIconIndex(83).setDescriptionId(u"item.shovelDiamond");

		pickaxeDiamond = new ItemPickaxe(22, ToolMaterialType::DIAMOND);
		pickaxeDiamond->setIconIndex(99).setDescriptionId(u"item.pickaxeDiamond");

		axeDiamond = new ItemAxe(23, ToolMaterialType::DIAMOND);
		axeDiamond->setIconIndex(115).setDescriptionId(u"item.hatchetDiamond");

		hoeDiamond = new ItemHoe(37, ToolMaterialType::DIAMOND);
		hoeDiamond->setIconIndex(131).setDescriptionId(u"item.hoeDiamond");

		
		swordGold = new ItemSword(27, ToolMaterialType::GOLD);
		swordGold->setIconIndex(68).setDescriptionId(u"item.swordGold");

		shovelGold = new ItemSpade(28, ToolMaterialType::GOLD);
		shovelGold->setIconIndex(84).setDescriptionId(u"item.shovelGold");

		pickaxeGold = new ItemPickaxe(29, ToolMaterialType::GOLD);
		pickaxeGold->setIconIndex(100).setDescriptionId(u"item.pickaxeGold");

		axeGold = new ItemAxe(30, ToolMaterialType::GOLD);
		axeGold->setIconIndex(116).setDescriptionId(u"item.hatchetGold");

		hoeGold = new ItemHoe(38, ToolMaterialType::GOLD);
		hoeGold->setIconIndex(132).setDescriptionId(u"item.hoeGold");

		stick = new Item(24);
		stick->setIconIndex(53).setDescriptionId(u"item.stick");

		hoeWood = new ItemHoe(34, ToolMaterialType::WOOD);
		hoeWood->setIconIndex(128).setDescriptionId(u"item.hoeWood");

		hoeStone = new ItemHoe(35, ToolMaterialType::STONE);
		hoeStone->setIconIndex(129).setDescriptionId(u"item.hoeStone");

		seeds = new ItemSeeds(39, 59);
		seeds->setIconIndex(9).setDescriptionId(u"item.seeds");

		wheat = new Item(40);
		wheat->setIconIndex(25).setDescriptionId(u"item.wheat");

		saddle = new ItemSaddle(73);
		saddle->setIconIndex(104).setDescriptionId(u"item.saddle");

		bread = new ItemFood(41, 5, false);
		bread->setIconIndex(41).setDescriptionId(u"item.bread");

		helmetLeather = new ItemArmor(42, 0, 0, 0);
		helmetLeather->setIconIndex(0).setDescriptionId(u"item.helmetCloth");
		plateLeather = new ItemArmor(43, 0, 0, 1);
		plateLeather->setIconIndex(16).setDescriptionId(u"item.chestplateCloth");
		legsLeather = new ItemArmor(44, 0, 0, 2);
		legsLeather->setIconIndex(32).setDescriptionId(u"item.leggingsCloth");
		bootsLeather = new ItemArmor(45, 0, 0, 3);
		bootsLeather->setIconIndex(48).setDescriptionId(u"item.bootsCloth");
		helmetChain = new ItemArmor(46, 1, 1, 0);
		helmetChain->setIconIndex(1).setDescriptionId(u"item.helmetChain");
		plateChain = new ItemArmor(47, 1, 1, 1);
		plateChain->setIconIndex(17).setDescriptionId(u"item.chestplateChain");
		legsChain = new ItemArmor(48, 1, 1, 2);
		legsChain->setIconIndex(33).setDescriptionId(u"item.leggingsChain");
		bootsChain = new ItemArmor(49, 1, 1, 3);
		bootsChain->setIconIndex(49).setDescriptionId(u"item.bootsChain");
		helmetIron = new ItemArmor(50, 2, 2, 0);
		helmetIron->setIconIndex(2).setDescriptionId(u"item.helmetIron");
		plateIron = new ItemArmor(51, 2, 2, 1);
		plateIron->setIconIndex(18).setDescriptionId(u"item.chestplateIron");
		legsIron = new ItemArmor(52, 2, 2, 2);
		legsIron->setIconIndex(34).setDescriptionId(u"item.leggingsIron");
		bootsIron = new ItemArmor(53, 2, 2, 3);
		bootsIron->setIconIndex(50).setDescriptionId(u"item.bootsIron");
		helmetDiamond = new ItemArmor(54, 3, 3, 0);
		helmetDiamond->setIconIndex(3).setDescriptionId(u"item.helmetDiamond");
		plateDiamond = new ItemArmor(55, 3, 3, 1);
		plateDiamond->setIconIndex(19).setDescriptionId(u"item.chestplateDiamond");
		legsDiamond = new ItemArmor(56, 3, 3, 2);
		legsDiamond->setIconIndex(35).setDescriptionId(u"item.leggingsDiamond");
		bootsDiamond = new ItemArmor(57, 3, 3, 3);
		bootsDiamond->setIconIndex(51).setDescriptionId(u"item.bootsDiamond");
		helmetGold = new ItemArmor(58, 1, 4, 0);
		helmetGold->setIconIndex(4).setDescriptionId(u"item.helmetGold");
		plateGold = new ItemArmor(59, 1, 4, 1);
		plateGold->setIconIndex(20).setDescriptionId(u"item.chestplateGold");
		legsGold = new ItemArmor(60, 1, 4, 2);
		legsGold->setIconIndex(36).setDescriptionId(u"item.leggingsGold");
		bootsGold = new ItemArmor(61, 1, 4, 3);
		bootsGold->setIconIndex(52).setDescriptionId(u"item.bootsGold");

		porkchopRaw = new ItemFood(63, 3, true);
		porkchopRaw->setIconIndex(87).setDescriptionId(u"item.porkchopRaw");

		porkchopCooked = new ItemFood(64, 8, true);
		porkchopCooked->setIconIndex(88).setDescriptionId(u"item.porkchopCooked");

		painting = new ItemPainting(65);
		painting->setIconIndex(26).setDescriptionId(u"item.painting");

		appleGold = new ItemFood(66, 42, false);
		appleGold->setIconIndex(11).setDescriptionId(u"item.appleGold");

		flint = new Item(62);
		flint->setIconIndex(6).setDescriptionId(u"item.flint");

		reed = new Item(82);
		reed->setIconIndex(27).setDescriptionId(u"item.reeds");

		doorWood = new ItemDoor(68, Tile::doorWood);
		doorWood->setIconIndex(43).setDescriptionId(u"item.doorWood");

		doorIron = new ItemDoor(74, Tile::doorIron);
		doorIron->setIconIndex(44).setDescriptionId(u"item.doorIron");

		minecart = new ItemMinecart(72, 0);
		minecart->setIconIndex(135).setDescriptionId(u"item.minecart");

		minecartPowered = new ItemMinecart(87, 2);
		minecartPowered->setIconIndex(167).setDescriptionId(u"item.minecartFurnace");

		minecartChest = new ItemMinecart(86, 1);
		minecartChest->setIconIndex(151).setDescriptionId(u"item.minecartChest");

		boat = new ItemBoat(77);
		boat->setIconIndex(136).setDescriptionId(u"item.boat");

		compass = new Item(89);
		compass->setIconIndex(54).setDescriptionId(u"item.compass");

		fishingRod = new ItemFishingRod(90);
		fishingRod->setIconIndex(69).setDescriptionId(u"item.fishingRod");

		clock = new Item(91);
		clock->setIconIndex(70).setDescriptionId(u"item.clock");

		record13 = new RecordItem(2000, u"13");
		record13->setIconIndex(240).setDescriptionId(u"item.record");

		recordCat = new RecordItem(2001, u"cat");
		recordCat->setIconIndex(241).setDescriptionId(u"item.record");

		coal = new ItemCoal(7);
		coal->setIconIndex(7).setDescriptionId(u"item.coal");

		diamond = new Item(8);
		diamond->setIconIndex(55).setDescriptionId(u"item.emerald");

		redstone = new ItemRedStone(75);
		redstone->setIconIndex(56).setDescriptionId(u"item.redstone");

		dyePowder = new ItemDye(95);
		dyePowder->setIconIndex(78);

		leather = new Item(78);
		leather->setIconIndex(103).setDescriptionId(u"item.leather");

		silk = new Item(31);
		silk->setIconIndex(8).setDescriptionId(u"item.string");

		feather = new Item(32);
		feather->setIconIndex(24).setDescriptionId(u"item.feather");

		gunpowder = new Item(33);
		gunpowder->setIconIndex(40).setDescriptionId(u"item.sulphur");

		bowlEmpty = new Item(25);
		bowlEmpty->setIconIndex(71).setDescriptionId(u"item.bowl");

		bowlSoup = new ItemSoup(26, 10);
		bowlSoup->setIconIndex(77).setDescriptionId(u"item.mushroomStew");

		bucketEmpty = new ItemBucket(69, 0);
		bucketEmpty->setIconIndex(74).setDescriptionId(u"item.bucket");

		bucketWater = new ItemBucket(70, Tile::water.id);
		bucketWater->setIconIndex(75).setDescriptionId(u"item.bucketWater").setContainerItem(*bucketEmpty);

		bucketLava = new ItemBucket(71, Tile::lava.id);
		bucketLava->setIconIndex(76).setDescriptionId(u"item.bucketLava").setContainerItem(*bucketEmpty);

		bucketMilk = new ItemBucket(79, -1);
		bucketMilk->setIconIndex(77).setDescriptionId(u"item.milk").setContainerItem(*bucketEmpty);

		brick = new Item(80);
		brick->setIconIndex(22).setDescriptionId(u"item.brick");

		clayItem = new Item(81);
		clayItem->setIconIndex(57).setDescriptionId(u"item.clay");

		paper = new Item(83);
		paper->setIconIndex(58).setDescriptionId(u"item.paper");

		book = new Item(84);
		book->setIconIndex(59).setDescriptionId(u"item.book");

		sugar = new Item(97);
		sugar->setIconIndex(13).setDescriptionId(u"item.sugar");

		glowstoneDust = new Item(92);
		glowstoneDust->setIconIndex(73).setDescriptionId(u"item.yellowDust");

		fishRaw = new ItemFood(93, 2, false);
		fishRaw->setIconIndex(89).setDescriptionId(u"item.fishRaw");

		fishCooked = new ItemFood(94, 5, false);
		fishCooked->setIconIndex(90).setDescriptionId(u"item.fishCooked");

		bone = new Item(96);
		bone->setIconIndex(28).setDescriptionId(u"item.bone");

		redstoneRepeater = new ItemRepeater(100);
		redstoneRepeater->setIconIndex(86).setDescriptionId(u"item.diode");

		cookie = new ItemFood(101, 1, false);
		cookie->setMaxStackSize(8).setIconIndex(92).setDescriptionId(u"item.cookie");

		snowball = new ItemSnowball(76);
		snowball->setIconIndex(14).setDescriptionId(u"item.snowball");

		slimeball = new Item(85);
		slimeball->setIconIndex(30).setDescriptionId(u"item.slimeball");

		shears = new ItemShears(103);
		shears->setIconIndex(93).setDescriptionId(u"item.shears");

		bed = new ItemBed(99);
		bed->setIconIndex(45).setDescriptionId(u"item.bed");

		map = new ItemMap(102);
		map->setIconIndex(60).setDescriptionId(u"item.map");

		cake = new ItemCake(98, Tile::cake);
		cake->setIconIndex(29).setDescriptionId(u"item.cake");

		egg = new ItemEgg(88);
		egg->setIconIndex(12).setDescriptionId(u"item.egg");

		sign = new ItemSign(67);
		sign->setIconIndex(42).setDescriptionId(u"item.sign");

		new ItemSlab(44 - 256);
	}
}

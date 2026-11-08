#include "world/entity/npc/Villager.h"

#include <cmath>
#include <map>
#include <utility>

#include "client/player/LocalPlayer.h"
#include "nbt/CompoundTag.h"
#include "world/entity/ai/Goals.h"
#include "world/entity/ai/PathNavigation.h"
#include "world/entity/effect/MobEffect.h"
#include "world/entity/monster/Monster.h"
#include "world/entity/monster/Zombie.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/item/PotionItem.h"
#include "world/item/enchantment/Enchantment.h"
#include "world/level/Level.h"
#include "world/level/tile/AllTiles.h"
#include "world/item/ItemArmor.h"
#include "world/item/ItemPickaxe.h"
#include "world/village/Village.h"
#include "util/Mth.h"

namespace
{
	// 1.5.2 VillagerEntity.BUY_OFFERS / SELL_OFFERS: item id -> (min, max) count
	using Range = std::pair<int_t, int_t>;

	std::map<int_t, Range> &buyOffers()
	{
		static std::map<int_t, Range> m;
		if (m.empty())
		{
			m[Items::coal->getShiftedIndex()] = {16, 24};
			m[Items::ingotIron->getShiftedIndex()] = {8, 10};
			m[Items::ingotGold->getShiftedIndex()] = {8, 10};
			m[Items::diamond->getShiftedIndex()] = {4, 6};
			m[Items::paper->getShiftedIndex()] = {24, 36};
			m[Items::book->getShiftedIndex()] = {11, 13};
			m[Items::writtenBook->getShiftedIndex()] = {1, 1};
			m[Items::enderPearl->getShiftedIndex()] = {3, 4};
			m[Items::eyeOfEnder->getShiftedIndex()] = {2, 3};
			m[Items::porkchopRaw->getShiftedIndex()] = {14, 18};
			m[Items::beefRaw->getShiftedIndex()] = {14, 18};
			m[Items::chickenRaw->getShiftedIndex()] = {14, 18};
			m[Items::fishCooked->getShiftedIndex()] = {9, 13};
			m[Items::seeds->getShiftedIndex()] = {34, 48};
			m[Items::melonSeeds->getShiftedIndex()] = {30, 38};
			m[Items::pumpkinSeeds->getShiftedIndex()] = {30, 38};
			m[Items::wheat->getShiftedIndex()] = {18, 22};
			m[Tile::wool.id] = {14, 22};
			m[Items::rottenFlesh->getShiftedIndex()] = {36, 64};
		}
		return m;
	}

	std::map<int_t, Range> &sellOffers()
	{
		static std::map<int_t, Range> m;
		if (m.empty())
		{
			m[Items::flintAndSteel->getShiftedIndex()] = {3, 4};
			m[Items::shears->getShiftedIndex()] = {3, 4};
			m[Items::swordIron->getShiftedIndex()] = {7, 11};
			m[Items::swordDiamond->getShiftedIndex()] = {12, 14};
			m[Items::axeIron->getShiftedIndex()] = {6, 8};
			m[Items::axeDiamond->getShiftedIndex()] = {9, 12};
			m[Items::pickaxeIron->getShiftedIndex()] = {7, 9};
			m[Items::pickaxeDiamond->getShiftedIndex()] = {10, 12};
			m[Items::shovelIron->getShiftedIndex()] = {4, 6};
			m[Items::shovelDiamond->getShiftedIndex()] = {7, 8};
			m[Items::hoeIron->getShiftedIndex()] = {4, 6};
			m[Items::hoeDiamond->getShiftedIndex()] = {7, 8};
			m[Items::bootsIron->getShiftedIndex()] = {4, 6};
			m[Items::bootsDiamond->getShiftedIndex()] = {7, 8};
			m[Items::helmetIron->getShiftedIndex()] = {4, 6};
			m[Items::helmetDiamond->getShiftedIndex()] = {7, 8};
			m[Items::plateIron->getShiftedIndex()] = {10, 14};
			m[Items::plateDiamond->getShiftedIndex()] = {16, 19};
			m[Items::legsIron->getShiftedIndex()] = {8, 10};
			m[Items::legsDiamond->getShiftedIndex()] = {11, 14};
			m[Items::bootsChain->getShiftedIndex()] = {5, 7};
			m[Items::helmetChain->getShiftedIndex()] = {5, 7};
			m[Items::plateChain->getShiftedIndex()] = {11, 15};
			m[Items::legsChain->getShiftedIndex()] = {9, 11};
			m[Items::bread->getShiftedIndex()] = {-4, -2};
			m[Items::melon->getShiftedIndex()] = {-8, -4};
			m[Items::apple->getShiftedIndex()] = {-8, -4};
			m[Items::cookie->getShiftedIndex()] = {-10, -7};
			m[Tile::glass.id] = {-5, -3};
			m[Tile::bookshelf.id] = {3, 4};
			m[Items::plateLeather->getShiftedIndex()] = {4, 5};
			m[Items::bootsLeather->getShiftedIndex()] = {2, 4};
			m[Items::helmetLeather->getShiftedIndex()] = {2, 4};
			m[Items::legsLeather->getShiftedIndex()] = {2, 4};
			m[Items::saddle->getShiftedIndex()] = {6, 8};
			m[Items::expBottle->getShiftedIndex()] = {-4, -1};
			m[Items::redstone->getShiftedIndex()] = {-4, -1};
			m[Items::compass->getShiftedIndex()] = {10, 12};
			m[Items::clock->getShiftedIndex()] = {10, 12};
			m[Tile::glowstone.id] = {-3, -1};
			m[Items::porkchopCooked->getShiftedIndex()] = {-7, -5};
			m[Items::beefCooked->getShiftedIndex()] = {-7, -5};
			m[Items::chickenCooked->getShiftedIndex()] = {-8, -6};
			m[Items::eyeOfEnder->getShiftedIndex()] = {7, 11};
			m[Items::arrow->getShiftedIndex()] = {-12, -8};
		}
		return m;
	}

	int_t pickCount(std::map<int_t, Range> &table, int_t item, Random &random)
	{
		auto it = table.find(item);
		if (it == table.end())
			return 1;
		const Range &r = it->second;
		return r.first >= r.second ? r.first : r.first + random.nextInt(r.second - r.first);
	}

	int_t emerald() { return Items::emerald->getShiftedIndex(); }

	void addItemForTradeIn(MerchantRecipeList &list, int_t item, Random &random, float chance)
	{
		if (random.nextFloat() < chance)
			list.add(std::make_shared<MerchantRecipe>(ItemInstance(item, pickCount(buyOffers(), item, random), 0), ItemInstance(emerald(), 1, 0)));
	}

	void addItemForPurchase(MerchantRecipeList &list, int_t item, Random &random, float chance)
	{
		if (random.nextFloat() >= chance)
			return;
		int_t count = pickCount(sellOffers(), item, random);
		if (count < 0)
			list.add(std::make_shared<MerchantRecipe>(ItemInstance(emerald(), 1, 0), ItemInstance(item, -count, 0)));
		else
			list.add(std::make_shared<MerchantRecipe>(ItemInstance(emerald(), count, 0), ItemInstance(item, 1, 0)));
	}

	template <class T> int_t id(T *item) { return item->getShiftedIndex(); }

	bool isVillager(Entity &e) { return dynamic_cast<Villager *>(&e) != nullptr; }
	bool isMob(Entity &e) { return dynamic_cast<Mob *>(&e) != nullptr; }
	bool isZombie(Entity &e) { return dynamic_cast<Zombie *>(&e) != nullptr; }
}

Villager::Villager(Level &level) : Villager(level, 0)
{
}

Villager::Villager(Level &level, int_t profession) : AgableMob(level), profession(profession)
{
	textureName = u"/mob/villager/villager.png";
	runSpeed = 0.5f;
	health = 20;
	setSize(0.6f, 1.8f);
	getNavigation().setCanOpenDoors(true);
	getNavigation().setAvoidWater(true);
	goalSelector->addGoal(0, std::make_unique<FloatGoal>(*this));
	goalSelector->addGoal(1, std::make_unique<AvoidEntityGoal>(*this, isZombie, false, 8.0f, 0.3f, 0.35f));
	goalSelector->addGoal(1, std::make_unique<TradeWithPlayerGoal>(*this));
	goalSelector->addGoal(1, std::make_unique<LookAtTradingPlayerGoal>(*this));
	goalSelector->addGoal(2, std::make_unique<MoveIndoorsGoal>(*this));
	goalSelector->addGoal(3, std::make_unique<RestrictOpenDoorGoal>(*this));
	goalSelector->addGoal(4, std::make_unique<OpenDoorGoal>(*this, true));
	goalSelector->addGoal(5, std::make_unique<MoveTowardsRestrictionGoal>(*this, 0.3f));
	goalSelector->addGoal(6, std::make_unique<MakeLoveGoal>(*this));
	// 7: FollowGolemGoal comes with the iron golem
	goalSelector->addGoal(8, std::make_unique<PlayGoal>(*this, 0.32f));
	goalSelector->addGoal(9, std::make_unique<InteractGoal>(*this, [](Entity &) { return true; }, true, 3.0f, 1.0f));
	goalSelector->addGoal(9, std::make_unique<InteractGoal>(*this, isVillager, false, 5.0f, 0.02f));
	goalSelector->addGoal(9, std::make_unique<RandomStrollGoal>(*this, 0.3f));
	goalSelector->addGoal(10, std::make_unique<LookAtPlayerGoal>(*this, isMob, false, 8.0f));
}

void Villager::serverAiMobStep()
{
	if (--villageUpdateInterval <= 0)
	{
		Villages &villages = level.getVillages();
		villages.queryUpdateAround(Mth::floor(x), Mth::floor(y), Mth::floor(z));
		villageUpdateInterval = 70 + random.nextInt(50);
		village = villages.getClosestVillage(Mth::floor(x), Mth::floor(y), Mth::floor(z), 32);
		if (village == nullptr)
			clearRestriction();
		else
		{
			const TilePos &center = village->getCenter();
			restrictTo(center.x, center.y, center.z, static_cast<int_t>(village->getRadius() * 0.6f));
			if (rewardPlayersOnFirstVillage)
			{
				rewardPlayersOnFirstVillage = false;
				village->rewardAllPlayers(5);
			}
		}
	}

	if (!isTrading() && updateMerchantTimer > 0)
	{
		updateMerchantTimer--;
		if (updateMerchantTimer <= 0)
		{
			if (addRecipeOnUpdate)
			{
				if (offers->size() > 1)
				{
					for (auto &recipe : offers->recipes)
						if (recipe->isDeprecated())
							recipe->increaseMaxUses(random.nextInt(6) + random.nextInt(6) + 2);
				}
				addOffers(1);
				addRecipeOnUpdate = false;
				if (village != nullptr && hasLastPlayerTradeName)
				{
					sendEntityEvent(14);
					village->modifyStanding(lastPlayerTradeName, 1);
				}
			}
			addEffect(MobEffectInstance(MobEffect::regeneration.id, 200, 0));
		}
	}

	AgableMob::serverAiMobStep();
}

bool Villager::interact(Player &player)
{
	ItemInstance *held = player.inventory.getSelected();
	bool holdsSpawnEgg = held != nullptr && held->itemID == id(Items::spawnEgg);
	if (!holdsSpawnEgg && isAlive() && !isTrading() && !isBaby())
	{
		if (!level.isOnline)
		{
			setTradingPlayer(&player);
			if (auto *local = dynamic_cast<LocalPlayer *>(&player))
				local->startTrading(*this, u"");
		}
		return true;
	}
	return AgableMob::interact(player);
}

jstring Villager::getTexture()
{
	switch (profession)
	{
	case 0: return u"/mob/villager/farmer.png";
	case 1: return u"/mob/villager/librarian.png";
	case 2: return u"/mob/villager/priest.png";
	case 3: return u"/mob/villager/smith.png";
	case 4: return u"/mob/villager/butcher.png";
	default: return AgableMob::getTexture();
	}
}

bool Villager::canDespawn()
{
	return false;
}

jstring Villager::getAmbientSound()
{
	return u"mob.villager.default";
}

jstring Villager::getHurtSound()
{
	return u"mob.villager.defaulthurt";
}

jstring Villager::getDeathSound()
{
	return u"mob.villager.defaultdeath";
}

// 1.5.2 setAttacker: the village remembers who hit its villagers
bool Villager::hurt(Entity *source, int_t dmg)
{
	bool result = AgableMob::hurt(source, dmg);
	if (result && village != nullptr && source != nullptr)
	{
		std::shared_ptr<Entity> ref = level.getEntityRef(*source);
		if (auto mob = std::dynamic_pointer_cast<Mob>(ref))
			village->addAggressor(mob);
		if (auto *player = dynamic_cast<Player *>(source))
		{
			village->modifyStanding(player->name, isBaby() ? -3 : -1);
			if (isAlive())
				sendEntityEvent(13);
		}
	}
	return result;
}

void Villager::die(Entity *source)
{
	if (village != nullptr)
	{
		if (source != nullptr)
		{
			if (auto *player = dynamic_cast<Player *>(source))
				village->modifyStanding(player->name, -2);
			else if (dynamic_cast<Monster *>(source) != nullptr)
				village->resetNoBreedTimer();
		}
		else if (level.getNearestPlayer(*this, 16.0) != nullptr)
			village->resetNoBreedTimer();
	}
	AgableMob::die(source);
}

void Villager::notifyTrade(MerchantRecipe &recipe)
{
	recipe.increaseUses();
	if (recipe.isSame(*offers->recipes.back()))
	{
		updateMerchantTimer = 40;
		addRecipeOnUpdate = true;
		hasLastPlayerTradeName = tradingPlayer != nullptr;
		if (tradingPlayer != nullptr)
			lastPlayerTradeName = tradingPlayer->name;
	}
	if (recipe.getBuyA().itemID == emerald())
		riches += recipe.getBuyA().stackSize;
}

MerchantRecipeList *Villager::getOffers(Player &player)
{
	(void)player;
	if (offers == nullptr)
		addOffers(1);
	return offers.get();
}

float Villager::getRecipeChance(float baseChance)
{
	float chance = baseChance + baseRecipeChanceMod;
	return chance > 0.9f ? 0.9f - (chance - 0.9f) : chance;
}

void Villager::addOffers(int_t count)
{
	baseRecipeChanceMod = offers != nullptr ? Mth::sqrt(static_cast<float>(offers->size())) * 0.2f : 0.0f;

	MerchantRecipeList list;
	switch (profession)
	{
	case 0:
		addItemForTradeIn(list, id(Items::wheat), random, getRecipeChance(0.9f));
		addItemForTradeIn(list, Tile::wool.id, random, getRecipeChance(0.5f));
		addItemForTradeIn(list, id(Items::chickenRaw), random, getRecipeChance(0.5f));
		addItemForTradeIn(list, id(Items::fishCooked), random, getRecipeChance(0.4f));
		addItemForPurchase(list, id(Items::bread), random, getRecipeChance(0.9f));
		addItemForPurchase(list, id(Items::melon), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::apple), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::cookie), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::shears), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::flintAndSteel), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::chickenCooked), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::arrow), random, getRecipeChance(0.5f));
		if (random.nextFloat() < getRecipeChance(0.5f))
			list.add(std::make_shared<MerchantRecipe>(ItemInstance(Tile::gravel.id, 10, 0), ItemInstance(emerald(), 1, 0), ItemInstance(id(Items::flint), 4 + random.nextInt(2), 0)));
		break;
	case 1:
		addItemForTradeIn(list, id(Items::paper), random, getRecipeChance(0.8f));
		addItemForTradeIn(list, id(Items::book), random, getRecipeChance(0.8f));
		addItemForTradeIn(list, id(Items::writtenBook), random, getRecipeChance(0.3f));
		addItemForPurchase(list, Tile::bookshelf.id, random, getRecipeChance(0.8f));
		addItemForPurchase(list, Tile::glass.id, random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::compass), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::clock), random, getRecipeChance(0.2f));
		if (random.nextFloat() < getRecipeChance(0.07f))
		{
			std::vector<Enchantment *> all;
			for (Enchantment *e : Enchantment::enchantments)
				if (e != nullptr)
					all.push_back(e);
			Enchantment *enchantment = all[random.nextInt(static_cast<int_t>(all.size()))];
			int_t level = Mth::nextInt(random, enchantment->getMinLevel(), enchantment->getMaxLevel());
			ItemInstance book(id(Items::enchantedBook), 1, 0);
			EnchantedBookItem::addEnchantment(book, enchantment->id, level);
			int_t cost = 2 + random.nextInt(5 + level * 10) + 3 * level;
			list.add(std::make_shared<MerchantRecipe>(ItemInstance(id(Items::book), 1, 0), ItemInstance(emerald(), cost, 0), book));
		}
		break;
	case 2:
	{
		addItemForPurchase(list, id(Items::eyeOfEnder), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::expBottle), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::redstone), random, getRecipeChance(0.4f));
		addItemForPurchase(list, Tile::glowstone.id, random, getRecipeChance(0.3f));
		int_t enchantable[] = {id(Items::swordIron), id(Items::swordDiamond), id(Items::plateIron), id(Items::plateDiamond),
			id(Items::axeIron), id(Items::axeDiamond), id(Items::pickaxeIron), id(Items::pickaxeDiamond)};
		for (int_t item : enchantable)
		{
			if (random.nextFloat() < getRecipeChance(0.05f))
			{
				ItemInstance result(item, 1, 0);
				for (const EnchantmentHelper::Entry &e : EnchantmentHelper::selectEnchantment(random, result, 5 + random.nextInt(15)))
					result.enchant(e.enchantment->id, e.level);
				list.add(std::make_shared<MerchantRecipe>(ItemInstance(item, 1, 0), ItemInstance(emerald(), 2 + random.nextInt(3), 0), result));
			}
		}
		break;
	}
	case 3:
		addItemForTradeIn(list, id(Items::coal), random, getRecipeChance(0.7f));
		addItemForTradeIn(list, id(Items::ingotIron), random, getRecipeChance(0.5f));
		addItemForTradeIn(list, id(Items::ingotGold), random, getRecipeChance(0.5f));
		addItemForTradeIn(list, id(Items::diamond), random, getRecipeChance(0.5f));
		addItemForPurchase(list, id(Items::swordIron), random, getRecipeChance(0.5f));
		addItemForPurchase(list, id(Items::swordDiamond), random, getRecipeChance(0.5f));
		addItemForPurchase(list, id(Items::axeIron), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::axeDiamond), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::pickaxeIron), random, getRecipeChance(0.5f));
		addItemForPurchase(list, id(Items::pickaxeDiamond), random, getRecipeChance(0.5f));
		addItemForPurchase(list, id(Items::shovelIron), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::shovelDiamond), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::hoeIron), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::hoeDiamond), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::bootsIron), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::bootsDiamond), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::helmetIron), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::helmetDiamond), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::plateIron), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::plateDiamond), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::legsIron), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::legsDiamond), random, getRecipeChance(0.2f));
		addItemForPurchase(list, id(Items::bootsChain), random, getRecipeChance(0.1f));
		addItemForPurchase(list, id(Items::helmetChain), random, getRecipeChance(0.1f));
		addItemForPurchase(list, id(Items::plateChain), random, getRecipeChance(0.1f));
		addItemForPurchase(list, id(Items::legsChain), random, getRecipeChance(0.1f));
		break;
	case 4:
		addItemForTradeIn(list, id(Items::coal), random, getRecipeChance(0.7f));
		addItemForTradeIn(list, id(Items::porkchopRaw), random, getRecipeChance(0.5f));
		addItemForTradeIn(list, id(Items::beefRaw), random, getRecipeChance(0.5f));
		addItemForPurchase(list, id(Items::saddle), random, getRecipeChance(0.1f));
		addItemForPurchase(list, id(Items::plateLeather), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::bootsLeather), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::helmetLeather), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::legsLeather), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::porkchopCooked), random, getRecipeChance(0.3f));
		addItemForPurchase(list, id(Items::beefCooked), random, getRecipeChance(0.3f));
		break;
	}

	if (list.empty())
		addItemForTradeIn(list, id(Items::ingotGold), random, 1.0f);

	// Collections.shuffle
	for (size_t i = list.recipes.size(); i > 1; --i)
		std::swap(list.recipes[i - 1], list.recipes[random.nextInt(static_cast<int_t>(i))]);

	if (offers == nullptr)
		offers = std::make_unique<MerchantRecipeList>();
	for (int_t i = 0; i < count && i < static_cast<int_t>(list.size()); ++i)
		offers->addIfNewOrBetter(list.recipes[i]);
}

void Villager::sendEntityEvent(byte_t event)
{
	// singleplayer has no server to bounce the event back, so show it here
	level.broadcastEntityEvent(level.getEntityRef(*this), event);
	if (!level.isOnline)
		handleEntityEvent(event);
}

void Villager::handleEntityEvent(byte_t event)
{
	if (event == 12)
		addParticlesAroundSelf(u"heart");
	else if (event == 13)
		addParticlesAroundSelf(u"angryVillager");
	else if (event == 14)
		addParticlesAroundSelf(u"happyVillager");
	else
		AgableMob::handleEntityEvent(event);
}

void Villager::addParticlesAroundSelf(const jstring &name)
{
	for (int_t i = 0; i < 5; ++i)
	{
		double xa = random.nextGaussian() * 0.02;
		double ya = random.nextGaussian() * 0.02;
		double za = random.nextGaussian() * 0.02;
		level.addParticle(name, x + random.nextFloat() * bbWidth * 2.0f - bbWidth, y + 1.0 + random.nextFloat() * bbHeight,
			z + random.nextFloat() * bbWidth * 2.0f - bbWidth, xa, ya, za);
	}
}

void Villager::setEntityVariety()
{
	profession = level.random.nextInt(5);
}

std::shared_ptr<AgableMob> Villager::getBreedOffspring(AgableMob &partner)
{
	return getBreedOffspringVillager(partner);
}

std::shared_ptr<Villager> Villager::getBreedOffspringVillager(AgableMob &partner)
{
	(void)partner;
	auto child = std::make_shared<Villager>(level);
	child->setEntityVariety();
	return child;
}

void Villager::addAdditionalSaveData(CompoundTag &tag)
{
	AgableMob::addAdditionalSaveData(tag);
	tag.putInt(u"Profession", profession);
	tag.putInt(u"Riches", riches);
	if (offers != nullptr)
		tag.put(u"Offers", offers->createTag());
}

void Villager::readAdditionalSaveData(CompoundTag &tag)
{
	AgableMob::readAdditionalSaveData(tag);
	profession = tag.getInt(u"Profession");
	riches = tag.getInt(u"Riches");
	if (tag.contains(u"Offers"))
		offers = std::make_unique<MerchantRecipeList>(*tag.getCompound(u"Offers"));
}

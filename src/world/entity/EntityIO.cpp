#include "world/entity/projectile/ThrownPotion.h"
#include "world/entity/projectile/ThrownExpBottle.h"
#include "world/entity/animal/MushroomCow.h"
#include "world/entity/projectile/ThrownEnderpearl.h"
#include "world/entity/EyeOfEnderSignal.h"
#include "world/entity/EntityIO.h"

#include <unordered_map>

#include "world/entity/Entity.h"
#include "world/entity/Mob.h"
#include "world/entity/projectile/EntityArrow.h"
#include "world/entity/projectile/EntitySnowball.h"
#include "world/entity/projectile/EntityThrownEgg.h"
#include "world/entity/animal/Cow.h"
#include "world/entity/animal/Squid.h"
#include "world/entity/animal/Wolf.h"
#include "world/entity/monster/Monster.h"
#include "world/entity/monster/Zombie.h"
#include "world/entity/animal/Pig.h"
#include "world/entity/animal/Sheep.h"
#include "world/entity/animal/Chicken.h"
#include "world/entity/monster/Spider.h"
#include "world/entity/monster/Creeper.h"
#include "world/entity/monster/PigZombie.h"
#include "world/entity/monster/Skeleton.h"
#include "world/entity/monster/Slime.h"
#include "world/entity/monster/Giant.h"
#include "world/entity/monster/Ghast.h"
#include "world/entity/monster/Enderman.h"
#include "world/entity/monster/CaveSpider.h"
#include "world/entity/monster/Silverfish.h"
#include "world/entity/npc/Villager.h"
#include "world/entity/monster/Blaze.h"
#include "world/entity/monster/LavaSlime.h"
#include "world/entity/boss/EnderDragon.h"
#include "world/entity/item/EnderCrystal.h"
#include "world/entity/projectile/SmallFireball.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/item/ExperienceOrb.h"
#include "world/entity/item/EntityBoat.h"
#include "world/entity/item/EntityMinecart.h"
#include "world/entity/item/EntityPainting.h"
#include "world/entity/item/FallingTile.h"
#include "world/entity/PrimedTNT.h"
namespace EntityIO
{

#define ENTITYIO_ID(type, name, id) { name, [](Level &level) -> std::shared_ptr<Entity> { return Util::make_shared<type>(level); } },
static std::unordered_map<jstring, std::shared_ptr<Entity>(*)(Level &level)> idClassMap = {
#include "world/entity/EntityIDs.h"
};
#undef ENTITYIO_ID

#define ENTITYIO_ID(type, name, id) { id, [](Level &level) -> std::shared_ptr<Entity> { return Util::make_shared<type>(level); } },
static std::unordered_map<int_t, std::shared_ptr<Entity>(*)(Level &level)> numClassMap = {
#include "world/entity/EntityIDs.h"
};
#undef ENTITYIO_ID

std::shared_ptr<Entity> newEntity(const jstring &name, Level &level)
{
	auto it = idClassMap.find(name);
	if (it == idClassMap.end())
		return nullptr;
	return it->second(level);
}

std::shared_ptr<Entity> newEntity(int_t id, Level &level)
{
	if (id == 48)
	{

		std::cout << "Skipping Entity with id " << id << '\n';
		return nullptr;
	}
	auto it = numClassMap.find(id);
	if (it == numClassMap.end())
	{
		std::cout << "Skipping Entity with id " << id << '\n';
		return nullptr;
	}
	return it->second(level);
}

jstring getEncodeId(int_t id)
{
	static const std::unordered_map<int_t, jstring> names = {
#define ENTITYIO_ID(type, name, id) { id, name },
#include "world/entity/EntityIDs.h"
#undef ENTITYIO_ID
	};
	auto it = names.find(id);
	return it != names.end() ? it->second : jstring();
}

const std::vector<SpawnEggData> &getSpawnEggs()
{
	// 1.5.2 Entities.registerWithSpawnEgg, for the mobs this port has
	static const std::vector<SpawnEggData> eggs = {
		{50, 894731, 0}, {51, 12698049, 4802889}, {52, 3419431, 11013646}, {54, 44975, 7969893},
		{55, 5349438, 8306542}, {56, 16382457, 12369084}, {57, 15373203, 5009705}, {58, 1447446, 0},
		{59, 803406, 11013646}, {60, 7237230, 3158064}, {61, 16167425, 16775294}, {62, 3407872, 16579584},
		{90, 15771042, 14377823}, {91, 15198183, 16758197}, {92, 4470310, 10592673}, {93, 10592673, 16711680},
		{94, 2243405, 7375001}, {95, 14144467, 13545366}, {96, 10489616, 12040119}, {120, 5651507, 12422002},
	};
	return eggs;
}

const SpawnEggData *getSpawnEgg(int_t id)
{
	for (const SpawnEggData &egg : getSpawnEggs())
		if (egg.id == id)
			return &egg;
	return nullptr;
}

std::shared_ptr<Entity> loadStatic(CompoundTag &tag, Level &level)
{
	try
	{
		std::shared_ptr<Entity> entity = idClassMap.at(tag.getString(u"id"))(level);
		entity->load(tag);
		return entity;
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
	return nullptr;
}

}

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
#include "world/entity/item/EntityItem.h"
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

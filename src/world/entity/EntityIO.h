#pragma once

#include <memory>
#include <vector>

#include "nbt/CompoundTag.h"

#include "java/Type.h"
#include "java/String.h"

#include "util/Memory.h"

class Entity;
class Level;

class EntityIO_detail;

namespace EntityIO
{

std::shared_ptr<Entity> loadStatic(CompoundTag &tag, Level &level);
std::shared_ptr<Entity> newEntity(const jstring &name, Level &level);
std::shared_ptr<Entity> newEntity(int_t id, Level &level);

// 1.5.2 Entities.getKey / SPAWN_EGG_DATA
jstring getEncodeId(int_t id);
struct SpawnEggData
{
	int_t id;
	int_t baseColor;
	int_t spotsColor;
};
const SpawnEggData *getSpawnEgg(int_t id);
const std::vector<SpawnEggData> &getSpawnEggs();

}

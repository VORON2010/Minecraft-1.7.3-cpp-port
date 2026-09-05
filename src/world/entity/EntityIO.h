#pragma once

#include <memory>

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

}

#pragma once

#include "java/Type.h"
#include "java/Random.h"

class Level;
class Entity;

class Teleporter
{
private:
	Random random;

public:
	void teleport(Level &level, Entity &entity);
	bool findPortal(Level &level, Entity &entity);
	bool createPortal(Level &level, Entity &entity);
};

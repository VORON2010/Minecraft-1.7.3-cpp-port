#pragma once

#include "java/Type.h"

class Level;
class Player;

// Checks for the content ported from Beta 1.8 (blocks, items, mobs, status effects).
bool runB18Smoke(Level &level, Player &player, int_t baseY);

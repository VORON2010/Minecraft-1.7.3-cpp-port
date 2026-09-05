#pragma once

#include "java/Type.h"

class Player;

class Input
{
public:
	float xa = 0.0f;
	float ya = 0.0f;
	bool wasJumping = false;
	bool jumping = false;
	bool sneaking = false;
	bool sprinting = false;

	virtual ~Input() {}

	virtual void tick(Player &player);
	virtual void releaseAllKeys();
	virtual void setKey(int_t key, bool eventKeyState);
};

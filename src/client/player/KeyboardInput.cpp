#include "client/player/KeyboardInput.h"

#include "client/Options.h"
#include "world/entity/player/Player.h"

KeyboardInput::KeyboardInput(Options &options) : options(options)
{

}

void KeyboardInput::setKey(int_t key, bool state)
{
	int_t id = -1;
	if (key == options.keyUp.key) id = KEY_UP;
	if (key == options.keyDown.key) id = KEY_DOWN;
	if (key == options.keyLeft.key) id = KEY_LEFT;
	if (key == options.keyRight.key) id = KEY_RIGHT;
	if (key == options.keyJump.key) id = KEY_JUMP;
	if (key == options.keySneak.key) id = KEY_SNEAK;
	if (key == options.keySprint.key) id = KEY_SPRINT;
	if (id >= 0)
		keys[id] = state;
}

void KeyboardInput::releaseAllKeys()
{
	keys.fill(false);
	sprintActive = false;
	wasForwardKeyDown = false;
	sprintTriggerTime = 0;
}

void KeyboardInput::tick(Player &player)
{
	xa = 0.0f;
	ya = 0.0f;

	if (keys[KEY_UP]) ya++;
	if (keys[KEY_DOWN]) ya--;
	if (keys[KEY_LEFT]) xa++;
	if (keys[KEY_RIGHT]) xa--;

	wasJumping = jumping;
	jumping = keys[KEY_JUMP];
	sneaking = keys[KEY_SNEAK];

	bool forwardKeyDown = keys[KEY_UP];
	if (options.doubleTapSprint)
	{
		if (!wasForwardKeyDown && forwardKeyDown)
		{
			if (sprintTriggerTime > 0)
			{
				sprintActive = true;
				sprintTriggerTime = 0;
			}
			else
			{
				sprintTriggerTime = 7;
			}
		}
		if (sprintTriggerTime > 0)
			sprintTriggerTime--;
	}
	wasForwardKeyDown = forwardKeyDown;

	if (keys[KEY_SPRINT] && forwardKeyDown)
		sprintActive = true;

	if (sprintActive)
	{
		if (!forwardKeyDown || ya <= 0.0f || sneaking || player.horizontalCollision)
			sprintActive = false;
	}

	sprinting = sprintActive || (keys[KEY_SPRINT] && forwardKeyDown);

	if (sneaking)
	{
		xa *= 0.3f;
		ya *= 0.3f;
	}
}

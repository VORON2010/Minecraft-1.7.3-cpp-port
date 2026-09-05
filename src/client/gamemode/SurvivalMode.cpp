#include "client/gamemode/SurvivalMode.h"

#include "client/Minecraft.h"

#include <cmath>
#include "world/level/tile/StepSound.h"

SurvivalMode::SurvivalMode(Minecraft &minecraft) : GameMode(minecraft)
{

}

void SurvivalMode::initPlayer(std::shared_ptr<Player> player)
{
	player->yRot = -180.0f;
}

void SurvivalMode::init()
{

}

bool SurvivalMode::destroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	int_t t = minecraft.level->getTile(x, y, z);
	int_t data = minecraft.level->getData(x, y, z);
	Tile *tile = Tile::tiles[t];
	bool changed = GameMode::destroyBlock(x, y, z, face);
	bool couldDestroy = tile != nullptr && minecraft.player->canDestroy(*tile);
	if (changed)
	{
		ItemInstance *selected = minecraft.player->getSelectedItem();
		if (selected != nullptr)
		{
			selected->mineBlock(t, x, y, z, *minecraft.player);
			if (selected->isEmpty())
				minecraft.player->removeSelectedItem();
		}
		if (couldDestroy)
		{
			tile->playerDestroy(*minecraft.level, x, y, z, data);
			tile->harvestBlock(*minecraft.level, *minecraft.player, x, y, z, data);
		}
	}
	return changed;
}

void SurvivalMode::startDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	int_t t = minecraft.level->getTile(x, y, z);
	if (t > 0 && destroyProgress == 0.0f)
		Tile::tiles[t]->attack(*minecraft.level, x, y, z, *minecraft.player);
	
	if (t > 0 && Tile::tiles[t]->getDestroyProgress(*minecraft.player) >= 1.0f)
		destroyBlock(x, y, z, face);
}

void SurvivalMode::stopDestroyBlock()
{
	destroyProgress = 0.0f;
	destroyDelay = 0;
}

void SurvivalMode::continueDestroyBlock(int_t x, int_t y, int_t z, Facing face)
{
	if (destroyDelay > 0)
	{
		destroyDelay--;
		return;
	}

	if (x == xDestroyBlock && y == yDestroyBlock && z == zDestroyBlock)
	{
		int_t t = minecraft.level->getTile(x, y, z);
		if (t == 0) return;

		Tile &tile = *Tile::tiles[t];
		destroyProgress += tile.getDestroyProgress(*minecraft.player);
		if (std::fmod(destroyTicks, 4.0f) == 0.0f)
		{
			if (tile.soundType != nullptr)
			{
				StepSound *ss = tile.soundType;
				minecraft.soundEngine.play(ss->getStepResourcePath(),
					(float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f,
					(ss->getVolume() + 1.0f) / 8.0f, ss->getPitch() * 0.5f);
			}
		}

		destroyTicks++;

		if (destroyProgress >= 1.0f)
		{
			destroyBlock(x, y, z, face);
			destroyProgress = 0.0f;
			oDestroyProgress = 0.0f;
			destroyTicks = 0.0f;
			destroyDelay = 5;
		}
	}
	else
	{
		destroyProgress = 0.0f;
		oDestroyProgress = 0.0f;
		destroyTicks = 0.0f;
		xDestroyBlock = x;
		yDestroyBlock = y;
		zDestroyBlock = z;
	}
}

void SurvivalMode::render(float a)
{
	if (destroyProgress <= 0.0f)
	{
		minecraft.gui.progress = 0.0f;
		minecraft.levelRenderer.destroyProgress = 0.0f;
	}
	else
	{
		float dp = oDestroyProgress + (destroyProgress - oDestroyProgress) * a;
		minecraft.gui.progress = dp;
		minecraft.levelRenderer.destroyProgress = dp;
	}
}

float SurvivalMode::getPickRange()
{
	return 4.0f;
}

void SurvivalMode::initLevel(std::shared_ptr<Level> level)
{
	GameMode::initLevel(level);
}

void SurvivalMode::tick()
{
	oDestroyProgress = destroyProgress;
}

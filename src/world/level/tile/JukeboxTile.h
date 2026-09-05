#pragma once

#include "world/level/tile/Tile.h"

class JukeboxTile : public Tile
{
public:
	JukeboxTile(int_t id, int_t tex, const Material &material);

	int_t getTexture(Facing face, int_t data) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void insertRecord(Level &level, int_t x, int_t y, int_t z, int_t recordId);
	void ejectRecord(Level &level, int_t x, int_t y, int_t z);
};
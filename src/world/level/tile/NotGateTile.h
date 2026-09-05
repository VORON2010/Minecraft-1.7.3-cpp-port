#pragma once

#include "world/level/tile/TorchTile.h"
#include <vector>

class NotGateTile : public TorchTile
{
public:
	NotGateTile(int_t id, int_t tex, bool torchActive);

	int_t getTickDelay() override { return 2; }

	bool getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool isSignalSource() override { return true; }

	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;

	int_t getResource(int_t data, Random &random) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;

private:
	bool torchActive;
	static std::vector<struct RedstoneUpdateInfo> torchUpdates;

	bool checkForBurnout(Level &level, int_t x, int_t y, int_t z, bool logUpdate);
	bool isAttachedBlockPowered(Level &level, int_t x, int_t y, int_t z);
};
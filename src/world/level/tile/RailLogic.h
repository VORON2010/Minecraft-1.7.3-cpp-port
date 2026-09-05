#pragma once

#include <vector>

#include "world/level/TilePos.h"
#include "java/Type.h"

class Level;
class RailTile;

class RailLogic
{
private:
	Level &level;
	const RailTile &rail;
	int_t x = 0;
	int_t y = 0;
	int_t z = 0;
	bool poweredRail = false;
	std::vector<TilePos> connectedTracks;

	void setConnections(int_t data);
	void refreshConnectedTracks();
	bool isTrackNearby(int_t x, int_t y, int_t z) const;
	RailLogic *getRailLogic(const TilePos &pos) const;
	bool isConnectedTo(const RailLogic &other) const;
	bool containsTrack(int_t x, int_t y, int_t z) const;
	int_t getAdjacentTrackCountInternal() const;
	bool canConnectTo(const RailLogic &other) const;
	void connectTo(const RailLogic &other);
	bool canRailConnect(int_t x, int_t y, int_t z) const;

public:
	RailLogic(const RailTile &rail, Level &level, int_t x, int_t y, int_t z);
	~RailLogic();

	void place(bool receivingPower, bool forceUpdate);

	static int_t getAdjacentTrackCount(const RailTile &rail, Level &level, int_t x, int_t y, int_t z);
};

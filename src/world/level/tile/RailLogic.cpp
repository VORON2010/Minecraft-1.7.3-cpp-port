#include "world/level/tile/RailLogic.h"

#include "world/level/Level.h"
#include "world/level/tile/RailTile.h"

RailLogic::RailLogic(const RailTile &rail, Level &level, int_t x, int_t y, int_t z)
	: level(level), rail(rail), x(x), y(y), z(z), poweredRail(rail.isPoweredRail())
{
	int_t data = level.getData(x, y, z);
	if (poweredRail)
		data &= ~8;
	setConnections(data);
}

RailLogic::~RailLogic() = default;

void RailLogic::setConnections(int_t data)
{
	connectedTracks.clear();
	if (data == 0)
	{
		connectedTracks.emplace_back(x, y, z - 1);
		connectedTracks.emplace_back(x, y, z + 1);
	}
	else if (data == 1)
	{
		connectedTracks.emplace_back(x - 1, y, z);
		connectedTracks.emplace_back(x + 1, y, z);
	}
	else if (data == 2)
	{
		connectedTracks.emplace_back(x - 1, y, z);
		connectedTracks.emplace_back(x + 1, y + 1, z);
	}
	else if (data == 3)
	{
		connectedTracks.emplace_back(x - 1, y + 1, z);
		connectedTracks.emplace_back(x + 1, y, z);
	}
	else if (data == 4)
	{
		connectedTracks.emplace_back(x, y + 1, z - 1);
		connectedTracks.emplace_back(x, y, z + 1);
	}
	else if (data == 5)
	{
		connectedTracks.emplace_back(x, y, z - 1);
		connectedTracks.emplace_back(x, y + 1, z + 1);
	}
	else if (data == 6)
	{
		connectedTracks.emplace_back(x + 1, y, z);
		connectedTracks.emplace_back(x, y, z + 1);
	}
	else if (data == 7)
	{
		connectedTracks.emplace_back(x - 1, y, z);
		connectedTracks.emplace_back(x, y, z + 1);
	}
	else if (data == 8)
	{
		connectedTracks.emplace_back(x - 1, y, z);
		connectedTracks.emplace_back(x, y, z - 1);
	}
	else if (data == 9)
	{
		connectedTracks.emplace_back(x + 1, y, z);
		connectedTracks.emplace_back(x, y, z - 1);
	}
}

void RailLogic::refreshConnectedTracks()
{
	for (std::size_t i = 0; i < connectedTracks.size();)
	{
		RailLogic *logic = getRailLogic(connectedTracks[i]);
		if (logic != nullptr && logic->isConnectedTo(*this))
		{
			connectedTracks[i] = TilePos(logic->x, logic->y, logic->z);
			delete logic;
			++i;
		}
		else
		{
			delete logic;
			connectedTracks.erase(connectedTracks.begin() + i);
		}
	}
}

bool RailLogic::isTrackNearby(int_t trackX, int_t trackY, int_t trackZ) const
{
	if (RailTile::isRail(level, trackX, trackY, trackZ))
		return true;
	if (RailTile::isRail(level, trackX, trackY + 1, trackZ))
		return true;
	return RailTile::isRail(level, trackX, trackY - 1, trackZ);
}

RailLogic *RailLogic::getRailLogic(const TilePos &pos) const
{
	if (RailTile::isRail(level, pos.x, pos.y, pos.z))
		return new RailLogic(rail, level, pos.x, pos.y, pos.z);
	if (RailTile::isRail(level, pos.x, pos.y + 1, pos.z))
		return new RailLogic(rail, level, pos.x, pos.y + 1, pos.z);
	if (RailTile::isRail(level, pos.x, pos.y - 1, pos.z))
		return new RailLogic(rail, level, pos.x, pos.y - 1, pos.z);
	return nullptr;
}

bool RailLogic::isConnectedTo(const RailLogic &other) const
{
	for (const TilePos &pos : connectedTracks)
	{
		if (pos.x == other.x && pos.z == other.z)
			return true;
	}
	return false;
}

bool RailLogic::containsTrack(int_t trackX, int_t trackY, int_t trackZ) const
{
	(void)trackY;
	for (const TilePos &pos : connectedTracks)
	{
		if (pos.x == trackX && pos.z == trackZ)
			return true;
	}
	return false;
}

int_t RailLogic::getAdjacentTrackCountInternal() const
{
	int_t count = 0;
	if (isTrackNearby(x, y, z - 1)) ++count;
	if (isTrackNearby(x, y, z + 1)) ++count;
	if (isTrackNearby(x - 1, y, z)) ++count;
	if (isTrackNearby(x + 1, y, z)) ++count;
	return count;
}

bool RailLogic::canConnectTo(const RailLogic &other) const
{
	if (isConnectedTo(other))
		return true;
	if (connectedTracks.size() == 2)
		return false;
	if (connectedTracks.empty())
		return true;
	return true;
}

void RailLogic::connectTo(const RailLogic &other)
{
	connectedTracks.emplace_back(other.x, other.y, other.z);

	bool north = containsTrack(x, y, z - 1);
	bool south = containsTrack(x, y, z + 1);
	bool west = containsTrack(x - 1, y, z);
	bool east = containsTrack(x + 1, y, z);

	int_t shape = -1;
	if (north || south)
		shape = 0;
	if (west || east)
		shape = 1;
	if (!poweredRail)
	{
		if (south && east && !north && !west) shape = 6;
		if (south && west && !north && !east) shape = 7;
		if (north && west && !south && !east) shape = 8;
		if (north && east && !south && !west) shape = 9;
	}

	if (shape == 0)
	{
		if (RailTile::isRail(level, x, y + 1, z - 1)) shape = 4;
		if (RailTile::isRail(level, x, y + 1, z + 1)) shape = 5;
	}
	if (shape == 1)
	{
		if (RailTile::isRail(level, x + 1, y + 1, z)) shape = 2;
		if (RailTile::isRail(level, x - 1, y + 1, z)) shape = 3;
	}
	if (shape < 0)
		shape = 0;

	int_t data = shape;
	if (poweredRail)
		data = (level.getData(x, y, z) & 8) | shape;
	const_cast<RailTile &>(rail).setRailData(level, x, y, z, data);
}

bool RailLogic::canRailConnect(int_t trackX, int_t trackY, int_t trackZ) const
{
	RailLogic *logic = getRailLogic(TilePos(trackX, trackY, trackZ));
	if (logic == nullptr)
		return false;
	logic->refreshConnectedTracks();
	bool canConnect = logic->canConnectTo(*this);
	delete logic;
	return canConnect;
}

void RailLogic::place(bool receivingPower, bool forceUpdate)
{
	bool north = canRailConnect(x, y, z - 1);
	bool south = canRailConnect(x, y, z + 1);
	bool west = canRailConnect(x - 1, y, z);
	bool east = canRailConnect(x + 1, y, z);

	int_t shape = -1;
	if ((north || south) && !west && !east)
		shape = 0;
	if ((west || east) && !north && !south)
		shape = 1;

	if (!poweredRail)
	{
		if (south && east && !north && !west) shape = 6;
		if (south && west && !north && !east) shape = 7;
		if (north && west && !south && !east) shape = 8;
		if (north && east && !south && !west) shape = 9;
	}

	if (shape == -1)
	{
		if (north || south)
			shape = 0;
		if (west || east)
			shape = 1;

		if (!poweredRail)
		{
			if (receivingPower)
			{
				if (south && east) shape = 6;
				if (west && south) shape = 7;
				if (east && north) shape = 9;
				if (north && west) shape = 8;
			}
			else
			{
				if (north && west) shape = 8;
				if (east && north) shape = 9;
				if (west && south) shape = 7;
				if (south && east) shape = 6;
			}
		}
	}

	if (shape == 0)
	{
		if (RailTile::isRail(level, x, y + 1, z - 1)) shape = 4;
		if (RailTile::isRail(level, x, y + 1, z + 1)) shape = 5;
	}
	if (shape == 1)
	{
		if (RailTile::isRail(level, x + 1, y + 1, z)) shape = 2;
		if (RailTile::isRail(level, x - 1, y + 1, z)) shape = 3;
	}
	if (shape < 0)
		shape = 0;

	setConnections(shape);
	int_t data = shape;
	if (poweredRail)
		data = (level.getData(x, y, z) & 8) | shape;

	if (forceUpdate || level.getData(x, y, z) != data)
	{
		const_cast<RailTile &>(rail).setRailData(level, x, y, z, data);

		for (const TilePos &pos : connectedTracks)
		{
			RailLogic *logic = getRailLogic(pos);
			if (logic == nullptr)
				continue;
			logic->refreshConnectedTracks();
			if (logic->canConnectTo(*this))
				logic->connectTo(*this);
			delete logic;
		}
	}
}

int_t RailLogic::getAdjacentTrackCount(const RailTile &rail, Level &level, int_t x, int_t y, int_t z)
{
	return RailLogic(rail, level, x, y, z).getAdjacentTrackCountInternal();
}

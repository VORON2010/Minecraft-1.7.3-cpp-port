#include "world/level/MapCoord.h"

MapCoord::MapCoord(MapData &mapData, byte_t icon, byte_t x, byte_t z, byte_t rot)
	: mapData(mapData), icon(icon), x(x), z(z), rot(rot)
{
}

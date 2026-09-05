#include "world/entity/item/PaintingArt.h"

const std::array<PaintingArt, 25> PaintingArt::values = {{
	{u"Kebab", 16, 16, 0, 0},
	{u"Aztec", 16, 16, 16, 0},
	{u"Alban", 16, 16, 32, 0},
	{u"Aztec2", 16, 16, 48, 0},
	{u"Bomb", 16, 16, 64, 0},
	{u"Plant", 16, 16, 80, 0},
	{u"Wasteland", 16, 16, 96, 0},
	{u"Pool", 32, 16, 0, 32},
	{u"Courbet", 32, 16, 32, 32},
	{u"Sea", 32, 16, 64, 32},
	{u"Sunset", 32, 16, 96, 32},
	{u"Creebet", 32, 16, 128, 32},
	{u"Wanderer", 16, 32, 0, 64},
	{u"Graham", 16, 32, 16, 64},
	{u"Match", 32, 32, 0, 128},
	{u"Bust", 32, 32, 32, 128},
	{u"Stage", 32, 32, 64, 128},
	{u"Void", 32, 32, 96, 128},
	{u"SkullAndRoses", 32, 32, 128, 128},
	{u"Fighters", 64, 32, 0, 96},
	{u"Pointer", 64, 64, 0, 192},
	{u"Pigscene", 64, 64, 64, 192},
	{u"BurningSkull", 64, 64, 128, 192},
	{u"Skeleton", 64, 48, 192, 64},
	{u"DonkeyKong", 64, 48, 192, 112}
}};

const PaintingArt *PaintingArt::find(const jstring &title)
{
	for (const PaintingArt &art : values)
		if (art.title == title)
			return &art;
	return nullptr;
}

const PaintingArt &PaintingArt::kebab()
{
	return values[0];
}

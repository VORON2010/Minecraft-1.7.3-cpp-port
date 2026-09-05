#include <cmath>
#include "world/level/tile/entity/NoteTileEntity.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"

void NoteTileEntity::load(CompoundTag &tag)
{
	TileEntity::load(tag);
	note = tag.getByte(u"note");
	if (note < 0)
		note = 0;
	if (note > 24)
		note = 24;
}

void NoteTileEntity::save(CompoundTag &tag)
{
	TileEntity::save(tag);
	tag.putByte(u"note", note);
}

void NoteTileEntity::changePitch()
{
	note = static_cast<byte_t>((note + 1) % 25);
	setChanged();
}

void NoteTileEntity::triggerNote(Level &level, int_t x, int_t y, int_t z)
{
	if (!level.isEmptyTile(x, y + 1, z))
		return;

	const Material &below = level.getMaterial(x, y - 1, z);
	jstring instrument = u"harp";
	if (&below == &Material::stone)
		instrument = u"bd";
	else if (&below == &Material::sand)
		instrument = u"snare";
	else if (&below == &Material::glass)
		instrument = u"hat";
	else if (&below == &Material::wood)
		instrument = u"bassattack";

	float pitch = static_cast<float>(std::pow(2.0, (static_cast<double>(note) - 12.0) / 12.0));
	level.playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5, u"note." + instrument, 3.0f, pitch);
	level.addParticle(u"note", static_cast<double>(x) + 0.5, static_cast<double>(y) + 1.2, static_cast<double>(z) + 0.5, static_cast<double>(note) / 24.0, 0.0, 0.0);
}
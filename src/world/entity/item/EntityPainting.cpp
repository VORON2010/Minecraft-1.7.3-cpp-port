#include "world/entity/item/EntityPainting.h"

#include <memory>
#include <vector>

#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/entity/item/EntityItem.h"
#include "world/item/ItemInstance.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

EntityPainting::EntityPainting(Level &level) : Entity(level)
{
	heightOffset = 0.0f;
	setSize(0.5f, 0.5f);
}

EntityPainting::EntityPainting(Level &level, int_t x, int_t y, int_t z, int_t direction)
	: EntityPainting(level)
{
	xPosition = x;
	yPosition = y;
	zPosition = z;
	std::vector<const PaintingArt *> validArt;
	for (const PaintingArt &candidate : PaintingArt::values)
	{
		art = &candidate;
		setDirection(direction);
		if (survives())
			validArt.push_back(&candidate);
	}
	if (!validArt.empty())
		art = validArt[random.nextInt(static_cast<int_t>(validArt.size()))];
	setDirection(direction);
}

EntityPainting::EntityPainting(Level &level, int_t x, int_t y, int_t z, int_t direction, const jstring &title)
	: EntityPainting(level)
{
	xPosition = x;
	yPosition = y;
	zPosition = z;
	art = PaintingArt::find(title);
	setDirection(direction);
}

void EntityPainting::setDirection(int_t direction)
{
	this->direction = direction;
	yRotO = yRot = static_cast<float>(direction * 90);
	float halfX = static_cast<float>(art->sizeX);
	float halfY = static_cast<float>(art->sizeY);
	float halfZ = static_cast<float>(art->sizeX);
	if (direction != 0 && direction != 2)
		halfX = 0.5f;
	else
		halfZ = 0.5f;

	halfX /= 32.0f;
	halfY /= 32.0f;
	halfZ /= 32.0f;
	float px = xPosition + 0.5f;
	float py = yPosition + 0.5f;
	float pz = zPosition + 0.5f;
	float wallOffset = 9.0f / 16.0f;
	if (direction == 0)
		pz -= wallOffset;
	if (direction == 1)
		px -= wallOffset;
	if (direction == 2)
		pz += wallOffset;
	if (direction == 3)
		px += wallOffset;
	if (direction == 0)
		px -= getArtOffset(art->sizeX);
	if (direction == 1)
		pz += getArtOffset(art->sizeX);
	if (direction == 2)
		px += getArtOffset(art->sizeX);
	if (direction == 3)
		pz -= getArtOffset(art->sizeX);
	py += getArtOffset(art->sizeY);
	setPos(px, py, pz);
	float margin = -(0.1f / 16.0f);
	bb.set(px - halfX - margin, py - halfY - margin, pz - halfZ - margin,
		px + halfX + margin, py + halfY + margin, pz + halfZ + margin);
}

float EntityPainting::getArtOffset(int_t size) const
{
	if (size == 32)
		return 0.5f;
	return size == 64 ? 0.5f : 0.0f;
}

void EntityPainting::tick()
{
	if (checkInterval++ == 100 && !level.isOnline)
	{
		checkInterval = 0;
		if (!survives())
		{
			remove();
			dropPainting();
		}
	}
}

bool EntityPainting::survives()
{
	if (!level.getCubes(*this, bb).empty())
		return false;
	int_t width = art->sizeX / 16;
	int_t height = art->sizeY / 16;
	int_t tileX = xPosition;
	int_t tileY = yPosition;
	int_t tileZ = zPosition;
	if (direction == 0)
		tileX = Mth::floor(x - static_cast<float>(art->sizeX) / 32.0f);
	if (direction == 1)
		tileZ = Mth::floor(z - static_cast<float>(art->sizeX) / 32.0f);
	if (direction == 2)
		tileX = Mth::floor(x - static_cast<float>(art->sizeX) / 32.0f);
	if (direction == 3)
		tileZ = Mth::floor(z - static_cast<float>(art->sizeX) / 32.0f);
	tileY = Mth::floor(y - static_cast<float>(art->sizeY) / 32.0f);

	for (int_t ix = 0; ix < width; ix++)
	{
		for (int_t iy = 0; iy < height; iy++)
		{
			const Material &material = direction == 0 || direction == 2
				? level.getMaterial(tileX + ix, tileY + iy, zPosition)
				: level.getMaterial(xPosition, tileY + iy, tileZ + ix);
			if (!material.isSolid())
				return false;
		}
	}

	const auto &entities = level.getEntities(this, bb);
	for (const auto &entity : entities)
		if (dynamic_cast<EntityPainting *>(entity.get()) != nullptr)
			return false;
	return true;
}

bool EntityPainting::isPickable()
{
	return true;
}

bool EntityPainting::hurt(Entity *source, int_t dmg)
{
	(void)source;
	(void)dmg;
	if (!removed && !level.isOnline)
	{
		remove();
		markHurt();
		dropPainting();
	}
	return true;
}

void EntityPainting::move(double xd, double yd, double zd)
{
	if (!level.isOnline && xd * xd + yd * yd + zd * zd > 0.0)
	{
		remove();
		dropPainting();
	}
}

void EntityPainting::push(double xd, double yd, double zd)
{
	if (!level.isOnline && xd * xd + yd * yd + zd * zd > 0.0)
	{
		remove();
		dropPainting();
	}
}

void EntityPainting::dropPainting()
{
	level.addEntity(std::make_shared<EntityItem>(level, x, y, z,
		ItemInstance(Items::painting->getShiftedIndex(), 1, 0)));
}

void EntityPainting::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putByte(u"Dir", static_cast<byte_t>(direction));
	tag.putString(u"Motive", art->title);
	tag.putInt(u"TileX", xPosition);
	tag.putInt(u"TileY", yPosition);
	tag.putInt(u"TileZ", zPosition);
}

void EntityPainting::readAdditionalSaveData(CompoundTag &tag)
{
	direction = tag.getByte(u"Dir");
	xPosition = tag.getInt(u"TileX");
	yPosition = tag.getInt(u"TileY");
	zPosition = tag.getInt(u"TileZ");
	art = PaintingArt::find(tag.getString(u"Motive"));
	if (art == nullptr)
		art = &PaintingArt::kebab();
	setDirection(direction);
}

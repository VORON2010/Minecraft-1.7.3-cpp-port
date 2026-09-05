#include "world/entity/item/EntityMinecart.h"

#include <cmath>

#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/entity/item/EntityItem.h"
#include "client/player/LocalPlayer.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/item/Item.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/Level.h"
#include "world/level/tile/RailTile.h"
#include "world/level/tile/ChestTile.h"
#include "nbt/ListTag.h"
#include "world/level/tile/Tile.h"
#include "world/phys/Vec3.h"

constexpr int_t EntityMinecart::TYPE_RIDEABLE;
constexpr int_t EntityMinecart::TYPE_CHEST;
constexpr int_t EntityMinecart::TYPE_FURNACE;

namespace
{
	constexpr int_t TRACK_OFFSETS[10][2][3] = {
		{{0, 0, -1}, {0, 0, 1}},
		{{-1, 0, 0}, {1, 0, 0}},
		{{-1, -1, 0}, {1, 0, 0}},
		{{-1, 0, 0}, {1, -1, 0}},
		{{0, 0, -1}, {0, -1, 1}},
		{{0, -1, -1}, {0, 0, 1}},
		{{0, 0, 1}, {1, 0, 0}},
		{{0, 0, 1}, {-1, 0, 0}},
		{{0, 0, -1}, {-1, 0, 0}},
		{{0, 0, -1}, {1, 0, 0}},
	};

	std::shared_ptr<Entity> findSharedEntity(Level &level, Entity *self)
	{
		for (const auto &entity : level.getAllEntities())
		{
			if (entity.get() == self)
				return entity;
		}
		return nullptr;
	}

	void spawnDrop(Level &level, double x, double y, double z, const ItemInstance &stack)
	{
		auto entity = std::make_shared<EntityItem>(level, x, y, z, stack);
		entity->throwTime = 10;
		level.addEntity(entity);
	}

void dropCargo(Level &level, double x, double y, double z, std::array<ItemInstance, 27> &items)
{
	for (ItemInstance &stack : items)
	{
		while (!stack.isEmpty())
		{
			int_t amount = level.random.nextInt(21) + 10;
			if (amount > stack.stackSize)
				amount = stack.stackSize;
			float xo = level.random.nextFloat() * 0.8f + 0.1f;
			float yo = level.random.nextFloat() * 0.8f + 0.1f;
			float zo = level.random.nextFloat() * 0.8f + 0.1f;
			ItemInstance dropped = stack.remove(amount);
			auto entity = std::make_shared<EntityItem>(level, x + xo, y + yo, z + zo, dropped);
			float spread = 0.05f;
			entity->xd = (level.random.nextFloat() * 2.0f - 1.0f) * spread;
			entity->yd = (level.random.nextFloat() * 2.0f - 1.0f) * spread + 0.2f;
			entity->zd = (level.random.nextFloat() * 2.0f - 1.0f) * spread;
			level.addEntity(entity);
		}
		stack = ItemInstance();
	}
}

	double clampHorizontalSpeed(double speed)
	{
		if (speed < -0.4)
			return -0.4;
		if (speed > 0.4)
			return 0.4;
		return speed;
	}
}

EntityMinecart::EntityMinecart(Level &level) : Entity(level)
{
	setSize(0.98f, 0.7f);
	heightOffset = bbHeight / 2.0f;
	makeStepSound = false;
}

EntityMinecart::EntityMinecart(Level &level, double x, double y, double z, int_t minecartType)
	: EntityMinecart(level)
{
	setPos(x, y + heightOffset, z);
	xOld = xo = x;
	yOld = yo = y + heightOffset;
	zOld = zo = z;
	this->minecartType = minecartType;
}

AABB *EntityMinecart::getCollideAgainstBox(Entity &entity)
{
	return entity.bb.copy();
}

double EntityMinecart::getRideHeight()
{
	return -0.3;
}

bool EntityMinecart::hurt(Entity *source, int_t dmg)
{
	(void)source;
	if (level.isOnline)
	{
		
		return true;
	}
	if (removed)
		return true;

	minecartRockDirection = -minecartRockDirection;
	minecartTimeSinceHit = 10;
	markHurt();
	minecartCurrentDamage += dmg * 10;
	if (minecartCurrentDamage <= 40)
		return true;

	if (rider != nullptr)
		rider->ride(nullptr);

	spawnDrop(level, x, y, z, ItemInstance(Items::minecart->getShiftedIndex(), 1, 0));
	if (minecartType == TYPE_CHEST)
	{
		dropCargo(level, x, y, z, cargoItems);
		spawnDrop(level, x, y, z, ItemInstance(Tile::chest.id, 1, 0));
	}
	else if (minecartType == TYPE_FURNACE)
	{
		spawnDrop(level, x, y, z, ItemInstance(Tile::furnace.id, 1, 0));
	}

	remove();
	return true;
}

void EntityMinecart::animateHurt()
{
	minecartRockDirection = -minecartRockDirection;
	minecartTimeSinceHit = 10;
	minecartCurrentDamage += minecartCurrentDamage * 10;
}

bool EntityMinecart::interact(Player &player)
{
	if (minecartType == TYPE_RIDEABLE)
	{
		if (rider != nullptr && rider->isPlayer() && rider.get() != &player)
			return true;
		if (!level.isOnline)
		{
			auto self = findSharedEntity(level, this);
			if (self != nullptr)
				player.ride(self);
		}
		return true;
	}

	if (minecartType == TYPE_CHEST)
	{
		if (level.isOnline)
		{
			
			return true;
		}
		LocalPlayer *localPlayer = dynamic_cast<LocalPlayer *>(&player);
		if (localPlayer == nullptr)
			return false;
		auto self = std::dynamic_pointer_cast<EntityMinecart>(findSharedEntity(level, this));
		if (self == nullptr)
			return false;
		localPlayer->startChest(self);
		return true;
	}

	if (minecartType == TYPE_FURNACE)
	{
		ItemInstance *selected = player.getSelectedItem();
		if (selected != nullptr && !selected->isEmpty() && selected->itemID == Items::coal->getShiftedIndex())
		{
			selected->stackSize--;
			if (selected->isEmpty())
				player.removeSelectedItem();
			fuel += 1200;
		}
		pushX = x - player.x;
		pushZ = z - player.z;
		return true;
	}

	return false;
}

void EntityMinecart::tick()
{
	Entity::tick();

	if (minecartTimeSinceHit > 0)
		minecartTimeSinceHit--;
	if (minecartCurrentDamage > 0)
		minecartCurrentDamage--;

	if (level.isOnline && lerpSteps > 0)
	{
		setPos(x + (lerpX - x) / lerpSteps, y + (lerpY - y) / lerpSteps, z + (lerpZ - z) / lerpSteps);
		setRot(yRot + (lerpYaw - yRot) / lerpSteps, xRot + (lerpPitch - xRot) / lerpSteps);
		lerpSteps--;
		return;
	}

	xo = x;
	yo = y;
	zo = z;
	if (riding == nullptr)
		yd -= 0.04;

	int_t tileX = Mth::floor(x);
	int_t tileY = Mth::floor(y);
	int_t tileZ = Mth::floor(z);
	if (RailTile::isRail(level, tileX, tileY - 1, tileZ))
		tileY--;

	double maxSpeed = 0.4;
	bool emittingSmoke = false;
	double slopeAccel = 1.0 / 128.0;
	int_t tileId = level.getTile(tileX, tileY, tileZ);
	if (RailTile::isRail(tileId))
	{
		Vec3 *railPos = getPosOnTrack(x, y, z);
		int_t data = level.getData(tileX, tileY, tileZ);
		y = static_cast<double>(tileY) + static_cast<double>(heightOffset);
		bool boosting = false;
		bool braking = false;
		if (tileId == 27)
		{
			boosting = (data & 8) != 0;
			braking = !boosting;
		}
		if (tileId == 27 || tileId == 28)
			data &= 7;

		if (data >= 2 && data <= 5)
			y = static_cast<double>(tileY + 1) + static_cast<double>(heightOffset);
		if (data == 2)
			xd -= slopeAccel;
		if (data == 3)
			xd += slopeAccel;
		if (data == 4)
			zd += slopeAccel;
		if (data == 5)
			zd -= slopeAccel;

		const int_t (*track)[3] = TRACK_OFFSETS[data];
		double trackXDir = track[1][0] - track[0][0];
		double trackZDir = track[1][2] - track[0][2];
		double trackLen = std::sqrt(trackXDir * trackXDir + trackZDir * trackZDir);
		double along = xd * trackXDir + zd * trackZDir;
		if (along < 0.0)
		{
			trackXDir = -trackXDir;
			trackZDir = -trackZDir;
		}

		double speed = std::sqrt(xd * xd + zd * zd);
		xd = speed * trackXDir / trackLen;
		zd = speed * trackZDir / trackLen;
		if (braking)
		{
			double brakeSpeed = std::sqrt(xd * xd + zd * zd);
			if (brakeSpeed < 0.03)
			{
				xd = 0.0;
				yd = 0.0;
				zd = 0.0;
			}
			else
			{
				xd *= 0.5;
				yd = 0.0;
				zd *= 0.5;
			}
		}

		double offset = 0.0;
		double startX = tileX + 0.5 + track[0][0] * 0.5;
		double startZ = tileZ + 0.5 + track[0][2] * 0.5;
		double endX = tileX + 0.5 + track[1][0] * 0.5;
		double endZ = tileZ + 0.5 + track[1][2] * 0.5;
		trackXDir = endX - startX;
		trackZDir = endZ - startZ;
		if (trackXDir == 0.0)
		{
			x = tileX + 0.5;
			offset = z - tileZ;
		}
		else if (trackZDir == 0.0)
		{
			z = tileZ + 0.5;
			offset = x - tileX;
		}
		else
		{
			double relX = x - startX;
			double relZ = z - startZ;
			offset = (relX * trackXDir + relZ * trackZDir) * 2.0;
		}

		x = startX + trackXDir * offset;
		z = startZ + trackZDir * offset;
		setPos(x, y, z);

		double moveX = xd;
		double moveZ = zd;
		if (rider != nullptr)
		{
			moveX *= 0.75;
			moveZ *= 0.75;
		}
		moveX = clampHorizontalSpeed(moveX);
		moveZ = clampHorizontalSpeed(moveZ);
		move(moveX, 0.0, moveZ);
		if (track[0][1] != 0 && Mth::floor(x) - tileX == track[0][0] && Mth::floor(z) - tileZ == track[0][2])
			setPos(x, y + track[0][1], z);
		else if (track[1][1] != 0 && Mth::floor(x) - tileX == track[1][0] && Mth::floor(z) - tileZ == track[1][2])
			setPos(x, y + track[1][1], z);

		if (rider != nullptr)
		{
			xd *= 0.997;
			yd = 0.0;
			zd *= 0.997;
		}
		else
		{
			if (minecartType == TYPE_FURNACE)
			{
				double pushLen = std::sqrt(pushX * pushX + pushZ * pushZ);
				if (pushLen > 0.01)
				{
					emittingSmoke = true;
					pushX /= pushLen;
					pushZ /= pushLen;
					double accel = 0.04;
					xd *= 0.8;
					yd = 0.0;
					zd *= 0.8;
					xd += pushX * accel;
					zd += pushZ * accel;
				}
				else
				{
					xd *= 0.9;
					yd = 0.0;
					zd *= 0.9;
				}
			}
			xd *= 0.96;
			yd = 0.0;
			zd *= 0.96;
		}

		Vec3 *newRailPos = getPosOnTrack(x, y, z);
		if (newRailPos != nullptr && railPos != nullptr)
		{
			double slopeDelta = (railPos->y - newRailPos->y) * 0.05;
			double curSpeed = std::sqrt(xd * xd + zd * zd);
			if (curSpeed > 0.0)
			{
				xd = xd / curSpeed * (curSpeed + slopeDelta);
				zd = zd / curSpeed * (curSpeed + slopeDelta);
			}
			setPos(x, newRailPos->y, z);
		}

		int_t newTileX = Mth::floor(x);
		int_t newTileZ = Mth::floor(z);
		if (newTileX != tileX || newTileZ != tileZ)
		{
			double curSpeed = std::sqrt(xd * xd + zd * zd);
			xd = curSpeed * (newTileX - tileX);
			zd = curSpeed * (newTileZ - tileZ);
		}

		if (minecartType == TYPE_FURNACE)
		{
			double pushLen = std::sqrt(pushX * pushX + pushZ * pushZ);
			if (pushLen > 0.01 && xd * xd + zd * zd > 0.001)
			{
				pushX /= pushLen;
				pushZ /= pushLen;
				if (pushX * xd + pushZ * zd < 0.0)
				{
					pushX = 0.0;
					pushZ = 0.0;
				}
				else
				{
					pushX = xd;
					pushZ = zd;
				}
			}
		}

		if (boosting)
		{
			double boostSpeed = std::sqrt(xd * xd + zd * zd);
			if (boostSpeed > 0.01)
			{
				xd += xd / boostSpeed * 0.06;
				zd += zd / boostSpeed * 0.06;
			}
			else if (data == 1)
			{
				if (level.isBlockNormalCube(tileX - 1, tileY, tileZ))
					xd = 0.02;
				else if (level.isBlockNormalCube(tileX + 1, tileY, tileZ))
					xd = -0.02;
			}
			else if (data == 0)
			{
				if (level.isBlockNormalCube(tileX, tileY, tileZ - 1))
					zd = 0.02;
				else if (level.isBlockNormalCube(tileX, tileY, tileZ + 1))
					zd = -0.02;
			}
		}
	}
	else
	{
		xd = clampHorizontalSpeed(xd);
		zd = clampHorizontalSpeed(zd);
		if (onGround)
		{
			xd *= 0.5;
			yd *= 0.5;
			zd *= 0.5;
		}
		move(xd, yd, zd);
		if (!onGround)
		{
			xd *= 0.95;
			yd *= 0.95;
			zd *= 0.95;
		}
	}

	xRot = 0.0f;
	double dx = xo - x;
	double dz = zo - z;
	if (dx * dx + dz * dz > 0.001)
	{
		yRot = static_cast<float>(std::atan2(dz, dx) * 180.0 / Mth::PI);
		if (flipped)
			yRot += 180.0f;
	}

	double yawDelta = yRot - yRotO;
	while (yawDelta >= 180.0)
		yawDelta -= 360.0;
	while (yawDelta < -180.0)
		yawDelta += 360.0;
	if (yawDelta < -170.0 || yawDelta >= 170.0)
	{
		yRot += 180.0f;
		flipped = !flipped;
	}
	setRot(yRot, xRot);

	const auto &nearby = level.getEntities(this, *bb.grow(0.2, 0.0, 0.2));
	for (const auto &other : nearby)
	{
		if (other.get() != rider.get() && other->isPushable())
			other->push(*this);
	}

	if (rider != nullptr && rider->removed)
		rider = nullptr;

	if (emittingSmoke && random.nextInt(4) == 0)
	{
		fuel--;
		if (fuel < 0)
			pushX = pushZ = 0.0;
		level.addParticle(u"largesmoke", x, y + 0.8, z, 0.0, 0.0, 0.0);
	}
}

Vec3 *EntityMinecart::getPosOffs(double px, double py, double pz, double offset) const
{
	int_t tileX = Mth::floor(px);
	int_t tileY = Mth::floor(py);
	int_t tileZ = Mth::floor(pz);
	if (RailTile::isRail(level, tileX, tileY - 1, tileZ))
		tileY--;

	int_t tileId = level.getTile(tileX, tileY, tileZ);
	if (!RailTile::isRail(tileId))
		return nullptr;

	int_t data = level.getData(tileX, tileY, tileZ);
	if (tileId == 27 || tileId == 28)
		data &= 7;
		double railY = tileY;
	if (data >= 2 && data <= 5)
		railY = tileY + 1;

	const int_t (*track)[3] = TRACK_OFFSETS[data];
	double trackXDir = track[1][0] - track[0][0];
	double trackZDir = track[1][2] - track[0][2];
	double trackLen = std::sqrt(trackXDir * trackXDir + trackZDir * trackZDir);
	trackXDir /= trackLen;
	trackZDir /= trackLen;
	px += trackXDir * offset;
	pz += trackZDir * offset;
	if (track[0][1] != 0 && Mth::floor(px) - tileX == track[0][0] && Mth::floor(pz) - tileZ == track[0][2])
		railY += track[0][1];
	else if (track[1][1] != 0 && Mth::floor(px) - tileX == track[1][0] && Mth::floor(pz) - tileZ == track[1][2])
		railY += track[1][1];
	return getPosOnTrack(px, railY, pz);
}

Vec3 *EntityMinecart::getPosOnTrack(double px, double py, double pz) const
{
	int_t tileX = Mth::floor(px);
	int_t tileY = Mth::floor(py);
	int_t tileZ = Mth::floor(pz);
	if (RailTile::isRail(level, tileX, tileY - 1, tileZ))
		tileY--;

	int_t tileId = level.getTile(tileX, tileY, tileZ);
	if (!RailTile::isRail(tileId))
		return nullptr;

	int_t data = level.getData(tileX, tileY, tileZ);
	double railY = tileY;
	if (tileId == 27 || tileId == 28)
		data &= 7;
	if (data >= 2 && data <= 5)
		railY = tileY + 1;

	const int_t (*track)[3] = TRACK_OFFSETS[data];
	double offset = 0.0;
	double startX = tileX + 0.5 + track[0][0] * 0.5;
	double startY = tileY + 0.5 + track[0][1] * 0.5;
	double startZ = tileZ + 0.5 + track[0][2] * 0.5;
	double endX = tileX + 0.5 + track[1][0] * 0.5;
	double endY = tileY + 0.5 + track[1][1] * 0.5;
	double endZ = tileZ + 0.5 + track[1][2] * 0.5;
	double dx = endX - startX;
	double dy = (endY - startY) * 2.0;
	double dz = endZ - startZ;
	if (dx == 0.0)
	{
		px = tileX + 0.5;
		offset = pz - tileZ;
	}
	else if (dz == 0.0)
	{
		pz = tileZ + 0.5;
		offset = px - tileX;
	}
	else
	{
		double relX = px - startX;
		double relZ = pz - startZ;
		offset = (relX * dx + relZ * dz) * 2.0;
	}

	px = startX + dx * offset;
	railY = startY + dy * offset;
	pz = startZ + dz * offset;
	if (dy < 0.0)
		railY += 1.0;
	if (dy > 0.0)
		railY += 0.5;
	return Vec3::newTemp(px, railY, pz);
}

ItemInstance &EntityMinecart::getItem(int_t slot)
{
	return cargoItems[slot];
}

const ItemInstance &EntityMinecart::getItem(int_t slot) const
{
	return cargoItems[slot];
}

void EntityMinecart::setItem(int_t slot, const ItemInstance &item)
{
	cargoItems[slot] = item;
	if (!cargoItems[slot].isEmpty() && cargoItems[slot].stackSize > cargoItems[slot].getMaxStackSize())
		cargoItems[slot].stackSize = cargoItems[slot].getMaxStackSize();
}

ItemInstance EntityMinecart::removeItem(int_t slot, int_t count)
{
	if (cargoItems[slot].isEmpty() || count <= 0)
		return ItemInstance();
	return cargoItems[slot].remove(count);
}

int_t EntityMinecart::getContainerSize() const
{
	return static_cast<int_t>(cargoItems.size());
}

bool EntityMinecart::canUse(Player &player) const
{
	return !removed && player.distanceToSqr(x, y, z) <= 64.0;
}

jstring EntityMinecart::getName() const
{
	return u"Minecart";
}

void EntityMinecart::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putInt(u"Type", minecartType);
	if (minecartType == TYPE_CHEST)
	{
		auto items = std::make_shared<ListTag>();
		for (int_t i = 0; i < static_cast<int_t>(cargoItems.size()); ++i)
		{
			if (cargoItems[i].isEmpty())
				continue;
			auto entry = std::make_shared<CompoundTag>();
			entry->putByte(u"Slot", static_cast<byte_t>(i));
			cargoItems[i].save(*entry);
			items->add(entry);
		}
		tag.put(u"Items", items);
	}
	else if (minecartType == TYPE_FURNACE)
	{
		tag.putDouble(u"PushX", pushX);
		tag.putDouble(u"PushZ", pushZ);
		tag.putShort(u"Fuel", static_cast<short_t>(fuel));
	}
}

void EntityMinecart::readAdditionalSaveData(CompoundTag &tag)
{
	minecartType = tag.getInt(u"Type");
	if (minecartType == TYPE_CHEST)
	{
		cargoItems = {};
		auto items = tag.getList(u"Items");
		if (items != nullptr)
		{
			for (int_t i = 0; i < items->size(); ++i)
			{
				auto entry = std::dynamic_pointer_cast<CompoundTag>(items->get(i));
				if (entry == nullptr)
					continue;
				int_t slot = entry->getByte(u"Slot") & 255;
				if (slot >= 0 && slot < static_cast<int_t>(cargoItems.size()))
					cargoItems[slot].load(*entry);
			}
		}
	}
	else if (minecartType == TYPE_FURNACE)
	{
		pushX = tag.getDouble(u"PushX");
		pushZ = tag.getDouble(u"PushZ");
		fuel = tag.getShort(u"Fuel");
	}
}

void EntityMinecart::lerpTo(double x, double y, double z, float yRot, float xRot, int_t steps)
{
	lerpX = x;
	lerpY = y + heightOffset;
	lerpZ = z;
	lerpYaw = yRot;
	lerpPitch = xRot;
	lerpSteps = steps + 2;
}

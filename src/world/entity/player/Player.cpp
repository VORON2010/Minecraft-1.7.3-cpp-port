#include "world/entity/player/Player.h"

#include "nbt/ListTag.h"
#include "util/Mth.h"
#include "world/entity/item/EntityItem.h"
#include "world/entity/item/EntityBoat.h"
#include "world/entity/item/EntityMinecart.h"
#include "world/entity/animal/Pig.h"
#include "world/entity/monster/Monster.h"
#include "world/entity/projectile/EntityArrow.h"
#include "world/inventory/Container.h"
#include "world/inventory/ContainerMenus.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/BedTile.h"
#include "world/stats/AchievementList.h"
#include "world/stats/Achievement.h"
#include "world/stats/StatBase.h"
#include "world/stats/StatList.h"

Player::Player(Level &level) : Mob(level)
{
	inventorySlots = std::make_shared<ContainerPlayer>(inventory, !level.isOnline);
	craftingInventory = inventorySlots;
	dataWatcher.addObject(16, static_cast<byte_t>(0));
	heightOffset = 1.62f;
	moveTo(level.xSpawn + 0.5, level.ySpawn + 1, level.zSpawn + 0.5, 0.0f, 0.0f);
	health = 20;
	modelName = u"humanoid";
	rotOffs = 180.0f;
	flameTime = 20;
	textureName = u"/mob/char.png";
}

void Player::prepareCustomTextures()
{
	cloakTexture = u"http://s3.amazonaws.com/MinecraftCloaks/" + name + u".png";
	customTextureUrl2 = cloakTexture;
}

void Player::tick()
{
	if (sleeping)
	{
		sleepTimer++;
		if (sleepTimer > 100)
			sleepTimer = 100;

		if (!level.isOnline)
		{
			if (!isInBed())
				wakeUpPlayer(true, true, false);
			else if (level.isDay())
				wakeUpPlayer(false, true, true);
		}
	}
	else if (sleepTimer > 0)
	{
		sleepTimer++;
		if (sleepTimer >= 110)
			sleepTimer = 0;
	}

	Mob::tick();
	if (!level.isOnline && craftingInventory != nullptr && !craftingInventory->canUse(*this))
	{
		closeContainer();
		craftingInventory = inventorySlots;
	}

	xCloakO = xCloak;
	yCloakO = yCloak;
	zCloakO = zCloak;

	double dx = x - xCloak;
	double dy = y - yCloak;
	double dz = z - zCloak;
	constexpr double maxDelta = 10.0;

	if (dx > maxDelta || dx < -maxDelta)
		xCloakO = xCloak = x;
	if (dy > maxDelta || dy < -maxDelta)
		yCloakO = yCloak = y;
	if (dz > maxDelta || dz < -maxDelta)
		zCloakO = zCloak = z;

	xCloak += dx * 0.25;
	yCloak += dy * 0.25;
	zCloak += dz * 0.25;
	addStat(*StatList::minutesPlayedStat, 1);
	if (riding == nullptr)
		hasMinecartStart = false;
}

Player::SleepStatus Player::sleepInBedAt(int_t bx, int_t by, int_t bz)
{
	if (!level.isOnline)
	{
		if (sleeping || !isAlive())
			return SleepStatus::OTHER_PROBLEM;

		if (!level.dimension->mayRespawn())
			return SleepStatus::NOT_POSSIBLE_HERE;

		if (level.isDay())
			return SleepStatus::NOT_POSSIBLE_NOW;

		if (Mth::abs(static_cast<float>(x - bx)) > 3.0f || Mth::abs(static_cast<float>(y - by)) > 2.0f || Mth::abs(static_cast<float>(z - bz)) > 3.0f)
			return SleepStatus::TOO_FAR_AWAY;
	}

	setSize(0.2f, 0.2f);
	heightOffset = 0.2f;

	if (level.hasChunkAt(bx, by, bz))
	{
		int_t data = level.getData(bx, by, bz);
		int_t dir = data & 3;
		float offX = 0.5f;
		float offZ = 0.5f;
		switch (dir)
		{
			case 0: offZ = 0.9f; break;
			case 1: offX = 0.1f; break;
			case 2: offZ = 0.1f; break;
			case 3: offX = 0.9f; break;
		}

		bedViewX = 0.0f;
		bedViewZ = 0.0f;
		switch (dir)
		{
			case 0: bedViewZ = -1.8f; break;
			case 1: bedViewX = 1.8f; break;
			case 2: bedViewZ = 1.8f; break;
			case 3: bedViewX = -1.8f; break;
		}

		setPos(bx + offX, by + 15.0f / 16.0f, bz + offZ);
	}
	else
	{
		setPos(bx + 0.5f, by + 15.0f / 16.0f, bz + 0.5f);
	}

	sleeping = true;
	sleepTimer = 0;
	bedX = bx;
	bedY = by;
	bedZ = bz;
	xd = yd = zd = 0.0;

	if (!level.isOnline)
		level.updateAllPlayersSleepingFlag();

	return SleepStatus::OK;
}

void Player::wakeUpPlayer(bool immediately, bool updateSleepFlag, bool setSpawn)
{
	setSize(0.6f, 1.8f);
	resetHeight();

	int_t wx = bedX, wy = bedY, wz = bedZ;
	if (wy != 0 && level.getTile(wx, wy, wz) == Tile::bed.id)
	{
		BedTile::setBedOccupied(level, wx, wy, wz, false);
	}

	if (wy != 0)
	{
		int_t data = level.getData(wx, wy, wz);
		int_t dir = data & 3;
		int_t foundX = wx, foundY = wy, foundZ = wz;
		bool found = false;
		for (int_t step = 0; step <= 1; step++)
		{
			int_t sx = wx - BedTile::headBlockToFootBlockMap[dir][0] * step - 1;
			int_t sz = wz - BedTile::headBlockToFootBlockMap[dir][1] * step - 1;
			for (int_t dx = 0; dx <= 2; dx++)
			{
				for (int_t dz = 0; dz <= 2; dz++)
				{
					int_t cx = sx + dx;
					int_t cz = sz + dz;
					if (level.isBlockNormalCube(cx, wy - 1, cz) && level.isEmptyTile(cx, wy, cz) && level.isEmptyTile(cx, wy + 1, cz))
					{
						foundX = cx;
						foundY = wy;
						foundZ = cz;
						found = true;
						break;
					}
				}
				if (found) break;
			}
			if (found) break;
		}
		if (!found)
		{
			foundX = wx;
			foundY = wy + 1;
			foundZ = wz;
		}
		setPos(foundX + 0.5f, foundY + heightOffset + 0.1f, foundZ + 0.5f);
	}

	sleeping = false;
	if (!level.isOnline && updateSleepFlag)
		level.updateAllPlayersSleepingFlag();

	if (immediately)
		sleepTimer = 0;
	else
		sleepTimer = 100;
}

bool Player::isInBed() const
{
	return level.getTile(bedX, bedY, bedZ) == Tile::bed.id;
}

float Player::getBedOrientationInDegrees() const
{
	if (bedY != 0)
	{
		int_t data = level.getData(bedX, bedY, bedZ);
		int_t dir = data & 3;
		switch (dir)
		{
			case 0: return 90.0f;
			case 1: return 0.0f;
			case 2: return 270.0f;
			case 3: return 180.0f;
		}
	}
	return 0.0f;
}

void Player::closeContainer()
{
	craftingInventory = inventorySlots;
}

void Player::resetHeight()
{
	heightOffset = 1.62f;
}

void Player::rideTick()
{
	double oldX = x;
	double oldY = y;
	double oldZ = z;
	Mob::rideTick();
	oBob = bob;
	bob = 0.0f;

	if (riding != nullptr)
	{
		int_t distance = static_cast<int_t>(std::round(Mth::sqrt((x - oldX) * (x - oldX) + (y - oldY) * (y - oldY) + (z - oldZ) * (z - oldZ)) * 100.0f));
		if (distance > 0)
		{
			if (dynamic_cast<EntityMinecart *>(riding.get()) != nullptr)
			{
				addStat(*StatList::distanceByMinecartStat, distance);
				int_t currentX = Mth::floor(x);
				int_t currentY = Mth::floor(y);
				int_t currentZ = Mth::floor(z);
				if (!hasMinecartStart)
				{
					hasMinecartStart = true;
					minecartStartX = currentX;
					minecartStartY = currentY;
					minecartStartZ = currentZ;
				}
				else
				{
					double dx = currentX - minecartStartX;
					double dy = currentY - minecartStartY;
					double dz = currentZ - minecartStartZ;
					if (dx * dx + dy * dy + dz * dz >= 1000.0)
						addStat(*AchievementList::onARail, 1);
				}
			}
			else if (dynamic_cast<EntityBoat *>(riding.get()) != nullptr)
				addStat(*StatList::distanceByBoatStat, distance);
			else if (dynamic_cast<Pig *>(riding.get()) != nullptr)
				addStat(*StatList::distanceByPigStat, distance);
		}
	}
}

void Player::resetPos()
{
	heightOffset = 1.62f;
	setSize(0.6f, 1.8f);
	Mob::resetPos();
	health = 20;
	deathTime = 0;
}

void Player::updateAi()
{
	if (swinging)
	{
		if (++swingTime == SWING_DURATION)
		{
			swingTime = 0;
			swinging = false;
		}
	}
	else
	{
		swingTime = 0;
	}

	attackAnim = swingTime / static_cast<float>(SWING_DURATION);
}

void Player::aiStep()
{
	if (level.difficulty == 0 && health < MAX_HEALTH && (tickCount % 20) * 12 == 0)
		heal(1);

	inventory.tick();
	oBob = bob;
	Mob::aiStep();

	float targetBob = Mth::sqrt(xd * xd + zd * zd);
	float targetTilt = static_cast<float>(std::atan(-yd * 0.2f)) * 15.0f;

	if (targetBob > 0.1f)
		targetBob = 0.1f;
	if (!onGround || health <= 0)
		targetBob = 0.0f;
	if (onGround || health <= 0)
		targetTilt = 0.0f;

	bob += (targetBob - bob) * 0.4f;
	tilt += (targetTilt - tilt) * 0.8f;

	if (health > 0)
	{
		auto &es = level.getEntities(this, *bb.grow(1.0, 0.0, 1.0));
		for (const auto &e : es)
		{
			if (!e->removed)
				touch(*e);
		}
	}
}

void Player::touch(Entity &e)
{
	e.playerTouch(*this);
}

void Player::swing()
{
	swingTime = -1;
	swinging = true;
}

void Player::attack(const std::shared_ptr<Entity> &entity)
{
	int_t attackDamage = inventory.getAttackDamage(*entity);
	if (attackDamage <= 0)
		return;
	if (yd < 0.0)
		attackDamage++;

	entity->hurt(this, attackDamage);
	ItemInstance *selected = getSelectedItem();
	if (selected != nullptr && dynamic_cast<Mob *>(entity.get()) != nullptr)
	{
		selected->hurtEnemy(*entity, *this);
		if (selected->isEmpty())
			removeSelectedItem();
	}
	if (dynamic_cast<Mob *>(entity.get()) != nullptr)
		addStat(*StatList::damageDealtStat, attackDamage);
}

void Player::respawn()
{
}

void Player::triggerAchievement(const StatBase &stat)
{
	addStat(stat, 1);
}

void Player::addStat(const StatBase &stat, int_t amount)
{
}

float Player::getDestroySpeed(Tile &tile)
{
	float speed = inventory.getDestroySpeed(tile);
	if (isUnderLiquid(static_cast<const Material &>(Material::water)))
		speed /= 5.0f;
	if (!onGround)
		speed /= 5.0f;
	return speed;
}

bool Player::canDestroy(Tile &tile)
{
	return inventory.canDestroySpecial(tile);
}

void Player::readAdditionalSaveData(CompoundTag &tag)
{
	Mob::readAdditionalSaveData(tag);
	dimension = tag.getInt(u"Dimension");
	auto inventoryTag = tag.getList(u"Inventory");
	if (inventoryTag != nullptr)
		inventory.load(*inventoryTag);
	if (tag.contains(u"SpawnX") && tag.contains(u"SpawnY") && tag.contains(u"SpawnZ"))
	{
		playerSpawnPosition = std::make_unique<TilePos>(
			tag.getInt(u"SpawnX"), tag.getInt(u"SpawnY"), tag.getInt(u"SpawnZ"));
	}
	else
	{
		playerSpawnPosition.reset();
	}

	sleeping = tag.getBoolean(u"Sleeping");
	sleepTimer = tag.getShort(u"SleepTimer");
	if (sleeping)
	{
		bedX = Mth::floor(x);
		bedY = Mth::floor(y);
		bedZ = Mth::floor(z);
	}
}

void Player::addAdditionalSaveData(CompoundTag &tag)
{
	Mob::addAdditionalSaveData(tag);
	tag.putInt(u"Dimension", dimension);
	auto inventoryTag = std::make_shared<ListTag>();
	inventory.save(*inventoryTag);
	tag.put(u"Inventory", inventoryTag);
	if (playerSpawnPosition != nullptr)
	{
		tag.putInt(u"SpawnX", playerSpawnPosition->x);
		tag.putInt(u"SpawnY", playerSpawnPosition->y);
		tag.putInt(u"SpawnZ", playerSpawnPosition->z);
	}

	tag.putBoolean(u"Sleeping", sleeping);
	tag.putShort(u"SleepTimer", static_cast<short_t>(sleepTimer));
}

const TilePos *Player::getPlayerSpawnPosition() const
{
	return playerSpawnPosition.get();
}

void Player::setPlayerSpawnPosition(const TilePos *position)
{
	if (position == nullptr)
		playerSpawnPosition.reset();
	else
		playerSpawnPosition = std::make_unique<TilePos>(*position);
}

float Player::getHeadHeight()
{
	return 0.12f;
}

double Player::getRidingHeight()
{
	return heightOffset - 0.5;
}

void Player::die(Entity *source)
{
	Mob::die(source);
	setSize(0.2f, 0.2f);
	setPos(x, y, z);
	yd = 0.1f;
	inventory.dropAll();
	if (source != nullptr)
	{
		float angle = (hurtDir + yRot) * Mth::DEGRAD;
		xd = -Mth::cos(angle) * 0.1f;
		zd = -Mth::sin(angle) * 0.1f;
	}
	else
	{
		xd = 0.0;
		zd = 0.0;
	}
	heightOffset = 0.1f;
	addStat(*StatList::deathsStat, 1);
}

bool Player::hurt(Entity *source, int_t dmg)
{
	noActionTime = 0;

	if (health <= 0)
		return false;
	if (dynamic_cast<Monster *>(source) != nullptr || dynamic_cast<EntityArrow *>(source) != nullptr)
	{
		if (level.difficulty == 0)
			dmg = 0;
		else if (level.difficulty == 1)
			dmg = dmg / 3 + 1;
		else if (level.difficulty == 3)
			dmg = dmg * 3 / 2;
	}

	if (dmg == 0)
		return false;
	addStat(*StatList::damageTakenStat, dmg);
	return Mob::hurt(source, dmg);
}

void Player::actuallyHurt(int_t dmg)
{
	int_t armorValue = 25 - inventory.getArmorValue();
	int_t totalDamage = dmg * armorValue + dmgSpill;
	inventory.hurtArmor(dmg);
	dmg = totalDamage / 25;
	dmgSpill = totalDamage % 25;
	Mob::actuallyHurt(dmg);
}

void Player::interact(const std::shared_ptr<Entity> &entity)
{
	if (entity->interact(*this))
		return;

	ItemInstance *selected = getSelectedItem();
	if (selected != nullptr)
	{
		if (Mob *mob = dynamic_cast<Mob *>(entity.get()))
		{
			selected->saddleEntity(*mob);
			if (selected->isEmpty())
				removeSelectedItem();
		}
	}
}

void Player::take(Entity &entity, int_t count)
{
}

ItemInstance *Player::getSelectedItem()
{
	return inventory.getSelected();
}

void Player::removeSelectedItem()
{
	inventory.setItem(inventory.currentItem, ItemInstance());
}

void Player::drop()
{
	ItemInstance selected = inventory.removeItem(inventory.currentItem, 1);
	if (!selected.isEmpty())
		drop(selected, false);
}

void Player::drop(ItemInstance &stack)
{
	drop(stack, false);
}

void Player::drop(ItemInstance &stack, bool randomSpread)
{
	if (stack.isEmpty())
		return;

	double dropY = y - 0.3f + getHeadHeight();
	auto itemEntity = std::make_shared<EntityItem>(level, x, dropY, z, stack);
	itemEntity->throwTime = 40;

	if (randomSpread)
	{
		float spread = random.nextFloat() * 0.5f;
		float angle = random.nextFloat() * Mth::PI * 2.0f;
		itemEntity->xd = -Mth::sin(angle) * spread;
		itemEntity->zd = Mth::cos(angle) * spread;
		itemEntity->yd = 0.2f;
	}
	else
	{
		float speed = 0.3f;
		float yRotRad = yRot * Mth::DEGRAD;
		float xRotRad = xRot * Mth::DEGRAD;
		itemEntity->xd = -Mth::sin(yRotRad) * Mth::cos(xRotRad) * speed;
		itemEntity->zd = Mth::cos(yRotRad) * Mth::cos(xRotRad) * speed;
		itemEntity->yd = -Mth::sin(xRotRad) * speed + 0.1f;
		speed = 0.02f;
		float randomAngle = random.nextFloat() * Mth::PI * 2.0f;
		speed *= random.nextFloat();
		itemEntity->xd += Mth::cos(randomAngle) * speed;
		itemEntity->yd += (random.nextFloat() - random.nextFloat()) * 0.1f;
		itemEntity->zd += Mth::sin(randomAngle) * speed;
	}

	reallyDrop(itemEntity);
	addStat(*StatList::dropStat, 1);
}

void Player::reallyDrop(std::shared_ptr<EntityItem> itemEntity)
{
	level.addEntity(itemEntity);
}

void Player::awardKillScore(Entity &source, int_t score)
{
	Mob::awardKillScore(source, score);
	if (dynamic_cast<Player *>(&source) != nullptr)
		addStat(*StatList::playerKillsStat, 1);
	else
		addStat(*StatList::mobKillsStat, 1);
	if (dynamic_cast<Monster *>(&source) != nullptr)
		triggerAchievement(*AchievementList::killEnemy);
}

void Player::jumpFromGround()
{
	Mob::jumpFromGround();
	addStat(*StatList::jumpStat, 1);
}

void Player::travel(float xInput, float zInput)
{
	double oldX = x;
	double oldY = y;
	double oldZ = z;
	Mob::travel(xInput, zInput);

	double dx = x - oldX;
	double dy = y - oldY;
	double dz = z - oldZ;
	if (riding != nullptr)
		return;

	if (isUnderLiquid(Material::water))
	{
		int_t distance = static_cast<int_t>(std::round(Mth::sqrt(dx * dx + dy * dy + dz * dz) * 100.0f));
		if (distance > 0)
			addStat(*StatList::distanceDoveStat, distance);
	}
	else if (isInWater())
	{
		int_t distance = static_cast<int_t>(std::round(Mth::sqrt(dx * dx + dz * dz) * 100.0f));
		if (distance > 0)
			addStat(*StatList::distanceSwumStat, distance);
	}
	else if (onLadder())
	{
		if (dy > 0.0)
			addStat(*StatList::distanceClimbedStat, static_cast<int_t>(std::round(dy * 100.0)));
	}
	else if (onGround)
	{
		int_t distance = static_cast<int_t>(std::round(Mth::sqrt(dx * dx + dz * dz) * 100.0f));
		if (distance > 0)
			addStat(*StatList::distanceWalkedStat, distance);
	}
	else
	{
		int_t distance = static_cast<int_t>(std::round(Mth::sqrt(dx * dx + dz * dz) * 100.0f));
		if (distance > 25)
			addStat(*StatList::distanceFlownStat, distance);
	}
}

void Player::causeFallDamage(float distance)
{
	if (distance >= 2.0f)
		addStat(*StatList::distanceFallenStat,
			static_cast<int_t>(std::round(static_cast<double>(distance) * 100.0)));
	Mob::causeFallDamage(distance);
}

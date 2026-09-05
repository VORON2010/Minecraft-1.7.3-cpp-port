#include "world/entity/animal/Wolf.h"

#include "nbt/CompoundTag.h"
#include "world/entity/animal/Sheep.h"
#include "world/entity/player/Player.h"
#include "world/entity/projectile/EntityArrow.h"
#include "world/item/Item.h"
#include "world/item/ItemFood.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "util/Mth.h"

namespace
{
	constexpr byte_t WOLF_SITTING = 1;
	constexpr byte_t WOLF_ANGRY = 2;
	constexpr byte_t WOLF_TAMED = 4;

	bool equalsIgnoreCase(const jstring &left, const jstring &right)
	{
		if (left.size() != right.size())
			return false;
		for (size_t i = 0; i < left.size(); ++i)
		{
			char16_t l = left[i];
			char16_t r = right[i];
			if (l >= u'A' && l <= u'Z') l += u'a' - u'A';
			if (r >= u'A' && r <= u'Z') r += u'a' - u'A';
			if (l != r)
				return false;
		}
		return true;
	}
}

Wolf::Wolf(Level &level) : Animal(level)
{
	dataWatcher.addObject(16, static_cast<byte_t>(0));
	dataWatcher.addObject(17, jstring());
	dataWatcher.addObject(18, 0);
	textureName = u"/mob/wolf.png";
	setSize(0.8f, 0.8f);
	runSpeed = 1.1f;
	health = 8;
	makeStepSound = false;
}

void Wolf::tick()
{
	Animal::tick();

	looksWithInterest = false;
	if (hasCurrentTarget() && !hasPath() && !isWolfAngry())
	{
		if (auto player = std::dynamic_pointer_cast<Player>(getCurrentTarget()))
		{
			ItemInstance *selected = player->getSelectedItem();
			if (selected != nullptr)
			{
				if (!isWolfTamed() && selected->itemID == Items::bone->getShiftedIndex())
					looksWithInterest = true;
				else if (isWolfTamed())
					if (auto food = dynamic_cast<ItemFood *>(selected->getItem()))
						looksWithInterest = food->isWolfsFavoriteMeat();
			}
		}
	}

	if (!interpolateOnly && wolfShaking && !shakeActive && !hasPath() && onGround)
	{
		shakeActive = true;
		shakeTime = 0.0f;
		shakeTimeOld = 0.0f;
		level.broadcastEntityEvent(level.getEntityRef(*this), static_cast<byte_t>(8));
	}

	interestedAngleOld = interestedAngle;
	interestedAngle += ((looksWithInterest ? 1.0f : 0.0f) - interestedAngle) * 0.4f;
	if (looksWithInterest)
		keepCurrentTarget(10);

	if (isWet())
	{
		wolfShaking = true;
		shakeActive = false;
		shakeTime = 0.0f;
		shakeTimeOld = 0.0f;
	}
	else if ((wolfShaking || shakeActive) && shakeActive)
	{
		if (shakeTime == 0.0f)
		{
			level.playSoundAtEntity(*this, u"mob.wolf.shake", getSoundVolume(),
				(random.nextFloat() - random.nextFloat()) * 0.2f + 1.0f);
		}

		shakeTimeOld = shakeTime;
		shakeTime += 0.05f;
		if (shakeTimeOld >= 2.0f)
		{
			wolfShaking = false;
			shakeActive = false;
			shakeTimeOld = 0.0f;
			shakeTime = 0.0f;
		}

		if (shakeTime > 0.4f)
		{
			float particleY = static_cast<float>(bb.y0);
			int_t count = static_cast<int_t>(Mth::sin((shakeTime - 0.4f) * Mth::PI) * 7.0f);
			for (int_t i = 0; i < count; ++i)
			{
				float offsetX = (random.nextFloat() * 2.0f - 1.0f) * bbWidth * 0.5f;
				float offsetZ = (random.nextFloat() * 2.0f - 1.0f) * bbWidth * 0.5f;
				level.addParticle(u"splash", x + offsetX, particleY + 0.8f, z + offsetZ, xd, yd, zd);
			}
		}
	}
}

jstring Wolf::getTexture()
{
	if (isWolfTamed())
		return u"/mob/wolf_tame.png";
	return isWolfAngry() ? u"/mob/wolf_angry.png" : Animal::getTexture();
}

bool Wolf::hurt(Entity *source, int_t damage)
{
	setWolfSitting(false);
	if (source != nullptr && dynamic_cast<Player *>(source) == nullptr && dynamic_cast<EntityArrow *>(source) == nullptr)
		damage = (damage + 1) / 2;
	if (!Animal::hurt(source, damage))
		return false;
	if (!isWolfTamed() && !isWolfAngry())
	{
		if (dynamic_cast<Player *>(source) != nullptr)
		{
			setWolfAngry(true);
			setTarget(level.getEntityRef(*source));
		}

		if (EntityArrow *arrow = dynamic_cast<EntityArrow *>(source))
		{
			if (auto arrowOwner = arrow->owner.lock())
				source = arrowOwner.get();
		}

		if (dynamic_cast<Mob *>(source) != nullptr)
		{
			AABB *aabb = AABB::newTemp(x, y, z, x + 1.0, y + 1.0, z + 1.0)->grow(16.0, 4.0, 16.0);
			for (const auto &entity : level.getEntities(nullptr, *aabb))
			{
				Wolf *wolf = dynamic_cast<Wolf *>(entity.get());
				if (wolf != nullptr && !wolf->isWolfTamed() && wolf->getTarget() == nullptr)
				{
					wolf->setTarget(level.getEntityRef(*source));
					if (dynamic_cast<Player *>(source) != nullptr)
						wolf->setWolfAngry(true);
				}
			}
		}
	}
	else if (source != this && source != nullptr)
	{
		Player *sourcePlayer = dynamic_cast<Player *>(source);
		if (isWolfTamed() && sourcePlayer != nullptr && equalsIgnoreCase(sourcePlayer->name, getWolfOwner()))
			return true;
		setTarget(level.getEntityRef(*source));
	}
	return true;
}

bool Wolf::interact(Player &player)
{
	ItemInstance *selected = player.getSelectedItem();
	if (!isWolfTamed())
	{
		if (selected != nullptr && selected->itemID == Items::bone->getShiftedIndex() && !isWolfAngry())
		{
			selected->remove(1);
			if (selected->stackSize <= 0)
				player.inventory.setItem(player.inventory.currentItem, ItemInstance());
			if (!level.isOnline)
			{
				if (random.nextInt(3) == 0)
				{
					setWolfTamed(true);
					setPath(nullptr);
					setWolfSitting(true);
					health = 20;
					setWolfOwner(player.name);
					showHeartsOrSmokeFX(true);
					level.broadcastEntityEvent(level.getEntityRef(*this), static_cast<byte_t>(7));
				}
				else
				{
					showHeartsOrSmokeFX(false);
					level.broadcastEntityEvent(level.getEntityRef(*this), static_cast<byte_t>(6));
				}
			}
			return true;
		}
		return false;
	}

	if (selected != nullptr)
	{
		Item *item = selected->getItem();
		if (auto food = dynamic_cast<ItemFood *>(item))
		{
			if (food->isWolfsFavoriteMeat() && dataWatcher.getWatchableObjectInt(18) < 20)
			{
				selected->remove(1);
				if (selected->stackSize <= 0)
					player.inventory.setItem(player.inventory.currentItem, ItemInstance());
				heal(static_cast<ItemFood *>(Items::porkchopRaw)->getHealAmount());
				return true;
			}
		}
	}
	if (equalsIgnoreCase(player.name, getWolfOwner()))
	{
		if (!level.isOnline)
		{
			setWolfSitting(!isWolfSitting());
			jumping = false;
			setPath(nullptr);
		}
		return true;
	}
	return false;
}

float Wolf::getHeadHeight()
{
	return bbHeight * 0.8f;
}

int_t Wolf::getMaxSpawnClusterSize()
{
	return 8;
}

bool Wolf::isWolfSitting() const { return (dataWatcher.getWatchableObjectByte(16) & WOLF_SITTING) != 0; }
void Wolf::setWolfSitting(bool sitting)
{
	byte_t flags = dataWatcher.getWatchableObjectByte(16);
	dataWatcher.updateObject(16, static_cast<byte_t>(sitting ? flags | WOLF_SITTING : flags & ~WOLF_SITTING));
}
bool Wolf::isWolfAngry() const { return (dataWatcher.getWatchableObjectByte(16) & WOLF_ANGRY) != 0; }
void Wolf::setWolfAngry(bool angry)
{
	byte_t flags = dataWatcher.getWatchableObjectByte(16);
	dataWatcher.updateObject(16, static_cast<byte_t>(angry ? flags | WOLF_ANGRY : flags & ~WOLF_ANGRY));
}
bool Wolf::isWolfTamed() const { return (dataWatcher.getWatchableObjectByte(16) & WOLF_TAMED) != 0; }
void Wolf::setWolfTamed(bool tamed)
{
	byte_t flags = dataWatcher.getWatchableObjectByte(16);
	dataWatcher.updateObject(16, static_cast<byte_t>(tamed ? flags | WOLF_TAMED : flags & ~WOLF_TAMED));
}
const jstring &Wolf::getWolfOwner() const { return dataWatcher.getWatchableObjectString(17); }
void Wolf::setWolfOwner(const jstring &newOwner) { dataWatcher.updateObject(17, newOwner); }

float Wolf::getTailRotation() const
{
	if (isWolfAngry())
		return Mth::PI * 0.49f;
	return isWolfTamed() ? (0.55f - (20 - dataWatcher.getWatchableObjectInt(18)) * 0.02f) * Mth::PI : Mth::PI * 0.2f;
}

float Wolf::getInterestedAngle(float a) const
{
	return (interestedAngleOld + (interestedAngle - interestedAngleOld) * a) * 0.15f * Mth::PI;
}

bool Wolf::getWolfShaking() const
{
	return wolfShaking;
}

float Wolf::getShadingWhileShaking(float a) const
{
	return 12.0f / 16.0f + (shakeTimeOld + (shakeTime - shakeTimeOld) * a) / 2.0f * 0.25f;
}

float Wolf::getShakeAngle(float a, float offset) const
{
	float phase = (shakeTimeOld + (shakeTime - shakeTimeOld) * a + offset) / 1.8f;
	if (phase < 0.0f)
		phase = 0.0f;
	else if (phase > 1.0f)
		phase = 1.0f;
	return Mth::sin(phase * Mth::PI) * Mth::sin(phase * Mth::PI * 11.0f) * 0.15f * Mth::PI;
}

void Wolf::showHeartsOrSmokeFX(bool hearts)
{
	const jstring particle = hearts ? u"heart" : u"smoke";
	for (int_t i = 0; i < 7; ++i)
	{
		double xa = random.nextGaussian() * 0.02;
		double ya = random.nextGaussian() * 0.02;
		double za = random.nextGaussian() * 0.02;
		level.addParticle(particle,
			x + random.nextFloat() * bbWidth * 2.0f - bbWidth,
			y + 0.5 + random.nextFloat() * bbHeight,
			z + random.nextFloat() * bbWidth * 2.0f - bbWidth,
			xa, ya, za);
	}
}

void Wolf::handleEntityEvent(byte_t event)
{
	if (event == 7)
		showHeartsOrSmokeFX(true);
	else if (event == 6)
		showHeartsOrSmokeFX(false);
	else if (event == 8)
	{
		shakeActive = true;
		shakeTime = 0.0f;
		shakeTimeOld = 0.0f;
	}
	else
		Animal::handleEntityEvent(event);
}

void Wolf::addAdditionalSaveData(CompoundTag &tag)
{
	Animal::addAdditionalSaveData(tag);
	tag.putBoolean(u"Angry", isWolfAngry());
	tag.putBoolean(u"Sitting", isWolfSitting());
	tag.putString(u"Owner", getWolfOwner());
}

void Wolf::readAdditionalSaveData(CompoundTag &tag)
{
	Animal::readAdditionalSaveData(tag);
	setWolfAngry(tag.getBoolean(u"Angry"));
	setWolfSitting(tag.getBoolean(u"Sitting"));
	setWolfOwner(tag.getString(u"Owner"));
	if (!getWolfOwner().empty())
		setWolfTamed(true);
}

bool Wolf::canDespawn()
{
	return !isWolfTamed();
}

bool Wolf::isMovementCeased()
{
	return isWolfSitting() || shakeActive;
}

int_t Wolf::getMaxHeadXRot()
{
	return isWolfSitting() ? 20 : Animal::getMaxHeadXRot();
}

std::shared_ptr<Entity> Wolf::findAttackTarget()
{
	if (isWolfAngry())
		return std::static_pointer_cast<Entity>(level.getNearestPlayer(*this, 16.0));
	return nullptr;
}

void Wolf::checkHurtTarget(Entity &entity, float distance)
{
	if (!(distance > 2.0f) || !(distance < 6.0f) || random.nextInt(10) != 0)
	{
		if (distance < 1.5f && entity.bb.y1 > bb.y0 && entity.bb.y0 < bb.y1)
		{
			attackTime = 20;
			entity.hurt(this, isWolfTamed() ? 4 : 2);
		}
	}
	else if (onGround)
	{
		double dx = entity.x - x;
		double dz = entity.z - z;
		float flatDistance = Mth::sqrt(dx * dx + dz * dz);
		xd = dx / flatDistance * 0.5 * 0.8f + xd * 0.2f;
		zd = dz / flatDistance * 0.5 * 0.8f + zd * 0.2f;
		yd = 0.4f;
	}
}

void Wolf::updateAi()
{
	Animal::updateAi();
	if (!holdGround && !hasPath() && isWolfTamed() && riding == nullptr)
	{
		std::shared_ptr<Player> ownerPlayer;
		for (const auto &player : level.players)
		{
			if (player != nullptr && player->name == getWolfOwner())
			{
				ownerPlayer = player;
				break;
			}
		}
		if (ownerPlayer != nullptr)
		{
			float ownerDistance = distanceTo(*ownerPlayer);
			if (ownerDistance > 5.0f)
			{
				setPath(level.getPathToEntity(*this, *ownerPlayer, 16.0f));
				if (!hasPath() && ownerDistance > 12.0f)
				{
					int_t baseX = Mth::floor(ownerPlayer->x) - 2;
					int_t baseZ = Mth::floor(ownerPlayer->z) - 2;
					int_t baseY = Mth::floor(ownerPlayer->bb.y0);
					for (int_t x = 0; x <= 4; ++x)
					{
						for (int_t z = 0; z <= 4; ++z)
						{
							if ((x < 1 || z < 1 || x > 3 || z > 3)
								&& level.isSolidTile(baseX + x, baseY - 1, baseZ + z)
								&& !level.isSolidTile(baseX + x, baseY, baseZ + z)
								&& !level.isSolidTile(baseX + x, baseY + 1, baseZ + z))
							{
								moveTo(baseX + x + 0.5f, baseY, baseZ + z + 0.5f, yRot, xRot);
								goto wolf_follow_done;
							}
						}
					}
				}
			}
		}
		else if (!isInWater())
		{
			setWolfSitting(true);
		}
	}
	else if (getTarget() == nullptr && !hasPath() && !isWolfTamed() && level.random.nextInt(100) == 0)
	{
		AABB *huntBox = AABB::newTemp(x, y, z, x + 1.0, y + 1.0, z + 1.0)->grow(16.0, 4.0, 16.0);
		std::vector<std::shared_ptr<Entity>> sheep;
		for (const auto &entity : level.getEntities(this, *huntBox))
		{
			if (dynamic_cast<Sheep *>(entity.get()) != nullptr)
				sheep.push_back(entity);
		}
		if (!sheep.empty())
			setTarget(sheep[level.random.nextInt(static_cast<int_t>(sheep.size()))]);
	}

wolf_follow_done:
	if (isInWater())
		setWolfSitting(false);
	if (!level.isOnline)
		dataWatcher.updateObject(18, health);
}

jstring Wolf::getAmbientSound()
{
	if (isWolfAngry())
		return u"mob.wolf.growl";
	if (random.nextInt(3) == 0)
		return isWolfTamed() && dataWatcher.getWatchableObjectInt(18) < 10 ? u"mob.wolf.whine" : u"mob.wolf.panting";
	return u"mob.wolf.bark";
}

jstring Wolf::getHurtSound() { return u"mob.wolf.hurt"; }
jstring Wolf::getDeathSound() { return u"mob.wolf.death"; }
float Wolf::getSoundVolume() { return 0.4f; }
int_t Wolf::getDeathLoot() { return -1; }

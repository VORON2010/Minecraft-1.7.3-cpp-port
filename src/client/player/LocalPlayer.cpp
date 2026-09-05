#include "client/player/LocalPlayer.h"

#include <utility>

#include "client/Minecraft.h"
#include "client/gamemode/GameMode.h"
#include "client/gui/ChestScreen.h"
#include "client/gui/DispenserScreen.h"
#include "client/gui/FurnaceScreen.h"
#include "client/gui/WorkbenchScreen.h"
#include "client/gui/AchievementToast.h"
#include "client/particle/TakeAnimationParticle.h"
#include "client/spc/SPCCommand.h"
#include "client/locale/Language.h"
#include "world/CompoundContainer.h"
#include "world/entity/item/EntityMinecart.h"
#include "world/level/tile/entity/ChestTileEntity.h"
#include "world/level/tile/entity/DispenserTileEntity.h"
#include "world/level/tile/entity/FurnaceTileEntity.h"
#include "world/inventory/BasicInventory.h"
#include "world/inventory/ContainerMenus.h"
#include "world/inventory/IInventory.h"
#include "world/level/tile/entity/SignTileEntity.h"
#include "client/gui/EditSignScreen.h"
#include "util/Memory.h"
#include "world/stats/Achievement.h"
#include "world/stats/AchievementList.h"
#include "world/stats/StatBase.h"
#include "world/stats/StatFileWriter.h"

LocalPlayer::LocalPlayer(Minecraft &minecraft, Level &level, User *user, int_t dimension) : Player(level), minecraft(minecraft)
{
	this->dimension = dimension;

	const jstring username = user == nullptr ? u"" : user->name;
	name = username;
	if (!username.empty())
		customTextureUrl = u"http://s3.amazonaws.com/MinecraftSkins/" + username + u".png";
}


bool LocalPlayer::hurt(Entity *source, int_t dmg)
{
	if (minecraft.gameMode->isCreativeMode())
		return false;
	return Player::hurt(source, dmg);
}

void LocalPlayer::updateAi()
{
	Player::updateAi();
	if (minecraft.screen != nullptr || sleeping)
	{
		xxa = 0.0f;
		yya = 0.0f;
		jumping = false;
	}
	else
	{
		xxa = input->xa;
		yya = input->ya;
		jumping = input->jumping;
		isSprinting = input->sprinting;

		if (!minecraft.gameMode->isCreativeMode()) {
			flyActive = false;
		}

		if (input->jumping && !input->wasJumping) {
			if (jumpTriggerTime > 0) {
				flyActive = !flyActive;
				jumpTriggerTime = 0;
			} else {
				jumpTriggerTime = 7;
			}
		}
		if (jumpTriggerTime > 0) jumpTriggerTime--;

		isFlying = minecraft.gameMode->isCreativeMode() && flyActive;
		if (isFlying) {
			if (jumping) yd = 0.5f;
			else if (input->sneaking) yd = -0.5f;
			else yd = 0.0f;
		}
	}
}

void LocalPlayer::handleInsidePortal()
{
	if (changingDimensionDelay > 0)
	{
		changingDimensionDelay = 10;
		return;
	}

	isInsidePortal = true;
}

void LocalPlayer::aiStep()
{
	if (minecraft.statFileWriter != nullptr && !minecraft.statFileWriter->hasAchievementUnlocked(*AchievementList::openInventory))
		minecraft.achievementToast->queueAchievementInformation(*AchievementList::openInventory);

	oPortalTime = portalTime;

	if (isInsidePortal)
	{
		if (!level.isOnline && riding != nullptr)
			ride(nullptr);

		if (minecraft.screen != nullptr)
			minecraft.setScreen(nullptr);

		if (portalTime == 0.0f)
			minecraft.soundEngine.playUI(u"portal.trigger", 1.0f, random.nextFloat() * 0.4f + 0.8f);

		portalTime += 0.0125f;
		if (portalTime >= 1.0f)
		{
			portalTime = 1.0f;
			if (!level.isOnline)
			{
				changingDimensionDelay = 10;
				minecraft.soundEngine.playUI(u"portal.travel", 1.0f, random.nextFloat() * 0.4f + 0.8f);
				minecraft.toggleDimension();
			}
		}

		isInsidePortal = false;
	}
	else
	{
		if (portalTime > 0.0f)
			portalTime -= 0.05f;

		if (portalTime < 0.0f)
			portalTime = 0.0f;
	}

	if (changingDimensionDelay > 0)
		--changingDimensionDelay;

	input->tick(*this);

	if (input->sneaking && ySlideOffset < 0.2f)
		ySlideOffset = 0.2f;

	Player::aiStep();
}

void LocalPlayer::addStat(const StatBase &stat, int_t amount)
{
	if (minecraft.statFileWriter == nullptr)
		return;

	if (stat.isAchievement())
	{
		auto &achievement = static_cast<const Achievement &>(stat);
		if (!minecraft.statFileWriter->canUnlockAchievement(achievement))
			return;
		if (!minecraft.statFileWriter->hasAchievementUnlocked(achievement))
			minecraft.achievementToast->queueTakenAchievement(const_cast<Achievement &>(achievement));
	}
	minecraft.statFileWriter->readStat(stat, amount);
}

void LocalPlayer::releaseAllKeys()
{
	input->releaseAllKeys();
}

void LocalPlayer::setKey(int_t eventKey, bool eventKeyState)
{
	input->setKey(eventKey, eventKeyState);
}

void LocalPlayer::addAdditionalSaveData(CompoundTag &tag)
{
	Player::addAdditionalSaveData(tag);
}

void LocalPlayer::readAdditionalSaveData(CompoundTag &tag)
{
	Player::readAdditionalSaveData(tag);
}

void LocalPlayer::closeContainer()
{
	Player::closeContainer();
	minecraft.setScreen(nullptr);
}

void LocalPlayer::respawn()
{
	minecraft.respawnPlayer();
}

void LocalPlayer::startCrafting(int_t x, int_t y, int_t z)
{
	auto menu = std::make_shared<ContainerWorkbench>(inventory, *minecraft.level, x, y, z);
	craftingInventory = menu;
	minecraft.setScreen(Util::make_shared<WorkbenchScreen>(minecraft, *minecraft.level, x, y, z));
	craftingInventory = std::move(menu);
}

void LocalPlayer::startChest(std::shared_ptr<ChestTileEntity> chest)
{
	auto menu = std::make_shared<ContainerChest>(inventory, chest);
	craftingInventory = menu;
	minecraft.setScreen(Util::make_shared<ChestScreen>(minecraft, chest));
	craftingInventory = std::move(menu);
}

void LocalPlayer::startChest(std::shared_ptr<CompoundContainer> chest)
{
	auto menu = std::make_shared<ContainerChest>(inventory, chest);
	craftingInventory = menu;
	minecraft.setScreen(Util::make_shared<ChestScreen>(minecraft, chest));
	craftingInventory = std::move(menu);
}

void LocalPlayer::startChest(std::shared_ptr<EntityMinecart> chest)
	{
		auto menu = std::make_shared<ContainerChest>(inventory, chest);
		craftingInventory = menu;
		minecraft.setScreen(Util::make_shared<ChestScreen>(minecraft, chest));
		craftingInventory = std::move(menu);
	}

void LocalPlayer::startChest(std::shared_ptr<IInventory> chest)
{
	auto menu = std::make_shared<ContainerChest>(inventory, chest);
	craftingInventory = menu;
	auto basic = std::dynamic_pointer_cast<BasicInventory>(chest);
	if (basic != nullptr)
		minecraft.setScreen(Util::make_shared<ChestScreen>(minecraft, basic));
	craftingInventory = std::move(menu);
}

void LocalPlayer::startFurnace(std::shared_ptr<FurnaceTileEntity> furnace)
{
	auto menu = std::make_shared<ContainerFurnace>(inventory, furnace);
	craftingInventory = menu;
	minecraft.setScreen(Util::make_shared<FurnaceScreen>(minecraft, furnace));
	craftingInventory = std::move(menu);
}

void LocalPlayer::startDispenser(std::shared_ptr<DispenserTileEntity> dispenser)
{
	auto menu = std::make_shared<ContainerDispenser>(inventory, dispenser);
	craftingInventory = menu;
	minecraft.setScreen(Util::make_shared<DispenserScreen>(minecraft, dispenser));
	craftingInventory = std::move(menu);
}
void LocalPlayer::openTextEdit(std::shared_ptr<SignTileEntity> sign)
{
	minecraft.setScreen(Util::make_shared<EditSignScreen>(minecraft, sign));
}



void LocalPlayer::take(Entity &entity, int_t count)
{
	(void)count;
	if (minecraft.level == nullptr)
		return;

	std::shared_ptr<Entity> itemEntityPtr = nullptr;
	for (const auto &e : minecraft.level->getAllEntities())
	{
		if (e.get() == &entity)
		{
			itemEntityPtr = e;
			break;
		}
	}

	if (itemEntityPtr == nullptr)
		return;

	std::shared_ptr<Entity> playerPtr = nullptr;
	for (const auto &p : minecraft.level->players)
	{
		if (p.get() == this)
		{
			playerPtr = p;
			break;
		}
	}

	if (playerPtr != nullptr)
		minecraft.particleEngine.add(std::make_unique<TakeAnimationParticle>(*minecraft.level, itemEntityPtr, playerPtr, -0.5f));
}

void LocalPlayer::prepareForTick()
{

}

bool LocalPlayer::isSneaking()
{
	return input->sneaking;
}

void LocalPlayer::displayClientMessage(const jstring &message)
{
	SPCCommand::addChatMessage(Language::getInstance().getElement(message));
}

void LocalPlayer::sendChatMessage(const jstring &message)
{
	if (!message.empty() && message[0] == u'/') {
		SPCCommand::execute(minecraft, message);
	} else {
		SPCCommand::addChatMessage(u"<" + name + u"> " + message);
	}
}

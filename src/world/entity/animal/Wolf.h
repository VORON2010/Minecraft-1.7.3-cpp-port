#pragma once

#include "world/entity/animal/Animal.h"

class Wolf : public Animal
{
private:
	bool looksWithInterest = false;
	float interestedAngle = 0.0f;
	float interestedAngleOld = 0.0f;
	bool wolfShaking = false;
	bool shakeActive = false;
	float shakeTime = 0.0f;
	float shakeTimeOld = 0.0f;

	void showHeartsOrSmokeFX(bool hearts);

public:
	Wolf(Level &level);
	void tick() override;
	jstring getEncodeId() const override { return u"Wolf"; }
	jstring getTexture() override;
	bool hurt(Entity *source, int_t damage) override;
	bool interact(Player &player) override;
	float getHeadHeight() override;
	int_t getMaxSpawnClusterSize() override;

	bool isWolfSitting() const;
	void setWolfSitting(bool sitting);
	bool isWolfAngry() const;
	void setWolfAngry(bool angry);
	bool isWolfTamed() const;
	void setWolfTamed(bool tamed);
	const jstring &getWolfOwner() const;
	void setWolfOwner(const jstring &owner);
	float getTailRotation() const;
	float getInterestedAngle(float a) const;
	bool getWolfShaking() const;
	float getShadingWhileShaking(float a) const;
	float getShakeAngle(float a, float offset) const;
	void handleEntityEvent(byte_t event) override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
	bool canDespawn() override;
	bool isMovementCeased() override;
	int_t getMaxHeadXRot() override;
	std::shared_ptr<Entity> findAttackTarget() override;
	void checkHurtTarget(Entity &entity, float distance) override;
	void updateAi() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDeathLoot() override;
};

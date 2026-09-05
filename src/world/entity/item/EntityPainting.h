#pragma once

#include "world/entity/Entity.h"
#include "world/entity/item/PaintingArt.h"

class EntityPainting : public Entity
{
private:
	int_t checkInterval = 0;

	float getArtOffset(int_t size) const;
	void dropPainting();

public:
	int_t direction = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	const PaintingArt *art = nullptr;

	explicit EntityPainting(Level &level);
	EntityPainting(Level &level, int_t x, int_t y, int_t z, int_t direction);
	EntityPainting(Level &level, int_t x, int_t y, int_t z, int_t direction, const jstring &title);

	jstring getEncodeId() const override { return u"Painting"; }
	void setDirection(int_t direction);
	void tick() override;
	bool survives();
	bool isPickable() override;
	bool hurt(Entity *source, int_t dmg) override;
	void move(double xd, double yd, double zd) override;
	void push(double xd, double yd, double zd) override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};

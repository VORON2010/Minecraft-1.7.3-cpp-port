#pragma once

#include "client/renderer/culling/FrustumData.h"

#include <array>

#include "java/Type.h"

class Frustum : public FrustumData
{
private:
	static Frustum frustum;

public:
	static FrustumData &getFrustum();

private:
	void normalizePlane(std::array<std::array<float, 16>, 16> &frustum, int_t side);
	void calculateFrustum();
};

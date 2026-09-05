#pragma once

#include "client/model/Cube.h"

class SignModel
{
public:
	Cube board = Cube(0, 0);
	Cube stick = Cube(0, 14);

	SignModel();

	void render();
};

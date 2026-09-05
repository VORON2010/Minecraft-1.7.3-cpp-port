#include "client/model/SignModel.h"

SignModel::SignModel()
{
	board.addBox(-12.0f, -14.0f, -1.0f, 24, 12, 2, 0.0f);
	stick.addBox(-1.0f, -2.0f, -1.0f, 2, 14, 2, 0.0f);
}

void SignModel::render()
{
	board.render(0.0625f);
	stick.render(0.0625f);
}

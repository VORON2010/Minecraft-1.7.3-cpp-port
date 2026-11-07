#pragma once

#include "client/model/ModelPart.h"

// 1.0 EnchantingTableBookModel (texture /item/book.png, 64x32)
class BookModel
{
public:
	ModelPart leftLid, rightLid, leftPages, rightPages, flipPage1, flipPage2, seam;

	BookModel();
	void render(float time, float flip1, float flip2, float open, float scale);
	void setupAnim(float time, float flip1, float flip2, float open);
};

// Render shapes added in b1.8: glass pane / iron bars, vines, pumpkin & melon stems, fence gate.
// Ported from the b1.8.1 block renderer. Light uses the Beta per-vertex brightness instead of the 1.8 lightmap.
#include "client/renderer/TileRenderer.h"

#include "client/renderer/Tesselator.h"
#include "world/level/Level.h"
#include "world/level/tile/PaneTile.h"
#include "world/level/tile/StemTile.h"
#include "world/level/tile/FenceGateTile.h"

bool TileRenderer::tesselatePaneInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	PaneTile &pane = static_cast<PaneTile &>(tt);
	int_t var5 = Level::DEPTH;
	Tesselator &t = Tesselator::instance;
	float br = tt.getBrightness(*level, x, y, z);
	int_t col = tt.getColor(*level, x, y, z);
	t.color(br * ((col >> 16) & 0xFF) / 255.0f, br * ((col >> 8) & 0xFF) / 255.0f, br * (col & 0xFF) / 255.0f);

	int_t var64 = 0;
	int_t var66 = 0;
	if (fixedTexture >= 0)
	{
		var64 = fixedTexture;
		var66 = fixedTexture;
	}
	else
	{
		var64 = tt.getTexture(Facing::DOWN, level->getData(x, y, z));
		var66 = pane.getTopTexture();
	}

	int_t var69 = (var64 & 15) << 4;
	int_t var15 = var64 & 240;
	double var16 = (double)((float)var69 / 256.0f);
	double var18 = (double)(((float)var69 + 7.99f) / 256.0f);
	double var20 = (double)(((float)var69 + 15.99f) / 256.0f);
	double var22 = (double)((float)var15 / 256.0f);
	double var24 = (double)(((float)var15 + 15.99f) / 256.0f);
	int_t var26 = (var66 & 15) << 4;
	int_t var27 = var66 & 240;
	double var28 = (double)((float)(var26 + 7) / 256.0f);
	double var30 = (double)(((float)var26 + 8.99f) / 256.0f);
	double var32 = (double)((float)var27 / 256.0f);
	double var34 = (double)((float)(var27 + 8) / 256.0f);
	double var36 = (double)(((float)var27 + 15.99f) / 256.0f);
	double var38 = (double)x;
	double var40 = (double)x + 0.5;
	double var42 = (double)(x + 1);
	double var44 = (double)z;
	double var46 = (double)z + 0.5;
	double var48 = (double)(z + 1);
	double var50 = (double)x + 0.5 - 0.0625;
	double var52 = (double)x + 0.5 + 0.0625;
	double var54 = (double)z + 0.5 - 0.0625;
	double var56 = (double)z + 0.5 + 0.0625;
	bool var58 = pane.attachesTo(level->getTile(x, y, z - 1));
	bool var59 = pane.attachesTo(level->getTile(x, y, z + 1));
	bool var60 = pane.attachesTo(level->getTile(x - 1, y, z));
	bool var61 = pane.attachesTo(level->getTile(x + 1, y, z));
	bool var62 = pane.shouldRenderFace(*level, x, y + 1, z, Facing::UP);
	bool var63 = pane.shouldRenderFace(*level, x, y - 1, z, Facing::DOWN);
	if ((!var60 || !var61) && (var60 || var61 || var58 || var59)) {
		if (var60 && !var61) {
			t.vertexUV(var38, (double)(y + 1), var46, var16, var22);
			t.vertexUV(var38, (double)(y + 0), var46, var16, var24);
			t.vertexUV(var40, (double)(y + 0), var46, var18, var24);
			t.vertexUV(var40, (double)(y + 1), var46, var18, var22);
			t.vertexUV(var40, (double)(y + 1), var46, var16, var22);
			t.vertexUV(var40, (double)(y + 0), var46, var16, var24);
			t.vertexUV(var38, (double)(y + 0), var46, var18, var24);
			t.vertexUV(var38, (double)(y + 1), var46, var18, var22);
			if (!var59 && !var58) {
				t.vertexUV(var40, (double)(y + 1), var56, var28, var32);
				t.vertexUV(var40, (double)(y + 0), var56, var28, var36);
				t.vertexUV(var40, (double)(y + 0), var54, var30, var36);
				t.vertexUV(var40, (double)(y + 1), var54, var30, var32);
				t.vertexUV(var40, (double)(y + 1), var54, var28, var32);
				t.vertexUV(var40, (double)(y + 0), var54, var28, var36);
				t.vertexUV(var40, (double)(y + 0), var56, var30, var36);
				t.vertexUV(var40, (double)(y + 1), var56, var30, var32);
			}

			if (var62 || y < var5 - 1 && (level->getTile(x - 1, y + 1, z) == 0)) {
				t.vertexUV(var38, (double)(y + 1) + 0.01, var56, var30, var34);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var56, var30, var36);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var54, var28, var36);
				t.vertexUV(var38, (double)(y + 1) + 0.01, var54, var28, var34);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var56, var30, var34);
				t.vertexUV(var38, (double)(y + 1) + 0.01, var56, var30, var36);
				t.vertexUV(var38, (double)(y + 1) + 0.01, var54, var28, var36);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var54, var28, var34);
			}

			if (var63 || y > 1 && (level->getTile(x - 1, y - 1, z) == 0)) {
				t.vertexUV(var38, (double)y - 0.01, var56, var30, var34);
				t.vertexUV(var40, (double)y - 0.01, var56, var30, var36);
				t.vertexUV(var40, (double)y - 0.01, var54, var28, var36);
				t.vertexUV(var38, (double)y - 0.01, var54, var28, var34);
				t.vertexUV(var40, (double)y - 0.01, var56, var30, var34);
				t.vertexUV(var38, (double)y - 0.01, var56, var30, var36);
				t.vertexUV(var38, (double)y - 0.01, var54, var28, var36);
				t.vertexUV(var40, (double)y - 0.01, var54, var28, var34);
			}
		} else if (!var60 && var61) {
			t.vertexUV(var40, (double)(y + 1), var46, var18, var22);
			t.vertexUV(var40, (double)(y + 0), var46, var18, var24);
			t.vertexUV(var42, (double)(y + 0), var46, var20, var24);
			t.vertexUV(var42, (double)(y + 1), var46, var20, var22);
			t.vertexUV(var42, (double)(y + 1), var46, var18, var22);
			t.vertexUV(var42, (double)(y + 0), var46, var18, var24);
			t.vertexUV(var40, (double)(y + 0), var46, var20, var24);
			t.vertexUV(var40, (double)(y + 1), var46, var20, var22);
			if (!var59 && !var58) {
				t.vertexUV(var40, (double)(y + 1), var54, var28, var32);
				t.vertexUV(var40, (double)(y + 0), var54, var28, var36);
				t.vertexUV(var40, (double)(y + 0), var56, var30, var36);
				t.vertexUV(var40, (double)(y + 1), var56, var30, var32);
				t.vertexUV(var40, (double)(y + 1), var56, var28, var32);
				t.vertexUV(var40, (double)(y + 0), var56, var28, var36);
				t.vertexUV(var40, (double)(y + 0), var54, var30, var36);
				t.vertexUV(var40, (double)(y + 1), var54, var30, var32);
			}

			if (var62 || y < var5 - 1 && (level->getTile(x + 1, y + 1, z) == 0)) {
				t.vertexUV(var40, (double)(y + 1) + 0.01, var56, var30, var32);
				t.vertexUV(var42, (double)(y + 1) + 0.01, var56, var30, var34);
				t.vertexUV(var42, (double)(y + 1) + 0.01, var54, var28, var34);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var54, var28, var32);
				t.vertexUV(var42, (double)(y + 1) + 0.01, var56, var30, var32);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var56, var30, var34);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var54, var28, var34);
				t.vertexUV(var42, (double)(y + 1) + 0.01, var54, var28, var32);
			}

			if (var63 || y > 1 && (level->getTile(x + 1, y - 1, z) == 0)) {
				t.vertexUV(var40, (double)y - 0.01, var56, var30, var32);
				t.vertexUV(var42, (double)y - 0.01, var56, var30, var34);
				t.vertexUV(var42, (double)y - 0.01, var54, var28, var34);
				t.vertexUV(var40, (double)y - 0.01, var54, var28, var32);
				t.vertexUV(var42, (double)y - 0.01, var56, var30, var32);
				t.vertexUV(var40, (double)y - 0.01, var56, var30, var34);
				t.vertexUV(var40, (double)y - 0.01, var54, var28, var34);
				t.vertexUV(var42, (double)y - 0.01, var54, var28, var32);
			}
		}
	} else {
		t.vertexUV(var38, (double)(y + 1), var46, var16, var22);
		t.vertexUV(var38, (double)(y + 0), var46, var16, var24);
		t.vertexUV(var42, (double)(y + 0), var46, var20, var24);
		t.vertexUV(var42, (double)(y + 1), var46, var20, var22);
		t.vertexUV(var42, (double)(y + 1), var46, var16, var22);
		t.vertexUV(var42, (double)(y + 0), var46, var16, var24);
		t.vertexUV(var38, (double)(y + 0), var46, var20, var24);
		t.vertexUV(var38, (double)(y + 1), var46, var20, var22);
		if (var62) {
			t.vertexUV(var38, (double)(y + 1) + 0.01, var56, var30, var36);
			t.vertexUV(var42, (double)(y + 1) + 0.01, var56, var30, var32);
			t.vertexUV(var42, (double)(y + 1) + 0.01, var54, var28, var32);
			t.vertexUV(var38, (double)(y + 1) + 0.01, var54, var28, var36);
			t.vertexUV(var42, (double)(y + 1) + 0.01, var56, var30, var36);
			t.vertexUV(var38, (double)(y + 1) + 0.01, var56, var30, var32);
			t.vertexUV(var38, (double)(y + 1) + 0.01, var54, var28, var32);
			t.vertexUV(var42, (double)(y + 1) + 0.01, var54, var28, var36);
		} else {
			if (y < var5 - 1 && (level->getTile(x - 1, y + 1, z) == 0)) {
				t.vertexUV(var38, (double)(y + 1) + 0.01, var56, var30, var34);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var56, var30, var36);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var54, var28, var36);
				t.vertexUV(var38, (double)(y + 1) + 0.01, var54, var28, var34);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var56, var30, var34);
				t.vertexUV(var38, (double)(y + 1) + 0.01, var56, var30, var36);
				t.vertexUV(var38, (double)(y + 1) + 0.01, var54, var28, var36);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var54, var28, var34);
			}

			if (y < var5 - 1 && (level->getTile(x + 1, y + 1, z) == 0)) {
				t.vertexUV(var40, (double)(y + 1) + 0.01, var56, var30, var32);
				t.vertexUV(var42, (double)(y + 1) + 0.01, var56, var30, var34);
				t.vertexUV(var42, (double)(y + 1) + 0.01, var54, var28, var34);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var54, var28, var32);
				t.vertexUV(var42, (double)(y + 1) + 0.01, var56, var30, var32);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var56, var30, var34);
				t.vertexUV(var40, (double)(y + 1) + 0.01, var54, var28, var34);
				t.vertexUV(var42, (double)(y + 1) + 0.01, var54, var28, var32);
			}
		}

		if (var63) {
			t.vertexUV(var38, (double)y - 0.01, var56, var30, var36);
			t.vertexUV(var42, (double)y - 0.01, var56, var30, var32);
			t.vertexUV(var42, (double)y - 0.01, var54, var28, var32);
			t.vertexUV(var38, (double)y - 0.01, var54, var28, var36);
			t.vertexUV(var42, (double)y - 0.01, var56, var30, var36);
			t.vertexUV(var38, (double)y - 0.01, var56, var30, var32);
			t.vertexUV(var38, (double)y - 0.01, var54, var28, var32);
			t.vertexUV(var42, (double)y - 0.01, var54, var28, var36);
		} else {
			if (y > 1 && (level->getTile(x - 1, y - 1, z) == 0)) {
				t.vertexUV(var38, (double)y - 0.01, var56, var30, var34);
				t.vertexUV(var40, (double)y - 0.01, var56, var30, var36);
				t.vertexUV(var40, (double)y - 0.01, var54, var28, var36);
				t.vertexUV(var38, (double)y - 0.01, var54, var28, var34);
				t.vertexUV(var40, (double)y - 0.01, var56, var30, var34);
				t.vertexUV(var38, (double)y - 0.01, var56, var30, var36);
				t.vertexUV(var38, (double)y - 0.01, var54, var28, var36);
				t.vertexUV(var40, (double)y - 0.01, var54, var28, var34);
			}

			if (y > 1 && (level->getTile(x + 1, y - 1, z) == 0)) {
				t.vertexUV(var40, (double)y - 0.01, var56, var30, var32);
				t.vertexUV(var42, (double)y - 0.01, var56, var30, var34);
				t.vertexUV(var42, (double)y - 0.01, var54, var28, var34);
				t.vertexUV(var40, (double)y - 0.01, var54, var28, var32);
				t.vertexUV(var42, (double)y - 0.01, var56, var30, var32);
				t.vertexUV(var40, (double)y - 0.01, var56, var30, var34);
				t.vertexUV(var40, (double)y - 0.01, var54, var28, var34);
				t.vertexUV(var42, (double)y - 0.01, var54, var28, var32);
			}
		}
	}

	if ((!var58 || !var59) && (var60 || var61 || var58 || var59)) {
		if (var58 && !var59) {
			t.vertexUV(var40, (double)(y + 1), var44, var16, var22);
			t.vertexUV(var40, (double)(y + 0), var44, var16, var24);
			t.vertexUV(var40, (double)(y + 0), var46, var18, var24);
			t.vertexUV(var40, (double)(y + 1), var46, var18, var22);
			t.vertexUV(var40, (double)(y + 1), var46, var16, var22);
			t.vertexUV(var40, (double)(y + 0), var46, var16, var24);
			t.vertexUV(var40, (double)(y + 0), var44, var18, var24);
			t.vertexUV(var40, (double)(y + 1), var44, var18, var22);
			if (!var61 && !var60) {
				t.vertexUV(var50, (double)(y + 1), var46, var28, var32);
				t.vertexUV(var50, (double)(y + 0), var46, var28, var36);
				t.vertexUV(var52, (double)(y + 0), var46, var30, var36);
				t.vertexUV(var52, (double)(y + 1), var46, var30, var32);
				t.vertexUV(var52, (double)(y + 1), var46, var28, var32);
				t.vertexUV(var52, (double)(y + 0), var46, var28, var36);
				t.vertexUV(var50, (double)(y + 0), var46, var30, var36);
				t.vertexUV(var50, (double)(y + 1), var46, var30, var32);
			}

			if (var62 || y < var5 - 1 && (level->getTile(x, y + 1, z - 1) == 0)) {
				t.vertexUV(var50, (double)(y + 1), var44, var30, var32);
				t.vertexUV(var50, (double)(y + 1), var46, var30, var34);
				t.vertexUV(var52, (double)(y + 1), var46, var28, var34);
				t.vertexUV(var52, (double)(y + 1), var44, var28, var32);
				t.vertexUV(var50, (double)(y + 1), var46, var30, var32);
				t.vertexUV(var50, (double)(y + 1), var44, var30, var34);
				t.vertexUV(var52, (double)(y + 1), var44, var28, var34);
				t.vertexUV(var52, (double)(y + 1), var46, var28, var32);
			}

			if (var63 || y > 1 && (level->getTile(x, y - 1, z - 1) == 0)) {
				t.vertexUV(var50, (double)y, var44, var30, var32);
				t.vertexUV(var50, (double)y, var46, var30, var34);
				t.vertexUV(var52, (double)y, var46, var28, var34);
				t.vertexUV(var52, (double)y, var44, var28, var32);
				t.vertexUV(var50, (double)y, var46, var30, var32);
				t.vertexUV(var50, (double)y, var44, var30, var34);
				t.vertexUV(var52, (double)y, var44, var28, var34);
				t.vertexUV(var52, (double)y, var46, var28, var32);
			}
		} else if (!var58 && var59) {
			t.vertexUV(var40, (double)(y + 1), var46, var18, var22);
			t.vertexUV(var40, (double)(y + 0), var46, var18, var24);
			t.vertexUV(var40, (double)(y + 0), var48, var20, var24);
			t.vertexUV(var40, (double)(y + 1), var48, var20, var22);
			t.vertexUV(var40, (double)(y + 1), var48, var18, var22);
			t.vertexUV(var40, (double)(y + 0), var48, var18, var24);
			t.vertexUV(var40, (double)(y + 0), var46, var20, var24);
			t.vertexUV(var40, (double)(y + 1), var46, var20, var22);
			if (!var61 && !var60) {
				t.vertexUV(var52, (double)(y + 1), var46, var28, var32);
				t.vertexUV(var52, (double)(y + 0), var46, var28, var36);
				t.vertexUV(var50, (double)(y + 0), var46, var30, var36);
				t.vertexUV(var50, (double)(y + 1), var46, var30, var32);
				t.vertexUV(var50, (double)(y + 1), var46, var28, var32);
				t.vertexUV(var50, (double)(y + 0), var46, var28, var36);
				t.vertexUV(var52, (double)(y + 0), var46, var30, var36);
				t.vertexUV(var52, (double)(y + 1), var46, var30, var32);
			}

			if (var62 || y < var5 - 1 && (level->getTile(x, y + 1, z + 1) == 0)) {
				t.vertexUV(var50, (double)(y + 1), var46, var28, var34);
				t.vertexUV(var50, (double)(y + 1), var48, var28, var36);
				t.vertexUV(var52, (double)(y + 1), var48, var30, var36);
				t.vertexUV(var52, (double)(y + 1), var46, var30, var34);
				t.vertexUV(var50, (double)(y + 1), var48, var28, var34);
				t.vertexUV(var50, (double)(y + 1), var46, var28, var36);
				t.vertexUV(var52, (double)(y + 1), var46, var30, var36);
				t.vertexUV(var52, (double)(y + 1), var48, var30, var34);
			}

			if (var63 || y > 1 && (level->getTile(x, y - 1, z + 1) == 0)) {
				t.vertexUV(var50, (double)y, var46, var28, var34);
				t.vertexUV(var50, (double)y, var48, var28, var36);
				t.vertexUV(var52, (double)y, var48, var30, var36);
				t.vertexUV(var52, (double)y, var46, var30, var34);
				t.vertexUV(var50, (double)y, var48, var28, var34);
				t.vertexUV(var50, (double)y, var46, var28, var36);
				t.vertexUV(var52, (double)y, var46, var30, var36);
				t.vertexUV(var52, (double)y, var48, var30, var34);
			}
		}
	} else {
		t.vertexUV(var40, (double)(y + 1), var48, var16, var22);
		t.vertexUV(var40, (double)(y + 0), var48, var16, var24);
		t.vertexUV(var40, (double)(y + 0), var44, var20, var24);
		t.vertexUV(var40, (double)(y + 1), var44, var20, var22);
		t.vertexUV(var40, (double)(y + 1), var44, var16, var22);
		t.vertexUV(var40, (double)(y + 0), var44, var16, var24);
		t.vertexUV(var40, (double)(y + 0), var48, var20, var24);
		t.vertexUV(var40, (double)(y + 1), var48, var20, var22);
		if (var62) {
			t.vertexUV(var52, (double)(y + 1), var48, var30, var36);
			t.vertexUV(var52, (double)(y + 1), var44, var30, var32);
			t.vertexUV(var50, (double)(y + 1), var44, var28, var32);
			t.vertexUV(var50, (double)(y + 1), var48, var28, var36);
			t.vertexUV(var52, (double)(y + 1), var44, var30, var36);
			t.vertexUV(var52, (double)(y + 1), var48, var30, var32);
			t.vertexUV(var50, (double)(y + 1), var48, var28, var32);
			t.vertexUV(var50, (double)(y + 1), var44, var28, var36);
		} else {
			if (y < var5 - 1 && (level->getTile(x, y + 1, z - 1) == 0)) {
				t.vertexUV(var50, (double)(y + 1), var44, var30, var32);
				t.vertexUV(var50, (double)(y + 1), var46, var30, var34);
				t.vertexUV(var52, (double)(y + 1), var46, var28, var34);
				t.vertexUV(var52, (double)(y + 1), var44, var28, var32);
				t.vertexUV(var50, (double)(y + 1), var46, var30, var32);
				t.vertexUV(var50, (double)(y + 1), var44, var30, var34);
				t.vertexUV(var52, (double)(y + 1), var44, var28, var34);
				t.vertexUV(var52, (double)(y + 1), var46, var28, var32);
			}

			if (y < var5 - 1 && (level->getTile(x, y + 1, z + 1) == 0)) {
				t.vertexUV(var50, (double)(y + 1), var46, var28, var34);
				t.vertexUV(var50, (double)(y + 1), var48, var28, var36);
				t.vertexUV(var52, (double)(y + 1), var48, var30, var36);
				t.vertexUV(var52, (double)(y + 1), var46, var30, var34);
				t.vertexUV(var50, (double)(y + 1), var48, var28, var34);
				t.vertexUV(var50, (double)(y + 1), var46, var28, var36);
				t.vertexUV(var52, (double)(y + 1), var46, var30, var36);
				t.vertexUV(var52, (double)(y + 1), var48, var30, var34);
			}
		}

		if (var63) {
			t.vertexUV(var52, (double)y, var48, var30, var36);
			t.vertexUV(var52, (double)y, var44, var30, var32);
			t.vertexUV(var50, (double)y, var44, var28, var32);
			t.vertexUV(var50, (double)y, var48, var28, var36);
			t.vertexUV(var52, (double)y, var44, var30, var36);
			t.vertexUV(var52, (double)y, var48, var30, var32);
			t.vertexUV(var50, (double)y, var48, var28, var32);
			t.vertexUV(var50, (double)y, var44, var28, var36);
		} else {
			if (y > 1 && (level->getTile(x, y - 1, z - 1) == 0)) {
				t.vertexUV(var50, (double)y, var44, var30, var32);
				t.vertexUV(var50, (double)y, var46, var30, var34);
				t.vertexUV(var52, (double)y, var46, var28, var34);
				t.vertexUV(var52, (double)y, var44, var28, var32);
				t.vertexUV(var50, (double)y, var46, var30, var32);
				t.vertexUV(var50, (double)y, var44, var30, var34);
				t.vertexUV(var52, (double)y, var44, var28, var34);
				t.vertexUV(var52, (double)y, var46, var28, var32);
			}

			if (y > 1 && (level->getTile(x, y - 1, z + 1) == 0)) {
				t.vertexUV(var50, (double)y, var46, var28, var34);
				t.vertexUV(var50, (double)y, var48, var28, var36);
				t.vertexUV(var52, (double)y, var48, var30, var36);
				t.vertexUV(var52, (double)y, var46, var30, var34);
				t.vertexUV(var50, (double)y, var48, var28, var34);
				t.vertexUV(var50, (double)y, var46, var28, var36);
				t.vertexUV(var52, (double)y, var46, var30, var36);
				t.vertexUV(var52, (double)y, var48, var30, var34);
			}
		}
	}

	return true;
}

bool TileRenderer::tesselateVineInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	Tesselator &t = Tesselator::instance;
	int_t var6 = tt.getTexture(Facing::DOWN, 0);
	if (fixedTexture >= 0)
		var6 = fixedTexture;

	float br = tt.getBrightness(*level, x, y, z);
	int_t col = tt.getColor(*level, x, y, z);
	t.color(br * ((col >> 16) & 0xFF) / 255.0f, br * ((col >> 8) & 0xFF) / 255.0f, br * (col & 0xFF) / 255.0f);

	int_t var8 = (var6 & 15) << 4;
	int_t var22 = var6 & 240;
	double var23 = (double)((float)var8 / 256.0f);
	double var12 = (double)(((float)var8 + 15.99f) / 256.0f);
	double var14 = (double)((float)var22 / 256.0f);
	double var16 = (double)(((float)var22 + 15.99f) / 256.0f);
	double var18 = 0.05f;
	int var20 = level->getData(x, y, z);
	if ((var20 & 2) != 0) {
		t.vertexUV((double)x + var18, (double)(y + 1), (double)(z + 1), var23, var14);
		t.vertexUV((double)x + var18, (double)(y + 0), (double)(z + 1), var23, var16);
		t.vertexUV((double)x + var18, (double)(y + 0), (double)(z + 0), var12, var16);
		t.vertexUV((double)x + var18, (double)(y + 1), (double)(z + 0), var12, var14);
		t.vertexUV((double)x + var18, (double)(y + 1), (double)(z + 0), var12, var14);
		t.vertexUV((double)x + var18, (double)(y + 0), (double)(z + 0), var12, var16);
		t.vertexUV((double)x + var18, (double)(y + 0), (double)(z + 1), var23, var16);
		t.vertexUV((double)x + var18, (double)(y + 1), (double)(z + 1), var23, var14);
	}

	if ((var20 & 8) != 0) {
		t.vertexUV((double)(x + 1) - var18, (double)(y + 0), (double)(z + 1), var12, var16);
		t.vertexUV((double)(x + 1) - var18, (double)(y + 1), (double)(z + 1), var12, var14);
		t.vertexUV((double)(x + 1) - var18, (double)(y + 1), (double)(z + 0), var23, var14);
		t.vertexUV((double)(x + 1) - var18, (double)(y + 0), (double)(z + 0), var23, var16);
		t.vertexUV((double)(x + 1) - var18, (double)(y + 0), (double)(z + 0), var23, var16);
		t.vertexUV((double)(x + 1) - var18, (double)(y + 1), (double)(z + 0), var23, var14);
		t.vertexUV((double)(x + 1) - var18, (double)(y + 1), (double)(z + 1), var12, var14);
		t.vertexUV((double)(x + 1) - var18, (double)(y + 0), (double)(z + 1), var12, var16);
	}

	if ((var20 & 4) != 0) {
		t.vertexUV((double)(x + 1), (double)(y + 0), (double)z + var18, var12, var16);
		t.vertexUV((double)(x + 1), (double)(y + 1), (double)z + var18, var12, var14);
		t.vertexUV((double)(x + 0), (double)(y + 1), (double)z + var18, var23, var14);
		t.vertexUV((double)(x + 0), (double)(y + 0), (double)z + var18, var23, var16);
		t.vertexUV((double)(x + 0), (double)(y + 0), (double)z + var18, var23, var16);
		t.vertexUV((double)(x + 0), (double)(y + 1), (double)z + var18, var23, var14);
		t.vertexUV((double)(x + 1), (double)(y + 1), (double)z + var18, var12, var14);
		t.vertexUV((double)(x + 1), (double)(y + 0), (double)z + var18, var12, var16);
	}

	if ((var20 & 1) != 0) {
		t.vertexUV((double)(x + 1), (double)(y + 1), (double)(z + 1) - var18, var23, var14);
		t.vertexUV((double)(x + 1), (double)(y + 0), (double)(z + 1) - var18, var23, var16);
		t.vertexUV((double)(x + 0), (double)(y + 0), (double)(z + 1) - var18, var12, var16);
		t.vertexUV((double)(x + 0), (double)(y + 1), (double)(z + 1) - var18, var12, var14);
		t.vertexUV((double)(x + 0), (double)(y + 1), (double)(z + 1) - var18, var12, var14);
		t.vertexUV((double)(x + 0), (double)(y + 0), (double)(z + 1) - var18, var12, var16);
		t.vertexUV((double)(x + 1), (double)(y + 0), (double)(z + 1) - var18, var23, var16);
		t.vertexUV((double)(x + 1), (double)(y + 1), (double)(z + 1) - var18, var23, var14);
	}

	if (level->isSolidTile(x, y + 1, z)) {
		t.vertexUV((double)(x + 1), (double)(y + 1) - var18, (double)(z + 0), var23, var14);
		t.vertexUV((double)(x + 1), (double)(y + 1) - var18, (double)(z + 1), var23, var16);
		t.vertexUV((double)(x + 0), (double)(y + 1) - var18, (double)(z + 1), var12, var16);
		t.vertexUV((double)(x + 0), (double)(y + 1) - var18, (double)(z + 0), var12, var14);
	}

	return true;
}

bool TileRenderer::tesselateStemInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	StemTile &stem = static_cast<StemTile &>(tt);
	Tesselator &t = Tesselator::instance;
	float br = tt.getBrightness(*level, x, y, z);
	int_t col = tt.getColor(*level, x, y, z);
	t.color(br * ((col >> 16) & 0xFF) / 255.0f, br * ((col >> 8) & 0xFF) / 255.0f, br * (col & 0xFF) / 255.0f);

	stem.updateShape(*level, x, y, z);
	int_t data = level->getData(x, y, z);
	int_t dir = stem.getConnectDirection(*level, x, y, z);
	if (dir < 0)
	{
		tesselateStemTexture(tt, data, stem.getHeight(), x, y, z);
	}
	else
	{
		tesselateStemTexture(tt, data, 0.5, x, y, z);
		tesselateStemDirTexture(tt, data, dir, stem.getHeight(), x, y, z);
	}
	return true;
}

void TileRenderer::tesselateStemTexture(Tile &tt, int_t data, double height, double x, double y, double z)
{
	Tesselator &t = Tesselator::instance;
	int_t tex = tt.getTexture(Facing::DOWN, data);
	if (fixedTexture >= 0)
		tex = fixedTexture;

	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	double u0 = (double)((float)xt / 256.0f);
	double u1 = (double)(((float)xt + 15.99f) / 256.0f);
	double v0 = (double)((float)yt / 256.0f);
	double v1 = ((double)yt + 15.99f * height) / 256.0;
	double x0 = x + 0.5 - 0.45f;
	double x1 = x + 0.5 + 0.45f;
	double z0 = z + 0.5 - 0.45f;
	double z1 = z + 0.5 + 0.45f;
	t.vertexUV(x0, y + height, z0, u0, v0);
	t.vertexUV(x0, y + 0.0, z0, u0, v1);
	t.vertexUV(x1, y + 0.0, z1, u1, v1);
	t.vertexUV(x1, y + height, z1, u1, v0);
	t.vertexUV(x1, y + height, z1, u0, v0);
	t.vertexUV(x1, y + 0.0, z1, u0, v1);
	t.vertexUV(x0, y + 0.0, z0, u1, v1);
	t.vertexUV(x0, y + height, z0, u1, v0);
	t.vertexUV(x0, y + height, z1, u0, v0);
	t.vertexUV(x0, y + 0.0, z1, u0, v1);
	t.vertexUV(x1, y + 0.0, z0, u1, v1);
	t.vertexUV(x1, y + height, z0, u1, v0);
	t.vertexUV(x1, y + height, z0, u0, v0);
	t.vertexUV(x1, y + 0.0, z0, u0, v1);
	t.vertexUV(x0, y + 0.0, z1, u1, v1);
	t.vertexUV(x0, y + height, z1, u1, v0);
}

void TileRenderer::tesselateStemDirTexture(Tile &tt, int_t data, int_t dir, double height, double x, double y, double z)
{
	Tesselator &t = Tesselator::instance;
	int_t tex = tt.getTexture(Facing::DOWN, data) + 16;
	if (fixedTexture >= 0)
		tex = fixedTexture;

	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	double u0 = (double)((float)xt / 256.0f);
	double u1 = (double)(((float)xt + 15.99f) / 256.0f);
	double v0 = (double)((float)yt / 256.0f);
	double v1 = ((double)yt + 15.99f * height) / 256.0;
	double x0 = x + 0.5 - 0.5;
	double x1 = x + 0.5 + 0.5;
	double z0 = z + 0.5 - 0.5;
	double z1 = z + 0.5 + 0.5;
	double xc = x + 0.5;
	double zc = z + 0.5;
	if ((dir + 1) / 2 % 2 == 1)
	{
		double tmp = u1;
		u1 = u0;
		u0 = tmp;
	}

	if (dir < 2)
	{
		t.vertexUV(x0, y + height, zc, u0, v0);
		t.vertexUV(x0, y + 0.0, zc, u0, v1);
		t.vertexUV(x1, y + 0.0, zc, u1, v1);
		t.vertexUV(x1, y + height, zc, u1, v0);
		t.vertexUV(x1, y + height, zc, u1, v0);
		t.vertexUV(x1, y + 0.0, zc, u1, v1);
		t.vertexUV(x0, y + 0.0, zc, u0, v1);
		t.vertexUV(x0, y + height, zc, u0, v0);
	}
	else
	{
		t.vertexUV(xc, y + height, z1, u0, v0);
		t.vertexUV(xc, y + 0.0, z1, u0, v1);
		t.vertexUV(xc, y + 0.0, z0, u1, v1);
		t.vertexUV(xc, y + height, z0, u1, v0);
		t.vertexUV(xc, y + height, z0, u1, v0);
		t.vertexUV(xc, y + 0.0, z0, u1, v1);
		t.vertexUV(xc, y + 0.0, z1, u0, v1);
		t.vertexUV(xc, y + height, z1, u0, v0);
	}
}

bool TileRenderer::tesselateFenceGateInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	int_t data = level->getData(x, y, z);
	bool open = FenceGateTile::isOpen(data);
	int_t facing = FenceGateTile::getFacing(data);
	auto box = [&](float x0, float y0, float z0, float x1, float y1, float z1) {
		tt.setShape(x0, y0, z0, x1, y1, z1);
		tesselateBlockInWorld(tt, x, y, z);
	};

	// posts
	if (facing != 3 && facing != 1)
	{
		box(0.0f, 0.3125f, 0.4375f, 0.125f, 1.0f, 0.5625f);
		box(0.875f, 0.3125f, 0.4375f, 1.0f, 1.0f, 0.5625f);
	}
	else
	{
		box(0.4375f, 0.3125f, 0.0f, 0.5625f, 1.0f, 0.125f);
		box(0.4375f, 0.3125f, 0.875f, 0.5625f, 1.0f, 1.0f);
	}

	if (!open)
	{
		if (facing != 3 && facing != 1)
		{
			box(0.375f, 0.375f, 0.4375f, 0.5f, 0.9375f, 0.5625f);
			box(0.5f, 0.375f, 0.4375f, 0.625f, 0.9375f, 0.5625f);
			box(0.625f, 0.375f, 0.4375f, 0.875f, 0.5625f, 0.5625f);
			box(0.625f, 0.75f, 0.4375f, 0.875f, 0.9375f, 0.5625f);
			box(0.125f, 0.375f, 0.4375f, 0.375f, 0.5625f, 0.5625f);
			box(0.125f, 0.75f, 0.4375f, 0.375f, 0.9375f, 0.5625f);
		}
		else
		{
			box(0.4375f, 0.375f, 0.375f, 0.5625f, 0.9375f, 0.5f);
			box(0.4375f, 0.375f, 0.5f, 0.5625f, 0.9375f, 0.625f);
			box(0.4375f, 0.375f, 0.625f, 0.5625f, 0.5625f, 0.875f);
			box(0.4375f, 0.75f, 0.625f, 0.5625f, 0.9375f, 0.875f);
			box(0.4375f, 0.375f, 0.125f, 0.5625f, 0.5625f, 0.375f);
			box(0.4375f, 0.75f, 0.125f, 0.5625f, 0.9375f, 0.375f);
		}
	}
	else if (facing == 3)
	{
		box(0.8125f, 0.375f, 0.0f, 0.9375f, 0.9375f, 0.125f);
		box(0.8125f, 0.375f, 0.875f, 0.9375f, 0.9375f, 1.0f);
		box(0.5625f, 0.375f, 0.0f, 0.8125f, 0.5625f, 0.125f);
		box(0.5625f, 0.375f, 0.875f, 0.8125f, 0.5625f, 1.0f);
		box(0.5625f, 0.75f, 0.0f, 0.8125f, 0.9375f, 0.125f);
		box(0.5625f, 0.75f, 0.875f, 0.8125f, 0.9375f, 1.0f);
	}
	else if (facing == 1)
	{
		box(0.0625f, 0.375f, 0.0f, 0.1875f, 0.9375f, 0.125f);
		box(0.0625f, 0.375f, 0.875f, 0.1875f, 0.9375f, 1.0f);
		box(0.1875f, 0.375f, 0.0f, 0.4375f, 0.5625f, 0.125f);
		box(0.1875f, 0.375f, 0.875f, 0.4375f, 0.5625f, 1.0f);
		box(0.1875f, 0.75f, 0.0f, 0.4375f, 0.9375f, 0.125f);
		box(0.1875f, 0.75f, 0.875f, 0.4375f, 0.9375f, 1.0f);
	}
	else if (facing == 0)
	{
		box(0.0f, 0.375f, 0.8125f, 0.125f, 0.9375f, 0.9375f);
		box(0.875f, 0.375f, 0.8125f, 1.0f, 0.9375f, 0.9375f);
		box(0.0f, 0.375f, 0.5625f, 0.125f, 0.5625f, 0.8125f);
		box(0.875f, 0.375f, 0.5625f, 1.0f, 0.5625f, 0.8125f);
		box(0.0f, 0.75f, 0.5625f, 0.125f, 0.9375f, 0.8125f);
		box(0.875f, 0.75f, 0.5625f, 1.0f, 0.9375f, 0.8125f);
	}
	else if (facing == 2)
	{
		box(0.0f, 0.375f, 0.0625f, 0.125f, 0.9375f, 0.1875f);
		box(0.875f, 0.375f, 0.0625f, 1.0f, 0.9375f, 0.1875f);
		box(0.0f, 0.375f, 0.1875f, 0.125f, 0.5625f, 0.4375f);
		box(0.875f, 0.375f, 0.1875f, 1.0f, 0.5625f, 0.4375f);
		box(0.0f, 0.75f, 0.1875f, 0.125f, 0.9375f, 0.4375f);
		box(0.875f, 0.75f, 0.1875f, 1.0f, 0.9375f, 0.4375f);
	}

	tt.setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

bool TileRenderer::tesselateLilyPadInWorld(Tile &tt, int_t x, int_t y, int_t z)
{
	Tesselator &t = Tesselator::instance;
	int_t tex = tt.tex;
	if (fixedTexture >= 0)
		tex = fixedTexture;

	int_t xt = (tex & 15) << 4;
	int_t yt = tex & 240;
	float h = 0.015625f;
	double u0 = static_cast<double>(static_cast<float>(xt) / 256.0f);
	double u1 = static_cast<double>((static_cast<float>(xt) + 15.99f) / 256.0f);
	double v0 = static_cast<double>(static_cast<float>(yt) / 256.0f);
	double v1 = static_cast<double>((static_cast<float>(yt) + 15.99f) / 256.0f);
	long_t seed = static_cast<long_t>(x * 3129871) ^ static_cast<long_t>(z) * 116129781LL ^ static_cast<long_t>(y);
	seed = seed * seed * 42317861LL + seed * 11LL;
	int_t dir = static_cast<int_t>(seed >> 16 & 3LL);
	float br = tt.getBrightness(*level, x, y, z);
	float cx = static_cast<float>(x) + 0.5f;
	float cz = static_cast<float>(z) + 0.5f;
	float a = static_cast<float>(dir & 1) * 0.5f * static_cast<float>(1 - dir / 2 % 2 * 2);
	float b = static_cast<float>((dir + 1) & 1) * 0.5f * static_cast<float>(1 - (dir + 1) / 2 % 2 * 2);
	int_t col = tt.getItemColor(0);
	auto setColor = [&](int_t c) {
		t.color(br * ((c >> 16) & 0xFF) / 255.0f, br * ((c >> 8) & 0xFF) / 255.0f, br * (c & 0xFF) / 255.0f);
	};
	setColor(col);
	t.vertexUV(cx + a - b, y + h, cz + a + b, u0, v0);
	t.vertexUV(cx + a + b, y + h, cz - a + b, u1, v0);
	t.vertexUV(cx - a + b, y + h, cz - a - b, u1, v1);
	t.vertexUV(cx - a - b, y + h, cz + a - b, u0, v1);
	setColor((col & 0xFEFEFE) >> 1);
	t.vertexUV(cx - a - b, y + h, cz + a - b, u0, v1);
	t.vertexUV(cx - a + b, y + h, cz - a - b, u1, v1);
	t.vertexUV(cx + a + b, y + h, cz - a + b, u1, v0);
	t.vertexUV(cx + a - b, y + h, cz + a + b, u0, v0);
	return true;
}

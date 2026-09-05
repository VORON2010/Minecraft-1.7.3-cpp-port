#include "client/renderer/entity/LightningBoltRenderer.h"

#include <array>

#include "OpenGL.h"
#include "client/renderer/Tesselator.h"
#include "java/Random.h"
#include "world/entity/EntityLightningBolt.h"

LightningBoltRenderer::LightningBoltRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: EntityRenderer(entityRenderDispatcher)
{
}


void LightningBoltRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	(void)rot;
	(void)a;
	EntityLightningBolt &lightning = static_cast<EntityLightningBolt &>(entity);
	Tesselator &t = Tesselator::instance;
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	std::array<double, 8> xOffsets{};
	std::array<double, 8> zOffsets{};
	double xOffset = 0.0;
	double zOffset = 0.0;
	Random random(lightning.randomSeed);

	for (int_t segment = 7; segment >= 0; --segment)
	{
		xOffsets[segment] = xOffset;
		zOffsets[segment] = zOffset;
		xOffset += random.nextInt(11) - 5;
		zOffset += random.nextInt(11) - 5;
	}

	for (int_t layer = 0; layer < 4; ++layer)
	{
		Random branchRandom(lightning.randomSeed);

		for (int_t branch = 0; branch < 3; ++branch)
		{
			int_t endSegment = 7;
			int_t startSegment = 0;
			if (branch > 0)
				endSegment = 7 - branch;
			if (branch > 0)
				startSegment = endSegment - 2;

			double branchX = xOffsets[endSegment] - xOffset;
			double branchZ = zOffsets[endSegment] - zOffset;

			for (int_t segment = endSegment; segment >= startSegment; --segment)
			{
				double previousX = branchX;
				double previousZ = branchZ;
				if (branch == 0)
				{
					branchX += branchRandom.nextInt(11) - 5;
					branchZ += branchRandom.nextInt(11) - 5;
				}
				else
				{
					branchX += branchRandom.nextInt(31) - 15;
					branchZ += branchRandom.nextInt(31) - 15;
				}

				t.begin(GL_TRIANGLE_STRIP);
				float brightness = 0.5f;
				t.color(0.9f * brightness, 0.9f * brightness, 1.0f * brightness, 0.3f);
				double upperRadius = 0.1 + layer * 0.2;
				if (branch == 0)
					upperRadius *= segment * 0.1 + 1.0;

				double lowerRadius = 0.1 + layer * 0.2;
				if (branch == 0)
					lowerRadius *= (segment - 1) * 0.1 + 1.0;

				for (int_t corner = 0; corner < 5; ++corner)
				{
					double upperX = x + 0.5 - upperRadius;
					double upperZ = z + 0.5 - upperRadius;
					if (corner == 1 || corner == 2)
						upperX += upperRadius * 2.0;
					if (corner == 2 || corner == 3)
						upperZ += upperRadius * 2.0;

					double lowerX = x + 0.5 - lowerRadius;
					double lowerZ = z + 0.5 - lowerRadius;
					if (corner == 1 || corner == 2)
						lowerX += lowerRadius * 2.0;
					if (corner == 2 || corner == 3)
						lowerZ += lowerRadius * 2.0;

					t.vertex(lowerX + branchX, y + segment * 16, lowerZ + branchZ);
					t.vertex(upperX + previousX, y + (segment + 1) * 16, upperZ + previousZ);
				}

				t.end();
			}
		}
	}

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}


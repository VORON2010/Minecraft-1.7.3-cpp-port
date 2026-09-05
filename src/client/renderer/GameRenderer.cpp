#include "client/renderer/GameRenderer.h"
#include "util/Profiler.h"

#include <chrono>
#include <thread>

#include "client/Minecraft.h"
#include "client/Lighting.h"
#include "client/gui/ScreenSizeCalculator.h"
#include "client/particle/RainParticle.h"
#include "client/particle/SmokeParticle.h"
#include "client/renderer/Tesselator.h"

#include "client/renderer/culling/FrustumCuller.h"

#include "world/level/chunk/ChunkCache.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/tile/Tile.h"

#include "util/Mth.h"
#include "util/GLU.h"

#include "lwjgl/Display.h"
#include "lwjgl/GLContext.h"

#include "java/System.h"

#include "OpenGL.h"

#include "world/level/material/LiquidMaterial.h"

#include <cstdint>

namespace
{
	int_t javaIntAdd(int_t left, int_t right)
	{
		return static_cast<int_t>(static_cast<std::uint32_t>(left) + static_cast<std::uint32_t>(right));
	}

	int_t javaIntMultiply(int_t left, int_t right)
	{
		return static_cast<int_t>(static_cast<std::uint32_t>(left) * static_cast<std::uint32_t>(right));
	}

	long_t javaLongMultiply(long_t left, long_t right)
	{
		return static_cast<long_t>(static_cast<std::uint64_t>(left) * static_cast<std::uint64_t>(right));
	}

	int_t rainColumnSeed(int_t x, int_t z)
	{
		int_t seed = javaIntMultiply(javaIntMultiply(x, x), 3121);
		seed = javaIntAdd(seed, javaIntMultiply(x, 45238971));
		seed = javaIntAdd(seed, javaIntMultiply(javaIntMultiply(z, z), 418711));
		return javaIntAdd(seed, javaIntMultiply(z, 13761));
	}
}

GameRenderer::GameRenderer(Minecraft &mc) : mc(mc), itemInHandRenderer(ItemInHandRenderer(mc))
{

}


void GameRenderer::tick()
{
	fogBrO = fogBr;
	float brightness = mc.level->getBrightness(Mth::floor(mc.player->x), Mth::floor(mc.player->y), Mth::floor(mc.player->z));
	float dist = (3.0f - mc.options.viewDistance) / 3.0f;
	float fogBrTarget = brightness * (1.0f - dist) + dist;
	fogBr += (fogBrTarget - fogBr) * 0.1f;

	ticks = javaIntAdd(ticks, 1);

	itemInHandRenderer.tick();
	addRainParticles();
}


void GameRenderer::addRainParticles()
{
	float rainStrength = mc.level->getRainStrength(1.0f);
	if (!mc.options.fancyGraphics)
		rainStrength /= 2.0f;

	if (rainStrength == 0.0f)
		return;

	random.setSeed(javaLongMultiply(static_cast<long_t>(ticks), 312987231LL));
	Entity &view = *mc.player;
	Level &level = *mc.level;
	int_t playerX = Mth::floor(view.x);
	int_t playerY = Mth::floor(view.y);
	int_t playerZ = Mth::floor(view.z);
	constexpr int_t radius = 10;
	double soundX = 0.0;
	double soundY = 0.0;
	double soundZ = 0.0;
	int_t rainColumns = 0;

	int_t attempts = static_cast<int_t>(100.0f * rainStrength * rainStrength);
	for (int_t i = 0; i < attempts; ++i)
	{
		int_t x = javaIntAdd(playerX, random.nextInt(radius));
		x = javaIntAdd(x, -random.nextInt(radius));
		int_t z = javaIntAdd(playerZ, random.nextInt(radius));
		z = javaIntAdd(z, -random.nextInt(radius));
		int_t y = level.findTopSolidBlock(x, z);
		int_t tileId = level.getTile(x, y - 1, z);
		const BiomeInfo &biome = level.getBiomeSource().getBiomeInfo(level.getBiomeSource().getBiome(x, z));
		if (y <= playerY + radius && y >= playerY - radius && biome.canSpawnLightningBolt())
		{
			float rx = random.nextFloat();
			float rz = random.nextFloat();
			if (tileId > 0)
			{
				Tile *tile = Tile::tiles[tileId];
				double particleY = static_cast<float>(y) + 0.1f - tile->yy0;
				if (&tile->material == static_cast<const Material *>(&Material::lava))
				{
					mc.particleEngine.add(std::make_unique<SmokeParticle>(level,
						static_cast<float>(x) + rx, particleY, static_cast<float>(z) + rz, 0.0, 0.0, 0.0));
				}
				else
				{
					if (random.nextInt(++rainColumns) == 0)
					{
						soundX = static_cast<float>(x) + rx;
						soundY = particleY;
						soundZ = static_cast<float>(z) + rz;
					}

					mc.particleEngine.add(std::make_unique<RainParticle>(level,
						static_cast<float>(x) + rx, particleY, static_cast<float>(z) + rz));
				}
			}
		}
	}

	if (rainColumns > 0)
	{
		int_t choice = random.nextInt(3);
		int_t threshold = rainSoundCounter;
		rainSoundCounter = javaIntAdd(rainSoundCounter, 1);
		if (choice < threshold)
		{
			rainSoundCounter = 0;
			if (soundY > view.y + 1.0 &&
				level.findTopSolidBlock(Mth::floor(view.x), Mth::floor(view.z)) > Mth::floor(view.y))
			{
				level.playSoundEffect(soundX, soundY, soundZ, u"ambient.weather.rain", 0.1f, 0.5f);
			}
			else
			{
				level.playSoundEffect(soundX, soundY, soundZ, u"ambient.weather.rain", 0.2f, 1.0f);
			}
		}
	}
}

void GameRenderer::itemPlaced()
{
	itemInHandRenderer.itemPlaced();
}

void GameRenderer::itemUsed()
{
	itemInHandRenderer.itemUsed();
}

void GameRenderer::pick(float a)
{
	if (mc.player == nullptr)
		return;

	double range = mc.gameMode->getPickRange();
	mc.hitResult = mc.player->pick(range, a);

	double hitRange = range;
	Vec3 *playerPos = mc.player->getPos(a);
	if (mc.hitResult.type != HitResult::Type::NONE)
		hitRange = mc.hitResult.pos->distanceTo(*playerPos);

	if (mc.gameMode->isCreativeMode())
	{
		range = 32.0;
		hitRange = 32.0;
	}
	else
	{
		if (hitRange > 3)
			hitRange = 3;
		range = hitRange;
	}

	Vec3 *look = mc.player->getViewVector(a);
	Vec3 *to = playerPos->add(look->x * range, look->y * range, look->z * range);
	hovered = nullptr;

	float skin = 1.0f;
	const auto &es = mc.level->getEntities(mc.player.get(), *mc.player->bb.expand(look->x * range, look->y * range, look->z * range)->grow(skin, skin, skin));

	double closestDist = 0.0;

	for (auto &entity : es)
	{
		if (entity->isPickable())
		{
			float radius = entity->getPickRadius();
			AABB *bb = entity->bb.grow(radius, radius, radius);
			HitResult hr = bb->clip(*playerPos, *to);
			if (bb->contains(*playerPos))
			{
				if (0.0 < closestDist || closestDist == 0.0)
				{
					hovered = entity;
					closestDist = 0.0;
				}
			}
			else if (hr.type != HitResult::Type::NONE)
			{
				double clipDist = playerPos->distanceTo(*hr.pos);
				if (clipDist < closestDist || closestDist == 0.0)
				{
					hovered = entity;
					closestDist = clipDist;
				}
			}
		}
	}

	if (hovered != nullptr && !mc.gameMode->isCreativeMode())
		mc.hitResult = HitResult(hovered);
}


float GameRenderer::getFov(float a)
{
	auto &player = mc.player;

	float result = 70.0f;
	if (player->isUnderLiquid(Material::water))
		result = 60.0f;

	if (player->health <= 0)
	{
		float time = player->deathTime + a;
		result /= (1.0f - 500.0f / (time + 500.0f)) * 2.0f + 1.0f;
	}

	return result;
}

void GameRenderer::bobHurt(float a)
{
	auto &player = *mc.player;
	float hurtTime = static_cast<float>(player.hurtTime) - a;

	if (player.health <= 0)
	{
		float deathTime = static_cast<float>(player.deathTime) + a;
		glRotatef(40.0f - 8000.0f / (deathTime + 200.0f), 0.0f, 0.0f, 1.0f);
	}

	if (hurtTime >= 0.0f)
	{
		hurtTime /= static_cast<float>(player.hurtDuration);
		hurtTime = Mth::sin(hurtTime * hurtTime * hurtTime * hurtTime * Mth::PI);
		float yaw = player.hurtDir;
		glRotatef(-yaw, 0.0f, 1.0f, 0.0f);
		glRotatef(-hurtTime * 14.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(yaw, 0.0f, 1.0f, 0.0f);
	}
}

void GameRenderer::bobView(float a)
{
	auto &localPlayer = *mc.player;
	float walkDelta = localPlayer.walkDist - localPlayer.walkDistO;
	float walkDist = -(localPlayer.walkDist + walkDelta * a);
	float bob = localPlayer.oBob + (localPlayer.bob - localPlayer.oBob) * a;
	float tilt = localPlayer.oTilt + (localPlayer.tilt - localPlayer.oTilt) * a;
	glTranslatef(Mth::sin(walkDist * Mth::PI) * bob * 0.5F, -std::abs(Mth::cos(walkDist * Mth::PI) * bob), 0.0F);
	glRotatef(Mth::sin(walkDist * Mth::PI) * bob * 3.0F, 0.0F, 0.0F, 1.0F);
	glRotatef(std::abs(Mth::cos(walkDist * Mth::PI - 0.2F) * bob) * 5.0F, 1.0F, 0.0F, 0.0F);
	glRotatef(tilt, 1.0F, 0.0F, 0.0F);
}

void GameRenderer::moveCameraToPlayer(float a)
{
	auto &player = *mc.player;
	float eyeOffset = player.heightOffset - 1.62f;
	double xOff = player.xOld + (player.x - player.xOld) * a;
	double yOff = player.yOld + (player.y - player.yOld) * a - eyeOffset;
	double zOff = player.zOld + (player.z - player.zOld) * a;

	if (player.sleeping)
	{
		int_t bedDir = 0;
		if (mc.level->hasChunkAt(player.bedX, player.bedY, player.bedZ))
		{
			int_t data = mc.level->getData(player.bedX, player.bedY, player.bedZ);
			bedDir = data & 3;
		}
		glRotatef(bedDir * 90.0f, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, player.heightOffset - 1.62f + 1.0f + 0.3f, 0.0f);
	}
	else if (mc.options.thirdPersonView)
	{
		double distance = 4.0;
		float yRot = player.yRot;
		float xRot = player.xRot;

		double vx = -Mth::sin(yRot / 180.0f * Mth::PI) * Mth::cos(xRot / 180.0f * Mth::PI) * distance;
		double vz = Mth::cos(yRot / 180.0f * Mth::PI) * Mth::cos(xRot / 180.0f * Mth::PI) * distance;
		double vy = -Mth::sin(xRot / 180.0f * Mth::PI) * distance;

		for (int_t i = 0; i < 8; i++)
		{
			float txo = static_cast<float>((i & 1) * 2 - 1);
			float tyo = static_cast<float>(((i >> 1) & 1) * 2 - 1);
			float tzo = static_cast<float>(((i >> 2) & 1) * 2 - 1);
			txo *= 0.1F;
			tyo *= 0.1F;
			tzo *= 0.1F;
			HitResult hr = mc.level->clip(*Vec3::newTemp(xOff + txo, yOff + tyo, zOff + tzo), *Vec3::newTemp(xOff - vx + txo + tzo, yOff - vy + tyo, zOff - vz + tzo));
			if (hr.type != HitResult::Type::NONE)
			{
				double hitd = hr.pos->distanceTo(*Vec3::newTemp(xOff, yOff, zOff));
				if (hitd < distance) distance = hitd;
			}
		}

		glRotatef(player.xRot - xRot, 1.0f, 0.0f, 0.0f);
		glRotatef(player.yRot - yRot, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, 0.0f, static_cast<float>(-distance));
		glRotatef(yRot - player.yRot, 0.0f, 1.0f, 0.0f);
		glRotatef(xRot - player.xRot, 1.0f, 0.0f, 0.0f);

		glRotatef(player.xRotO + (player.xRot - player.xRotO) * a, 1.0f, 0.0f, 0.0f);
		glRotatef(player.yRotO + (player.yRot - player.yRotO) * a + 180.0f, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, player.heightOffset - 1.62f, 0.0f);
	}
	else
	{
		glTranslatef(0.0f, 0.0f, -0.1f);
		glRotatef(player.xRotO + (player.xRot - player.xRotO) * a, 1.0f, 0.0f, 0.0f);
		glRotatef(player.yRotO + (player.yRot - player.yRotO) * a + 180.0f, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, player.heightOffset - 1.62f, 0.0f);
	}
}


void GameRenderer::setupCamera(float a, int_t eye)
{
	renderDistance = static_cast<float>(256 >> mc.options.viewDistance);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float eyeDist = 0.07f;
	if (mc.options.anaglyph3d)
		glTranslatef(-(eye * 2 - 1) * eyeDist, 0.0f, 0.0f);

	if (zoom != 1.0)
	{
		glTranslatef(static_cast<float>(zoom_x), static_cast<float>(-zoom_y), 0.0f);
		glScaled(zoom, zoom, 1.0);
		gluPerspective(getFov(a), static_cast<float>(mc.width) / static_cast<float>(mc.height), 0.05f, renderDistance * 2.0f);
	}
	else
	{
		gluPerspective(getFov(a), static_cast<float>(mc.width) / static_cast<float>(mc.height), 0.05f, renderDistance * 2.0f);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (mc.options.anaglyph3d)
		glTranslatef((eye * 2 - 1) * 0.1f, 0.0f, 0.0f);

	bobHurt(a);
	if (mc.options.bobView)
		bobView(a);

	float portalTime = mc.player->oPortalTime + (mc.player->portalTime - mc.player->oPortalTime) * a;
	if (portalTime > 0.0f)
	{
		float scale = 5.0f / (portalTime * portalTime + 5.0f) - portalTime * 0.04f;
		scale *= scale;
		glRotatef((ticks + a) * 20.0f, 0.0f, 1.0f, 1.0f);
		glScalef(1.0f / scale, 1.0f, 1.0f);
		glRotatef(-(ticks + a) * 20.0f, 0.0f, 1.0f, 1.0f);
	}

	moveCameraToPlayer(a);
}

void GameRenderer::renderItemInHand(float a, int_t eye)
{
	glLoadIdentity();
	if (mc.options.anaglyph3d)
		glTranslatef((eye * 2 - 1) * 0.1f, 0.0f, 0.0f);

	glPushMatrix();

	bobHurt(a);
	if (mc.options.bobView)
		bobView(a);
	if (!mc.options.thirdPersonView && !mc.player->sleeping && !mc.options.hideGui)
		itemInHandRenderer.render(a);

	glPopMatrix();

	if (!mc.options.thirdPersonView && !mc.player->sleeping)
	{
		itemInHandRenderer.renderScreenEffect(a);
		bobHurt(a);
	}
}

void GameRenderer::render(float a)
{
	if (!mc.unattended && !lwjgl::Display::isActive())
	{
		if (System::currentTimeMillis() - lastActiveTime > 500)
			mc.pauseGame();
	}
	else
	{
		lastActiveTime = System::currentTimeMillis();
	}

	
	if (mc.mouseGrabbed)
	{
		mc.mouseHandler.poll();
		if (!mc.player->sleeping)
		{
			float sens = mc.options.mouseSensitivity * 0.6f + 0.2f;
			float sens2 = sens * sens * sens * 8.0f;
			float dx = mc.mouseHandler.xd * sens2;
			float dy = mc.mouseHandler.yd * sens2;
			if (mc.options.smoothCamera)
			{
				dx = mouseFilterXAxis.smooth(dx, 0.05f * sens2);
				dy = mouseFilterYAxis.smooth(dy, 0.05f * sens2);
			}

			mc.player->turn(dx, dy * (mc.options.invertYMouse ? -1 : 1));
		}
	}

	if (mc.noRender)
		return;

	ScreenSizeCalculator ssc(mc.options, mc.width, mc.height);
	int_t w = ssc.getWidth();
	int_t h = ssc.getHeight();

	int_t xm = lwjgl::Mouse::getX() * w / mc.width;
	int_t ym = h - lwjgl::Mouse::getY() * h / mc.height - 1;

	
	
	
	long_t frameBudget = 1000000000LL / (mc.options.limitFramerate == 2 ? 40 : 120);

	if (mc.level != nullptr)
	{
		if (mc.options.limitFramerate == 0)
			renderLevel(a, 0);
		else
			renderLevel(a, lastRenderNano + frameBudget);

		if (mc.options.limitFramerate == 2)
		{
			long_t ms = (lastRenderNano + frameBudget - System::nanoTime()) / 1000000LL;
			if (ms > 0 && ms < 500)
				std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		}

		lastRenderNano = System::nanoTime();
		if (!mc.options.hideGui || mc.screen != nullptr)
		{
			Profiler::Scope guiProfile(Profiler::Section::Gui);
			mc.gui.render(a, mc.screen != nullptr, xm, ym);
		}
	}
	else
	{
		glViewport(0, 0, mc.width, mc.height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		setupGuiScreen();

		if (mc.options.limitFramerate == 2)
		{
			long_t ms = (lastRenderNano + frameBudget - System::nanoTime()) / 1000000LL;
			if (ms < 0)
				ms += 10;
			if (ms > 0 && ms < 500)
				std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		}

		lastRenderNano = System::nanoTime();
	}

	auto screen = mc.screen; 
	if (screen != nullptr)
	{
		Profiler::Scope guiProfile(Profiler::Section::Gui);
		glClear(GL_DEPTH_BUFFER_BIT);
		screen->render(xm, ym, a);
	}
}

void GameRenderer::renderLevel(float a, long_t deadline)
{
	pick(a);

	auto &player = mc.player;
	auto &levelRenderer = mc.levelRenderer;
	
	double xOff = player->xOld + (player->x - player->xOld) * a;
	double yOff = player->yOld + (player->y - player->yOld) * a;
	double zOff = player->zOld + (player->z - player->zOld) * a;


	for (int_t eye = 0; eye < 2; eye++)
	{
		if (mc.options.anaglyph3d)
		{
			if (eye == 0)
				glColorMask(false, true, true, true);
			else
				glColorMask(true, false, false, true);
		}

		glViewport(0, 0, mc.width, mc.height);
		setupClearColor(a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		setupCamera(a, eye);
		Frustum::getFrustum();

		if (mc.options.viewDistance < 2)
		{
			setupFog(-1);
			levelRenderer.renderSky(a);
		}

		glEnable(GL_FOG);
		setupFog(1);

		FrustumCuller culler;
		culler.prepare(xOff, yOff, zOff);

		mc.levelRenderer.cull(culler, a);
		
		
		while (!mc.levelRenderer.updateDirtyChunks(*player, Minecraft::FLYBY_MODE) && deadline != 0)
		{
			long_t remaining = deadline - System::nanoTime();
			if (remaining < 0 || remaining > 1000000000LL)
				break;
		}
		
		setupFog(0);

		glEnable(GL_FOG);
		glBindTexture(GL_TEXTURE_2D, mc.textures.loadTexture(u"/terrain.png"));

		if (mc.options.ambientOcclusion)
			glShadeModel(GL_SMOOTH);

		Lighting::turnOff();
		{
			Profiler::Scope terrainProfile(Profiler::Section::TerrainRender);
			levelRenderer.render(*player, 0, a);
		}
		glShadeModel(GL_FLAT);

		Lighting::turnOn();
		{
			Profiler::Scope entityProfile(Profiler::Section::EntityRender);
			levelRenderer.renderEntities(*player->getPos(a), culler, a);
		}
		{
			Profiler::Scope particleProfile(Profiler::Section::Particles);
			mc.particleEngine.renderLit(*player, a);
		}
		Lighting::turnOff();

		setupFog(0);

		{
			Profiler::Scope particleProfile(Profiler::Section::Particles);
			mc.particleEngine.render(*player, a);
		}

		if (mc.hitResult.type != HitResult::Type::NONE && player->isUnderLiquid(Material::water))
		{
			glDisable(GL_ALPHA_TEST);
			levelRenderer.renderHit(*player, mc.hitResult, 0, player->inventory.getCurrentItem(), a);
			levelRenderer.renderHitOutline(*player, mc.hitResult, 0, player->inventory.getCurrentItem(), a);
			glEnable(GL_ALPHA_TEST);
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		setupFog(0);
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glBindTexture(GL_TEXTURE_2D, mc.textures.loadTexture(u"/terrain.png"));

		if (mc.options.fancyGraphics)
		{
			Profiler::Scope terrainProfile(Profiler::Section::TerrainRender);
			if (mc.options.ambientOcclusion)
				glShadeModel(GL_SMOOTH);

			glColorMask(false, false, false, false);
			int_t count = levelRenderer.render(*player, 1, a);
			glColorMask(true, true, true, true);
		
			if (mc.options.anaglyph3d)
			{
				if (eye == 0)
					glColorMask(false, true, true, true);
				else
					glColorMask(true, false, false, true);
			}
		
			if (count > 0)
				levelRenderer.renderSameAsLast(1, a);

			glShadeModel(GL_FLAT);
		}
		else
		{
			Profiler::Scope terrainProfile(Profiler::Section::TerrainRender);
			if (mc.options.ambientOcclusion)
				glShadeModel(GL_SMOOTH);

			levelRenderer.render(*player, 1, a);

			glShadeModel(GL_FLAT);
		}

		glDepthMask(true);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);

		if (zoom == 1.0 && mc.hitResult.type != HitResult::Type::NONE && !player->isUnderLiquid(Material::water))
		{
			glDisable(GL_ALPHA_TEST);
			levelRenderer.renderHit(*player, mc.hitResult, 0, player->inventory.getCurrentItem(), a);
			levelRenderer.renderHitOutline(*player, mc.hitResult, 0, player->inventory.getCurrentItem(), a);
			glEnable(GL_ALPHA_TEST);
		}

		renderRainSnow(a);
		glDisable(GL_FOG);
		if (hovered != nullptr)
		{

		}

		setupFog(0);
		glEnable(GL_FOG);
		levelRenderer.renderClouds(a);
		glDisable(GL_FOG);

		if (!Minecraft::FLYBY_MODE)
		{
			setupFog(1);
			if (zoom == 1.0)
			{
				glClear(GL_DEPTH_BUFFER_BIT);
				renderItemInHand(a, eye);
			}
		}

		if (!mc.options.anaglyph3d)
			return;
	}

	glColorMask(true, true, true, true);
}


void GameRenderer::renderRainSnow(float a)
{
	float rainStrength = mc.level->getRainStrength(a);
	if (rainStrength <= 0.0f)
		return;

	Entity &view = *mc.player;
	Level &level = *mc.level;
	int_t playerX = Mth::floor(view.x);
	int_t playerY = Mth::floor(view.y);
	int_t playerZ = Mth::floor(view.z);
	Tesselator &t = Tesselator::instance;
	glDisable(GL_CULL_FACE);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 0.01f);
	glBindTexture(GL_TEXTURE_2D, mc.textures.loadTexture(u"/environment/snow.png"));
	double cameraX = view.xOld + (view.x - view.xOld) * a;
	double cameraY = view.yOld + (view.y - view.yOld) * a;
	double cameraZ = view.zOld + (view.z - view.zOld) * a;
	int_t cameraFloorY = Mth::floor(cameraY);
	int_t radius = 5;
	if (mc.options.fancyGraphics)
		radius = 10;

	int_t diameter = radius * 2 + 1;
	BiomeSource &biomeSource = level.getBiomeSource();
	biomeSource.getBiomeBlock(playerX - radius, playerZ - radius, diameter, diameter);
	const std::vector<BiomeId> &biomes = biomeSource.biomes;
	int_t biomeIndex = 0;

	for (int_t x = playerX - radius; x <= playerX + radius; ++x)
	{
		for (int_t z = playerZ - radius; z <= playerZ + radius; ++z)
		{
			const BiomeInfo &biome = biomeSource.getBiomeInfo(biomes[static_cast<std::size_t>(biomeIndex++)]);
			if (biome.enableSnow)
			{
				int_t top = level.findTopSolidBlock(x, z);
				if (top < 0)
					top = 0;
				int_t brightnessY = top;
				if (top < cameraFloorY)
					brightnessY = cameraFloorY;
				int_t y0 = playerY - radius;
				int_t y1 = playerY + radius;
				if (y0 < top)
					y0 = top;
				if (y1 < top)
					y1 = top;
				float scale = 1.0f;
				if (y0 != y1)
				{
					int_t seed = rainColumnSeed(x, z);
					random.setSeed(static_cast<long_t>(seed));
					float time = ticks + a;
					float scroll = ((ticks & 511) + a) / 512.0f;
					float randomU = random.nextFloat();
					float gaussianU = static_cast<float>(random.nextGaussian());
					float uOffset = randomU + time * 0.01f * gaussianU;
					float randomV = random.nextFloat();
					float gaussianV = static_cast<float>(random.nextGaussian());
					float vOffset = randomV + time * gaussianV * 0.001f;
					double dx = static_cast<float>(x) + 0.5f - view.x;
					double dz = static_cast<float>(z) + 0.5f - view.z;
					float distance = Mth::sqrt(dx * dx + dz * dz) / radius;
					t.begin();
					float brightness = level.getBrightness(x, brightnessY, z);
					glColor4f(brightness, brightness, brightness,
						((1.0f - distance * distance) * 0.3f + 0.5f) * rainStrength);
					t.offset(-cameraX * 1.0, -cameraY * 1.0, -cameraZ * 1.0);
					t.vertexUV(x + 0, y0, z + 0.5, 0.0f * scale + uOffset,
						y0 * scale / 4.0f + scroll * scale + vOffset);
					t.vertexUV(x + 1, y0, z + 0.5, 1.0f * scale + uOffset,
						y0 * scale / 4.0f + scroll * scale + vOffset);
					t.vertexUV(x + 1, y1, z + 0.5, 1.0f * scale + uOffset,
						y1 * scale / 4.0f + scroll * scale + vOffset);
					t.vertexUV(x + 0, y1, z + 0.5, 0.0f * scale + uOffset,
						y1 * scale / 4.0f + scroll * scale + vOffset);
					t.vertexUV(x + 0.5, y0, z + 0, 0.0f * scale + uOffset,
						y0 * scale / 4.0f + scroll * scale + vOffset);
					t.vertexUV(x + 0.5, y0, z + 1, 1.0f * scale + uOffset,
						y0 * scale / 4.0f + scroll * scale + vOffset);
					t.vertexUV(x + 0.5, y1, z + 1, 1.0f * scale + uOffset,
						y1 * scale / 4.0f + scroll * scale + vOffset);
					t.vertexUV(x + 0.5, y1, z + 0, 0.0f * scale + uOffset,
						y1 * scale / 4.0f + scroll * scale + vOffset);
					t.offset(0.0, 0.0, 0.0);
					t.end();
				}
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D, mc.textures.loadTexture(u"/environment/rain.png"));
	if (mc.options.fancyGraphics)
		radius = 10;

	biomeIndex = 0;
	for (int_t x = playerX - radius; x <= playerX + radius; ++x)
	{
		for (int_t z = playerZ - radius; z <= playerZ + radius; ++z)
		{
			const BiomeInfo &biome = biomeSource.getBiomeInfo(biomes[static_cast<std::size_t>(biomeIndex++)]);
			if (biome.canSpawnLightningBolt())
			{
				int_t top = level.findTopSolidBlock(x, z);
				int_t y0 = playerY - radius;
				int_t y1 = playerY + radius;
				if (y0 < top)
					y0 = top;
				if (y1 < top)
					y1 = top;
				float scale = 1.0f;
				if (y0 != y1)
				{
					int_t seed = rainColumnSeed(x, z);
					random.setSeed(static_cast<long_t>(seed));
					int_t phase = javaIntAdd(ticks, seed) & 31;
					float scroll = (phase + a) / 32.0f * (3.0f + random.nextFloat());
					double dx = static_cast<float>(x) + 0.5f - view.x;
					double dz = static_cast<float>(z) + 0.5f - view.z;
					float distance = Mth::sqrt(dx * dx + dz * dz) / radius;
					t.begin();
					float brightness = level.getBrightness(x, 128, z) * 0.85f + 0.15f;
					glColor4f(brightness, brightness, brightness,
						((1.0f - distance * distance) * 0.5f + 0.5f) * rainStrength);
					t.offset(-cameraX * 1.0, -cameraY * 1.0, -cameraZ * 1.0);
					t.vertexUV(x + 0, y0, z + 0.5, 0.0f * scale,
						y0 * scale / 4.0f + scroll * scale);
					t.vertexUV(x + 1, y0, z + 0.5, 1.0f * scale,
						y0 * scale / 4.0f + scroll * scale);
					t.vertexUV(x + 1, y1, z + 0.5, 1.0f * scale,
						y1 * scale / 4.0f + scroll * scale);
					t.vertexUV(x + 0, y1, z + 0.5, 0.0f * scale,
						y1 * scale / 4.0f + scroll * scale);
					t.vertexUV(x + 0.5, y0, z + 0, 0.0f * scale,
						y0 * scale / 4.0f + scroll * scale);
					t.vertexUV(x + 0.5, y0, z + 1, 1.0f * scale,
						y0 * scale / 4.0f + scroll * scale);
					t.vertexUV(x + 0.5, y1, z + 1, 1.0f * scale,
						y1 * scale / 4.0f + scroll * scale);
					t.vertexUV(x + 0.5, y1, z + 0, 0.0f * scale,
						y1 * scale / 4.0f + scroll * scale);
					t.offset(0.0, 0.0, 0.0);
					t.end();
				}
			}
		}
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glAlphaFunc(GL_GREATER, 0.1f);
}

void GameRenderer::setupGuiScreen()
{
	ScreenSizeCalculator ssc(mc.options, mc.width, mc.height);
	int_t w = ssc.getWidth();
	int_t h = ssc.getHeight();

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, w, h, 0.0, 1000.0, 3000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -2000.0f);
}


void GameRenderer::setupClearColor(float a)
{
	auto &level = mc.level;
	auto &player = mc.player;

	float dist = 1.0f / (4 - mc.options.viewDistance);
	dist = 1.0f - static_cast<float>(std::pow(dist, 0.25));

	Vec3 *skyColor = level->getSkyColor(*mc.player, a);
	float sr = static_cast<float>(skyColor->x);
	float sg = static_cast<float>(skyColor->y);
	float sb = static_cast<float>(skyColor->z);

	Vec3 *fogColor = level->getFogColor(a);
	fr = static_cast<float>(fogColor->x);
	fg = static_cast<float>(fogColor->y);
	fb = static_cast<float>(fogColor->z);

	fr += (sr - fr) * dist;
	fg += (sg - fg) * dist;
	fb += (sb - fb) * dist;

	float rainStrength = level->getRainStrength(a);
	if (rainStrength > 0.0f)
	{
		float redGreenScale = 1.0f - rainStrength * 0.5f;
		float blueScale = 1.0f - rainStrength * 0.4f;
		fr *= redGreenScale;
		fg *= redGreenScale;
		fb *= blueScale;
	}

	float thunderStrength = level->getThunderStrength(a);
	if (thunderStrength > 0.0f)
	{
		float scale = 1.0f - thunderStrength * 0.5f;
		fr *= scale;
		fg *= scale;
		fb *= scale;
	}

	
	if (mc.player->isUnderLiquid(Material::water))
	{
		fr = 0.02f;
		fg = 0.02f;
		fb = 0.2f;
	}
	else if (mc.player->isUnderLiquid(Material::lava))
	{
		fr = 0.6f;
		fg = 0.1f;
		fb = 0.0f;
	}
	float fogBrNow = fogBrO + (fogBr - fogBrO) * a;
	fr *= fogBrNow;
	fg *= fogBrNow;
	fb *= fogBrNow;

	if (mc.options.anaglyph3d)
	{
		float frr = (fr * 30.0f + fg * 59.0f + fb * 11.0f) / 100.0f;
		float fgg = (fr * 30.0f + fg * 70.0f) / 100.0f;
		float fbb = (fr * 30.0f + fb * 70.0f) / 100.0f;
		fr = frr;
		fg = fgg;
		fb = fbb;
	}

	glClearColor(fr, fg, fb, 0.0f);
}

void GameRenderer::setupFog(int_t mode)
{
	auto &player = mc.player;

	float fog[4] = {fr, fg, fb, 1.0f};
	glFogfv(GL_FOG_COLOR, fog);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	
	if (mc.player->isUnderLiquid(Material::water))
	{
		glFogi(GL_FOG_MODE, GL_EXP);
		glFogf(GL_FOG_DENSITY, 0.1f);
	}
	else if (mc.player->isUnderLiquid(Material::lava))
	{
		glFogi(GL_FOG_MODE, GL_EXP);
		glFogf(GL_FOG_DENSITY, 2.0f);
	}
	else
	{
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, renderDistance * 0.25f);
		glFogf(GL_FOG_END, renderDistance);

		if (mode < 0)
		{
			glFogf(GL_FOG_START, 0.0f);
		glFogf(GL_FOG_END, renderDistance * 0.8f);
		}

		if (lwjgl::GLContext::getCapabilities()["GL_NV_fog_distance"])
			glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);

		if (mc.level->dimension->foggy)
			glFogf(GL_FOG_START, 0.0f);
	}

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT);
}

void GameRenderer::updateAllChunks()
{
	mc.levelRenderer.updateDirtyChunks(*mc.player, true);
}

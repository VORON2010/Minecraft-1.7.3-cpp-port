#include "Minecraft.h"

#include <iostream>
#include <stdexcept>
#include <thread>

#include "SharedConstants.h"

#include "client/renderer/Chunk.h"
#include "client/renderer/Tesselator.h"
#include "client/gui/DeathScreen.h"
#include "client/gui/ConnectingScreen.h"
#include "client/gui/InventoryScreen.h"
#include "client/gui/ScreenSizeCalculator.h"
#include "client/gui/ChatScreen.h"
#include "client/gui/PauseScreen.h"
#include "client/gui/AchievementToast.h"
#include "client/gui/SleepScreen.h"
#include "client/title/TitleScreen.h"
#include "client/player/KeyboardInput.h"
#include "client/spc/SPCCommand.h"
#include "client/ScreenShotHelper.h"
#include "client/gui/ConflictWarningScreen.h"
#include "MinecraftException.h"

#include "client/gamemode/SurvivalMode.h"
#include "client/gamemode/CreativeMode.h"
#include "client/gamemode/MultiplayerGameMode.h"

#include "world/phys/Vec3.h"
#include "world/phys/AABB.h"
#include "world/level/chunk/ChunkCache.h"
#include "world/level/Level.h"
#include "world/level/SaveConverterMcRegion.h"
#include "world/level/Teleporter.h"
#include "nbt/CompoundTag.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/SlabTile.h"
#include "world/level/tile/StoneTile.h"
#include "world/item/Items.h"
#include "world/stats/Achievement.h"
#include "world/stats/AchievementList.h"
#include "world/stats/StatFileWriter.h"
#include "world/stats/StatList.h"

#include "java/System.h"
#include "java/Runtime.h"
#include "java/File.h"

#include "util/Mth.h"
#include "util/Profiler.h"

#include "lwjgl/Display.h"
#include "lwjgl/Keyboard.h"

#include "CrashHandler.h"

const jstring Minecraft::VERSION_STRING = u"CPPCRAFT " + SharedConstants::VERSION_STRING;

std::array<long_t, 512> Minecraft::frameTimes = {};
std::array<long_t, 512> Minecraft::tickTimes = {};
int_t Minecraft::frameTimePos = 0;

std::shared_ptr<File> Minecraft::workDir;

Minecraft::Minecraft(int_t width, int_t height, bool fullscreen)
{
	this->width = width;
	this->height = height;
	this->fullscreen = fullscreen;
}

void Minecraft::onCrash(const std::string &message, const std::exception &e)
{
	CrashHandler::Crash(message + ": " + e.what());
}

void Minecraft::init(std::shared_ptr<File> directory)
{
	Tile::initTiles();
	Items::initItems();
	
	if (fullscreen)
	{
		lwjgl::Display::setFullscreen(true);
		width = lwjgl::Display::getDisplayMode().getWidth();
		height = lwjgl::Display::getDisplayMode().getHeight();
		if (width < 1)
			width = 1;
		if (height < 1)
			height = 1;
	}
	else
	{
		lwjgl::Display::setDisplayMode(lwjgl::DisplayMode(width, height));
	}

	lwjgl::Display::setTitle(VERSION_STRING);

	lwjgl::Display::create(unattended);

	workingDirectory = directory != nullptr ? std::move(directory) : getWorkingDirectory();

	if (!unattended)
		options.open(workingDirectory.get());
	if (!unattended)
		texturePackRepository.updateListAndSelect();
	textures.setTileSize();

	font = std::make_unique<Font>(options, u"/font/default.png", textures);
	SPCCommand::setMessageFont(font.get());
	StatList::init();
	AchievementList::openInventory->setDescriptionFormatter([this](const jstring &description)
	{
		jstring result = description;
		size_t position = result.find(u"%1$s");
		if (position != jstring::npos)
			result.replace(position, 4, lwjgl::Keyboard::getKeyName(options.keyInventory.key));
		return result;
	});
	statFileWriter = std::make_unique<StatFileWriter>(*user, *workingDirectory);
	achievementToast = std::make_unique<AchievementToast>(*this);

	checkGlError("Pre startup");

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);
	glCullFace(GL_BACK);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	checkGlError("Startup");

	
	soundEngine.init(&options);
	std::unique_ptr<File> resourceDir(File::openResourceDirectory());
	if (resourceDir && resourceDir->exists() && resourceDir->isDirectory())
		loadAllSounds(resourceDir.get(), u"");
	if (!unattended)
	{
		if (serverHost.empty())
			setScreen(Util::make_shared<TitleScreen>(*this));
		else
			setScreen(Util::make_shared<ConnectingScreen>(*this, serverHost, serverPort));
	}
}

void Minecraft::renderLoadingScreen()
{
	ScreenSizeCalculator ssc(options, width, height);
	int_t w = ssc.getWidth();
	int_t h = ssc.getHeight();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, w, h, 0.0, 1000.0, 3000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -2000.0f);

	glViewport(0, 0, width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	Tesselator &tesselator = Tesselator::instance;
	
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_FOG);

	glBindTexture(GL_TEXTURE_2D, textures.loadTexture(u"/title/mojang.png"));

	tesselator.begin();
	tesselator.color(0xFFFFFF);
	tesselator.vertexUV(0.0, height, 0.0, 0.0, 0.0);
	tesselator.vertexUV(width, height, 0.0, 0.0, 0.0);
	tesselator.vertexUV(width, 0.0, 0.0, 0.0, 0.0);
	tesselator.vertexUV(0.0, 0.0, 0.0, 0.0, 0.0);
	tesselator.end();

	short_t gw = 256;
	short_t gh = 256;
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	tesselator.color(0xFFFFFF);
	blit((width / 2 - gw) / 2, (height / 2 - gh) / 2, 0, 0, gw, gh);

	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);

	lwjgl::Display::swapBuffers();
}

void Minecraft::blit(int_t x, int_t y, int_t sx, int_t sy, int_t w, int_t h)
{
	const float us = 1.0f / 256.0f;
	const float vs = 1.0f / 256.0f;
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.vertexUV(x + 0, y + h, 0.0, (sx + 0) * us, (sy + h) * vs);
	t.vertexUV(x + w, y + h, 0.0, (sx + w) * us, (sy + h) * vs);
	t.vertexUV(x + w, y + 0, 0.0, (sx + w) * us, (sy + 0) * vs);
	t.vertexUV(x + 0, y + 0, 0.0, (sx + 0) * us, (sy + 0) * vs);
	t.end();
}

const std::shared_ptr<File> &Minecraft::getWorkingDirectory()
{
	if (workDir == nullptr)
	{
		workDir.reset(File::openWorkingDirectory(u".mcbetacpp"));
		if (!workDir->exists() && !workDir->mkdirs())
			throw std::runtime_error(String::toUTF8(u"The working directory could not be created: " + workDir->toString()));
	}

	return workDir;
}

void Minecraft::setScreen(std::shared_ptr<Screen> screen)
{
	if (this->screen != nullptr)
		this->screen->removed();
	if (statFileWriter != nullptr)
		statFileWriter->syncStats();

	if (screen == nullptr && level == nullptr)
		screen = Util::make_shared<TitleScreen>(*this);
	else if (screen == nullptr && player != nullptr && player->health <= 0)
		screen = Util::make_shared<DeathScreen>(*this);
	if (std::dynamic_pointer_cast<TitleScreen>(screen) != nullptr)
		SPCCommand::messages.clear();

	this->screen = std::move(screen);
	if (this->screen != nullptr)
	{
		releaseMouse();
		ScreenSizeCalculator ssc(options, width, height);
		int_t w = ssc.getWidth();
		int_t h = ssc.getHeight();
		this->screen->init(w, h);
		noRender = false;
	}
	else
	{
		grabMouse();
	}
}

void Minecraft::checkGlError(const std::string &at)
{
	GLenum error_code = glGetError();
	if (error_code != GL_NO_ERROR)
	{
		std::string error_string;
		switch (error_code)
		{
			default:
				error_string = "unknown error code";
				break;
			case GL_NO_ERROR:
				error_string = "no error";
				break;
			case GL_INVALID_ENUM:
				error_string = "invalid enumerant";
				break;
			case GL_INVALID_VALUE:
				error_string = "invalid value";
				break;
			case GL_INVALID_OPERATION:
				error_string = "invalid operation";
				break;
			case GL_STACK_OVERFLOW:
				error_string = "stack overflow";
				break;
			case GL_STACK_UNDERFLOW:
				error_string = "stack underflow";
				break;
			case GL_OUT_OF_MEMORY:
				error_string = "out of memory";
				break;
		}

		std::cout << "########## GL ERROR ##########\n";
		std::cout << "@ " << at << '\n';
		std::cout << error_code << ": " << error_string << '\n';
	}
}

Minecraft::~Minecraft()
{
	SPCCommand::setMessageFont(nullptr);
	for (std::thread &thread : connectionThreads)
	{
		if (thread.joinable())
			thread.join();
	}
	if (statFileWriter != nullptr)
	{
		statFileWriter->syncStats();
		statFileWriter.reset();
	}
	achievementToast.reset();
	MemoryTracker::release();
}

void Minecraft::generateFlyby()
{
	gameMode = Util::make_shared<SurvivalMode>(*this);
	selectLevel(u"flyby");
	setScreen(nullptr);

	double player_y = 0.0;

	
	std::vector<char> image_data(width * height * 3);
	std::unique_ptr<File> flyby_file(File::open(*workingDirectory, u"flyby"));
	flyby_file->mkdir();

	char header[18] = {};
	header[2] = 2;
	header[12] = width & 0xFF;
	header[13] = (width >> 8) & 0xFF;
	header[14] = height & 0xFF;
	header[15] = (height >> 8) & 0xFF;
	header[16] = 24;

	
	int_t frame = -20;
	int_t seconds = 60 * 5 + 52;
	int_t frames = seconds * 60;

	player->yRot = player->yRotO = 12.0f;
	double sin = -std::sin(player->yRot * Mth::PI / 180.0);
	double cos = std::cos(player->yRot * Mth::PI / 180.0);

	player->x = player->xo = player->xOld = 0.0;
	player->z = player->zo = player->zOld = 0.0;

	level->time = 0;

	for (; frame < frames; frame++)
	{
		if ((frame % 100) == 0)
		{
			std::cout << (static_cast<double>(frame) * 100.0 / static_cast<double>(frames)) << "%, free: " << (float)(Runtime::getRuntime().freeMemory() / 1024) / 1024.0f << " MB\n";
		}

		double speed = 0.125 + (static_cast<double>(frame) / static_cast<double>(frames)) * 5.0;

		
		AABB::resetPool();
		Vec3::resetPool();

		if (frame < 0)
		{
			level->setSpawnSettings(options.difficulty > 0, true);
			level->tick();
		}

		gameRenderer.tick();

		glEnable(GL_TEXTURE_2D);

		while (level->updateLights());

		
		player->x = player->xo = player->xOld += sin * speed;
		player->z = player->zo = player->zOld += cos * speed;

		int_t check_distance = 100;
		double min_height = 0.0;
		double check_step = 1.0;

		for (double d = -4.0; d < check_distance; d += check_step)
		{
			for (int_t i = 0; i < 9; i++)
			{
				double x = ((i % 3) / 2.0f) - 0.5;
				double z = ((i / 3) / 2.0f) - 0.5;
				double y = level->getHeightmap(Mth::floor(player->x + sin * d + x), Mth::floor(player->z + cos * d + z));
				if (y > min_height)
					min_height = y;
			}
		}

		double target_y = min_height + 4.0;
		if (player_y == 0.0)
			player_y = target_y;
		else
			player_y += (target_y - player_y) * speed / check_distance * 4.0;

		player->xRot = player->xRotO = static_cast<float>((player_y - 64.0) / 2.0);
		player->y = player->yo = player->yOld = player_y;

		
		gameRenderer.renderLevel(1.0f, 0);

		lwjgl::Display::update();

		
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, image_data.data());

		if (frame >= 0)
		{
			jstring name = String::toString(frame);
			for (; name.size() < 6; name = u"0" + name);

			std::unique_ptr<File> file(File::open(*flyby_file, name + u".tga"));
			std::unique_ptr<std::ostream> out(file->toStreamOut());
			out->write(header, 18);
			out->write(image_data.data(), image_data.size());
		}
	}
}

void Minecraft::run()
{
	
	running = true;

#ifdef NDEBUG
	try
	{
#endif
		init();
#ifdef NDEBUG
	}
	catch (std::exception &e)
	{
		onCrash("Failed to start game", e);
		return;
	}
#endif

#ifdef NDEBUG
	try
	{
#endif
		if (FLYBY_MODE)
			generateFlyby();

		long_t fpsMs = System::currentTimeMillis();
		int_t fpsFrames = 0;
		int_t fpsTicks = 0;

		while (running)
		{
			Profiler::Scope frameProfile(Profiler::Section::Frame);
			AABB::resetPool();
			Vec3::resetPool();

			if (lwjgl::Display::isCloseRequested())
				stop();

			if (pause && level != nullptr)
			{
				float aO = timer.a;
				timer.advanceTime();
				timer.a = aO;
			}
			else
			{
				timer.advanceTime();
			}

			
			long_t tickNano = System::nanoTime();
			for (byte_t i = 0; i < timer.ticks; i++)
			{
				ticks++;
				fpsTicks++;
				try
				{
					tick();
				}
				catch (MinecraftException &)
				{
					
					
					level = nullptr;
					setLevel(nullptr);
					setScreen(Util::make_shared<ConflictWarningScreen>(*this));
					break;
				}
			}
			long_t tickNanos = System::nanoTime() - tickNano;

			
			if (player != nullptr)
			{
				Profiler::Scope soundProfile(Profiler::Section::Sound);
				soundEngine.update(player.get(), timer.a);
			}

			checkGlError("Pre render");

			glEnable(GL_TEXTURE_2D);
			glEnable(GL_ALPHA_TEST);

			if (level != nullptr && !level->isOnline)
				level->updateLights();
			
			if (level != nullptr && level->isOnline)
				level->updateLights();


			if (!lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_F7))
				lwjgl::Display::update();

			if (!noRender)
			{
				if (gameMode != nullptr)
					gameMode->render(timer.a);
				gameRenderer.render(timer.a);
				if (achievementToast != nullptr)
					achievementToast->render();
			}

			if (!lwjgl::Display::isActive())
			{
				if (fullscreen)
					toggleFullscreen();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			if (options.showDebugInfo)
			{
				renderFpsMeter(tickNanos);
			}
			else
			{
				lastTimer = System::nanoTime();
			}

			std::this_thread::yield();
			if (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_F7))
				lwjgl::Display::update();

			screenshotListener();

			if (!fullscreen)
			{
				int_t w = lwjgl::Display::getWidth();
				int_t h = lwjgl::Display::getHeight();
				if (w != width || h != height)
				{
					width = w;
					height = h;
					if (width <= 0)
						width = 1;
					if (height <= 0)
						height = 1;
					resize(width, height);
				}
			}

			checkGlError("Post render");
			fpsFrames++;

			pause = !isOnline() && screen != nullptr && screen->isPauseScreen();

			while (System::currentTimeMillis() >= fpsMs + 1000)
			{
				fpsString = String::fromUTF8(std::to_string(fpsFrames) + " fps, " + std::to_string(Chunk::updates) + " chunk updates");
				Chunk::updates = 0;
				fpsMs += 1000;
				fpsFrames = 0;
			}
		}
#ifdef NDEBUG
	}
	catch (std::exception &e)
	{
		onCrash("Unexpected error", e);
		return;
	}
#endif
}

void Minecraft::renderFpsMeter(long_t tickNanos)
{
	
	long_t target = 16666666LL; 
	if (lastTimer == -1)
		lastTimer = System::nanoTime();

	long_t now = System::nanoTime();

	tickTimes[frameTimePos & (frameTimes.size() - 1)] = tickNanos;
	frameTimes[frameTimePos & (frameTimes.size() - 1)] = now - lastTimer;
	frameTimePos++;

	lastTimer = now;

	
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, height, 0.0, 1000.0, 3000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -2000.0f);

	glLineWidth(1.0f);
	glDisable(GL_TEXTURE_2D);

	Tesselator &t = Tesselator::instance;
	t.begin(GL_QUADS);

	int_t targetHeight = static_cast<int_t>(target / 200000LL);

	t.color(0x20000000);
	t.vertex(0.0, this->height - targetHeight, 0.0);
	t.vertex(0.0, this->height, 0.0);
	t.vertex(static_cast<double>(frameTimes.size()), this->height, 0.0);
	t.vertex(static_cast<double>(frameTimes.size()), this->height - targetHeight, 0.0);

	t.color(0x20200000);
	t.vertex(0.0, this->height - targetHeight * 2, 0.0);
	t.vertex(0.0, this->height - targetHeight, 0.0);
	t.vertex(static_cast<double>(frameTimes.size()), this->height - targetHeight, 0.0);
	t.vertex(static_cast<double>(frameTimes.size()), this->height - targetHeight * 2, 0.0);

	t.end();

	long_t total = 0;
	for (int_t i = 0; i < frameTimes.size(); i++)
		total += frameTimes[i];
	int_t avg = static_cast<int_t>(total / 200000LL / static_cast<long_t>(frameTimes.size()));

	t.begin(GL_QUADS);
	t.color(0x20400000);
	t.vertex(0.0, (height - avg), 0.0);
	t.vertex(0.0, height, 0.0);
	t.vertex(static_cast<double>(frameTimes.size()), height, 0.0);
	t.vertex(static_cast<double>(frameTimes.size()), (height - avg), 0.0);
	t.end();

	t.begin(GL_LINES);
	for (int_t i = 0; i < frameTimes.size(); i++)
	{
		int_t x = ((i - frameTimePos) & (static_cast<int_t>(frameTimes.size()) - 1)) * 255 / static_cast<int_t>(frameTimes.size());
		int_t m = x * x / 255;
		m = m * m / 255;
		int_t n = m * m / 255;
		n = n * n / 255;

		if (frameTimes[i] > target)
			t.color(0xFF000000 + (m * 0x010000));
		else
			t.color(0xFF000000 + (m * 0x0100));

		long_t ft = frameTimes[i] / 200000LL;
		long_t tt = tickTimes[i] / 200000LL;

		t.vertex((i + 0.5F), (static_cast<float>(height - ft) + 0.5F), 0.0);
		t.vertex((i + 0.5F), (height + 0.5F), 0.0);

		t.color(0xFF000000 + (m * 0x010000) + (m * 0x0100) + (m * 0x01));
		t.vertex((i + 0.5F), (static_cast<float>(height - ft) + 0.5F), 0.0);
		t.vertex((i + 0.5F), (static_cast<float>(height - ft - tt) + 0.5F), 0.0);
	}
	t.end();

	glEnable(GL_TEXTURE_2D);
}

void Minecraft::stop()
{
	running = false;
}

void Minecraft::grabMouse()
{
	if (unattended || !lwjgl::Display::isActive())
		return;
	if (mouseGrabbed)
		return;
	mouseGrabbed = true;
	mouseHandler.grab();
	setScreen(nullptr);
	lastClickTick = ticks + 10000;
}

void Minecraft::releaseMouse()
{
	if (!mouseGrabbed)
		return;
	mouseGrabbed = false;
	mouseHandler.release();
}

void Minecraft::pauseGame()
{
	if (this->screen != nullptr)
		return;
	setScreen(Util::make_shared<PauseScreen>(*this));
}

void Minecraft::handleMouseDown(int_t button, bool down)
{
	if (SPCCommand::weWandEnabled && this->player)
	{
		ItemInstance *selected = this->player->getSelectedItem();
		if (selected != nullptr && selected->itemID == 271)
			return; // Don't break blocks when holding wand
	}
	if (gameMode->instaBuild)
		return;
	if (button == 0 && missTime > 0)
		return;
	if (down && hitResult.type == HitResult::Type::TILE && button == 0)
	{
		int_t x = hitResult.x;
		int_t y = hitResult.y;
		int_t z = hitResult.z;
		gameMode->continueDestroyBlock(x, y, z, hitResult.f);
		particleEngine.crack(x, y, z, static_cast<int_t>(hitResult.f));
	}
	else
	{
		gameMode->stopDestroyBlock();
	}
}

void Minecraft::handleMouseClick(int_t button)
{
	if (button == 0 && missTime > 0)
		return;
	if (button == 0)
		player->swing();

	bool canUseItem = true;

	auto player = std::static_pointer_cast<Player>(this->player);
	
	if (hitResult.type == HitResult::Type::TILE && SPCCommand::weWandEnabled)
	{
		ItemInstance *selected = player->getSelectedItem();
		if (selected != nullptr && selected->itemID == 271) // Wooden axe
		{
			if (button == 0 || button == 1)
			{
				if (button == 0) {
					SPCCommand::weHasPos1 = true;
					SPCCommand::wePos1X = hitResult.x;
					SPCCommand::wePos1Y = hitResult.y;
					SPCCommand::wePos1Z = hitResult.z;
					SPCCommand::addMessage(u"Pos1 set");
				} else {
					SPCCommand::weHasPos2 = true;
					SPCCommand::wePos2X = hitResult.x;
					SPCCommand::wePos2Y = hitResult.y;
					SPCCommand::wePos2Z = hitResult.z;
					SPCCommand::addMessage(u"Pos2 set");
				}
				this->player->swing();
				return;
			}
		}
	}

	if (hitResult.type == HitResult::Type::NONE)
	{
		if (button == 0 && !gameMode->isCreativeMode())
			missTime = 10;
	}
	else if (hitResult.type == HitResult::Type::ENTITY)
	{
		if (button == 0)
			gameMode->attack(player, hitResult.entity);
		else if (button == 1)
			gameMode->interact(player, hitResult.entity);
	}
	else if (hitResult.type == HitResult::Type::TILE)
	{
		int_t x = hitResult.x;
		int_t y = hitResult.y;
		int_t z = hitResult.z;
		Facing f = hitResult.f;

		if (button == 0)
		{
			level->extinguishFire(x, y, z, hitResult.f);
			Tile *targetTile = Tile::tiles[level->getTile(x, y, z)];
			if (targetTile != &Tile::bedrock || player->userType >= 100)
				gameMode->startDestroyBlock(x, y, z, hitResult.f);
		}
		else if (button == 1)
		{
			ItemInstance *selected = player->getSelectedItem();
			int_t selectedCount = selected != nullptr
				? selected->stackSize.load(std::memory_order_relaxed) : 0;
			if (gameMode->useItemOn(player, *level, selected, x, y, z, f))
			{
				canUseItem = false;
				if (!player->sleeping)
					this->player->swing();
			}

			if (selected == nullptr)
				return;
			if (selected->isEmpty())
				return;
			if (selected->stackSize != selectedCount)
				gameRenderer.itemPlaced();
		}
	}

	if (canUseItem && button == 1)
	{
		ItemInstance *selected = player->getSelectedItem();
		if (selected != nullptr && gameMode->useItem(player, *level, selected))
			gameRenderer.itemUsed();
	}
}

void Minecraft::toggleFullscreen()
{
	fullscreen = !fullscreen;
	std::cout << "Toggle fullscreen!\n";

	lwjgl::Display::setFullscreen(fullscreen);
	width = lwjgl::Display::getWidth();
	height = lwjgl::Display::getHeight();
	if (width <= 0)
		width = 1;
	if (height <= 0)
		height = 1;

	releaseMouse();
	lwjgl::Display::update();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	if (fullscreen)
		grabMouse();

	if (screen != nullptr)
	{
		releaseMouse();
		resize(width, height);
	}

	std::cout << "Size: " << width << ", " << height << '\n';
}

void Minecraft::resize(int_t w, int_t h)
{
	if (w <= 0)
		w = 1;
	if (h <= 0)
		h = 1;

	width = w;
	height = h;

	if (screen != nullptr)
	{
		ScreenSizeCalculator ssc(options, width, height);
		int_t w = ssc.getWidth();
		int_t h = ssc.getHeight();
		screen->init(w, h);
	}
}


void Minecraft::screenshotListener()
{
	if (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_F2))
	{
		if (!isTakingScreenshot)
		{
			isTakingScreenshot = true;
			SPCCommand::addChatMessage(ScreenShotHelper::saveScreenshot(*getWorkingDirectory(), width, height));
		}
	}
	else
	{
		isTakingScreenshot = false;
	}
}

void Minecraft::handleGrabTexture()
{
	if (hitResult.type != HitResult::Type::NONE)
	{
		int_t i = level->getTile(hitResult.x, hitResult.y, hitResult.z);
		if (i == Tile::grass.id)
			i = Tile::dirt.id;
		if (i == Tile::slabDouble.id)
			i = Tile::slabSingle.id;
		if (i == Tile::bedrock.id)
			i = Tile::rock.id;

		if (gameMode->isCreativeMode()) {
			player->inventory.setItem(player->inventory.currentItem, ItemInstance(i, 1, level->getData(hitResult.x, hitResult.y, hitResult.z)));
		} else {
			player->inventory.setCurrentItem(i);
		}
	}
}

void Minecraft::tick()
{
	Profiler::Scope tickProfile(Profiler::Section::Tick);
	{
		Profiler::Scope soundProfile(Profiler::Section::Sound);
		soundEngine.playMusicTick();
	}

	gameRenderer.pick(1.0f);

	if (player != nullptr)
	{
		player->prepareForTick();

	}

	
	if (!pause && level != nullptr)
		gameMode->tick();

	
	glBindTexture(GL_TEXTURE_2D, textures.loadTexture(u"/terrain.png"));
	if (!pause)
		textures.tick();

	if (screen == nullptr && player != nullptr)
	{
		if (player->health <= 0)
			setScreen(nullptr);
		else if (player->isPlayerSleeping() && level != nullptr && level->isOnline)
			setScreen(Util::make_shared<SleepScreen>(*this));
	}
	else if (screen != nullptr && dynamic_cast<SleepScreen *>(screen.get()) != nullptr
		&& player != nullptr && !player->isPlayerSleeping())
	{
		setScreen(nullptr);
	}

	
	if (screen != nullptr)
		lastClickTick = ticks + 10000;
	if (screen != nullptr)
	{
		std::shared_ptr<Screen> eventScreen = screen;
		eventScreen->updateEvents();
		if (screen != nullptr)
		{
			std::shared_ptr<Screen> tickScreen = screen;
			tickScreen->tick();
		}
	}

	
	if (!unattended && (screen == nullptr || screen->passEvents))
	{
		while (lwjgl::Mouse::next())
		{
			long_t tickDelta = System::currentTimeMillis() - lastTickTime;
			if (tickDelta > 200)
				continue;

			int_t dwheel = lwjgl::Mouse::getEventDWheel();
			if (dwheel != 0 && screen == nullptr && player != nullptr)
				player->inventory.changeCurrentItem(dwheel);

			if (screen == nullptr)
			{
				if (!mouseGrabbed && lwjgl::Mouse::getEventButtonState())
				{
					grabMouse();
					continue;
				}
				if (lwjgl::Mouse::getEventButton() == 0 && lwjgl::Mouse::getEventButtonState())
				{
					handleMouseClick(0);
					lastClickTick = ticks;
				}
				if (lwjgl::Mouse::getEventButton() == 1 && lwjgl::Mouse::getEventButtonState())
				{
					handleMouseClick(1);
					lastClickTick = ticks;
				}
				if (lwjgl::Mouse::getEventButton() == 2 && lwjgl::Mouse::getEventButtonState())
					handleGrabTexture();
				continue;
			}
			if (screen != nullptr)
			{
				std::shared_ptr<Screen> activeScreen = screen;
				activeScreen->mouseEvent();
			}
		}

		if (missTime > 0)
			missTime--;

		while (lwjgl::Keyboard::next())
		{
			player->setKey(lwjgl::Keyboard::getEventKey(), lwjgl::Keyboard::getEventKeyState());

			if (lwjgl::Keyboard::getEventKeyState())
			{
				if (lwjgl::Keyboard::getEventKey() == lwjgl::Keyboard::KEY_F11)
				{
					toggleFullscreen();
					continue;
				}

				if (screen != nullptr)
				{
					std::shared_ptr<Screen> activeScreen = screen;
					activeScreen->keyboardEvent();
				}
				else
				{
					if (lwjgl::Keyboard::getEventKey() == lwjgl::Keyboard::KEY_ESCAPE)
						pauseGame();
					if (lwjgl::Keyboard::getEventKey() == lwjgl::Keyboard::KEY_F1)
						options.hideGui = !options.hideGui;
					if (lwjgl::Keyboard::getEventKey() == lwjgl::Keyboard::KEY_F3)
						options.showDebugInfo = !options.showDebugInfo;
					if (lwjgl::Keyboard::getEventKey() == lwjgl::Keyboard::KEY_S && lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_F3))
						reloadSound();
					if (lwjgl::Keyboard::getEventKey() == lwjgl::Keyboard::KEY_F5)
						options.thirdPersonView = !options.thirdPersonView;
					if (lwjgl::Keyboard::getEventKey() == lwjgl::Keyboard::KEY_F8)
						options.smoothCamera = !options.smoothCamera;
					if (lwjgl::Keyboard::getEventKey() == options.keyDrop.key && player != nullptr)
						player->drop();
					if (lwjgl::Keyboard::getEventKey() == options.keyInventory.key && player != nullptr)
						setScreen(Util::make_shared<InventoryScreen>(*this));
					if (lwjgl::Keyboard::getEventKey() == options.keyChat.key && player != nullptr)
						setScreen(Util::make_shared<ChatScreen>(*this));
				}

				for (int_t i = 0; i < 9; i++)
				{
					if (player != nullptr && lwjgl::Keyboard::getEventKey() == lwjgl::Keyboard::KEY_1 + i)
						player->inventory.currentItem = i;
				}

				if (lwjgl::Keyboard::getEventKey() == options.keyFog.key)
					options.toggle(Options::Option::RENDER_DISTANCE, (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) || lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT)) ? -1 : 1);
			}
		}

		if (screen == nullptr)
		{
			if (lwjgl::Mouse::isButtonDown(0) && static_cast<float>(ticks - lastClickTick) >= (timer.ticksPerSecond / 4.0f) && mouseGrabbed)
			{
				handleMouseClick(0);
				lastClickTick = ticks;
			}
			if (lwjgl::Mouse::isButtonDown(1) && static_cast<float>(ticks - lastClickTick) >= (timer.ticksPerSecond / 4.0f) && mouseGrabbed)
			{
				handleMouseClick(1);
				lastClickTick = ticks;
			}
		}

		handleMouseDown(0, (screen == nullptr && lwjgl::Mouse::isButtonDown(0) && mouseGrabbed));
	}

	
	if (level != nullptr)
	{
		if (player != nullptr)
		{
			recheckPlayerIn++;
			if (recheckPlayerIn == 30)
			{
				recheckPlayerIn = 0;
				level->ensureAdded(player);
			}
		}

		level->difficulty = options.difficulty;
		if (level->isOnline)
			level->difficulty = 3;

		if (!pause)
			gameRenderer.tick();
		if (!pause)
			levelRenderer.tick();
		if (!pause)
		{
			if (level->lastLightningBolt > 0)
				--level->lastLightningBolt;
			level->tickEntities();
		}
		if (!pause || isOnline())
		{
			level->setSpawnSettings(!unattended && options.difficulty > 0, !unattended);
			level->tick();
		}
		if (!pause && level != nullptr)
			level->animateTick(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));
		if (!pause)
			particleEngine.tick();
		if (statFileWriter != nullptr)
			statFileWriter->tick();
		if (!pause)
			gui.tick();
	}

	lastTickTime = System::currentTimeMillis();
}

void Minecraft::stressTick()
{
	++ticks;
	tick();
}

void Minecraft::reloadSound()
{
	soundEngine.updateOptions();
}

void Minecraft::loadAllSounds(File *dir, const jstring &prefix)
{
	if (!dir || !dir->exists() || !dir->isDirectory())
		return;

	auto files = dir->listFiles();
	for (auto &file : files)
	{
		if (file->isDirectory())
		{
			jstring newPrefix = prefix + file->getName() + u"/";
			loadAllSounds(file.get(), newPrefix);
		}
		else
		{
			jstring name = file->getName();
			
			if (name.length() < 4)
				continue;
			jstring ext = name.substr(name.length() - 4);
			if (ext != u".ogg" && ext != u".mus" && ext != u".wav")
				continue;
			jstring fullName = prefix + name;
			fileDownloaded(fullName, file.get());
		}
	}
}

void Minecraft::fileDownloaded(const jstring &name, File *file)
{
	if (!file || !file->exists())
		return;

	
	size_t slashPos = name.find(u'/');
	if (slashPos == jstring::npos)
		return;

	jstring category = name.substr(0, slashPos);
	jstring soundName = name.substr(slashPos + 1);
	std::string filePath = String::toUTF8(file->toString());

	if (category == u"sound" || category == u"newsound")
		soundEngine.add(soundName, filePath);
	else if (category == u"streaming")
		soundEngine.addStreaming(soundName, filePath);
	else if (category == u"music" || category == u"newmusic")
		soundEngine.addMusic(soundName, filePath);
}

bool Minecraft::isOnline()
{
	return level != nullptr && level->isOnline;
}

void Minecraft::selectLevel(const jstring &name)
{
	selectLevel(name, name, Random().nextLong());
}

void Minecraft::selectLevel(const jstring &name, const jstring &levelName, long_t seed)
{
	setLevel(nullptr);

	std::unique_ptr<File> saves(File::open(*getWorkingDirectory(), u"saves"));
	SaveConverterMcRegion converter(*saves);
	if (converter.isOldMapFormat(name))
	{
		progressRenderer->progressStart(u"Converting World to " + converter.getFormatName());
		progressRenderer->progressStage(u"This may take a while :)");
		converter.convertMapFormat(name, *progressRenderer);
		selectLevel(name, levelName, 0LL);
		return;
	}
	std::shared_ptr<Level> new_level = Util::make_shared<Level>(saves.release(), name, levelName, seed);
	if (statFileWriter != nullptr)
	{
		if (new_level->isNew && StatList::createWorldStat != nullptr)
			statFileWriter->readStat(*StatList::createWorldStat, 1);
		else if (!new_level->isNew && StatList::loadWorldStat != nullptr)
			statFileWriter->readStat(*StatList::loadWorldStat, 1);
		if (StatList::startGameStat != nullptr)
			statFileWriter->readStat(*StatList::startGameStat, 1);
	}
	if (new_level->isNew)
		setLevel(new_level, u"Generating level");
	else
		setLevel(new_level, u"Loading level");
}


void Minecraft::toggleDimension()
{
	if (level == nullptr || player == nullptr)
		return;

	std::cout << "Toggling dimension!!" << '\n';
	int_t newDimension = (player->dimension == -1) ? 0 : -1;
	player->dimension = newDimension;

	level->removeEntity(player);
	player->removed = false;

	double x = player->x;
	double z = player->z;
	double scale = 8.0;
	if (newDimension == -1)
	{
		x /= scale;
		z /= scale;
	}
	else
	{
		x *= scale;
		z *= scale;
	}
	player->moveTo(x, player->y, z, player->yRot, player->xRot);
	
	
	if (player->isAlive())
		level->tick(player, false);

	
	
	
	CompoundTag playerTag;
	player->saveWithoutId(playerTag);

	std::shared_ptr<Level> newLevel = Util::make_shared<Level>(*level, newDimension);
	std::shared_ptr<LocalPlayer> newPlayer = std::static_pointer_cast<LocalPlayer>(gameMode->createPlayer(*newLevel));
	newPlayer->load(playerTag);
	newPlayer->dimension = newDimension;
	newPlayer->moveTo(x, player->y, z, player->yRot, player->xRot);
	this->player = newPlayer;

	setLevel(newLevel, newDimension == -1 ? u"Entering the Nether" : u"Leaving the Nether", newPlayer);

	if (newPlayer->isAlive())
	{
		newPlayer->moveTo(x, newPlayer->y, z, newPlayer->yRot, newPlayer->xRot);
		newLevel->tick(newPlayer, false);
		Teleporter().teleport(*newLevel, *newPlayer);
	}
}

void Minecraft::setLevel(std::shared_ptr<Level> level)
{
	setLevel(level, u"");
}

void Minecraft::setLevel(std::shared_ptr<Level> level, const jstring &title)
{
	setLevel(level, title, nullptr);
}

void Minecraft::setLevel(std::shared_ptr<Level> level, const jstring &title, std::shared_ptr<Player> player)
{
	if (statFileWriter != nullptr)
		statFileWriter->syncStats();
	progressRenderer->progressStart(title);
	progressRenderer->progressStage(u"");
	soundEngine.playStreaming(jstring(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	if (this->level != nullptr)
		this->level->forceSave(progressRenderer);
	this->level = level;

	std::cout << "Player is " << this->player << '\n';

	if (level != nullptr)
	{
		gameMode->initLevel(level);

		if (!isOnline())
		{
			if (player == nullptr)
				this->player = nullptr; 
		}
		else if (this->player != nullptr)
		{
			this->player->resetPos();
			if (level != nullptr)
				level->addEntity(this->player);
		}

		if (!level->isOnline)
			prepareLevel(title);

		std::cout << "Player is now " << this->player << '\n';

		if (this->player == nullptr)
		{
			this->player = std::static_pointer_cast<LocalPlayer>(gameMode->createPlayer(*level));
			this->player->resetPos();
			gameMode->initPlayer(this->player);
		}
		this->player->input = std::make_unique<KeyboardInput>(options);

		levelRenderer.setLevel(level);

		particleEngine.setLevel(level.get());

		gameMode->adjustPlayer(this->player);
		if (player != nullptr)
			level->clearLoadedPlayerData();


		level->loadPlayer(this->player);

		if (level->isNew)
			level->forceSave(progressRenderer);
	}
	else
	{
		this->player = nullptr;
	}
}

void Minecraft::prepareLevel(const jstring &title)
{
	progressRenderer->progressStart(title);
	progressRenderer->progressStage(u"Building terrain");

	short_t radius = 128;
	int_t i = 0;
	int_t max = radius * 2 / 16 + 1;
	max *= max;

	int_t xSpawn = level->xSpawn;
	int_t zSpawn = level->zSpawn;
	if (player != nullptr)
	{
		xSpawn = static_cast<int_t>(player->x);
		zSpawn = static_cast<int_t>(player->z);
	}


	for (int_t x = -radius; x <= radius; x += 16)
	{
		for (int_t z = -radius; z <= radius; z += 16)
		{
			progressRenderer->progressStagePercentage(i++ * 100 / max);
			level->getTile(xSpawn + x, 64, zSpawn + z);

			while (level->updateLights());
		}
	}

	progressRenderer->progressStage(u"Simulating world for a bit");
}

jstring Minecraft::gatherStats1()
{
	return levelRenderer.gatherStats1();
}

jstring Minecraft::gatherStats2()
{
	return levelRenderer.gatherStats2();
}

jstring Minecraft::gatherStats4()
{
	return level->gatherChunkSourceStats();
}

jstring Minecraft::gatherStats3()
{
	return u"P: " + particleEngine.countParticles() + u". T: " + level->gatherStats();
}

void Minecraft::respawnPlayer(int_t dimension)
{
	if (level == nullptr || gameMode == nullptr)
		return;

	if (!level->isOnline && level->dimension != nullptr && !level->dimension->mayRespawn())
		toggleDimension();

	level->setSpawnLocation();
	level->updateEntityList();

	int_t entityId = 0;
	if (player != nullptr)
	{
		entityId = player->entityId;
		level->removeEntity(player);
	}

	this->player = std::static_pointer_cast<LocalPlayer>(gameMode->createPlayer(*level));
	this->player->dimension = dimension;
	this->player->resetPos();
	gameMode->initPlayer(this->player);
	level->loadPlayer(this->player);
	this->player->input = std::make_unique<KeyboardInput>(options);
	this->player->entityId = entityId;
	this->player->animateRespawn();
	gameMode->adjustPlayer(this->player);
	prepareLevel(u"Respawning");

	if (std::dynamic_pointer_cast<DeathScreen>(screen) != nullptr)
		setScreen(nullptr);
}

void Minecraft::start(const jstring *name, const jstring *sessionId)
{
	startAndConnectTo(name, sessionId, nullptr);
}

void Minecraft::startAndConnectTo(const jstring *name, const jstring *sessionId, const jstring *ip)
{
	std::unique_ptr<Minecraft> minecraft = std::make_unique<Minecraft>(854, 480, false);

	minecraft->serverDomain = u"www.minecraft.net";
	if (name != nullptr && sessionId != nullptr)
		minecraft->user = std::make_unique<User>(*name, *sessionId);
	else
		minecraft->user = std::make_unique<User>(u"Player" + String::fromUTF8(std::to_string(System::currentTimeMillis() % 1000)), u"");
	
	if (ip != nullptr)
	{
		std::string address = String::toUTF8(*ip);
		size_t separator = address.rfind(':');
		if (separator == std::string::npos)
			throw std::invalid_argument("Server address must be host:port");
		int port = std::stoi(address.substr(separator + 1));
		if (port < 0 || port > 65535)
			throw std::out_of_range("Server port is out of range");
		minecraft->serverHost = address.substr(0, separator);
		minecraft->serverPort = static_cast<std::uint16_t>(port);
	}

	minecraft->run();
	
}

#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <thread>
#include <vector>

#include "client/Options.h"
#include "client/ProgressRenderer.h"
#include "client/Timer.h"
#include "client/User.h"
#include "client/MouseHandler.h"
#include "client/OpenGLCapabilities.h"

#include "client/gui/Gui.h"
#include "client/gui/Font.h"
#include "client/gui/Screen.h"

#include "client/renderer/LevelRenderer.h"
#include "client/renderer/GameRenderer.h"
#include "client/renderer/Textures.h"
#include "client/particle/ParticleEngine.h"

#include "client/skins/TexturePackRepository.h"

#include "client/sound/SoundEngine.h"

#include "client/gamemode/GameMode.h"

#include "client/player/LocalPlayer.h"

#include "world/level/Level.h"

#include "world/phys/HitResult.h"

#include "java/Type.h"
#include "java/System.h"
#include "java/File.h"


class Minecraft
{
	friend class ConnectingScreen;

public:
	
	static constexpr bool FLYBY_MODE = false;
	static const jstring VERSION_STRING;

	std::shared_ptr<GameMode> gameMode;
	std::shared_ptr<class NetClientHandler> connection;

private:
	bool fullscreen = false;
	std::vector<std::thread> connectionThreads;
	
public:
	int_t width = 0;
	int_t height = 0;

private:
	OpenGLCapabilities openGLCapabilities;

	Timer timer = Timer(20.0f);

public:
	std::shared_ptr<Level> level;
	Options options = Options(*this);
	TexturePackRepository texturePackRepository{*this};
	Textures textures{texturePackRepository, options, *this};
	LevelRenderer levelRenderer = LevelRenderer(*this, textures);

	std::shared_ptr<LocalPlayer> player;

	std::unique_ptr<User> user;
	jstring serverDomain;
	std::string serverHost;
	std::uint16_t serverPort = 25565;

	volatile bool pause = false;
	std::unique_ptr<Font> font;

	std::shared_ptr<Screen> screen;
	std::shared_ptr<ProgressRenderer> progressRenderer = std::make_unique<ProgressRenderer>(*this);

	GameRenderer gameRenderer = GameRenderer(*this);

	ParticleEngine particleEngine = ParticleEngine(nullptr, &textures);

private:
	int_t ticks = 0;
	int_t missTime = 0;

public:
	Gui gui = Gui(*this);
	std::unique_ptr<class AchievementToast> achievementToast;
	std::unique_ptr<class StatFileWriter> statFileWriter;

	bool noRender = false;
	bool unattended = false;

	HitResult hitResult = HitResult();

	MouseHandler mouseHandler = MouseHandler(*this);
	SoundEngine soundEngine;

	std::shared_ptr<File> workingDirectory;

	static std::array<long_t, 512> frameTimes;
	static std::array<long_t, 512> tickTimes;
	static int_t frameTimePos;

private:
	static std::shared_ptr<File> workDir;

public:
	volatile bool running = true;

public:
	jstring fpsString = u"";
	
private:
	bool wasDown = false;
	long_t lastTimer = -1;

public:
	bool mouseGrabbed = false;

private:
	int_t lastClickTick = 0;
	
public:
	bool isRaining = false;

private:
	long_t lastTickTime = System::currentTimeMillis();
	int_t recheckPlayerIn = 0;

public:
	Minecraft(int_t width, int_t height, bool fullscreen);

	void onCrash(const std::string &msg, const std::exception &e);

	void init(std::shared_ptr<File> directory = nullptr);
	
private:
	void renderLoadingScreen();
	void blit(int_t dstx, int_t dsty, int_t srcx, int_t srcy, int_t w, int_t h);
	void loadAllSounds(File *dir, const jstring &prefix);
	void fileDownloaded(const jstring &name, File *file);

public:
	static const std::shared_ptr<File> &getWorkingDirectory();

	void setScreen(std::shared_ptr<Screen> screen);

private:
	void checkGlError(const std::string &at);

public:
	~Minecraft();

	void generateFlyby();

	void run();

	void renderFpsMeter(long_t tickNanos);
	void screenshotListener();
	bool isTakingScreenshot = false;

	void stop();
	
	void grabMouse();
	void releaseMouse();
	
	void pauseGame();

	void handleMouseDown(int_t button, bool down);
	void handleMouseClick(int_t button);

	void toggleFullscreen();
	void resize(int_t w, int_t h);

	void handleGrabTexture();

	void tick();
	void stressTick();

	void reloadSound();

	bool isOnline();

	void selectLevel(const jstring &name);
	void selectLevel(const jstring &name, const jstring &levelName, long_t seed);

	void toggleDimension();

	void setLevel(std::shared_ptr<Level> level);
	void setLevel(std::shared_ptr<Level> level, const jstring &title);
	void setLevel(std::shared_ptr<Level> level, const jstring &title, std::shared_ptr<Player> player);

	void prepareLevel(const jstring &title);

	jstring gatherStats1();
	jstring gatherStats2();
	jstring gatherStats4();
	jstring gatherStats3();
	void respawnPlayer(int_t dimension = 0);
	static void start(const jstring *name, const jstring *sessionId);
	static void startAndConnectTo(const jstring *name, const jstring *sessionId, const jstring *ip);
	

	friend int main(int argc, char *argv[]);

	enum class OS
	{
		linux,
		solaris,
		windows,
		macos,
		unknown,
	};
};

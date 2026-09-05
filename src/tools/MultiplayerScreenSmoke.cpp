#include "tools/MultiplayerScreenSmoke.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "client/Minecraft.h"
#include "client/Options.h"
#include "client/User.h"
#include "client/gui/ChestScreen.h"
#include "client/gui/ContainerScreen.h"
#include "client/gui/DispenserScreen.h"
#include "client/gui/FurnaceScreen.h"
#include "client/gui/InventoryScreen.h"
#include "client/gui/MultiplayerScreen.h"
#include "client/gui/WorkbenchScreen.h"
#include "client/renderer/MouseFilter.h"
#include "java/File.h"

namespace
{

class ContainerScreenProbe : public ContainerScreen
{
public:
	using ContainerScreen::shouldClose;
};

static_assert(std::is_base_of<ContainerScreen, ChestScreen>::value, "ChestScreen must derive from ContainerScreen");
static_assert(std::is_base_of<ContainerScreen, FurnaceScreen>::value, "FurnaceScreen must derive from ContainerScreen");
static_assert(std::is_base_of<ContainerScreen, DispenserScreen>::value, "DispenserScreen must derive from ContainerScreen");
static_assert(std::is_base_of<ContainerScreen, InventoryScreen>::value, "InventoryScreen must derive from ContainerScreen");
static_assert(std::is_base_of<ContainerScreen, WorkbenchScreen>::value, "WorkbenchScreen must derive from ContainerScreen");

bool expect(bool condition, const char *message)
{
	if (!condition)
		std::cerr << "FAILED: " << message << std::endl;
	return condition;
}

bool testUsernameOptions()
{
	bool ok = true;
	std::unique_ptr<File> directory(File::open(u"build/multiplayer-screen-smoke-options"));
	directory->mkdirs();
	std::unique_ptr<File> optionsFile(File::open(*directory, u"options.txt"));
	optionsFile->remove();

	Minecraft minecraft(1, 1, false);
	minecraft.user = std::make_unique<User>(u"Startup_User", u"-");
	minecraft.options.open(directory.get());
	ok &= expect(minecraft.options.username == u"Startup_User" &&
		minecraft.user->name == u"Startup_User",
		"a missing username option keeps the active startup username");

	minecraft.options.setUsername(u"Persisted_User");
	minecraft.options.save();
	bool usernameLineFound = false;
	std::unique_ptr<std::istream> input(optionsFile->toStreamIn());
	std::string line;
	while (input && std::getline(*input, line))
	{
		if (line == "username:Persisted_User")
			usernameLineFound = true;
	}
	ok &= expect(usernameLineFound,
		"options.txt writes the configured multiplayer username");

	minecraft.user->name = u"CommandLineName";
	Options reloaded(minecraft);
	reloaded.open(directory.get());
	ok &= expect(reloaded.username == u"Persisted_User" &&
		minecraft.user->name == u"Persisted_User",
		"a persisted username reloads and becomes the active connection identity");

	reloaded.setUsername(u"");
	ok &= expect(reloaded.username == u"Persisted_User",
		"an empty username edit keeps the last valid username");
	reloaded.setUsername(u"12345678901234567");
	ok &= expect(reloaded.username == u"1234567890123456",
		"configured usernames are capped at the Beta protocol limit");

	input.reset();
	optionsFile->remove();
	directory->remove();
	return ok;
}

}

int runMultiplayerScreenSmoke()
{
	bool ok = true;
	ok &= testUsernameOptions();
	MouseFilter positiveFilter;
	ok &= expect(std::abs(positiveFilter.smooth(10.0f, 0.1f) - 0.5f) < 0.000001f &&
		std::abs(positiveFilter.smooth(0.0f, 0.1f) - 0.725f) < 0.000001f &&
		std::abs(positiveFilter.smooth(-4.0f, 0.1f) - 0.4775f) < 0.000001f,
		"smooth-camera mouse filtering keeps the exact Beta accumulator and clamp sequence");
	MouseFilter negativeFilter;
	ok &= expect(std::abs(negativeFilter.smooth(-10.0f, 0.1f) + 0.5f) < 0.000001f,
		"smooth-camera mouse filtering applies the symmetric negative clamp");
	ok &= expect(!ContainerScreenProbe::shouldClose(true, false),
		"a live container player keeps the screen open");
	ok &= expect(ContainerScreenProbe::shouldClose(false, false),
		"a dead container player closes the screen");
	ok &= expect(ContainerScreenProbe::shouldClose(true, true),
		"a removed container player closes the screen");
	auto expectAddress = [&](const jstring &text, const std::string &host, int_t port,
		const char *message)
	{
		MultiplayerScreen::ServerAddress address = MultiplayerScreen::parseServerAddress(text);
		ok &= expect(address.host == host && address.port == port, message);
	};

	expectAddress(u" example.org:25570 ", "example.org", 25570,
		"address parsing trims the complete server field");
	expectAddress(u"[2001:db8::1]:25566", "2001:db8::1", 25566,
		"bracketed IPv6 address keeps its explicit port");
	expectAddress(u"[2001:db8::1]", "2001:db8::1", 25565,
		"bracketed IPv6 address uses the default port");
	expectAddress(u"2001:db8::1", "2001:db8::1", 25565,
		"unbracketed multi-colon address is treated as the complete host");
	expectAddress(u"server.example:12x", "server.example", 25565,
		"invalid Java integer text falls back to the default port");
	expectAddress(u"server.example:+123", "server.example", 123,
		"Java integer parsing accepts an explicit plus sign");
	expectAddress(u"server.example:2147483648", "server.example", 25565,
		"Java integer overflow falls back to the default port");
	expectAddress(u"server.example:-1", "server.example", -1,
		"syntactically valid out-of-range socket ports are not defaulted");
	expectAddress(u"server.example:", "server.example", 25565,
		"Java split discards a trailing empty port component");
	expectAddress(u"[::1] ignored:123", "::1", 25565,
		"bracket suffix without a leading colon is ignored");
	expectAddress(u"\x001fserver.example:25568\x0020", "server.example", 25568,
		"Java trim removes every leading and trailing code unit through U+0020");

	bool colonOnlyThrows = false;
	try
	{
		MultiplayerScreen::parseServerAddress(u":");
	}
	catch (const std::out_of_range &)
	{
		colonOnlyThrows = true;
	}
	ok &= expect(colonOnlyThrows,
		"Java split leaves the Beta colon-only address path without a host element");

	if (ok)
		std::cout << "Multiplayer screen smoke passed" << std::endl;
	return ok ? 0 : 1;
}

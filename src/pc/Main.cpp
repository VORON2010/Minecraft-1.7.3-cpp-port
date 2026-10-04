#include <thread>
#define SDL_MAIN_HANDLED
#include "SDL.h"

#ifdef _MSC_VER
#ifndef NDEBUG
#pragma comment(linker, "/SUBSYSTEM:console") // Release is a windowed app, see CMakeLists
#endif
#endif

#include <cstring>

#include "client/Minecraft.h"
#include "java/System.h"
#include "tools/BlockSmoke.h"
#include "tools/MultiplayerScreenSmoke.h"
#include "tools/NetworkSmoke.h"
#include "tools/SaveConverterSmoke.h"
#include "tools/SoundSmoke.h"

#include "external/SDLException.h"

#include "lwjgl/GLContext.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

// Writes the SEH code plus a symbolized stack to crash_dump.txt for native crashes (access violations etc.)
static LONG WINAPI NativeCrashFilter(EXCEPTION_POINTERS *ep)
{
	FILE *f = fopen("crash_dump.txt", "w");
	if (!f)
		return EXCEPTION_CONTINUE_SEARCH;
	EXCEPTION_RECORD *er = ep->ExceptionRecord;
	fprintf(f, "Native exception 0x%08lX at %p\n", er->ExceptionCode, er->ExceptionAddress);
	if (er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2)
		fprintf(f, "  %s address 0x%llX\n", er->ExceptionInformation[0] == 0 ? "read" : er->ExceptionInformation[0] == 1 ? "write" : "execute",
			(unsigned long long)er->ExceptionInformation[1]);

	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	SymInitialize(process, nullptr, TRUE);

	CONTEXT ctx = *ep->ContextRecord;
	STACKFRAME64 frame = {};
	frame.AddrPC.Offset = ctx.Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = ctx.Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = ctx.Rsp;
	frame.AddrStack.Mode = AddrModeFlat;

	alignas(SYMBOL_INFO) char symbuf[sizeof(SYMBOL_INFO) + 512];
	for (int i = 0; i < 64; i++)
	{
		if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame, &ctx, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
			break;
		DWORD64 addr = frame.AddrPC.Offset;
		if (addr == 0)
			break;
		SYMBOL_INFO *sym = reinterpret_cast<SYMBOL_INFO *>(symbuf);
		memset(symbuf, 0, sizeof(symbuf));
		sym->SizeOfStruct = sizeof(SYMBOL_INFO);
		sym->MaxNameLen = 511;
		DWORD64 disp = 0;
		const char *name = SymFromAddr(process, addr, &disp, sym) ? sym->Name : "?";
		IMAGEHLP_LINE64 line = {};
		line.SizeOfStruct = sizeof(line);
		DWORD ldisp = 0;
		if (SymGetLineFromAddr64(process, addr, &ldisp, &line))
			fprintf(f, "  #%02d %s  %s:%lu\n", i, name, line.FileName, line.LineNumber);
		else
			fprintf(f, "  #%02d %s  (0x%llX)\n", i, name, (unsigned long long)addr);
	}
	fclose(f);
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
	SetUnhandledExceptionFilter(NativeCrashFilter);
#endif
	try {
	if (argc >= 2 && std::strcmp(argv[1], "--block-smoke") == 0)
		return runBlockSmoke();
	if (argc >= 2 && std::strcmp(argv[1], "--network-smoke") == 0)
		return runNetworkSmoke();
	if (argc >= 2 && std::strcmp(argv[1], "--save-converter-smoke") == 0)
		return runSaveConverterSmoke();
	if (argc >= 2 && std::strcmp(argv[1], "--sound-smoke") == 0)
		return runSoundSmoke();
	if (argc >= 2 && std::strcmp(argv[1], "--multiplayer-screen-smoke") == 0)
		return runMultiplayerScreenSmoke();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
		throw SDLException();
	lwjgl::GLContext::instantiate();

	jstring username = u"Player" + String::toString(System::currentTimeMillis() % 1000);
	if (argc >= 2)
		username = String::fromUTF8(argv[1]);

	jstring auth = u"-";
	if (argc >= 3)
		auth = String::fromUTF8(argv[2]);

	if (argc >= 4)
	{
		jstring server = String::fromUTF8(argv[3]);
		Minecraft::startAndConnectTo(&username, &auth, &server);
	}
	else
	{
		Minecraft::start(&username, &auth);
	}

	return 0;
	} catch (const std::exception& e) {
		FILE* f = fopen("crash.txt", "w");
		fprintf(f, "Exception: %s\n", e.what());
		fclose(f);
		return 1;
	} catch (...) {
		FILE* f = fopen("crash.txt", "w");
		fprintf(f, "Exception: Unknown exception caught in main!\n");
		fclose(f);
		return 1;
	}
}


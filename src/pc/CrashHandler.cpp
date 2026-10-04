#include "CrashHandler.h"

#include "SDL_messagebox.h"

namespace CrashHandler
{

void Crash(const std::string &message, const std::string &stackTrace)
{
	std::string text = message + "\n\n" + stackTrace;
    FILE* f = fopen("crash_dump.txt", "w");
    if (f) {
        fprintf(f, "%s\n", text.c_str());
        fclose(f);
    }
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Minecraft has crashed!", text.c_str(), nullptr);
    exit(1);
}

}

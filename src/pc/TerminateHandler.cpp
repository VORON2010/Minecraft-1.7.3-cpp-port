#include <exception>
#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <string>

#ifdef _MSC_VER
#pragma init_seg(compiler)
#endif

static void my_terminate_handler() {
    FILE* f = fopen("debug_log.txt", "w");
    fprintf(f, "std::terminate was called!\n");
    try {
        std::rethrow_exception(std::current_exception());
    } catch (const std::exception& e) {
        fprintf(f, "Exception: %s\n", e.what());
    } catch (...) {
        fprintf(f, "Unknown exception\n");
    }
    fclose(f);
    MessageBoxA(NULL, "std::terminate called!", "Crash", 0);
    abort();
}

static struct TerminateInstaller {
    TerminateInstaller() {
        std::set_terminate(my_terminate_handler);
    }
} terminateInstaller;

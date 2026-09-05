#include "java/Runtime.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>

Runtime Runtime::instance;

Runtime &Runtime::getRuntime()
{
	return instance;
}

long_t Runtime::maxMemory()
{
	
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (!GlobalMemoryStatusEx(&statex))
		return 1;
	return statex.ullTotalPhys;
}

long_t Runtime::totalMemory()
{
	
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	return pmc.PeakWorkingSetSize;
}

long_t Runtime::freeMemory()
{
	
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	return pmc.PeakWorkingSetSize - pmc.WorkingSetSize;
}

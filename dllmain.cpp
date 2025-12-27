// dllmain.cpp : Defines the entry point for the DLL application.
#include <windows.h>
#include <memory>
#include "Packages/injector/injector.hpp"
#include "Packages/mINI/src/mini/ini.h"
#include "config.h"

char g_ConfigPath[MAX_PATH];

Config config;

void Config::Load()
{
    mINI::INIFile file(g_ConfigPath);
    mINI::INIStructure ini;
    file.read(ini);

    if (ini.has("MAIN"))
    {
        READ_UINT("MAIN", MainPoolSize);
        READ_UINT("MAIN", FastMemPoolSize);
        READ_UINT("MAIN", ePolySlotPoolSize);
        READ_UINT("MAIN", PersistentMemoryPoolSize);
        READ_UINT("MAIN", CarLoaderPoolSize);
        READ_UINT("MAIN", TrackStreamerPoolSize);
        READ_UINT("MAIN", FEngMemoryPoolSize);
        READ_UINT("MAIN", GManagerTempSize);
    }
}

int& MemoryInitialized = *(int*)0x90E688;
void(__cdecl* bInitMemoryPool)(int pool_num, void* mem, int mem_size, const char* debug_name)
    = (void(__cdecl*)(int pool_num, void* mem, int mem_size, const char* debug_name))0x465240;

void __stdcall bMemoryInit()
{
    if (!MemoryInitialized)
    {
        int bware_memory_size = config.MainPoolSize;
        bInitMemoryPool(0, malloc(bware_memory_size), bware_memory_size, "Main Pool");
        MemoryInitialized = true;
    }
}

void FastMemEmergencyInitialization(
    unsigned int& bytes,
    const char*& name,
    unsigned int& expansionsize,
    unsigned int& trackingsize
)
{
    bytes = config.FastMemPoolSize;
    name = "gFastMem";
    expansionsize = 0x2000;
    trackingsize = 512;
}

void WriteMemoryPoolSizes()
{
    if (*(uint32_t*)0x5009D2 < config.ePolySlotPoolSize)
    {
        injector::WriteMemory<uint32_t>(0x5009D2, config.ePolySlotPoolSize, true);
        injector::WriteMemory<uint32_t>(0x5009DC, config.ePolySlotPoolSize, true);
        injector::WriteMemory<uint32_t>(0x500A01, config.ePolySlotPoolSize, true);
        injector::WriteMemory<uint32_t>(0x500A12, config.ePolySlotPoolSize, true);
    }

    if (*(uint32_t*)0x665FDB < config.PersistentMemoryPoolSize)
        injector::WriteMemory<uint32_t>(0x665FDB, config.PersistentMemoryPoolSize, true);
    
    if (*(uint32_t*)0x8F7EF0 < config.CarLoaderPoolSize)
        injector::WriteMemory<int>(0x8F7EF0, config.CarLoaderPoolSize, true);
    
    injector::WriteMemory<uint32_t>(0x72D9DD, 0x40000, true); // track streamer allocation flags
    
    if (*(uint32_t*)0x664A36 < config.TrackStreamerPoolSize)
    {
        injector::WriteMemory<uint32_t>(0x664A36, config.TrackStreamerPoolSize, true); // track streamer pool
        injector::WriteMemory<uint32_t>(0x664A43, config.TrackStreamerPoolSize - 0x40000, true); // track streamer pool w/ EmergencySaveMemory reduction
    }
    
    if (*(uint32_t*)0x8F5790 < config.FEngMemoryPoolSize)
        injector::WriteMemory<uint32_t>(0x8F5790, config.FEngMemoryPoolSize, true);
    
    if (*(uint32_t*)0x5F7395 < config.GManagerTempSize || *(uint32_t*)0x5F7395 != 0x5F73B1)
    {
        injector::WriteMemory<uint32_t>(0x5F7395, config.GManagerTempSize, true);
        injector::WriteMemory<uint32_t>(0x5F73B1, config.GManagerTempSize, true);
    }
}

bool (*CheckMultipleInstance)(const char*, int);
bool CheckMultipleInstanceHook(const char* a, int b)
{
    WriteMemoryPoolSizes();
    return CheckMultipleInstance(a, b);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        GetFullPathNameA("Mempoolulator.ini", MAX_PATH, g_ConfigPath, NULL); // store path of the INI as the working dir changes later
        config.Load();

        if (config.MainPoolSize > 0)
        {
            injector::MakeJMP(0x4652A0, bMemoryInit); // main pool, must happen here to get ahead of first bWareMalloc call
            injector::MakeRangedNOP(0x4640AE, 0x4640B9); // remove PC-exclusive forced bLargestMalloc limit
        }

        if (config.FastMemPoolSize > 0)
            injector::MakeJMP(0x64A560, FastMemEmergencyInitialization); // also has to happen early as it's initialized in crt startup

        // hook to ensure that this stuff is set after (at least most) other mods load
        CheckMultipleInstance = injector::MakeCALL(0x666597, CheckMultipleInstanceHook, true).get();
    }

    return TRUE;
}


#pragma once

#include <cstdint>

#define READ_BOOL(section, key)                                                                                        \
    if (ini[section].has(#key))                                                                                        \
        this->key = std::stol(ini[section][#key]) != 0;

#define READ_INT(section, key)                                                                                         \
    if (ini[section].has(#key))                                                                                        \
        this->key = std::stol(ini[section][#key]);

#define READ_UINT(section, key)                                                                                         \
    if (ini[section].has(#key))                                                                                        \
        this->key = std::stoul(ini[section][#key]);

#define READ_FLOAT(section, key)                                                                                       \
    if (ini[section].has(#key))                                                                                        \
        this->key = std::stof(ini[section][#key]);

#define READ_STR(section, key)                                                                                       \
    if (ini[section].has(#key))                                                                                        \
        this->key = ini[section][#key];

#define READ_CHAR(section, key)                                                                                         \
    if (ini[section].has(#key))                                                                                        \
        strcpy(this->key, ini[section][#key].c_str());


class Config
{
public:
    // main
    uint32_t MainPoolSize = 0x15900000;
    uint32_t FastMemPoolSize = 0x25F400;
    uint32_t ePolySlotPoolSize = 0x12C000;
    uint32_t PersistentMemoryPoolSize = 0x80000;
    uint32_t CarLoaderPoolSize = 55000;
    uint32_t TrackStreamerPoolSize = 204800000;
    uint32_t FEngMemoryPoolSize = 390000;
    uint32_t GManagerTempSize = 390000;

    void Load();
};

extern Config config;

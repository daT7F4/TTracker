#pragma once

#include "FS.h"
#include "SD_MMC.h"
#include "fpv.hpp"

#include "globalCalls.hpp"
#include "handlers/trackHandler.hpp"
#include "songData.hpp"
#include "whatever.hpp"

#define FILE_VERSION 0

class fileHandler{
    public:
        fileHandler() = default;
        void begin();
    private:
        void resample(const char *file);
        void process();
        void loadProject(const char *file);
        void saveProject(const char *file);

        int16_t *rawAudio = nullptr;
        int16_t *delta = nullptr;
        uint8_t channels = 0;
};
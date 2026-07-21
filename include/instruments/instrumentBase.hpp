#pragma once

#include <Arduino.h>
#include <map>

#include "lookup.h"
#include "fpv.hpp"
#include "essentials.hpp"
#include "globalCalls.hpp"

struct instrumentBase{
    instrumentBase() = default;
    virtual ~instrumentBase() = 0;
    virtual void resize(size_t bufferSize) = 0;
    virtual void render(size_t renderSize) = 0;
    virtual void drawUI() = 0;
    virtual void resetVoices() = 0;
    virtual void pressVoice(uint8_t idx, uint8_t pitch, uint8_t velocity) = 0;
    virtual void releaseVoice(uint8_t idx) = 0;
    virtual void updateEffect(uint8_t idx, uint8_t effectType, uint8_t effectAmount) = 0;

    uint16_t type = 0;
    const uint8_t maxVoices = 1;
    fpv volumeIdx = 384;
    int16_t* buffer = nullptr;
    std::map<uint16_t, storedVariable> data;
};

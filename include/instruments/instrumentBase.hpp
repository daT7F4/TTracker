#pragma once

#include <Arduino.h>
#include <map>

#include "lookup.h"
#include "fpv.hpp"
#include "whatever.hpp"
#include "essentials.hpp"
#include "globalCalls.hpp"

struct instrumentBase{
    instrumentBase() = default;
    virtual ~instrumentBase();
    virtual void resize(size_t bufferSize);
    virtual void render(size_t renderSize);
    virtual void resetVoices();
    virtual void pressVoice(uint8_t idx, uint8_t pitch, uint8_t velocity);
    virtual void releaseVoice(uint8_t idx);
    virtual void updateEffect(uint8_t idx, uint8_t effectType, uint8_t effectAmount);
    virtual void customVariable(void* data);
    
    void loadVariable(uint16_t id, specialVariable var);

    uint16_t type = 0;
    const uint8_t maxVoices = 1;
    fpv volumeIdx = 384;
    int16_t* buffer = nullptr;
    std::map<uint16_t, specialVariable> data;
};

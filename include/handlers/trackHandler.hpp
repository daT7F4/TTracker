#pragma once

#include "effects/effectBase.hpp"
#include "instruments/instrumentBase.hpp"
#include "instruments/basicInstrument.hpp"
#include "songData.hpp"

const uint8_t totalTracks = 8;
const uint8_t maxEffects = 4;

struct cell {
    uint8_t note;
    uint8_t velocity;
    uint8_t effect;
    uint8_t amount;
};

struct pattern{
    uint16_t length = 0;
    cell* cells = nullptr;
};

struct track {
    track() = default;
    instrumentBase* instrument = nullptr; // new basicInstrument();
    effectBase* effects[maxEffects]{}; // new gain();
    pattern patterns[256]{};
    uint8_t orderTable[256];
    uint16_t row = 0;
    uint8_t orderIdx = 0;
    uint8_t loopEnd = 0;
    uint8_t loopStart = 255;
};

extern bool playing;

extern track tracks[totalTracks];
void prepareTracks();
void allocateInstrument(uint8_t track, uint16_t type);
void allocateEffect(uint8_t track, uint8_t idx, uint16_t type);
void updateVoices();
#include "handlers/trackHandler.hpp"

bool playing = false;
track tracks[totalTracks]{};

void prepareTracks() {
    for (uint8_t t = 0; t < totalTracks; t++) {
        allocateInstrument(t, 0);
        for (uint8_t e = 0; e < maxEffects; e++) allocateEffect(t, e, 0);
    }
}

void allocateInstrument(uint8_t track, uint16_t type) {
    delete tracks[track].instrument;
    switch (type) {
        case 1:
            tracks[track].instrument = nullptr;  // new basicInstrument;
            break;
        default:
            tracks[track].instrument = nullptr;  // new instrumentBase;
            break;
    }
    tracks[track].instrument->type = type;
}

void allocateEffect(uint8_t track, uint8_t idx, uint16_t type) {
    if (idx > maxEffects) return;
    delete tracks[track].effects[idx];
    switch (type) {
        default:
            tracks[track].effects[idx] = nullptr;  // new effectBase;
            break;
    }
    tracks[track].effects[idx]->type = type;
}

void updateVoices() {
    if (playing) {
        for (auto t : tracks) {
            uint8_t patternIdx = t.orderTable[t.orderIdx];
            if (t.instrument != nullptr) {
                uint32_t cellIdx = t.row * t.instrument->maxVoices;
                for (uint8_t v = 0; v < t.instrument->maxVoices; v++) {
                    cell c = t.patterns[patternIdx].cells[cellIdx];
                    if (c.note == 0x80)
                        t.instrument->releaseVoice(v);
                    else
                        t.instrument->pressVoice(v, c.note, c.velocity);
                    if (c.effect != 0)
                        t.instrument->updateEffect(v, c.effect, c.amount);
                    cellIdx++;
                }
            }
            t.row++;
            if (t.row > t.patterns[patternIdx].length) {
                t.row = 0;
                t.orderIdx++;
            }
            if (t.orderIdx > t.loopEnd) t.orderIdx = t.loopStart;
        }
    }
}
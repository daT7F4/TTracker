#include "handlers/trackHandler.hpp"

bool playing = false;
track tracks[totalTracks]{};

void prepareTracks() {
    uint8_t i = 0;
    for (auto t : tracks) {
        allocateInstrument(i, 0);
        allocateEffect(i, 0, 0);

        i++;
    }
}

void allocateInstrument(uint8_t track, uint16_t type) {
    delete tracks[track].instrument;
    switch (type) {
        case 0:
            tracks[track].instrument = new basicInstrument;
            break;
        default:
            tracks[track].instrument = new instrumentBase;
            break;
    }
    tracks[track].instrument->type = type;
}

void allocateEffect(uint8_t track, uint8_t idx, uint16_t type) {
    delete tracks[track].effect;
    switch (type) {
        default:
            tracks[track].effect = new effectBase;
            break;
    }
    tracks[track].effect->type = type;
}

void updateVoices() {
    if (playing) {
        for (auto t : tracks) {
            uint8_t patternIdx = t.orderTable[t.orderIdx];
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
            t.row++;
            if (t.row > t.patterns[patternIdx].length) {
                t.row = 0;
                t.orderIdx++;
            }
            if (t.orderIdx > t.loopEnd) t.orderIdx = t.loopStart;
        }
    }
}
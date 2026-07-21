#include "handlers/fileHandler.hpp"

void fileHandler::begin() {
    addLog("Starting SD card interface...");
    if (!SD_MMC.setPins(43, 44, 39, 40, 41, 42)) {
        stop("Failed to set SD card pins!", "fileHandler::begin");
        return;
    }

    if (!SD_MMC.begin()) {
        stop("Failed to begin SD card!", "fileHandler::begin");
        return;
    }

    SD_MMC.mkdir("projects");
    SD_MMC.mkdir("samples");
    SD_MMC.mkdir("resampled");
}

void fileHandler::resample(const char* file) {
    if (!SD_MMC.exists(file)) {
        stop("File to resample doesn't exist!", "fileHandler::resample");
        return;
    }

    File f = SD_MMC.open(file, FILE_READ);
    if (!f) {
        stop("Couldn't open file to resample!", "fileHandler::resample");
        f.close();
        return;
    }
    f.seek(22);
    channels = f.read();
    if (channels > 2) {
        stop("File has more than 2 channels!", "fileHandler::resample");
        f.close();
        return;
    }

    f.seek(34);
    if (f.read() != 16) {
        stop("File isn't 16-bit!", "fileHandler::resample");
        f.close();
        return;
    }

    f.seek(40);
    uint32_t size;
    f.readBytes((char*)&size, sizeof(size));

    rawAudio = static_cast<int16_t*>(heap_caps_malloc(size, MALLOC_CAP_SPIRAM));
    delta = static_cast<int16_t*>(heap_caps_malloc(size, MALLOC_CAP_SPIRAM));
    f.readBytes((char*)rawAudio, size);
    f.close();
}

void fileHandler::process() {}

void fileHandler::loadProject(const char* file) {
    if (!SD_MMC.exists(file)) {
        stop("Project file doesn't exist!", "fileHandler::loadProject");
        return;
    }

    File f = SD_MMC.open(file, FILE_READ);
    if (!f) {
        stop("Couldn't open project file!", "fileHandler::loadProject");
        f.close();
        return;
    }

    uint8_t version = f.read();
    f.readBytes((char*)&songData::BPM, 2);
    songData::RPB = f.read();
    uint8_t trackCount = f.read();
    for (uint8_t trackIdx = 0; trackIdx < trackCount; trackIdx++) {
        uint16_t instrumentType = 0;
        f.readBytes((char*)&instrumentType, 2);
        addLog("Reading track #" + String(trackIdx) + "...");
        allocateInstrument(trackIdx, instrumentType);
        uint32_t instrumentDataLength = 0;
        f.readBytes((char*)&instrumentDataLength, 4);
        for (uint32_t i = 0; i < instrumentDataLength; i++) {
            addLog("Instrument type #" + String(instrumentType));
            popup("Reading instrument data", i, instrumentDataLength);
            uint32_t id = 0;
            f.readBytes((char*)&id, 2);
            uint32_t size = 0;
            f.readBytes((char*)&size, 4);
            auto it = tracks[trackIdx].instrument->data.find(id);
            if (it != tracks[trackIdx].instrument->data.end()) {
                if (size != it->second.size) {
                    addLog("Variable " + it->second.name + "(" +
                           String(it->first) + ")" +
                           " doesn't match size in file.");
                    f.seek(f.position() + size);
                } else {
                    f.readBytes((char*)it->second.ptr, size);
                }
            } else {
                f.seek(f.position() + size);
                addLog("Variable " + it->second.name + "(" + String(it->first) +
                       ")" + " doesn't exist in the instrument.");
            }
        }
        uint8_t effectCount = f.read();
        for (uint8_t effectIdx = 0; effectIdx < effectCount; effectIdx++) {
            if (effectIdx > maxEffects) {
                addLog("Too many effects in track (" + String(effectCount) +
                       "), skipping...");
                break;
            }
            uint16_t effectType = 0;
            f.readBytes((char*)&effectType, 2);
            addLog("Reading effect #" + String(effectIdx) + " type #" +
                   String(effectType) + "...");
            allocateEffect(trackIdx, effectIdx, effectType);
            uint32_t effectDataLength = 0;
            f.readBytes((char*)&effectDataLength, 4);
            for (uint32_t i = 0; i < effectDataLength; i++) {
                popup("Reading effect data", i, effectDataLength);
                uint32_t id = 0;
                f.readBytes((char*)&id, 2);
                String name = f.readString();
                uint32_t size = 0;
                f.readBytes((char*)&size, 4);
                auto it = tracks[trackIdx].effects[effectIdx]->data.find(id);
                if (it != tracks[trackIdx].effects[effectIdx]->data.end()) {
                    if (size != it->second.size) {
                        addLog("Variable " + it->second.name + "(" +
                               String(it->first) + ")" +
                               " doesn't match size in file, skipping....");
                        f.seek(f.position() + size);
                    } else {
                        f.readBytes((char*)it->second.ptr, size);
                    }
                } else {
                    f.seek(f.position() + size);
                    addLog("Variable " + it->second.name + "(" +
                           String(it->first) + ")" +
                           " doesn't exist in the effect, skipping...");
                }
            }
            f.readBytes((char*)tracks[trackIdx].orderTable, 256);
            uint8_t patternCount = f.read();
            for (uint8_t patternIdx = 0; patternIdx < patternCount;
                 patternIdx++) {
                popup("Reading patterns", patternIdx, patternCount);
                uint32_t patternLength = 0;
                f.readBytes((char*)&patternLength, 2);
                if (patternLength) {
                    tracks[trackIdx].patterns[patternIdx].length =
                        patternLength;
                    uint32_t cellCount =
                        patternLength * tracks[trackIdx].instrument->maxVoices;
                    tracks[trackIdx].patterns[patternIdx].cells =
                        static_cast<cell*>(heap_caps_malloc(
                            sizeof(cell) * cellCount, MALLOC_CAP_SPIRAM));
                    for (uint32_t i = 0; i < cellCount; i++) {
                        uint8_t b = f.read();
                        cell c;
                        if (b & 0x80) {
                            uint8_t r = 1;
                            if (b & 0x10) r = f.read();
                            c.note = b & 0x1 ? f.read() : 0;
                            c.velocity = b & 0x2 ? f.read() : 0;
                            c.effect = b & 0x4 ? f.read() : 0;
                            c.amount = b & 0x8 ? f.read() : 0;
                            for (uint8_t j = 0; j < r; j++)
                                tracks[trackIdx]
                                    .patterns[patternIdx]
                                    .cells[i++] = c;
                        } else {
                            c.note = b;
                            c.velocity = f.read();
                            c.effect = f.read();
                            c.amount = f.read();
                            tracks[trackIdx].patterns[patternIdx].cells[i] = c;
                        }
                    }
                } else {
                    tracks[trackIdx].patterns[patternIdx].cells = nullptr;
                    tracks[trackIdx].patterns[patternIdx].length = 0;
                }
            }
        }
    }
    addLog("Read " + String(f.position()) + " bytes");
    f.close();
}

void fileHandler::saveProject(const char* file) {
    if (SD_MMC.exists(file)) {
        SD_MMC.remove(file);
    }
    File f = SD_MMC.open(file, FILE_WRITE, true);
    if (!f) {
        stop("Couldn't open project file!", "fileHandler::saveProject");
        f.close();
        return;
    }
    f.write(FILE_VERSION);
    f.write((uint8_t*)&songData::BPM, sizeof(songData::BPM));
    f.write(songData::RPB);
    f.write(totalTracks);
    uint8_t trackIdx = 0;
    for (auto t : tracks) {
        addLog("Saving track #" + String(trackIdx) + "...");
        if (t.instrument != nullptr) {
            addLog("Instrument type #" + String(t.instrument->type));
            f.write(t.instrument->type);
            uint32_t size = t.instrument->data.size();
            f.write((uint8_t*)size, sizeof(size));
            for (auto it : t.instrument->data) {
                f.write((uint8_t*)&it.first, sizeof(it.first));
                f.write((uint8_t*)&it.second.size, sizeof(it.second.size));
                f.write((uint8_t*)&it.second.ptr, sizeof(it.second.size));
            }
        } else {
            f.write(0);
            f.write(0);
        }
        f.write(maxEffects);
        for (auto e : t.effects) {
            if (e != nullptr) {
                addLog("Effect type #" + String(e->type));
                f.write((uint8_t*)&e->type, sizeof(e->type));
                uint32_t size = e->data.size();
                f.write((uint8_t*)&size, sizeof(size));
                for (auto it : e->data) {
                    f.write((uint8_t*)&it.first, sizeof(it.first));
                    f.write((uint8_t*)&it.second.size, sizeof(it.second.size));
                    f.write((uint8_t*)&it.second.ptr, sizeof(it.second.size));
                }
            } else {
                f.write(0);
                f.write(0);
            }
        }
        f.write(t.orderTable, sizeof(t.orderTable));
        f.write(255);
        for (auto p : t.patterns) {
            if(p.length != 0){
                uint8_t repeat = 0;
                cell prev = cell(0, 0, 0, 0);
                cell curr = p.cells[0];
                for (size_t i = 1; i < t.instrument->maxVoices * p.length; i++) {
                    prev = curr;
                    curr = p.cells[i];
                    if (curr.note == prev.note && curr.velocity == prev.velocity &&
                        curr.effect == prev.velocity &&
                        curr.amount == prev.amount || repeat == 255) {
                        repeat++;
                    } else {
                        if (repeat != 0) {
                            f.write(0x90 | (p.cells[i].note != 0) ? 0x1 : 0
                                        | (p.cells[i].velocity != 0) ? 0x2 : 0
                                        | (p.cells[i].effect != 0) ? 0xC : 0
                                        | (p.cells[i].amount != 0) ? 0x8 : 0);
                            f.write(repeat);
                        } else {
                            uint8_t temp = (p.cells[i].note != 0) ? 0x1 : 0
                                        | (p.cells[i].velocity != 0) ? 0x2 :0
                                        | (p.cells[i].effect != 0) ? 0xC : 0
                                        | (p.cells[i].amount != 0) ? 0x8 : 0;
                            if (temp != 0xF) f.write(temp | 0x80);
                        }
                        if (p.cells[i].note != 0) f.write(p.cells[i].note);
                        if (p.cells[i].velocity != 0) f.write(p.cells[i].velocity);
                        if (p.cells[i].effect != 0) f.write(p.cells[i].effect);
                        if (p.cells[i].amount != 0) f.write(p.cells[i].amount);
                        repeat = 0;
                    }
                }
            } else{
                f.write(0);
            }
        }
    }
}
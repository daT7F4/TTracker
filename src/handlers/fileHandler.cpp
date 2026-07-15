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
    size_t size;
    f.readBytes(reinterpret_cast<char*>(&size), 4);

    rawAudio = static_cast<int16_t*>(heap_caps_malloc(size, MALLOC_CAP_SPIRAM));
    delta = static_cast<int16_t*>(heap_caps_malloc(size, MALLOC_CAP_SPIRAM));
    f.readBytes(reinterpret_cast<char*>(rawAudio), size);
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
    f.readBytes(reinterpret_cast<char*>(&songData::BPM), 2);
    songData::RPB = f.read();
    uint8_t trackCount = f.read();
    for (uint8_t trackIdx = 0; trackIdx < trackCount; trackIdx++) {
        uint16_t instrumentType = 0;
        f.readBytes(reinterpret_cast<char*>(&instrumentType), 2);
        allocateInstrument(trackIdx, instrumentType);
        uint32_t trackDataLength = 0;
        f.readBytes(reinterpret_cast<char*>(&trackDataLength), 4);
        for(uint32_t i = 0; i < trackDataLength; i++){
            uint32_t id = 0;
            f.readBytes(reinterpret_cast<char*>(&id), 2);
            String name = f.readString();
            uint32_t size = 0;
            f.readBytes(reinterpret_cast<char*>(&size), 4);
            void* data = nullptr;
            f.readBytes(reinterpret_cast<char*>(data), size);
            specialVariable variable;
            specialVariable var;
                var.value.visit(
                    [&](uint8_t &v){v = reinterpret_cast<uint8_t>(data);},
                    [&](int8_t &v){v = reinterpret_cast<int8_t>(data);},
                    [&](uint16_t &v){v = reinterpret_cast<uint16_t>(data);},
                    [&](int16_t &v){v = reinterpret_cast<int16_t>(data);},
                    [&](uint32_t &v){v = reinterpret_cast<uint32_t>(data);},
                    [&](int32_t &v){v = reinterpret_cast<int32_t>(data);}
                    // to come
                );
                tracks[trackIdx].instrument->customVariable(data);
            variable.name = name;
            tracks[trackIdx].instrument->loadVariable(id, variable);
        }
        uint8_t effectCount = f.read();
        for (uint8_t effectIdx = 0; effectIdx < effectCount; effectIdx++) {
            uint16_t effectType = 0;
            f.readBytes(reinterpret_cast<char*>(&effectType), 2);
            allocateEffect(trackIdx, effectIdx, effectType);
            uint32_t effectDataLength = 0;
            f.readBytes(reinterpret_cast<char*>(&effectDataLength), 4);
            for(uint32_t i = 0; i < effectDataLength; i++){
                uint32_t id = 0;
                f.readBytes(reinterpret_cast<char*>(&id), 2);
                String name = f.readString();
                uint32_t size = 0;
                f.readBytes(reinterpret_cast<char*>(&size), 4);
                void* data = malloc(size);
                f.readBytes(reinterpret_cast<char*>(data), size);
                specialVariable var;
                var.value.visit(
                    [&](uint8_t &v){v = reinterpret_cast<uint8_t>(data);},
                    [&](int8_t &v){v = reinterpret_cast<int8_t>(data);},
                    [&](uint16_t &v){v = reinterpret_cast<uint16_t>(data);},
                    [&](int16_t &v){v = reinterpret_cast<int16_t>(data);},
                    [&](uint32_t &v){v = reinterpret_cast<uint32_t>(data);},
                    [&](int32_t &v){v = reinterpret_cast<int32_t>(data);}
                    // to come
                );
                tracks[trackIdx].effect[effectIdx].customVariable(data);
                var.name = name;
                tracks[trackIdx].effect[effectIdx].loadVariable(id, var);
            }
            f.readBytes(reinterpret_cast<char*>(tracks[trackIdx].orderTable),
                        256);
            uint8_t patternCount = f.read();
            tracks[trackIdx].patterns = static_cast<pattern*>(heap_caps_malloc(
                sizeof(pattern) * patternCount, MALLOC_CAP_SPIRAM));
            for (uint8_t patternIdx = 0; patternIdx < patternCount;
                 patternIdx++) {
                uint32_t patternLength = 0;
                f.readBytes(reinterpret_cast<char*>(&patternLength), 2);
                tracks[trackIdx].patterns[patternIdx].length = patternLength;
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
                        if (b & 0x10) {
                            r = f.read();
                        }
                        c.note = b & 0x1 ? f.read() : 0;
                        c.velocity = b & 0x2 ? f.read() : 0;
                        c.effect = b & 0x4 ? f.read() : 0;
                        c.amount = b & 0x8 ? f.read() : 0;
                        for (uint8_t j = 0; j < r; j++)
                            tracks[trackIdx].patterns[patternIdx].cells[i++] =
                                c;
                    } else {
                        c.note = b;
                        c.velocity = f.read();
                        c.effect = f.read();
                        c.amount = f.read();
                        tracks[trackIdx].patterns[patternIdx].cells[i] = c;
                    }
                }
            }
        }
    }
    f.close();
}

void fileHandler::saveProject(const char *file){
    if(SD_MMC.exists(file)){
        SD_MMC.remove(file);
    }
    File f = SD_MMC.open(file, FILE_WRITE, true);
    if(!f){
        stop("Couldn't open project file!", "fileHandler::saveProject");
        f.close();
        return;
    }
    f.write(FILE_VERSION);
    f.write(reinterpret_cast<uint8_t*>(&songData::BPM), 2);
    f.write(songData::RPB);
    f.write(totalTracks);
    for(auto t : tracks){
        f.write(t.instrument->type);
    }
}
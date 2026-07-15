#include "instruments/instrumentBase.hpp"

instrumentBase::~instrumentBase() { heap_caps_free(buffer); }

void instrumentBase::resize(size_t bufferSize) {
    if (buffer != nullptr) heap_caps_free(buffer);
    buffer = (int16_t*)heap_caps_aligned_alloc(
        16, bufferSize, MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (buffer == nullptr) {
        stop("Failed to resize buffer!", "instrumentBase::resize");
    }
}

void instrumentBase::render(size_t renderSize) {}
void instrumentBase::resetVoices() {}
void instrumentBase::pressVoice(uint8_t idx, uint8_t pitch, uint8_t velocity) {}
void instrumentBase::releaseVoice(uint8_t idx) {}
void instrumentBase::updateEffect(uint8_t idx, uint8_t effectType, uint8_t effectAmount) {}
void instrumentBase::loadVariable(uint16_t id, specialVariable var){
    if(data.contains(id))
        data[id] = var;
}


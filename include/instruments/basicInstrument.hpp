#pragma once

#include "instrumentBase.hpp"

class basicInstrument : public instrumentBase {
   public:
    basicInstrument();
    ~basicInstrument();
    void resize(size_t bufferSize) override;
    void render(size_t renderSize) override;
    void drawUI() override;
    void resetVoices() override;
    void pressVoice(uint8_t idx, uint8_t pitch, uint8_t velocity) override;
    void releaseVoice(uint8_t idx) override;
    void updateEffect(uint8_t idx, uint8_t effectType,
                      uint8_t effectAmount) override;

    uint16_t type = 1;
    const uint8_t maxVoices = 8;
    fpv volumeIdx = 384;
    int16_t* buffer = nullptr;
    std::map<uint16_t, storedVariable> data;

   private:
};
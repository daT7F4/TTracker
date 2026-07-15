#pragma once

#define SAMPLE_RATE 40000
#define BCLK -1
#define WSEL -1
#define DIN -1

#include <ESP_I2S.h>

#include "esp_cache.h"
#include "esp_dsp.h"
#include "globalCalls.hpp"
#include "handlers/trackHandler.hpp"
#include "lookup.h"
#include "songData.hpp"

class audioInterface {
   public:
    audioInterface();
    void startTask(TaskHandle_t &handle);
    void allocateBuffers();

   private:
    void loop();
    static void bridge(void *p);

    I2SClass _i2s;

    int16_t* _buffer1 = nullptr;
    int16_t* _buffer2 = nullptr;
    bool _bufferSelect = false;
    size_t _renderSize = 0;

    size_t _floorSize = 0;
    size_t _ceilSize = 0;
    fpv _overflow, _error;
};
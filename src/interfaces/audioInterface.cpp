#include "interfaces/audioInterface.hpp"

audioInterface::audioInterface() = default;

void audioInterface::allocateBuffers() {
    fpv size =
        (static_cast<fpv>(SAMPLE_RATE * 60) / (songData::BPM / songData::RPB)) *
        2;
    _floorSize = size.toInt();
    _ceilSize = ceil(size.toFloat());

    _renderSize = _floorSize;

    if (_buffer1 != nullptr) heap_caps_free(_buffer1);
    if (_buffer2 != nullptr) heap_caps_free(_buffer2);

    _buffer1 = (int16_t*)heap_caps_aligned_alloc(
        64, _ceilSize << 1,
        MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    _buffer2 = (int16_t*)heap_caps_aligned_alloc(
        64, _ceilSize << 1,
        MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    uint8_t i = 0;
    for (auto t : tracks) {
        t.instrument->resize(_ceilSize << 1);
        for (uint8_t i = 0; i < 1; i++) t.effects[i]->resize(_ceilSize << 1);
        popup("Resizing buffers", i, 8);
        i++;
    }
}

void audioInterface::startTask(TaskHandle_t& handle) {
    xTaskCreate(bridge, "Audio Interface", 8192, this, 5, &handle);
}

void audioInterface::bridge(void* p) {
    audioInterface* instance = static_cast<audioInterface*>(p);
    instance->loop();
    vTaskDelete(NULL);
}

void audioInterface::loop() {
    addLog("Starting audio interface...");
    _i2s.setPins(BCLK, WSEL, DIN);
    if (!_i2s.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT,
                    I2S_SLOT_MODE_STEREO)) {
        stop("Failed to begin I2S device!", "audioInterface::begin()");
        return;
    }
    addLog("Allocating audio buffers...");
    allocateBuffers();

    while (true) {
        updateVoices();
        for (auto t : tracks) {
            t.instrument->render(_renderSize);
            for (uint8_t i = 0; i < 1; i++)
                t.effects[i]->process(t.instrument->buffer, _renderSize);
            if (_bufferSelect)
                dsps_add_s16_ansi(t.instrument->buffer, _buffer2, _buffer2,
                                  _renderSize, 1, 1, 1, 0);
            else
                dsps_add_s16_ansi(t.instrument->buffer, _buffer1, _buffer1,
                                  _renderSize, 1, 1, 1, 0);
        }

        if (_bufferSelect) {
            _i2s.write(reinterpret_cast<uint8_t*>(_buffer2), _renderSize);
        } else {
            _i2s.write(reinterpret_cast<uint8_t*>(_buffer1), _renderSize);
        }
        _bufferSelect = !_bufferSelect;
        _error += _overflow;
        _renderSize = _floorSize;
        if (_error >= 1) {
            _error -= 1;
            _renderSize = _ceilSize;
        }
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}
#include "interfaces/rotaryInterface.hpp"

bool upInt = false;
bool downInt = false;
bool buttonInt = false;

rotaryInterface::rotaryInterface()
    : left(ROTINT_CS, ROTINT_MISO, ROTINT_MOSI, ROTINT_SCK, 0),
      right(ROTINT_CS, ROTINT_MISO, ROTINT_MOSI, ROTINT_SCK, 1),
      button(ROTINT_CS, ROTINT_MISO, ROTINT_MOSI, ROTINT_SCK, 2) {};

void IRAM_ATTR upInterrupt(){
    upInt = true;
}

void IRAM_ATTR downInterrupt(){
    downInt = true;
}

void IRAM_ATTR buttonInterrupt(){
    buttonInt = true;
}

void IRAM_ATTR rotaryInterface::crr(uint8_t idx, fpv &ref, fpv step, fpv min, fpv max){
    references[idx] = &ref;
    steps[idx] = step;
    minimum[idx] = min;
    maximum[idx] = max;
}

void IRAM_ATTR rotaryInterface::cbc(uint8_t idx, void (*ref)(uint8_t idx, bool state)){
    presses[idx] = ref;
}

void rotaryInterface::startTask(TaskHandle_t &handle){
    xTaskCreate(bridge, "Rotary Interface", 4096, this, 5, &handle);
}

void rotaryInterface::bridge(void *p){
    rotaryInterface* instance = static_cast<rotaryInterface*>(p);
    instance->loop();
    vTaskDelete(NULL);
}

void rotaryInterface::loop() {
        pinMode(ROTINT_UINT, INPUT);
    pinMode(ROTINT_DINT, INPUT);
    pinMode(ROTINT_BINT, INPUT);

    attachInterrupt(ROTINT_UINT, upInterrupt, RISING);
    attachInterrupt(ROTINT_DINT, downInterrupt, RISING);
    attachInterrupt(ROTINT_BINT, buttonInterrupt, RISING);

    left.setSPIspeed(MCP23S17_MAX_SPI_SPEED);
    right.setSPIspeed(MCP23S17_MAX_SPI_SPEED);
    button.setSPIspeed(MCP23S17_MAX_SPI_SPEED);

    SPI.begin();

    left.begin(true);
    right.begin(true);
    button.begin(true);

    left.pinMode16(0);
    left.enableInterrupt8(0, 0xFF, CHANGE);
    left.mirrorInterrupts(true);

    left.enableHardwareAddress();

    button.enableInterrupt8(1, 0xFF, CHANGE);

    while (true) {
        if(upInt){
            uint16_t i = left.read16();
            uint8_t f = left.getInterruptFlagRegister8(0);
            uint16_t m = 1;
            for(uint8_t j = 0; j < 8; j++){
                if(f & m){
                    if((i & m) == (i & (m << 8))){
                        *references[j] += steps[j];
                        if(*references[j] > maximum[j])
                            *references[j] = maximum[j];
                    }
                    else{
                        *references[j] -= steps[j];
                        if(*references[j] < minimum[j])
                            *references[j] = minimum[j];
                    }
                }
                m <<= 1;
            }
            upInt = false;
        }
        if(downInt){
            uint16_t i = right.read16();
            uint8_t f = right.getInterruptFlagRegister8(0);
            uint16_t m = 1;
            for(uint8_t j = 0; j < 8; j++){
                if(f & m){
                    if((i & m) == (i & (m << 8))){
                        *references[j + 8] += steps[j + 8];
                        if(*references[j + 8] > maximum[j + 8])
                            *references[j + 8] = maximum[j + 8];
                    }
                    else{
                        *references[j + 8] -= steps[j + 8];
                        if(*references[j + 8] < minimum[j + 8])
                            *references[j + 8] = minimum[j + 8];
                    }
                }
                m <<= 1;
            }
            downInt = false;
        }
        if(buttonInt){
            uint16_t i = button.read16();
            uint16_t f = button.getInterruptFlagRegister();
            uint16_t m = 1;
            for(uint8_t b = 0; b < 16; b++){
                if(f & m)
                    presses[i](b, i & m);
            }
            buttonInt = false;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
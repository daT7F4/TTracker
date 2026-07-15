#pragma once

#include <Arduino.h>
#include <MCP23S17.h>

#include "fpv.hpp"

#define ROTINT_CS 46
#define ROTINT_MOSI 47
#define ROTINT_MISO 48
#define ROTINT_SCK 33

#define ROTINT_UINT 20
#define ROTINT_DINT 22
#define ROTINT_BINT 21

void upInterrupt();
void downInterrupt();
void buttonInterrupt();

extern bool upInt;
extern bool downInt;
extern bool buttonInt;

class rotaryInterface {
   public:
    rotaryInterface();
    void startTask(TaskHandle_t &handle);

    void crr(uint8_t idx, fpv& ref, fpv step, fpv min, fpv max);
    void cbc(uint8_t idx, void (*ref)(uint8_t idx, bool state));

   private:
    void loop();
    static void bridge(void* p);

    MCP23S17 left;
    MCP23S17 right;
    MCP23S17 button;

    fpv* references[16];
    fpv steps[16];
    fpv minimum[16];
    fpv maximum[16];

    void (*presses[16])(uint8_t idx, bool state);
};
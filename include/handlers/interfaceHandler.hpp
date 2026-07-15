#pragma once

#include <Arduino.h>

#include "interfaces/audioInterface.hpp"
#include "interfaces/displayInterface.hpp"
#include "interfaces/rotaryInterface.hpp"
#include "handlers/fileHandler.hpp"
#include "globalCalls.hpp"

void begin();
void stopWrapper(String message, String dataType);
void logWrapper(String log);
void popupWrapper(String title, fpv progress, fpv total);
void crrWrapper(uint8_t idx, fpv &ref, fpv step, fpv min, fpv max);
void cbcWrapper(uint8_t idx, void (*call)(uint8_t idx, bool state));

extern audioInterface audio;
extern displayInterface display;
extern rotaryInterface rotary;
extern fileHandler file;

extern TaskHandle_t audioHandle;
extern TaskHandle_t displayHandle;
extern TaskHandle_t rotaryHandle;
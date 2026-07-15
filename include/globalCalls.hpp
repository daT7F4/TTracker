#pragma once

#include <Arduino.h>

#include "interfaces/displayInterface.hpp"
#include "fpv.hpp"

extern void (*addLog)(String log);
extern void (*popup)(String text, fpv progress, fpv total);
extern void (*stop)(String message, String dataType);

extern void (*rotaryReference)(uint8_t idx, fpv &ref, fpv step, fpv min, fpv max);
extern void (*buttonCallback)(uint8_t idx, void (*ref)(uint8_t idx, bool state));
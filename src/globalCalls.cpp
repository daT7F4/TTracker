#include "globalCalls.hpp"

void (*addLog)(String log) = nullptr;
void (*popup)(String text, fpv progress, fpv total) = nullptr;
void (*stop)(String message, String dataType) = nullptr;

void (*rotaryReference)(uint8_t idx, fpv &ref, fpv step, fpv min, fpv max) = nullptr;
void (*buttonCallback)(uint8_t idx, void (*ref)(uint8_t idx, bool state)) = nullptr;
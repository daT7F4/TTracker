#include <Arduino.h>

#include "handlers/interfaceHandler.hpp"

void setup() {
    Serial.begin(115200);
    begin();
}

void loop() {
    vTaskDelete(NULL);
}
#pragma once
#include <Arduino.h>

#include "displayingFunctions.hpp"
#include "globalCalls.hpp"
#include "fpv.hpp"

extern fpv testVariables[16];
extern bool testToggle[16];

void toggleCallback(uint8_t idx, bool state);

class displayInterface {
   public:
    displayInterface();
    void startTask(TaskHandle_t &handle);
    void writeError(String msg, String dataType);
    void writeLog(String log);
    void popup(String title, fpv progress, fpv total);
    void updateDisplay();
    void setReferences();

   private:
    static void bridge(void *p);
    void loop();

    String logs[34];
    uint8_t startPos = 0;
    uint32_t lastLogTimestamp = 0;
    uint32_t lastPopupTimestamp = 0;
    String popupTitle;
    fpv percentage;
    bool showPopup = false;
    bool showLog = false;
};

#include "handlers/interfaceHandler.hpp"

audioInterface audio;
displayInterface display;
rotaryInterface rotary;
fileHandler file;

TaskHandle_t audioHandle = NULL;
TaskHandle_t displayHandle = NULL;
TaskHandle_t rotaryHandle = NULL;

void begin(){
    display.startTask(displayHandle);
    stop = stopWrapper;
    addLog = logWrapper;
    popup = popupWrapper;
    rotaryReference = crrWrapper;
    buttonCallback = cbcWrapper;
    addLog("TTracker v.0.0");
    file.begin();
    audio.startTask(audioHandle);
    rotary.startTask(rotaryHandle);
}

void crrWrapper(uint8_t idx, fpv &ref, fpv step, fpv min, fpv max){
    rotary.crr(idx, ref, step, min, max);
}

void cbcWrapper(uint8_t idx, void (*call)(uint8_t idx, bool state)){
    rotary.cbc(idx, call);
}

void popupWrapper(String title, fpv progress, fpv total){
    display.popup(title, progress, total);
}

void logWrapper(String log){
    display.writeLog(log);
}

void stopWrapper(String message, String dataType){
    vTaskDelete(audioHandle);
    vTaskDelete(displayHandle);
    vTaskDelete(rotaryHandle);
    display.writeError(message, dataType);
}
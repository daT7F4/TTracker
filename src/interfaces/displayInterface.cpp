#include "interfaces/displayInterface.hpp"

displayInterface::displayInterface() = default;

fpv testVariables[16]{};
bool testToggle[16]{};

void displayInterface::startTask(TaskHandle_t &handle){
    xTaskCreate(bridge, "Display Interface", 8192, this, 1, &handle);
}

void displayInterface::bridge(void *p){
    displayInterface *instance = static_cast<displayInterface*>(p);
    instance->loop();
    vTaskDelete(NULL);
}

void displayInterface::setReferences(){
    for(uint8_t i = 0; i < 16; i++){
        rotaryReference(i, testVariables[i], 1, -100, 100);
        buttonCallback(i, toggleCallback);
    }
}

void toggleCallback(uint8_t idx, bool state){
    if(!state){
        testToggle[idx] = !testToggle[idx];
    }
}

void displayInterface::loop() {
    for (uint8_t i = 0; i < 34; i++) logs[i] = "";
    if (!tft.init()) {
        while (1);
    }
    tft.setRotation(1);
    tft.setColorDepth(24);
    tft.fillScreen(0);
    canvas.createSprite(tft.width(), tft.height());
    canvas.setColorDepth(24);
    canvas.fillScreen(0);

    while (true) {
        if (showLog){
            for (uint8_t i = 0; i < 34; i++) {
                if (logs[i])
                    drawText(0, i * 7, 1, logs[i], 0xFFFFFF, false);
            }
            if (lastLogTimestamp + 1000 < millis()) {
                startPos = 0;
                for (uint8_t i = 0; i < 34; i++) logs[i] = "";
                showLog = false;
            }
        } else{
            for(uint8_t i = 0; i < 16; i++){
                drawText(i*17, 0, 1, String(testVariables[i].toInt()), 0xFFFFFF, testToggle[i]);
            }
        }
        if(showPopup){
            uint16_t x = (tft.width() - 114) / 2;
            uint16_t y = (tft.height() - 50) / 2;
            canvas.setColor(0);
            canvas.fillRect(x, y, 114, 50);
            canvas.setColor(0xFFFFFF);
            canvas.drawRect(x, y, 114, 50);
            canvas.drawFastHLine(x, y + 10, 114);
            drawText(x + 2, y + 2, 1, popupTitle, 0xFFFFFF, false);
            canvas.drawRect((tft.width() - 102) / 2, y + 30, 102, 12);
            canvas.fillRect((tft.width() - 100) / 2, y + 31, percentage.toInt(), 10);
            Serial.print(lastPopupTimestamp);
            Serial.print(" ");
            Serial.println(millis());
            if(lastPopupTimestamp + 1000 < millis()){
                showPopup = false;
            }
        }
        updateDisplay();
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

void displayInterface::updateDisplay() {
    canvas.pushSprite(0, 0);
    canvas.fillScreen(0);
}

void displayInterface::writeLog(String log) {
    logs[startPos++] = log;
    startPos %= 34;
    showLog = true;
    lastLogTimestamp = millis();
}

void displayInterface::popup(String title, fpv progress, fpv total){
    popupTitle = title;
    percentage = (progress / total) * 100;
    showPopup = true;
    lastPopupTimestamp = millis();
}

void displayInterface::writeError(String msg, String dataType) {
    drawText(0, 0, 3, "Error!", 0xFF0000, true);

    drawText(0, 20, 1, dataType, 0xFFFFFF, false);
    drawText(0, 28, 1, msg, 0xFFFFFF, false);
    updateDisplay();
}
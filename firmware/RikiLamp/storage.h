#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "lamp_types.h"

class StorageManager {
public:
    void begin();

    bool hasWiFi();
    void saveWiFi(const String& ssid, const String& password);
    String getSSID();
    String getPassword();

    void saveColour(LampColour colour);
    LampColour getColour();

    void saveEffect(LampEffect effect);
    LampEffect getEffect();

    void savePower(bool on);
    bool getPower();

    void saveBrightness(uint8_t brightness);
    uint8_t getBrightness();

    void clear();

private:
    Preferences prefs;
};

extern StorageManager storage;

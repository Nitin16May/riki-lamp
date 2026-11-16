#pragma once

#include <Arduino.h>

class OTAManager {
public:
    void begin();
    void loop();

    /// Updates firmware over HTTPS/HTTP from a binary file URL (Cloud OTA)
    bool updateFromUrl(const char* url);

private:
    bool _active = false;
};

extern OTAManager ota;

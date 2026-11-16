#include "ota.h"
#include "led.h"

#include <ArduinoOTA.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

OTAManager ota;

void OTAManager::begin() {
    if (_active) {
        return;
    }

    // Set a friendly network hostname for Arduino IDE discovery
    const uint64_t mac = ESP.getEfuseMac();
    String hostname = String("riki-lamp-") + String((uint32_t)(mac >> 16), HEX);
    ArduinoOTA.setHostname(hostname.c_str());

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {
            type = "filesystem";
        }
        Serial.println("OTA: Start flashing " + type);
        led.setSolidColour(LampColour::Blue);
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA: Update Complete! Rebooting...");
        led.setSolidColour(LampColour::Green);
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
        led.setSolidColour(LampColour::Red);
    });

    ArduinoOTA.begin();
    _active = true;

    Serial.print("OTA service started. Network Hostname: ");
    Serial.println(hostname);
}

void OTAManager::loop() {
    if (!_active) {
        return;
    }
    ArduinoOTA.handle();
}

bool OTAManager::updateFromUrl(const char* url) {
    if (!url || strlen(url) == 0) {
        return false;
    }

    Serial.print("Cloud OTA: Starting download from ");
    Serial.println(url);

    led.setSolidColour(LampColour::Purple);

    WiFiClientSecure client;
    client.setInsecure(); // Allow downloading from GitHub / Cloud without strict certificate check

    httpUpdate.rebootOnUpdate(true);
    t_httpUpdate_return ret = httpUpdate.update(client, url);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("Cloud OTA Failed. Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            led.setSolidColour(LampColour::Red);
            return false;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("Cloud OTA: No updates found");
            return false;

        case HTTP_UPDATE_OK:
            Serial.println("Cloud OTA: Success! Rebooting...");
            led.setSolidColour(LampColour::Green);
            return true;
    }

    return false;
}

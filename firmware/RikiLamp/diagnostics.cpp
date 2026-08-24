#include "diagnostics.h"

#include "config.h"
#include "led.h"
#include "mqtt.h"
#include "riki_network.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_system.h>

DiagnosticsManager diagnostics;

namespace {

const char* resetReasonToString(esp_reset_reason_t reason) {
    switch (reason) {
        case ESP_RST_POWERON: return "POWERON";
        case ESP_RST_SW: return "SOFTWARE";
        case ESP_RST_PANIC: return "PANIC";
        case ESP_RST_INT_WDT: return "INT_WDT";
        case ESP_RST_TASK_WDT: return "TASK_WDT";
        case ESP_RST_WDT: return "WDT";
        case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
        case ESP_RST_BROWNOUT: return "BROWNOUT";
        case ESP_RST_SDIO: return "SDIO";
        default: return "UNKNOWN";
    }
}

} // namespace

void DiagnosticsManager::begin() {
    Serial.print("Restart reason: ");
    Serial.println(resetReasonToString(esp_reset_reason()));

    Serial.print("Firmware version: ");
    Serial.println(FIRMWARE_VERSION);
}

void DiagnosticsManager::loop() {
    static unsigned long lastLogMs = 0;
    const unsigned long now = millis();

    if (now - lastLogMs < DIAGNOSTICS_INTERVAL_MS) {
        return;
    }

    lastLogMs = now;

    Serial.println("--- Diagnostics ---");
    Serial.print("WiFi: ");
    Serial.println(rikiNetwork.isConnected() ? "Connected" : "Disconnected");

    if (rikiNetwork.isConnected()) {
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI: ");
        Serial.println(WiFi.RSSI());
    }

    Serial.print("MQTT: ");
    Serial.println(mqtt.connected() ? "Connected" : "Reconnecting");

    Serial.print("Colour: ");
    Serial.println(lampColourName(led.currentColour()));

    Serial.print("Effect: ");
    Serial.println(lampEffectName(led.currentEffect()));

    Serial.print("Power: ");
    Serial.println(led.isOn() ? "ON" : "OFF");
}

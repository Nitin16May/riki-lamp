#include "local_server.h"
#include "led.h"
#include "lamp_types.h"

#include <ArduinoJson.h>

LocalServer localServer;

void LocalServer::begin() {
    if (_active) {
        return;
    }

    _server.on("/api/status", HTTP_GET, [this]() {
        _server.sendHeader("Access-Control-Allow-Origin", "*");
        JsonDocument doc;
        doc["lamp"] = "chakri";
        doc["power"] = led.isOn();
        doc["brightness"] = led.brightness();
        doc["colour"] = lampColourName(led.currentColour());
        doc["effect"] = lampEffectName(led.currentEffect());
        String res;
        serializeJson(doc, res);
        _server.send(200, "application/json", res);
    });

    _server.on("/api/power", HTTP_POST, [this]() {
        _server.sendHeader("Access-Control-Allow-Origin", "*");
        if (_server.hasArg("plain")) {
            JsonDocument doc;
            deserializeJson(doc, _server.arg("plain"));
            bool on = doc["on"] | true;
            if (on) {
                led.lampOn();
            } else {
                led.lampOff();
            }
        }
        _server.send(200, "application/json", "{\"success\":true}");
    });

    _server.on("/api/brightness", HTTP_POST, [this]() {
        _server.sendHeader("Access-Control-Allow-Origin", "*");
        if (_server.hasArg("plain")) {
            JsonDocument doc;
            deserializeJson(doc, _server.arg("plain"));
            uint8_t b = doc["brightness"] | 255;
            led.setBrightness(b);
        }
        _server.send(200, "application/json", "{\"success\":true}");
    });

    // Handle OPTIONS for browser CORS preflight
    _server.onNotFound([this]() {
        if (_server.method() == HTTP_OPTIONS) {
            _server.sendHeader("Access-Control-Allow-Origin", "*");
            _server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            _server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
            _server.send(204);
            return;
        }
        _server.send(404, "text/plain", "Not found");
    });

    _server.begin();
    _active = true;
    Serial.println("Local HTTP server ready on port 80");
}

void LocalServer::loop() {
    if (_active) {
        _server.handleClient();
    }
}

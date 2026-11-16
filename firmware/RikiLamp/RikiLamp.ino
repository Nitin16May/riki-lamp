#include "config.h"
#include "diagnostics.h"
#include "led.h"
#include "mqtt.h"
#include "ota.h"
#include "portal.h"
#include "riki_network.h"
#include "storage.h"
#include "touch.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP.h>
#include <cstring>

namespace {

bool publishColour(LampColour colour) {
    JsonDocument doc;
    doc["type"] = "colour";
    doc["colour"] = lampColourName(colour);

    return mqtt.publish(mqtt.lightTopic(), doc);
}

bool publishEffect(LampEffect effect) {
    JsonDocument doc;
    doc["type"] = "effect";
    doc["effect"] = lampEffectName(effect);

    return mqtt.publish(mqtt.lightTopic(), doc);
}

void handleRemoteLightMessage(JsonDocument& doc) {
    const char* type = doc["type"];
    if (!type) {
        return;
    }

    if (strcmp(type, "colour") == 0) {
        const char* colourName = doc["colour"];
        LampColour colour = LampColour::White;
        if (colourName && parseLampColour(colourName, colour)) {
            led.setSolidColour(colour);
        }
        return;
    }

    if (strcmp(type, "effect") == 0) {
        const char* effectName = doc["effect"];
        LampEffect effect = LampEffect::Static;
        if (effectName && parseLampEffect(effectName, effect)) {
            led.setEffect(effect);
        }
        return;
    }
}

void onMqttMessage(const String& topic, JsonDocument& doc) {
    if (topic.endsWith("/light")) {
        handleRemoteLightMessage(doc);
        return;
    }

    if (topic.endsWith("/ota")) {
        const char* url = doc["url"];
        if (url) {
            ota.updateFromUrl(url);
        }
        return;
    }
}

void onTouchGesture(TouchGesture gesture) {
    switch (gesture) {
        case TouchGesture::SingleTap: {
            led.nextColour();
            publishColour(led.currentColour());
            break;
        }

        case TouchGesture::DoubleTap: {
            led.nextEffect();
            publishEffect(led.currentEffect());
            break;
        }

        case TouchGesture::TripleTap:
            led.nextBrightness();
            break;

        case TouchGesture::LongPress: {
            if (led.isOn()) {
                led.lampOff();
            } else {
                led.lampOn();
            }
            break;
        }

        case TouchGesture::VeryLongPress: {
            Serial.println("Factory reset requested");
            storage.clear();
            ESP.restart();
            break;
        }
    }
}

} // namespace

void setup() {
    Serial.begin(115200);

    // Wait up to 1 second for Serial port to open (needed for debugging on PC),
    // but don't block boot when powered from a standalone USB adapter/power bank.
    for (int i = 0; i < 100 && !Serial; ++i) {
        delay(10);
    }

    storage.begin();
    diagnostics.begin();

    Serial.println();
    Serial.println("==========================");
    Serial.println("      RiKi Lamp");
    Serial.println("==========================");

    led.begin();
    touch.begin();
    touch.onGesture(onTouchGesture);

    rikiNetwork.begin();

    if (rikiNetwork.isProvisioning()) {
        portal.begin();
        return;
    }

    mqtt.begin();
    mqtt.onMessage(onMqttMessage);

    ota.begin();
}

void loop() {
    touch.loop();
    led.loop();
    rikiNetwork.loop();

    if (rikiNetwork.isProvisioning()) {
        portal.loop();
        return;
    }

    mqtt.loop();
    diagnostics.loop();
    ota.loop();
}

#include "riki_network.h"

#include "config.h"
#include "storage.h"

#include <WiFi.h>

RiKiNetwork rikiNetwork;

void RiKiNetwork::begin() {
    if (storage.hasWiFi()) {
        Serial.println("WiFi credentials found");

        if (connectStation()) {
            _mode = NetworkMode::Station;
            return;
        }

        Serial.println("STA connect failed — starting setup AP");
    } else {
        Serial.println("No WiFi configured");
    }

    startAccessPoint();
    _mode = NetworkMode::Provisioning;
}

void RiKiNetwork::loop() {
    if (_mode != NetworkMode::Station) {
        return;
    }

    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    const unsigned long now = millis();
    if (now - _lastReconnectAttempt < WIFI_RECONNECT_INTERVAL_MS) {
        return;
    }

    _lastReconnectAttempt = now;
    Serial.println("WiFi disconnected — reconnecting...");

    connectStation();
}

NetworkMode RiKiNetwork::mode() const {
    return _mode;
}

bool RiKiNetwork::isProvisioning() const {
    return _mode == NetworkMode::Provisioning;
}

bool RiKiNetwork::isConnected() const {
    return _mode == NetworkMode::Station && WiFi.status() == WL_CONNECTED;
}

bool RiKiNetwork::connectStation() {
    const String ssid = storage.getSSID();
    const String password = storage.getPassword();

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false, false);
    WiFi.begin(ssid.c_str(), password.c_str());

    const unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection failed");
        return false;
    }

    Serial.print("WiFi connected. IP: ");
    Serial.println(WiFi.localIP());
    return true;
}

void RiKiNetwork::startAccessPoint() {
    WiFi.mode(WIFI_AP_STA);
    delay(500);

    const bool ok = WiFi.softAP(AP_SSID, nullptr, 6, false, 4);

    Serial.printf("AP started: %s\n", ok ? "yes" : "no");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

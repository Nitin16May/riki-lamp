#pragma once

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

class MQTTManager {
public:
    using MessageCallback = void (*)(const String& topic, JsonDocument& doc);

    MQTTManager();

    void begin();
    void loop();

    bool connected();
    bool publish(const String& topic, const JsonDocument& doc, bool retained = false);
    bool subscribe(const String& topic);

    void onMessage(MessageCallback callback);

    /// Returns `riki/v1/<lampId>/light` for this device.
    String lightTopic() const;

private:
    bool connectBroker();
    void resubscribeAll();
    void subscribeDefaults();
    void handleMessage(char* topic, uint8_t* payload, unsigned int length);
    static void messageCallback(char* topic, uint8_t* payload, unsigned int length);

    String clientId() const;
    String buildTopic(const char* suffix) const;
    String partnerWildcardTopic() const;
    bool rememberSubscription(const String& topic);

    WiFiClientSecure _wifiClient;
    PubSubClient _client;

    MessageCallback _onMessage = nullptr;
    unsigned long _lastReconnectAttempt = 0;
    bool _active = false;
    bool _wasConnected = false;

    static constexpr int MAX_SUBSCRIPTIONS = 8;
    String _subscriptions[MAX_SUBSCRIPTIONS];
    int _subscriptionCount = 0;
};

extern MQTTManager mqtt;

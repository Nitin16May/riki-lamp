#include "mqtt.h"

#include <Arduino.h>
#include <WiFi.h>
#include <lwip/dns.h>

MQTTManager mqtt;

namespace {

MQTTManager* activeInstance = nullptr;

constexpr char MQTT_BROKER[] = "1ea6ac40907c404eb1eb3edfd51eedce.s1.eu.hivemq.cloud";
constexpr uint16_t MQTT_PORT = 8883;
constexpr char MQTT_USER[] = "rikilamp";
constexpr char MQTT_PASS[] = "ChakriLovesNikki2903";

constexpr char LAMP_ID[] = "chakri";
constexpr char PARTNER_LAMP_ID[] = "nikki";

constexpr char MQTT_TOPIC_ROOT[] = "riki/v1";
constexpr unsigned long MQTT_RECONNECT_INTERVAL_MS = 5000;
constexpr uint16_t MQTT_KEEPALIVE_SEC = 30;

// Replace with your broker Root CA if TLS verification fails.
// ISRG Root X1 — valid for HiveMQ Cloud (Let's Encrypt).
static const char HIVEMQ_ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

} // namespace

MQTTManager::MQTTManager() : _client(_wifiClient) {}

String MQTTManager::clientId() const {
    const uint64_t mac = ESP.getEfuseMac();
    return String("riki-") + LAMP_ID + "-" + String((uint32_t)(mac >> 16), HEX);
}

String MQTTManager::buildTopic(const char* suffix) const {
    return String(MQTT_TOPIC_ROOT) + "/" + LAMP_ID + "/" + suffix;
}

String MQTTManager::lightTopic() const {
    return buildTopic("light");
}

String MQTTManager::partnerWildcardTopic() const {
    return String(MQTT_TOPIC_ROOT) + "/" + PARTNER_LAMP_ID + "/#";
}

void MQTTManager::messageCallback(char* topic, uint8_t* payload, unsigned int length) {
    if (activeInstance) {
        activeInstance->handleMessage(topic, payload, length);
    }
}

void MQTTManager::handleMessage(char* topic, uint8_t* payload, unsigned int length) {
    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, payload, length);

    if (err) {
        Serial.print("MQTT invalid JSON on ");
        Serial.println(topic);
        return;
    }

    if (_onMessage) {
        const String topicStr = topic;
        _onMessage(topicStr, doc);
    }
}

bool MQTTManager::rememberSubscription(const String& topic) {
    for (int i = 0; i < _subscriptionCount; i++) {
        if (_subscriptions[i] == topic) {
            return true;
        }
    }

    if (_subscriptionCount >= MAX_SUBSCRIPTIONS) {
        Serial.println("MQTT subscription limit reached");
        return false;
    }

    _subscriptions[_subscriptionCount++] = topic;
    return true;
}

void MQTTManager::subscribeDefaults() {
    subscribe(partnerWildcardTopic());
    subscribe(buildTopic("ota"));
}

void MQTTManager::resubscribeAll() {
    for (int i = 0; i < _subscriptionCount; i++) {
        if (_client.subscribe(_subscriptions[i].c_str())) {
            Serial.println("Subscribed:");
            Serial.println(_subscriptions[i]);
        }
    }
}

bool MQTTManager::connectBroker() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    // Ensure Google DNS & Cloudflare are registered in lwIP to prevent ISP DNS drops
    ip_addr_t d1, d2;
    d1.type = IPADDR_TYPE_V4;
    d1.u_addr.ip4.addr = ipaddr_addr("8.8.8.8");
    d2.type = IPADDR_TYPE_V4;
    d2.u_addr.ip4.addr = ipaddr_addr("1.1.1.1");
    dns_setserver(0, &d1);
    dns_setserver(1, &d2);

    const String id = clientId();
    const String statusTopic = buildTopic("status");

    Serial.print("Resolving broker: ");
    Serial.println(MQTT_BROKER);

    IPAddress resolvedIp;
    if (WiFi.hostByName(MQTT_BROKER, resolvedIp)) {
        Serial.print("Broker IP: ");
        Serial.println(resolvedIp);
    } else {
        Serial.println("DNS failed to resolve MQTT_BROKER!");
    }

    Serial.print("Connecting MQTT as ");
    Serial.print(id);
    Serial.println("...");

    const bool ok = _client.connect(
        id.c_str(),
        MQTT_USER,
        MQTT_PASS,
        statusTopic.c_str(),
        0,
        true,
        "OFFLINE"
    );

    if (!ok) {
        Serial.print("MQTT connect failed, rc=");
        Serial.println(_client.state());
        return false;
    }

    Serial.println("Connected");

    _client.publish(statusTopic.c_str(), "ONLINE", true);

    resubscribeAll();
    if (_subscriptionCount == 0) {
        subscribeDefaults();
    }

    return true;
}

void MQTTManager::begin() {
    if (_active) {
        return;
    }

    activeInstance = this;

    // Use setInsecure to bypass clock sync requirement on ESP32 while keeping TLS port 8883 encryption
    _wifiClient.setInsecure();

    _client.setServer(MQTT_BROKER, MQTT_PORT);
    _client.setCallback(MQTTManager::messageCallback);
    _client.setKeepAlive(MQTT_KEEPALIVE_SEC);
    _client.setBufferSize(1024);

    _active = true;
    _wasConnected = false;
    _lastReconnectAttempt = 0;

    connectBroker();
    _wasConnected = _client.connected();
}

void MQTTManager::loop() {
    if (!_active) {
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        _wasConnected = false;
        return;
    }

    const bool isConnected = _client.connected();
    if (_wasConnected && !isConnected) {
        Serial.println("MQTT Lost");
    }
    _wasConnected = isConnected;

    _client.loop();

    if (_client.connected()) {
        return;
    }

    const unsigned long now = millis();
    if (now - _lastReconnectAttempt < MQTT_RECONNECT_INTERVAL_MS) {
        return;
    }

    _lastReconnectAttempt = now;
    Serial.println("MQTT Reconnecting...");
    connectBroker();
    _wasConnected = _client.connected();
}

bool MQTTManager::connected() {
    return _active && _client.connected();
}

bool MQTTManager::publish(const String& topic, const JsonDocument& doc, bool retained) {
    if (!_client.connected()) {
        return false;
    }

    String payload;
    serializeJson(doc, payload);

    const bool ok = _client.publish(topic.c_str(), payload.c_str(), retained);

    if (ok) {
        Serial.print("MQTT publish (retained=");
        Serial.print(retained ? "true" : "false");
        Serial.print(") ");
        Serial.println(topic);
    }

    return ok;
}

bool MQTTManager::subscribe(const String& topic) {
    rememberSubscription(topic);

    if (!_client.connected()) {
        return false;
    }

    const bool ok = _client.subscribe(topic.c_str());

    if (ok) {
        Serial.println("Subscribed:");
        Serial.println(topic);
    }

    return ok;
}

void MQTTManager::onMessage(MessageCallback callback) {
    _onMessage = callback;
}

#pragma once

#include <Arduino.h>

enum class NetworkMode {
    Provisioning,
    Station
};

class RiKiNetwork {
public:
    void begin();
    void loop();

    NetworkMode mode() const;
    bool isProvisioning() const;
    bool isConnected() const;

private:
    bool connectStation();
    void startAccessPoint();

    NetworkMode _mode = NetworkMode::Provisioning;
    unsigned long _lastReconnectAttempt = 0;
};

extern RiKiNetwork rikiNetwork;

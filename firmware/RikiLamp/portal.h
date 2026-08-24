#pragma once

#include <WebServer.h>
#include <DNSServer.h>

class PortalManager {
public:
    void begin();
    void loop();

private:
    void setupRoutes();
    void redirectToPortal();

    void handleRoot();
    void handleScan();
    void handleConnect();

    WebServer _server{80};
    DNSServer _dns;
    bool _active = false;
};

extern PortalManager portal;

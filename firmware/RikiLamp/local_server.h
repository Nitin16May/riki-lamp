#pragma once

#include <WebServer.h>

class LocalServer {
public:
    void begin();
    void loop();

private:
    WebServer _server{80};
    bool _active = false;
};

extern LocalServer localServer;

#ifndef WEB_SERVICE_H
#define WEB_SERVICE_H

#include <iostream>
#include <map>
#include <Arduino.h>
#include "EthernetManager.h"

// Define type for route handler (callback)
typedef std::function<void(EthernetClient& client, const String& path, const String& body)> RouteHandler;

class WebService {
  private:

    // Ethernet server which handling HTTP protocol (from <Ethernet.h>)
    EthernetServer server;

    // Ethernet connection object (from ethernet manager)
    EthernetManager& eth;
    
    // Temporary storage for mapping path and handler/callback
    std::map<String, RouteHandler> routes;

    // unpacking request
    String readRequest(EthernetClient& client, String &method, String &path, String &body);

  public:
    WebService(EthernetManager& ethernet, int port);
    // Begin http server
    void begin();

    // Handling client request
    void handleClient();

    // Register route and handler
    void on(const String& path, RouteHandler handler);
    
    
};

#endif

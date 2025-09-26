#ifndef WEB_SERVICE_H
#define WEB_SERVICE_H

#include <Arduino.h>
#include "EthernetManager.h"

// Define type for route handler (callback)
typedef std::function<void(EthernetClient&, const String& path, const String& body)> RouteHandler;

class WebService {
  private:

    // Route structure
    struct Route {
        String path;
        RouteHandler handler;
      };

    // Ethernet server which handling HTTP protocol (from <Ethernet.h>)
    EthernetServer server;

    // Ethernet connection object (from ethernet manager)
    EthernetManager& eth;
    
    // Store mapping path
    std::vector<Route> routes;

    // Function
    String readRequest(EthernetClient& client);
    String parsePath(const String& request);
    void serveResponse(EthernetClient& client, const String& request);


  public:
    WebService(EthernetManager& ethernet, int port);
    void begin();
    void handleClient();
    // Add route
    void on(const String& path, RouteHandler handler);
    
};

#endif

#include "WebService.h"


WebService::WebService(EthernetManager& ethernet, int port) 
  : server(port), eth(ethernet) {}

void WebService::begin() {
  server.begin();
}

void WebService::on(const String& path, RouteHandler handler) {
  routes.push_back({path, handler});
}

void WebService::handleClient() {
  EthernetClient client = server.available();
  if (client) {
    IPAddress clientIP = client.remoteIP();
    if (!eth.isAllowed(clientIP)) {
      client.println("HTTP/1.1 403 Forbidden");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("Access Denied");
      client.stop();
      return;
    }

    String request = readRequest(client);
    String path = parsePath(request);
    bool handled = false;

    for (auto& route : routes) {
      if (path == route.path) {
        route.handler(client);  // Execute registered handler
        handled = true;
        break;
      }
    }

    if (!handled) {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("404 - Not Found");
    }

    client.stop();
  }
}

String WebService::readRequest(EthernetClient& client) {
  String req = "";
  unsigned long timeout = millis();
  while (client.connected() && (millis() - timeout < 1000)) {
    if (client.available()) {
      char c = client.read();
      req += c;
      if (req.endsWith("\r\n\r\n")) break;
    }
  }
  return req;
}

String WebService::parsePath(const String& request) {
  int start = request.indexOf("GET ") + 4;
  int end = request.indexOf(' ', start);
  if (start == -1 || end == -1) return "/";
  return request.substring(start, end);
}
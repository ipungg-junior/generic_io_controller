#include "WebService.h"


WebService::WebService(EthernetManager& ethernet, int port) 
  : server(port), eth(ethernet) {}

void WebService::begin() {
  server.begin();
}

void WebService::on(const String& path, RouteHandler handler) {
  routes[path] = handler;
}

void WebService::handleClient() {
  EthernetClient availableClient = server.available();
  if (availableClient) {
    EthernetClient& client = availableClient;
    IPAddress clientIP = client.remoteIP();
    if (!eth.isAllowed(clientIP)) {
      client.println("HTTP/1.1 403 Forbidden");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("Access Denied");
      client.stop();
      Serial.println("Request denied (block IP / not whitelist)");
      return;
    }

    String method, path, body;
    Serial.println("Request in");
    readRequest(client, method, path, body);

    if (routes.find(path) != routes.end()) {
      Serial.println("Calling handler");
      routes[path](client, path, body);
      client.stop();
    } else {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("Not Found");
      client.stop();
    }
  }
}

String WebService::readRequest(EthernetClient& client, String &method, String &path, String &body) {
  String requestLine = client.readStringUntil('\r');
  client.readStringUntil('\n');

  int firstSpace = requestLine.indexOf(' ');
  int secondSpace = requestLine.indexOf(' ', firstSpace + 1);

  if (firstSpace > 0 && secondSpace > 0) {
    method = requestLine.substring(0, firstSpace);
    path   = requestLine.substring(firstSpace + 1, secondSpace);
  }

  Serial.print(method);
  Serial.print(" ");
  Serial.println(path);
  // Skip headers
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r" || line.length() == 0) break;
  }

  // Read body
  body = "";
  while (client.available()) {
    body += (char)client.read();
  }
  Serial.println(body);
  return requestLine;
}


#include <SPI.h>
#include "EthernetManager.h"
#include "WebService.h"
#include "Parser.h"
#include "PinController.h"

#include <ArduinoJson.h>

// Config
byte mac[] = { 0xDE, 0xAA, 0xBE, 0xEF, 0x00, 0x02 };
IPAddress staticIP(10, 251, 2, 126);
IPAddress gateway(10, 251, 2, 1);
IPAddress dns(10, 251, 2, 1);
IPAddress subnet(255, 255, 255, 0);

// Whitelist IP
IPAddress whitelist[] = {
    IPAddress(10, 251, 12, 133),
    IPAddress(10, 251, 12, 109),
    IPAddress(10, 251, 2, 103)
};

// Ethernet connection object
EthernetManager eth(mac, staticIP, dns, gateway, subnet);
WebService http(eth, 80);
PinController pinController;


void io_handler(EthernetClient& client, const String& path, const String& body) {
  Parser parser(body);

  if (!parser.isValid()) {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Invalid JSON");
    return;
  }

  String cmd = parser.getCommand();

  if (cmd == "set_pin") {
    if (!parser.hasKey("pin_number")) {
      client.println("HTTP/1.1 400 Bad Request");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.print("{\"status\":false,");
      client.print("\"message\":\"Please set your pin!\"}");

      return;
    }
    int pin = parser.getInt("pin_number");
    if (!parser.hasKey("value")) {
      client.println("HTTP/1.1 400 Bad Request");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.print("{\"status\":false,");
      client.print("\"message\":\"Please set value!\"}");
      return;
    }
    
    int setVal = parser.getInt("value");
    
    // Check if auto_reverse parameter exists
    if (parser.hasKey("auto_reverse")) {
      int interVal = parser.getInt("auto_reverse");
      pinController.setPin(pin, setVal, interVal);
    } else {
      // No auto reverse, just set the pin
      pinController.setPin(pin, setVal, 0);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    client.print("{");
    client.print("\"status\":true,");
    client.print("\"pin_num\":");
    client.print(pin);
    client.print(", \"message\":\"Pin io setting up completed\"");
    client.print("}");

  }
  else if (cmd == "off_all") {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();

    // create loop to set all pin to 0 or off state
    // call function here (pinControler.off_all()) even pin that not registered

    client.print("{");
    client.print("\"status\":true,");
    client.print("\"message\":\"Set all GPIO to 0 (off)\"");
    client.print("}");
  }
  else if (cmd == "on_all") {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();

    // create loop to set all pin to 1 or on state
    // call function here (pinControler.on_all()) even pin that not registered

    client.print("{");
    client.print("\"status\":true,");
    client.print("\"message\":\"Set all GPIO to 0 (off)\"");
    client.print("}");
  }
  else {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print("{");
    client.print("\"status\":false,");
    client.print("\"pin_num\":");
    client.print(pin);
    client.print(", \"message\":\"Unknown command on GPIO\"");
    client.print("}");
  }
}

void coreHandling(EthernetClient& client, const String& path, const String& body) {
  Parser parser(body);

  if (!parser.isValid()) {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Invalid JSON");
    return;
  }

  String cmd = parser.getCommand();
  if (cmd == "restart") {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.print("{");
    client.print("\"status\":true,");
    client.print("\"pin_num\":");
    client.print(pin);
    client.print(", \"message\":\"Trying to restart, see you :)\"");
    client.print("}");
    ESP.restart();
  }
  else {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print("{");
    client.print("\"status\":false,");
    client.print("\"pin_num\":");
    client.print(pin);
    client.print(", \"message\":\"Unknown command on core\"");
    client.print("}");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(32, OUTPUT);
  eth.begin(5);
  eth.setWhitelist(whitelist, sizeof(whitelist) / sizeof(whitelist[0]));
  http.begin();

  // Setup route handler
  http.on("/core", coreHandling);
  http.on("/relay", io_handler);
  http.on("/relay/on", relayOn);
  http.on("/relay/off", relayOff);

}

void loop() {
  // waiting client forever
  http.handleClient();

  // IO routine
  pinController.processAutoReverse();

}
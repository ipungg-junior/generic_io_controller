#include <SPI.h>
#include "EthernetManager.h"
#include "WebService.h"
#include "Parser.h"
#include "PinController.h"
#include <ArduinoJson.h>

// Config
byte mac[] = { 0xDE, 0xAA, 0xBE, 0xEF, 0x00, 0x02 };
IPAddress staticIP(10, 251, 1, 10);
IPAddress gateway(10, 251, 1, 1);
IPAddress dns(10, 251, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress whitelist[] = {
    IPAddress(10, 251, 1, 25)
};

// Ethernet connection object
EthernetManager eth(mac, staticIP, dns, gateway, subnet);
WebService http(eth, 80);
PinController pinController;


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
  if (cmd == "name") {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.print("{");
    client.print("\"status\":true,");
    client.print("\"message\":\"hai ");
    client.print(cmd);
    client.print("\"}");
  }
  else {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print("{");
    client.print("\"status\":false,");
    client.print("\"message\":\"Unknown command on core\"");
    client.print("}");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Setup ethernet network
  eth.begin(5);
  eth.setWhitelist(whitelist, sizeof(whitelist) / sizeof(whitelist[0]));
  
  // Setup route handler
  http.begin();
  http.on("/core", coreHandling);

  // Setup button pins (example: pin 15 as button input)
  // You can add more buttons by calling pinController.setPinAsInput(pinNumber)
  pinController.setPinAsInput(13);
}

void loop() {
  // waiting client forever
  http.handleClient();

  // IO routine
  pinController.processAutoReverse();
  
  // Scan for button presses
  pinController.scanButtons();

  if (pinController.getState(13) == 1) {
    // Button on pin 15 is pressed, do something
    pinController.setPin(32, 1, 5000);
  }

}

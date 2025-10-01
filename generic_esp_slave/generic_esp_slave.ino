#include <SPI.h>
#include "EthernetManager.h"
#include "WebService.h"
#include "Parser.h"
#include "PinController.h"
#include "MySQLConnector.h"
#include <ArduinoJson.h>

// Network profile cofiguration
IPAddress whitelist[] = {
  IPAddress(10, 251, 12, 133),
  IPAddress(10, 251, 12, 109),
  IPAddress(10, 251, 2, 103)
};
byte mac[] = { 0xDE, 0xAA, 0xBE, 0xEF, 0x00, 0x02 };
IPAddress staticIP(10, 251, 2, 126);
IPAddress gateway(10, 251, 2, 1);
IPAddress dns(10, 251, 2, 1);
IPAddress subnet(255, 255, 255, 0);
EthernetManager eth(mac, staticIP, dns, gateway, subnet);

// Database configuration
IPAddress mysql_address(10, 251, 2, 114);
MySQLConnector mysql;

// Web service configuration
WebService http(eth, 80);

// GPIO configuration
PinController pinController;

// Sensor or relay or ext. module
#include "Wiegand.h"
WIEGAND wg;


unsigned long prevMillisWiegand;


void gpioHandling(EthernetClient& client, const String& path, const String& body) {
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
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    pinController.offAll();

    client.print("{");
    client.print("\"status\":true,");
    client.print("\"message\":\"Set all GPIO to 0 (off)\"");
    client.print("}");
  }
  else if (cmd == "on_all") {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    pinController.onAll();

    client.print("{");
    client.print("\"status\":true,");
    client.print("\"message\":\"Set all GPIO to 1 (on)\"");
    client.print("}");
  }
  else {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print("{");
    client.print("\"status\":false,");
    client.print("\"message\":\"Unknown command on GPIO\"");
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
    client.print("\"message\":\"Trying to restart, see you :)\"");
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
    client.print("\"message\":\"Unknown command on core\"");
    client.print("}");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(32, OUTPUT);

  // Network ethernet setup
  eth.begin(5);
  eth.setWhitelist(whitelist, sizeof(whitelist) / sizeof(whitelist[0]));
  
  // Webservice Setup
  http.begin();
  http.on("/core", coreHandling);
  http.on("/gpio", gpioHandling);

  // Setup button pins
  pinController.setPinAsInput(13);
  pinController.setPinAsInput(14);
  
  // Setup MySQL connection (example configuration)
  if (mysql.connect(mysql_address, 3306, "uprod", "P@ssw0rd*1", "doorlock")) {
    Serial.println("Connected to MySQL database");
  } else {
    Serial.println("Failed to connect to MySQL database");
  }

  // Wiegand scanner RFID setup
  wg.begin(16, 17);
}

void loop() {
  // Global current millis for countdown
  unsigned long currentMillis = millis();

  // waiting client forever
  http.handleClient();

  // IO routine
  pinController.processAutoReverse();
  
  // Scan for button presses
  pinController.scanButtons();

  // Receptionist btn check routine
  if (pinController.getState(13) == 1) {
    // Button on pin 15 is pressed, do something
    Serial.println("Button 13 pressed");
    pinController.setPin(32, 1, 5000);
  }

  // Exit btn check routine
  if (pinController.getState(14) == 1) {
    // Button on pin 15 is pressed, do something
    pinController.setPin(32, 1, 5000);
  }
  

  // Wiegand routine
  if (wg.available()){

    // check if interval has passed
    if (currentMillis - prevMillisWiegand >= 2000) {
      bool is_opened = false;
      prevMillisWiegand = currentMillis;  // save the last time
      String wgData = String(wg.getCode());

      // Skip noise
      if (wgData.length() < 6){
        return;
      }
      if (mysql.queryf("SELECT employee_card.id, employee.name FROM employee_card JOIN employee ON employee.id = employee_card.employee_id WHERE employee_card.card_number = '%s'", wgData)) {
        // Fetch and process rows
        while (mysql.fetchRow()) {
          int id = mysql.getInt(0);
          const char* name = mysql.getString(1);
          Serial.print("ID: ");
          Serial.print(id);
          Serial.print(", Name: ");
          Serial.println(name);
          if (!is_opened){
            is_opened = true;
            pinController.setPin(32, 1, 3000);
          }
        }
        mysql.closeCursor();
      }
      Serial.print("Wiegand scan result : ");
      Serial.println(wgData);

    }
  }
  
                   
  
}

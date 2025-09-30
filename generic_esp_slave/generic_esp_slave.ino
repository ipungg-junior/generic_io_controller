#include <SPI.h>
#include "EthernetManager.h"
#include "WebService.h"
#include "Parser.h"
#include "PinController.h"
#include "MySQLConnector.h"
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

// MySql ip address
IPAddress mysql_address(192, 168, 1, 100);  // MySQL server IP

// Ethernet connection object
EthernetManager eth(mac, staticIP, dns, gateway, subnet);
WebService http(eth, 80);
PinController pinController;
MySQLConnector mysql;


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
  eth.begin(5);
  eth.setWhitelist(whitelist, sizeof(whitelist) / sizeof(whitelist[0]));
  http.begin();

  // Setup route handler
  http.on("/core", coreHandling);
  http.on("/gpio", gpioHandling);

  // Setup button pins (example: pin 15 as button input)
  // You can add more buttons by calling pinController.setPinAsInput(pinNumber)
  pinController.setPinAsInput(13);
  pinController.setPinAsInput(14);
  
  // Setup MySQL connection (example configuration)
  // Replace with your actual MySQL server IP, credentials, and database name
  if (mysql.connect(mysql_address, 3306, "user", "password", "database")) {
    Serial.println("Connected to MySQL database");
  } else {
    Serial.println("Failed to connect to MySQL database");
  }
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
    Serial.println("Button 13 pressed");
    pinController.setPin(32, 1, 5000);
  }
  if (pinController.getState(14) == 1) {
    // Button on pin 15 is pressed, do something
    pinController.setPin(32, 1, 5000);
  }
  
  // Example of using MySQL connector in loop
  // This could be used for logging button presses or other events
  static unsigned long lastQueryTime = 0;
  if (millis() - lastQueryTime > 10000) {  // Every 10 seconds
    if (mysql.connected()) {
      // Example query to insert data
      mysql.query("INSERT INTO events (event_type, timestamp) VALUES ('system_check', NOW())");
      
      // Example of using variables in queries
      int button13State = pinController.getState(13);
      int button14State = pinController.getState(14);
      mysql.query("INSERT INTO button_states (button13, button14, timestamp) VALUES (%d, %d, NOW())",
                   button13State, button14State);
                   
      // Example of using SELECT queries to retrieve data
      if (mysql.query("SELECT id, name, value FROM config WHERE active=1")) {
        // Get cursor to process results
        MySQL_Cursor* cursor = mysql.getCursor();
        
        // Process rows
        while (cursor->get_next_row()) {
          // Get column values
          long id = cursor->get_long(0);
          const char* name = cursor->get_string(1);
          long value = cursor->get_long(2);
          
          Serial.print("Config - ID: ");
          Serial.print(id);
          Serial.print(", Name: ");
          Serial.print(name);
          Serial.print(", Value: ");
          Serial.println(value);
        }
      }
    }
    lastQueryTime = millis();
  }
}

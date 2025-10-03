#include <SPI.h>
#include "EthernetManager.h"
#include "WebService.h"
#include "PinController.h"
#include "TransactionLog.h"

// Network profile cofiguration
byte mac[] = { 0xDE, 0xAA, 0xBE, 0xEF, 0x00, 0x02 };
EthernetManager eth(mac);

// Database configuration
TransactionLog logger;
TransactionEntry entry;

// Web service configuration
WebService http(eth, 80);

// GPIO configuration
PinController pinController;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Network ethernet setup
  eth.begin(5);
  
  // Webservice Setup
  http.begin();

  // Setup button pins
  pinController.setPinAsInput(13);
  pinController.setPinAsInput(14);

  
  // Show log from flash-mem
  logger.begin();
  for (int i = 0; i < logger.size(); i++) {
      entry = logger.get(i);
      Serial.print("Log #"); Serial.print(i);
      Serial.print(" ID: "); Serial.print(entry.id);
      Serial.print(" UID: "); Serial.println(entry.uid);
  }
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
    pinController.setPin(32, 1, 5000);
  }

  // Exit btn check routine
  if (pinController.getState(14) == 1) {
    // Button on pin 15 is pressed, do something
    pinController.setPin(32, 1, 1500);
  }
  
                   
  
}

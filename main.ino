#include "sti.h"

#define PASSTI_TX 17
#define PASSTI_RX 16

void setup() {
  Serial.begin(115200);
  Passti::serial().begin(38400, SERIAL_8N1, PASSTI_RX, PASSTI_TX);
  delay(500);

  Passti::sendInit(true);
  for (size_t i = 0; i < Passti::responseLen; i++) {
    Serial.printf("0x%02X ", Passti::responseBuffer[i]);
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Main loop");
  delay(1000);
}

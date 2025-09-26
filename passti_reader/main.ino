#include "sti.h"

#define PASSTI_TX 17
#define PASSTI_RX 16

void setup() {
  Serial.begin(115200);
  delay(2000);
  // Setup passti reader and init device
  Passti::setupSerial(2, PASSTI_RX, PASSTI_TX);
  Passti::init(true);


}

void loop() {
  Passti::readSerialFrame();
  delay(1000);
  Serial.println("Main loop");
}

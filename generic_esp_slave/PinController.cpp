#include "PinController.h"

PinController::PinController() : pinCount(0) {
  // Initialize all pin states
  for (int i = 0; i < MAX_PINS; i++) {
    pinStates[i].pin = -1;  // -1 indicates unused slot
    pinStates[i].value = 0;
    pinStates[i].originalValue = 0;
    pinStates[i].lastChange = 0;
    pinStates[i].interval = 0;
    pinStates[i].isReversed = false;
  }
}

void PinController::setPin(int pinNum, int value, unsigned long interval) {
  // Check if pin already exists
  bool found = false;
  for (int i = 0; i < pinCount; i++) {
    if (pinStates[i].pin == pinNum) {
      // For auto-reverse functionality, we typically want to return to LOW (0) state
      // regardless of what the pin was previously set to
      pinStates[i].originalValue = 0;  // Default to LOW as "original" state for reversal
      pinStates[i].value = value;
      pinStates[i].lastChange = millis();
      pinStates[i].interval = interval;
      pinStates[i].isReversed = false;  // Reset reversal flag
      digitalWrite(pinNum, value ? HIGH : LOW);
      found = true;
      break;
    }
  }

  // If not found, add new pin
  if (!found && pinCount < MAX_PINS) {
    pinStates[pinCount].pin = pinNum;
    // For auto-reverse functionality, we typically want to return to LOW (0) state
    pinStates[pinCount].originalValue = 0;  // Default to LOW as "original" state for reversal
    pinStates[pinCount].value = value;
    pinStates[pinCount].lastChange = millis();
    pinStates[pinCount].interval = interval;
    pinStates[pinCount].isReversed = false;  // Reset reversal flag

    pinMode(pinNum, OUTPUT);
    digitalWrite(pinNum, value);

    pinCount++;
  }
}

void PinController::processAutoReverse() {
  unsigned long now = millis();
  for (int i = 0; i < pinCount; i++) {
    if (pinStates[i].interval > 0 && (now - pinStates[i].lastChange >= pinStates[i].interval) && !pinStates[i].isReversed) {
      // Revert to original state ONCE
      digitalWrite(pinStates[i].pin, pinStates[i].originalValue);

      // Mark as reversed so it won't repeat
      pinStates[i].isReversed = true;
    }
  }
}

int PinController::getPinCount() const {
  return pinCount;
}

const PinState& PinController::getPinState(int index) const {
  return pinStates[index];
}
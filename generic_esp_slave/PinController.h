#ifndef PIN_CONTROLLER_H
#define PIN_CONTROLLER_H

#include <Arduino.h>

struct PinState {
  int pin;
  int value;          // Current value (0 = LOW, 1 = HIGH)
  int originalValue;  // Original state before auto-reverse
  unsigned long lastChange;
  unsigned long interval;  // in ms
  bool isReversed;         // Flag to track if reversal has occurred
  bool isInput;            // Flag to track if pin is configured as input
  int lastButtonState;     // Last button state for edge detection
  int currentButtonState;  // Current button state
  unsigned long lastDebounceTime; // Last debounce time
};

class PinController {
  private:
    static const int MAX_PINS = 20;
    PinState pinStates[MAX_PINS];
    int pinCount;

  public:
    PinController();
    
    // Add or update a pin with specified value and auto-reverse interval
    void setPin(int pinNum, int value, unsigned long interval = 0);
    
    // Set all GPIO pins (0-39) to LOW
    void offAll();
    
    // Set all GPIO pins (0-39) to HIGH
    void onAll();
    
    // Process auto-reverse for all pins
    void processAutoReverse();
    
    // Set a pin as input mode for button handling
    void setPinAsInput(int pinNum);
    
    // Scan all input pins for button presses
    void scanButtons();
    
    // Get the current state of a pin (1 or 0)
    int getState(int pinNum) const;
    
    // Get current pin count
    int getPinCount() const;
    
    // Get pin state by index
    const PinState& getPinState(int index) const;
};

#endif
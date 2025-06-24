#ifndef STI_H
#define STI_H

#include <Arduino.h>
#include <map>

#define MAX_FRAME_LEN 128

class Passti {
public:

  static HardwareSerial* mSerial;
  static byte responseBuffer[MAX_FRAME_LEN];
  static size_t responseLen;
  static bool responseReady;
  static std::map<String, String> sti_cmd;

  static void setupSerial(int uartNum, int rx, int tx);
  static void sendCommand(const char* cmdHex, const char* dataHex, bool debug);
  static void init(const char* key);
  static void init(bool debug);  // versi default
  static void getUid();
  static void checkBalance();
  static void readSerialFrame();
};

#endif

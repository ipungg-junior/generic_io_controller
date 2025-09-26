#include "sti.h"

HardwareSerial* Passti::mSerial = nullptr;
byte Passti::responseBuffer[MAX_FRAME_LEN];
size_t Passti::responseLen = 0;
bool Passti::responseReady = false;
bool Passti::is_busy = false;

std::map<String, String> Passti::sti_cmd = {
  { "init", "EF0101" },
  { "get_uid", "EF0105" },
  { "check_balance", "EF0102" }
};

void Passti::setupSerial(int uartNum, int rx, int tx) {
  Passti::mSerial = new HardwareSerial(uartNum);
  Passti::mSerial->begin(38400, SERIAL_8N1, rx, tx);
}

void Passti::sendCommand(const char* cmdHex, const char* dataHex, bool debug) {
  if (Passti::is_busy) {
    if (debug) Serial.println("⚠️ Device sedang sibuk. Command dilewati.");
    return;
  }

  if (strlen(cmdHex) != 6 || strlen(dataHex) % 2 != 0) {
    Serial.println("Command Hex error!");
    return;
  }

  if (debug) Serial.println("Start packing frame data . . .");

  uint8_t dataLen = strlen(dataHex) / 2;
  uint8_t totalLen = 3 + dataLen;
  byte frame[4 + totalLen + 1];
  uint8_t idx = 0;

  frame[idx++] = 0x02;
  frame[idx++] = (totalLen >> 8) & 0xFF;
  frame[idx++] = totalLen & 0xFF;

  for (int i = 0; i < 3; i++) {
    char byteStr[3] = { cmdHex[i * 2], cmdHex[i * 2 + 1], '\0' };
    frame[idx++] = (byte)strtol(byteStr, NULL, 16);
  }

  for (uint8_t i = 0; i < dataLen; i++) {
    char byteStr[3] = { dataHex[i * 2], dataHex[i * 2 + 1], '\0' };
    frame[idx++] = (byte)strtol(byteStr, NULL, 16);
  }

  byte lrc = frame[1];
  for (uint8_t i = 2; i < idx; i++) lrc ^= frame[i];
  frame[idx++] = lrc;

  if (debug) Serial.println("Start sending frame data . . .");

  Passti::is_busy = true;
  Passti::mSerial->write(frame, idx);

  if (debug) {
    Serial.print("Sending data : ");
    for (size_t i = 0; i < idx; i++) Serial.printf("0x%02X ", frame[i]);
    Serial.println("\nData sent completed.");
  }
}

void Passti::init(const char* key) {
  sendCommand(sti_cmd["init"].c_str(), key, true);
  Passti::readSerialFrame();
}

void Passti::init(bool debug) {
  const char* defaultKey = "758F40D46D95D1641448AA19B9282C05";
  if (debug) Serial.println("Ready to init device . . .");
  init(defaultKey);
}

void Passti::getUid() {
  sendCommand(sti_cmd["get_uid"].c_str(), "", true);
}

void Passti::checkBalance() {
  const char* dt = "18062025091540";
  const char* timeout = "0008";
  String data = String(dt) + timeout;
  sendCommand(sti_cmd["check_balance"].c_str(), data.c_str(), true);
}

void Passti::readSerialFrame() {
  static enum { WAIT_STX, READ_LEN_H, READ_LEN_L, READ_BODY } state = WAIT_STX;
  static byte lenH = 0, lenL = 0;
  static size_t index = 0;
  static size_t expectedLen = 0;

  if (Passti::mSerial->available()) {
    byte b = Passti::mSerial->read();

    switch (state) {
      case WAIT_STX:
        if (b == 0x02) {
          index = 0;
          responseBuffer[index++] = b;
          state = READ_LEN_H;
        }
        break;

      case READ_LEN_H:
        lenH = b;
        responseBuffer[index++] = b;
        state = READ_LEN_L;
        break;

      case READ_LEN_L:
        lenL = b;
        responseBuffer[index++] = b;
        expectedLen = 3 + ((lenH << 8) | lenL) + 1;
        state = READ_BODY;
        break;

      case READ_BODY:
        responseBuffer[index++] = b;
        if (index >= expectedLen) {
          responseLen = index;
          responseReady = true;
          is_busy = false;
          state = WAIT_STX;
          return;
        }
        break;
    }
  }
}

void Passti::parseResponse() {
  if (!responseReady) return;

  if (responseLen < 7 || responseBuffer[0] != 0x02) {
    Serial.println("❌ Frame tidak valid atau terlalu pendek.");
    return;
  }

  uint16_t length = (responseBuffer[1] << 8) | responseBuffer[2];
  if (length + 4 != responseLen) {
    Serial.println("❌ Panjang frame tidak sesuai.");
    return;
  }

  byte calcLRC = responseBuffer[1];
  for (size_t i = 2; i < responseLen - 1; i++) calcLRC ^= responseBuffer[i];

  byte recvLRC = responseBuffer[responseLen - 1];
  if (calcLRC != recvLRC) {
    Serial.printf("❌ LRC mismatch! Hitung: 0x%02X, Terima: 0x%02X\n", calcLRC, recvLRC);
    return;
  }

  byte cmd[3] = {
    responseBuffer[3],
    responseBuffer[4],
    responseBuffer[5]
  };
  Serial.printf("✅ CMD: %02X %02X %02X\n", cmd[0], cmd[1], cmd[2]);

  int dataLen = length - 3;
  if (dataLen > 4) {
    Serial.print("📦 DATA: ");
    for (int i = 0; i < dataLen; i++) {
      Serial.printf("%02X ", responseBuffer[6 + i]);
    }
    Serial.println();
  } else {
    for (int i = 0; i < dataLen; i++) {
      Serial.printf("%02X ", responseBuffer[6 + i]);
    }
    Serial.println();
  }

  responseReady = false;
  responseLen = 0;
  memset(responseBuffer, 0, MAX_FRAME_LEN);
}

bool Passti::isBusy() {
  return is_busy;
}

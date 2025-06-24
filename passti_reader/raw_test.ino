#include <Arduino.h>

#define TX_PIN 17  // TX ke MAX3232 (DB9 pin 2)
#define RX_PIN 16  // RX dari MAX3232 (DB9 pin 3)
#define MAX_FRAME_LEN 128

HardwareSerial passtiSerial(2); // Gunakan UART2

byte responseBuffer[MAX_FRAME_LEN];
size_t responseLen = 0;
bool responseReady = false;

// Fungsi membangun dan kirim frame PASSTI sebagai byte (bukan ASCII)
void sendPasstiCommand(const char* cmdHex, const char* dataHex) {
  if (strlen(cmdHex) != 6 || strlen(dataHex) % 2 != 0) {
    Serial.println("❌ CMD harus 6 karakter hex, DATA harus genap.");
    return;
  }

  size_t dataLen = strlen(dataHex) / 2;
  size_t totalLen = 3 + dataLen;
  byte frame[4 + totalLen + 1]; // STX + LEN-H + LEN-L + CMD + DATA + LRC
  size_t idx = 0;

  frame[idx++] = 0x02; // STX
  frame[idx++] = (totalLen >> 8) & 0xFF; // LEN-H
  frame[idx++] = totalLen & 0xFF;        // LEN-L

  // CMD
  for (int i = 0; i < 3; i++) {
    char byteStr[3] = { cmdHex[i * 2], cmdHex[i * 2 + 1], '\0' };
    frame[idx++] = (byte)strtol(byteStr, NULL, 16);
  }

  // DATA
  for (size_t i = 0; i < dataLen; i++) {
    char byteStr[3] = { dataHex[i * 2], dataHex[i * 2 + 1], '\0' };
    frame[idx++] = (byte)strtol(byteStr, NULL, 16);
  }

  // LRC
  byte lrc = frame[1];
  for (size_t i = 2; i < idx; i++) lrc ^= frame[i];
  frame[idx++] = lrc;

  passtiSerial.write(frame, idx);

  Serial.print("\n📤 Kirim: ");
  for (size_t i = 0; i < idx; i++) {
    Serial.printf("0x%02X ", frame[i]);
  }
  Serial.println();
}

// Fungsi GET UID (tanpa data)
void sendGetUID() {
  sendPasstiCommand("EF0105", "");
}

// Fungsi INIT (16 byte key)
void sendInit() {
  const char* initKey = "758F40D46D95D1641448AA19B9282C05";
  sendPasstiCommand("EF0101", initKey);
}

// Fungsi cek saldo dengan datetime & timeout
void sendCheckBalance() {
  const char* dt = "180625091540"; // ddmmyyyyhhmmss
  const char* timeout = "0800";
  String data = String(dt) + timeout;
  sendPasstiCommand("EF0102", data.c_str());
}

// Serial frame reader (non-blocking)
void readSerialFrame(HardwareSerial &port) {
  static enum { WAIT_STX, READ_LEN_H, READ_LEN_L, READ_BODY } state = WAIT_STX;
  static byte lenH = 0, lenL = 0;
  static size_t index = 0;
  static size_t expectedLen = 0;

  while (port.available()) {
    byte b = port.read();

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
        expectedLen = 3 + ((lenH << 8) | lenL) + 1; // STX + LEN + CMD+DATA + LRC
        state = READ_BODY;
        break;

      case READ_BODY:
        responseBuffer[index++] = b;
        if (index >= expectedLen) {
          responseLen = index;
          responseReady = true;
          state = WAIT_STX;
          return;
        }
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  passtiSerial.begin(38400, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(1000);

  Serial.println("\n🚀 Kirim INIT...");
  sendInit();
  delay(500);

  Serial.println("\n📡 Kirim CHECK BALANCE...");
  sendCheckBalance();
  delay(500);

  Serial.println("\n🔁 Siap baca respon...");
}

void loop() {
  readSerialFrame(passtiSerial);

  if (responseReady) {
    Serial.println("\n📥 Data lengkap diterima:");
    for (size_t i = 0; i < responseLen; i++) {
      Serial.printf("0x%02X ", responseBuffer[i]);
    }
    Serial.println();

    // TODO: parsing, validasi LRC, dll

    responseReady = false;
    responseLen = 0;
  }

  // kode lain tetap bisa jalan tanpa terganggu
} 
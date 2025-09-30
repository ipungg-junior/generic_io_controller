#include <Ethernet.h>
#include <EthernetUdp.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <SPI.h>
#include "Wiegand.h"
#include <ArduinoJson.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <TimeLib.h>
#include <Update.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>

// Konfigurasi pin untuk smart card reader
const int RX_PIN = 16;// Pin RX
const int TX_PIN = 17;// Pin TX
const int RELAY_PIN = 32;// Pin Relay
const int BUTTON_PIN_1 = 13;// Pin Button 1
const int BUTTON_PIN_2 = 14;// Pin Button 2
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0x02 };// Alamat MAC
IPAddress ip(10, 251, 2, 126);// Alamat IP 

// Whitelists IP
IPAddress whitelist[] = {IPAddress(10, 251, 12, 133), IPAddress(10, 251, 2, 103), IPAddress(10, 251, 2, 109)};
int whitelistSize = sizeof(whitelist) / sizeof(whitelist[0]);

// DB Server
IPAddress server_addr(10, 251, 2, 114);// Alamat IP server MySQL
char user[] = "uprod";// username MySQL
char dbPassword[] = "P@ssw0rd*1";// password MySQL
EthernetClient client;
EthernetClient sqlClient;
MySQL_Connection conn((Client *)&sqlClient);
EthernetServer server(80);// Web server on port 80
EthernetUDP udp;
unsigned int localPort = 8888;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

// Server NTP → pakai IP langsung lebih aman
IPAddress ntpServerIP(103, 83, 142, 30); // pool.ntp.org Indonesia

//Inisialisasi Serial dan Wiegand
WIEGAND wg;

// SPIFFS
const int MAX_LOG_ENTRIES = 100;
const String LOG_FILE_PATH = "/debug_log.json";
long expectedFileSize = 0;

// #define FILE_NAME "uploaded_firmware.bin"
int otaProgress = 0;
unsigned long restartTime = 0;// Store the time when the update is completed
bool restartPending = false;// Flag to indicate if a restart is pending

String getDBTime() {
  String dbTime = "";

  if (conn.connect(server_addr, 3306, user, dbPassword, "doorlock")) {
    MySQL_Cursor *cur = new MySQL_Cursor(&conn);
    cur->execute("SELECT NOW()");  // ambil waktu server

    column_names *cols = cur->get_columns();
    row_values *row = cur->get_next_row();

    if (row) {
      dbTime = String(row->values[0]);  
      Serial.println("DB Time: " + dbTime);
    }

    delete cur;
    conn.close();
  } else {
    Serial.println("Failed to connect to DB for time.");
  }

  return dbTime;
}
// Function to check if card exists in the database
bool checkCardInDatabase(const String &uidCard, String &name, String &employeeNumber) {
    // Connect to MySQL server
    if (conn.connect(server_addr, 3306, user, dbPassword, "doorlock")) {
        Serial.println("Connected to MySQL server.");

        // Construct the query string to check if the card exists
        char query[512]; // Increased size for the join query
        snprintf(query, sizeof(query), 
                 "SELECT ec.nip, e.name FROM employee_card ec "
                 "JOIN employee e ON ec.employee_id = e.id "
                 "WHERE ec.card_number = '%s'", uidCard.c_str());

        // Create a MySQL cursor object to execute the query
        MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
        cur_mem->execute(query);

        // Fetch the result
        column_names *cols = cur_mem->get_columns();
        row_values *row = cur_mem->get_next_row();

        if (row != NULL) {
            // If row is found, assign the values to name and employeeNumber
            employeeNumber = row->values[0];
            name = row->values[1];

            // Clean up cursor and close the connection
            delete cur_mem;
            conn.close();
            return true; // Card exists in the database
        } else {
            // No row found, the card does not exist in the database
            delete cur_mem;
            conn.close();
            return false; // Card does not exist
        }
    } else {
        Serial.println("Connection to MySQL server failed.");
        return false;
    }
}

// Function to save card data to the database with employee ID
void saveCardToDB(String cardId, int employeeId, String employeeNumber) {
    if (conn.connect(server_addr, 3306, user, dbPassword, "doorlock")) {
        MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
        char query[256];
        // Insert card data with employee ID and employee number (NIP)
        sprintf(query, "INSERT INTO employee_card (card_number, employee_id, nip) VALUES ('%s', '%d', '%s')", cardId.c_str(), employeeId, employeeNumber.c_str());
        cur_mem->execute(query);

        // Cleanup
        delete cur_mem;
        conn.close();  // Close connection after execution
    } else {
        Serial.println("Failed to connect to database.");
    }
}

// Utility function to extract query parameters from the request string
String getQueryParam(const String &request, const String &param) {
    int paramStart = request.indexOf(param + "=");
    if (paramStart == -1) return "";
    paramStart += param.length() + 1;
    int paramEnd = request.indexOf('&', paramStart);
    if (paramEnd == -1) paramEnd = request.indexOf(' ', paramStart);
    return request.substring(paramStart, paramEnd);
}

// Fungsi untuk mengaktifkan relay
void activateRelay(const int _RELAY_PIN) {
  digitalWrite(_RELAY_PIN, HIGH);  // Nyalakan relay
  logMessage("Relay Aktif");
  delay(3000);                    // Biarkan relay menyala selama 3 detik
  digitalWrite(_RELAY_PIN, LOW);  // Matikan relay
  logMessage("Relay Nonaktif");
}

void setup() {
  Serial.begin(115200);                 // Inisialisasi Serial

  // Koneksi ke Ethernet
  Ethernet.init(5);  // CS pin
  Ethernet.begin(mac, ip);
  delay(1000);  // Beri waktu untuk menginisialisasi Ethernet


  // Ambil waktu dari MySQL
  if (client.connect(server_addr, 3306)) {
    if (conn.connect(server_addr, 3306, user, dbPassword, "doorlock")) {
      MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
      cur_mem->execute("SELECT NOW()");
      column_names *cols = cur_mem->get_columns();
      row_values *row = cur_mem->get_next_row();
      if (row) {
        Serial.print("DB Time: ");
        Serial.println(row->values[0]);   // contoh output: 2025-09-22 18:10:05
        logMessage("DB Time: " + String(row->values[0]));
      } else {
        Serial.println("Gagal ambil waktu dari DB");
      }
      delete cur_mem;
      conn.close();
    } else {
      Serial.println("MySQL Connection Failed.");
    }
    client.stop();
  }
}

void loop() {

    // Koneksi ke database MySQL untuk insert log
    if (conn.connect(server_addr, 3306, user, dbPassword, "doorlock")) {
      MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
      char query[256];
      sprintf(query, "CALL InsertLogButton('%d')", 1);
      cur_mem->execute(query);
      delete cur_mem;
      conn.close();  // Tutup koneksi setelah eksekusi
      logMessage("Insert success to MySQL for button 1.");
    } else {
      logMessage("Connection failed to MySQL for button 1.");
    }
  

  // Cek apakah ada data dari Wiegand reader
  if (wg.available()) {
    String uidCard = String(wg.getCode());
    logMessage("UID Card = " + uidCard);

    // Koneksi ke database MySQL untuk cek dan insert data kartu
    if (conn.connect(server_addr, 3306, user, dbPassword, "doorlock")) {
      char query_get_employee_id[256];
      sprintf(query_get_employee_id, "SELECT employee_card.id, employee.name FROM employee_card JOIN employee ON employee.id = employee_card.employee_id WHERE employee_card.card_number = '%s'", String(wg.getCode()).c_str());
      MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
      cur_mem->execute(query_get_employee_id);
      column_names *cols = cur_mem->get_columns();
      String employee_id;
      String employee_name;
      row_values *row = NULL;
      do {
        row = cur_mem->get_next_row();
        if (row != NULL) {
          employee_id = String(row->values[0]);
          employee_name = String(row->values[1]);
        }
      } while (row != NULL);

      delete cur_mem;
      conn.close();  // Tutup koneksi setelah eksekusi
    } else {
      logMessage("Connection to MySQL lost. Trying to reconnect...");
    }
  }
}
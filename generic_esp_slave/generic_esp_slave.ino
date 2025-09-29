#include <SPI.h>
#include "EthernetManager.h"
#include "WebService.h"
#include "Parser.h"

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
    IPAddress(10, 251, 2, 103)
};

struct PinState {
  int pin;
  int value;         // 0 = LOW, 1 = HIGH
  unsigned long lastChange;  
  unsigned long interval;    // in ms
};

const int MAX_PINS = 20;   // adjust for your project
PinState pinStates[MAX_PINS];
int pinCount = 0;

String restartKey = "";   // temp key

// Ethernet connection object
EthernetManager eth(mac, staticIP, dns, gateway, subnet);
WebService http(eth, 80);

void handleSetPin(int pinNum, int value, unsigned long interval) {
  // check if pin already exists
  bool found = false;
  for (int i = 0; i < pinCount; i++) {
    if (pinStates[i].pin == pinNum) {
      pinStates[i].value = value;
      pinStates[i].lastChange = millis();
      pinStates[i].interval = interval;
      digitalWrite(pinNum, value ? HIGH : LOW);
      found = true;
      break;
    }
  }

  // if not found, add new pin
  if (!found && pinCount < MAX_PINS) {
    pinStates[pinCount].pin = pinNum;
    pinStates[pinCount].value = value;
    pinStates[pinCount].lastChange = millis();
    pinStates[pinCount].interval = interval;

    pinMode(pinNum, OUTPUT);
    digitalWrite(pinNum, value);

    pinCount++;
  }
}

void relayOff(EthernetClient& client, const String& path, const String& body) {
  Serial.println("relayOff executed");


  digitalWrite(32, LOW);
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.print("{\"status\":\"OFF\",\"relay\":");
  client.print(body);
  client.print("}");
}

void relayOn(EthernetClient& client, const String& path, const String& body) {
  Serial.println("relayOn executed");
  digitalWrite(32, HIGH);
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println("Relay OFF");
}

void servLanding(EthernetClient& client, const String& path, const String& body){
    Serial.println(" executed");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close"); 
    client.println();
    
    client.println("<!DOCTYPE HTML>");
    client.println("<html><head><title>BSR Doorlock Web Server</title>");
    // JavaScript code for handling confirmation and sending the OTA request
    client.println("<script src='https://cdn.jsdelivr.net/npm/sweetalert2@11'></script>");  // Include SweetAlert2 library
    client.println("<script>");
    client.println("function confirmOTA() {");
    client.println("  Swal.fire({");
    client.println("    title: 'Authentication Required',");
    client.println("    text: 'Enter your password to proceed with the firmware update:',");
    client.println("    icon: 'warning',");
    client.println("    input: 'password',");  // Add password input
    client.println("    inputAttributes: {");
    client.println("      autocapitalize: 'off'");
    client.println("    },");
    client.println("    showCancelButton: true,");
    client.println("    confirmButtonColor: '#3085d6',");
    client.println("    cancelButtonColor: '#d33',");
    client.println("    confirmButtonText: 'Confirm',");
    client.println("    cancelButtonText: 'Cancel',");
    client.println("    preConfirm: (password) => {");
    client.println("      if (!password) {");  // Check if password is entered
    client.println("        Swal.showValidationMessage('Password is required!');");
    client.println("        return false;");
    client.println("      }");
    client.println("      return password;");  // Return password to be used in the next step
    client.println("    }");
    client.println("  }).then((result) => {");
    client.println("    if (result.isConfirmed) {");
    client.println("      // You can add a check here for the password if needed");
    client.println("      if (result.value === 'Bsr12345') {");  // Replace with your actual password
    client.println("        Swal.fire({");  // Show loading after confirmation
    client.println("          title: 'Updating...',");
    client.println("          text: 'Please wait while the firmware is being updated.',");
    client.println("          allowOutsideClick: false,");
    client.println("          didOpen: () => {");
    client.println("            Swal.showLoading();");  // Show loading spinner
    client.println("            window.location.href = '/ota';");  // Redirect to OTA update if confirmed
    client.println("          }");
    client.println("        });");
    client.println("      } else {");
    client.println("        Swal.fire({");
    client.println("          icon: 'error',");
    client.println("          title: 'Invalid Password',");
    client.println("          text: 'The password you entered is incorrect.'");
    client.println("        });");
    client.println("      }");
    client.println("    }");
    client.println("  });");
    client.println("}");
    client.println("</script>");

    // Add CSS to center the page content
    client.println("<style>");
    client.println("body, html { height: 100%; margin: 0; display: flex; justify-content: center; }");
    client.println(".container { text-align: center; margin-top: 20px; }");  // Adjust margin-top to suit your needs
    client.println(".container-btn { text-align: center; margin-top: 20px; }");
    client.println(".container-btn button {");
    client.println("  padding: 15px 30px;");
    client.println("  margin: 5px;");
    client.println("  font-size: 18px;");
    client.println("  background-color: #007BFF;");
    client.println("  color: white;");
    client.println("  border: none;");
    client.println("  border-radius: 5px;");
    client.println("  cursor: pointer;");
    client.println("  transition: background-color 0.3s ease;");
    client.println("}");
    client.println(".container-btn button:hover {");
    client.println("  background-color: #0056b3;");
    client.println("}");
    client.println("</style>");
    client.println("</head>");

    client.println("<body>");
    client.println("<div class='container'>");
    client.println("<a href='https://ibb.co.com/C9CjtN8'><img src='https://i.ibb.co.com/C9CjtN8/key-card.png' alt='key-card' border='0' /></a>");
    client.println("<h1>Doorlock Web Server - Lantai 2</h1>");
    client.println("<div class='container-btn'>");
    // Add buttons for navigation
    client.println("<button onclick='restart_esp()' style='padding:10px 20px; background-color:red;'>Force restart</button>");
    client.println("<button onclick='confirmOTA()' style='padding:10px 20px;'>OTA Update</button>");
    client.println("<button onclick=\"location.href='/logs'\" style='padding:10px 20px;'>View Logs</button>");
    client.println("<button onclick=\"location.href='/open_door'\" style='padding:10px 20px;'>Open the door</button>");
    client.println("<button onclick=\"location.href='/register_card'\" style='padding:10px 20px;'>Register Card</button>");
    client.println("</div>");
    client.println("<br><br>");
    client.println("</div>");
    client.println("<script>");
    client.println("  async function restart() {");
    client.println("    // 1️⃣ Immediately call restart API");
    client.println("    await fetch('/force-restart', { method: 'GET' });");
    client.println("    // 2️⃣ After 3 minutes, refresh page");
    client.println("    setTimeout(() => { location.reload(); }, 120000);");
    client.println("  }");
    client.println("function restart_esp() {");
    client.println("  Swal.fire({");
    client.println("    title: 'Authentication Required',");
    client.println("    text: 'Enter your password to proceed restart:',");
    client.println("    icon: 'warning',");
    client.println("    input: 'password',");  // Add password input
    client.println("    inputAttributes: {");
    client.println("      autocapitalize: 'off'");
    client.println("    },");
    client.println("    showCancelButton: true,");
    client.println("    confirmButtonColor: '#3085d6',");
    client.println("    cancelButtonColor: '#d33',");
    client.println("    confirmButtonText: 'Confirm',");
    client.println("    cancelButtonText: 'Cancel',");
    client.println("    preConfirm: (password) => {");
    client.println("      if (!password) {");  // Check if password is entered
    client.println("        Swal.showValidationMessage('Password is required!');");
    client.println("        return false;");
    client.println("      }");
    client.println("      return password;");  // Return password to be used in the next step
    client.println("    }");
    client.println("  }).then((result) => {");
    client.println("    if (result.isConfirmed) {");
    client.println("      // You can add a check here for the password if needed");
    client.println("      if (result.value === 'Bsr12345') {");  // Replace with your actual password
    client.println("            restart();");  // Redirect to OTA update if confirmed
    client.println("      } else {");
    client.println("        Swal.fire({");
    client.println("          icon: 'error',");
    client.println("          title: 'Invalid Password',");
    client.println("          text: 'The password you entered is incorrect.'");
    client.println("        });");
    client.println("      }");
    client.println("    }");
    client.println("  });");
    client.println("}");
    client.println("</script>");
    client.println("</body></html>");
}

void restartController(EthernetClient& client, const String& path, const String& body){
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close"); 
  client.println();
  ESP.restart();
}

void io_handler(EthernetClient& client, const String& path, const String& body) {
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
    
    pinMode(pin, OUTPUT);
    digitalWrite(pin, setVal);

    if (!parser.hasKey("auto_reverse")) {
      int interVal = parser.getInt("auto_reverse");
      handleSetPin(pin, setVal, interVal);
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
  else if (cmd == "restart_all") {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Restarting...");
    ESP.restart();
  } 
  else {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Unknown command");
  }
}

void io_routine(){
  unsigned long now = millis();
  for (int i = 0; i < pinCount; i++) {
    if (pinStates[i].interval > 0 && (now - pinStates[i].lastChange >= pinStates[i].interval)) {
      // change state ONCE
      digitalWrite(pinStates[i].pin, pinStates[i].value ? HIGH : LOW);

      // mark as done (reset interval so it won’t repeat)
      pinStates[i].interval = 0;  
    }
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
  http.on("/", servLanding);
  http.on("/relay", io_handler);
  http.on("/relay/on", relayOn);
  http.on("/relay/off", relayOff);

}

void loop() {
  // waiting client forever
  http.handleClient();

  // IO routine auto reverse
  io_routine();

}
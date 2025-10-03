# Doorlock Controller by esp32

![Dev](https://img.shields.io/badge/Branch-stable-green) 

## Introduction Project
This project is main program for controlling door with [Wiegand card reader](https://www.alibaba.com/countrysearch/CN/wiegand-card-reader.html) which read RFID/Mifare/NFC


## Tools

- Arduino IDE [download here](https://downloads.arduino.cc/arduino-ide/arduino-ide_latest_Windows_64bit.exe)
- CH340 Serial Driver (unofficial) [download here](https://www.driverscape.com/download/usb-serial-ch340)



# How to use
### Cloning Repository
```bash
# clone repository
git clone https://github.com/ipungg-junior/generic_io_controller.git -b doorlock-v2

# Create python env inside repository
cd generic_io_controller/doorlock
```

### Network Configuration
#### The device can be use static IP configuration:
- **IP Address**: `YOUR-STATIC-IP`
- **Gateway**: `YOUR-GATEWAY`
- **Subnet**: `255.255.255.0` (default)
- **DNS**: `YOUR-DNS`

```cpp
byte mac[] = { 0xDE, 0xAA, 0xBE, 0xEF, 0x00, 0x02 };
IPAddress staticIP(10, 251, 2, 126);
IPAddress gateway(10, 251, 2, 1);
IPAddress dns(10, 251, 2, 1);
IPAddress subnet(255, 255, 255, 0);

// Create ether obj
EthernetManager eth(mac, staticIP, dns, gateway, subnet);
```

#### Allowed IP whitelist:
How to setup whitelist IP, if you limit the IP that can have access

```cpp
// example whitelist
IPAddress whitelist[] = {
  IPAddress(10, 251, 12, 133)
};

// Apply config
eth.setWhitelist(whitelist, sizeof(whitelist) / sizeof(whitelist[0]));
```

### HTTP API Documentation

The doorlock controller provides REST-like HTTP API endpoints for remote control

### Base URL
```
http://YOUR-IP/
```
### Request Format
All requests must be HTTP POST with `Content-Type: application/json`.
### Response Format
All responses return JSON with the following structure:
```json
{
  "status": true/false,
  "message": "Description of the result",
  ... (additional fields depending on command)
}
```
---
### API Endpoints

#### 1. GPIO Control
#### Set pin state
Endpoint: `POST /gpio`
Command: `set_pin`

**Request Body**:
```json
{
  "command": "set_pin",
  "pin_number": 32,
  "value": 1,
  "auto_reverse": 5000
}
```

**Parameters**:
- `pin_number` (integer, required): GPIO pin number to control
- `value` (integer, required): Pin state (0 = LOW, 1 = HIGH)
- `auto_reverse` (integer, optional): Auto-reverse state delay in milliseconds (0 = no auto-reverse)

**Response**:
```json
{
  "status": true,
  "pin_num": 32,
  "message": "Pin io setting up completed"
}
```

**Example**:
```bash
curl -X POST http://YOUR-IP/gpio \
  -H "Content-Type: application/json" \
  -d '{"command": "set_pin", "pin_number": 32, "value": 1, "auto_reverse": 5000}'
```

#### Turn Off All Pins
Endpoint: `POST /gpio`
Command: `off_all`

**Request Body**:
```json
{
  "command": "off_all"
}
```

**Response**:
```json
{
  "status": true,
  "message": "Set all GPIO to 0 (off)"
}
```

#### Turn On All Pins
Endpoint: `POST /gpio`
Command: `on_all`

**Request Body**:
```json
{
  "command": "on_all"
}
```

**Response**:
```json
{
  "status": true,
  "message": "Set all GPIO to 1 (on)"
}
```
---
### 2. Core System (`/core`)

#### Restart Device
Endpoint: `POST /core`
Command: `restart`

**Request Body**:
```json
{
  "command": "restart"
}
```

**Response**:
```json
{
  "status": true,
  "message": "Trying to restart, see you :)"
}
```

**Note**: This command will restart the ESP32 device immediately.

#### Register New RFID Card
Endpoint: `POST /core`
Command: `register_card`

**Request Body**:
```json
{
  "command": "register_card"
}
```

**Response**:
```json
{
  "status": true,
  "uid": "1234567890",
  "message": "Found card uid"
}
```

**Example**:
```bash
curl -X POST http://YOUR-IP/core \
  -H "Content-Type: application/json" \
  -d '{"command": "register_card"}'
```

---

## Error Responses

### Invalid JSON
```json
{
  "status": false,
  "message": "Invalid JSON"
}
```

### Missing Parameters
```json
{
  "status": false,
  "message": "Please set your pin!"
}
```

### Unknown Command
```json
{
  "status": false,
  "message": "Unknown command on GPIO"
}
```

### Wiegand Timeout (Card Registration)
```json
{
  "status": false,
  "message": "Wiegand timeout"
}
```

---

**Note**: Device automatically synchronizes system time with MySQL server on startup for accurate timestamp logging.

***

## Authors
#### [Muhammad Thoyfur](https://github.com/ipungg-junior) | Software Engineer 
#### [Ahmad Wildan](https://github.com/matwildan?tab=repositories) | Hardware Embedded


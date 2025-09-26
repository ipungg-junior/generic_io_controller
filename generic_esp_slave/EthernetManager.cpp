#include "EthernetManager.h"

EthernetManager::EthernetManager(byte macAddress[6], IPAddress ipAddr, IPAddress dnsAddr, IPAddress gatewayAddr, IPAddress subnetMask)
  : ip(ipAddr), dns(dnsAddr), gateway(gatewayAddr), subnet(subnetMask),
    whitelist(nullptr), whitelistCount(0) {
  memcpy(mac, macAddress, 6);
}

void EthernetManager::begin(uint8_t pin) {
  Ethernet.init(pin);
  Ethernet.begin(mac);
  Ethernet.setLocalIP(ip); // IP ESP LANTAI 2
  Ethernet.setGatewayIP(dns);
  Ethernet.setDnsServerIP(gateway);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.");
    while (true);
  }
}

void EthernetManager::setWhitelist(IPAddress* list, int count) {
  whitelist = list;
  whitelistCount = count;
}

bool EthernetManager::isAllowed(IPAddress clientIP) {
  if (!whitelist || whitelistCount == 0) return true;
  for (int i = 0; i < whitelistCount; i++) {
    if (clientIP == whitelist[i]) return true;
  }
  return false;
}

IPAddress EthernetManager::getLocalIP() {
  return Ethernet.localIP();
}

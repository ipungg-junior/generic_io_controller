#ifndef ETHERNET_MANAGER_H
#define ETHERNET_MANAGER_H

#include <Arduino.h>
#include <Ethernet.h>

class EthernetManager {
  private:
    byte mac[6];
    IPAddress ip, dns, gateway, subnet;
    IPAddress* whitelist;
    int whitelistCount;

  public:
    EthernetManager(byte macAddress[6], IPAddress ipAddr, IPAddress dnsAddr, IPAddress gatewayAddr, IPAddress subnetMask);
    void begin(uint8_t pin);
    void setWhitelist(IPAddress* list, int count);
    bool isAllowed(IPAddress clientIP);
    IPAddress getLocalIP();
};

#endif

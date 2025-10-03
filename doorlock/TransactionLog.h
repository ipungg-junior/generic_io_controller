#ifndef TRANSACTIONLOG_H
#define TRANSACTIONLOG_H

#include <EEPROM.h>

struct TransactionEntry {
    uint16_t id;
    char uid[9];  // 8 digit + null terminator
};

class TransactionLog {
public:
    TransactionLog(int eepromSize = 512, int maxLogs = 5);

    void begin();

    bool add(const TransactionEntry& entry);
    TransactionEntry get(uint8_t index) const;
    uint8_t size() const;
    void clear();

private:
    void saveHeader();
    void loadHeader();

    int _eepromSize;
    int _maxLogs;
    int _logStartAddr;
    uint8_t _head;
    uint8_t _count;
};

#endif // TRANSACTIONLOG_H

#include "TransactionLog.h"

TransactionLog::TransactionLog(int eepromSize, int maxLogs)
    : _eepromSize(eepromSize),
      _maxLogs(maxLogs),
      _logStartAddr(10),
      _head(0),
      _count(0) {}

void TransactionLog::begin() {
    EEPROM.begin(_eepromSize);
    loadHeader();
}

void TransactionLog::loadHeader() {
    _head = EEPROM.read(0);
    _count = EEPROM.read(1);
    if (_count > _maxLogs) {
        _count = 0;
        _head = 0;
    }
}

void TransactionLog::saveHeader() {
    EEPROM.write(0, _head);
    EEPROM.write(1, _count);
    EEPROM.commit();
}

bool TransactionLog::add(const TransactionEntry& entry) {
    if (_maxLogs == 0) return false;

    int addr = _logStartAddr + (_head * sizeof(TransactionEntry));
    EEPROM.put(addr, entry);
    EEPROM.commit();

    _head = (_head + 1) % _maxLogs;
    if (_count < _maxLogs) _count++;
    saveHeader();

    return true;
}

TransactionEntry TransactionLog::get(uint8_t index) const {
    TransactionEntry entry;
    if (index >= _count) {
        entry.id = 0;
        entry.uid[0] = '\0';
        return entry;
    }

    int readPos = (_head + _maxLogs - _count + index) % _maxLogs;
    int addr = _logStartAddr + (readPos * sizeof(TransactionEntry));
    EEPROM.get(addr, entry);
    return entry;
}

uint8_t TransactionLog::size() const {
    return _count;
}

void TransactionLog::clear() {
    _head = 0;
    _count = 0;
    saveHeader();

    // Optional: clear EEPROM logs
    TransactionEntry emptyEntry = {0, ""};
    for (int i = 0; i < _maxLogs; i++) {
        int addr = _logStartAddr + (i * sizeof(TransactionEntry));
        EEPROM.put(addr, emptyEntry);
    }
    EEPROM.commit();
}

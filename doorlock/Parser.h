#ifndef PARSER_H
#define PARSER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>

class Parser {
  private:
    DynamicJsonDocument doc;
    bool valid;

  public:
    Parser(const String& body);

    bool isValid() const;
    String getCommand() const;

    bool hasKey(const char* key) const;
    String getString(const char* key) const;
    int getInt(const char* key) const;
};

#endif

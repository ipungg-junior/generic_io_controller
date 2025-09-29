#include "Parser.h"

Parser::Parser(const String& body) 
  : doc(512), valid(false) 
{
  DeserializationError error = deserializeJson(doc, body);
  if (!error) {
    valid = true;
  }
}

bool Parser::isValid() const {
  return valid;
}

String Parser::getCommand() const {
  if (!valid || !doc.containsKey("command")) return "";
  return doc["command"].as<String>();
}

bool Parser::hasKey(const char* key) const {
  return valid && doc.containsKey(key);
}

String Parser::getString(const char* key) const {
  if (!hasKey(key)) return "";
  return doc[key].as<String>();
}

int Parser::getInt(const char* key) const {
  if (!hasKey(key)) return -1;
  return doc[key].as<int>();
}

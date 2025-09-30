#include "MySQLConnector.h"
#include <Ethernet.h>

MySQLConnector::MySQLConnector() : connection(nullptr), client(nullptr), cursor(nullptr), isConnected(false), lastQuery(nullptr) {
}

MySQLConnector::~MySQLConnector() {
  close();
  if (lastQuery) {
    delete[] lastQuery;
  }
}

bool MySQLConnector::connect(const IPAddress& server, int port, const char* user, const char* password, const char* database) {
  // Close any existing connection
  close();
  
  // Create a new EthernetClient
  client = new EthernetClient();
  
  // Create a new connection
  connection = new MySQL_Connection(client);
  
  // Connect to MySQL server
  if (connection->connect(server, port, user, password, database)) {
    isConnected = true;
    return true;
  } else {
    isConnected = false;
    delete connection;
    connection = nullptr;
    delete client;
    client = nullptr;
    return false;
  }
}

bool MySQLConnector::query(const char* sql) {
  if (!isConnected || !connection) {
    Serial.println("Not connected to database");
    return false;
  }
  
  // Create a cursor if we don't have one
  if (cursor) {
    delete cursor;
  }
  cursor = new MySQL_Cursor(connection);
  
  // Execute the query
  bool result = cursor->execute(sql);
  
  if (!result) {
    Serial.println("Query execution failed");
  }
  
  return result;
}

bool MySQLConnector::query(const char* format, ...) {
  if (!isConnected || !connection) {
    Serial.println("Not connected to database");
    return false;
  }
  
  // Create a buffer for the formatted query
  char queryBuffer[256];  // Adjust size as needed for your queries
  
  // Format the query with variables
  va_list args;
  va_start(args, format);
  vsnprintf(queryBuffer, sizeof(queryBuffer), format, args);
  va_end(args);
  
  // Execute the formatted query
  return query(queryBuffer);
}

MySQL_Cursor* MySQLConnector::getCursor() {
  return cursor;
}

void MySQLConnector::close() {
  // Close cursor if it exists
  if (cursor) {
    delete cursor;
    cursor = nullptr;
  }
  
  // Close connection if it exists
  if (connection) {
    connection->close();
    delete connection;
    connection = nullptr;
  }
  
  // Delete client if it exists
  if (client) {
    delete client;
    client = nullptr;
  }
  
  isConnected = false;
}

bool MySQLConnector::connected() const {
  return isConnected && connection && connection->connected();
}
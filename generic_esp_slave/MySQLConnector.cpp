#include "MySQLConnector.h"
#include <Ethernet.h>

MySQLConnector::MySQLConnector() : connection(nullptr), cursor(nullptr), isConnected(false) {
}

MySQLConnector::~MySQLConnector() {
  close();
}

bool MySQLConnector::connect(const IPAddress& server, int port, const char* user, const char* password, const char* database) {
  // Close any existing connection
  close();
  
  // Create a new connection
  connection = new MySQL_Connection(new Client_TCPSOCKET(server, port));
  
  // Try to connect
  if (connection->connect(user, password, database)) {
    isConnected = true;
    return true;
  } else {
    isConnected = false;
    delete connection;
    connection = nullptr;
    return false;
  }
}

bool MySQLConnector::query(const char* sql) {
  if (!isConnected || !connection) {
    Serial.println("Not connected to database");
    return false;
  }
  
  // Create a cursor if we don't have one
  if (!cursor) {
    cursor = new MySQL_Cursor(connection);
  }
  
  // Execute the query
  if (cursor->execute(sql)) {
    return true;
  } else {
    Serial.println("Query execution failed");
    return false;
  }
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
  
  isConnected = false;
}

bool MySQLConnector::connected() const {
  return isConnected && connection && connection->connected();
}
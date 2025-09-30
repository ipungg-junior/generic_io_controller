#ifndef MYSQL_CONNECTOR_H
#define MYSQL_CONNECTOR_H

#include <Arduino.h>
// Include Chuck Bell's MySQL library
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <MySQL_Encrypt.h>
#include <MySQL_Packet.h>
#include <MySQL_Parser.h>
#include <MySQL_Buffer.h>
#include <MySQL_SHA1.h>

class MySQLConnector {
private:
  MySQL_Connection* connection;
  MySQL_Cursor* cursor;
  bool isConnected;

public:
  MySQLConnector();
  ~MySQLConnector();
  
  // Connect to MySQL database
  bool connect(const IPAddress& server, int port, const char* user, const char* password, const char* database);
  
  // Execute a query
  bool query(const char* sql);
  
  // Close the database connection
  void close();
  
  // Check if connected
  bool connected() const;
};

#endif
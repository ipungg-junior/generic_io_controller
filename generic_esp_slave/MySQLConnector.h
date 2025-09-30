#ifndef MYSQL_CONNECTOR_H
#define MYSQL_CONNECTOR_H

#include <Arduino.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <stdarg.h>
#include <stdio.h>

class MySQLConnector {
private:
  MySQL_Connection* connection;
  MySQL_Cursor* cursor;
  bool isConnected;
  char* lastQuery;

public:
  MySQLConnector();
  ~MySQLConnector();
  
  // Connect to MySQL database
  bool connect(const IPAddress& server, int port, const char* user, const char* password, const char* database);
  
  // Execute a query
  bool query(const char* sql);
  
  // Execute a query with variable substitution
  bool query(const char* format, ...);
  
  // Get cursor for result set processing
  MySQL_Cursor* getCursor();
  
  // Close the database connection
  void close();
  
  // Check if connected
  bool connected() const;
};

#endif
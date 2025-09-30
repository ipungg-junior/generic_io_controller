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

// Forward declarations
class MySQL_Connection;
class MySQL_Cursor;

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
  
  // Execute a query with variable substitution (up to 10 variables)
  bool query(const char* format, ...);
  
  // Fetch the next row from the result set
  bool fetchRow();
  
  // Get the number of columns in the result set
  int getColumnCount() const;
  
  // Get string value of a column by index
  const char* getString(int columnIndex) const;
  
  // Get integer value of a column by index
  long getInt(int columnIndex) const;
  
  // Get float value of a column by index
  float getFloat(int columnIndex) const;
  
  // Close the database connection
  void close();
  
  
  // Check if connected
  bool connected() const;
};

#endif
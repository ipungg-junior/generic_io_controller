#ifndef MYSQL_CONNECTOR_H
#define MYSQL_CONNECTOR_H

#include <Arduino.h>
#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <stdarg.h>
#include <stdio.h>
#include <vector>

// Structure to hold a single row of data as strings
struct RowData {
  std::vector<String> values;
};

// Type definition for a collection of rows
typedef std::vector<RowData> QueryResult;

class MySQLConnector {
private:
  EthernetClient* client;
  MySQL_Connection* connection;
  MySQL_Cursor* currentCursor;
  row_values* currentRow;
  bool isConnected;

  // Save DB authenticate
  IPAddress saveAddress;
  int savePort;
  char saveUser[32];
  char savePassword[32];
  char saveDatabase[32];

  // unsigned time millis for last query
  unsigned long lastQuery;

public:
  MySQLConnector();
  ~MySQLConnector();
  
  // Connect to MySQL database
  bool connect(const IPAddress& server, int port, const char* user, const char* password, const char* database);
  
  // Execute a query (INSERT, UPDATE, DELETE, or SELECT)
  bool query(const char* sql);
  
  // Execute a query with variable substitution (INSERT, UPDATE, DELETE, or SELECT)
  bool queryf(const char* format, ...);
  
  // Fetch the next row from a SELECT query result
  bool fetchRow();
  
  // Get integer value from current row at specified column index
  int getInt(int columnIndex);
  
  // Get string value from current row at specified column index
  const char* getString(int columnIndex);
  
  // Execute a SELECT query and return cursor for result processing
  MySQL_Cursor* select(const char* sql);
  
  // Execute a SELECT query with variable substitution and return cursor for result processing
  MySQL_Cursor* selectf(const char* format, ...);
  
  // Execute a SELECT query and return results as QueryResult
  bool selectQuery(const char* sql, QueryResult& result);
  
  // Execute a SELECT query with variable substitution and return results as QueryResult
  bool selectQueryf(QueryResult& result, const char* format, ...);
  
  // Get the connection object for direct access to cursor creation
  MySQL_Connection* getConnection();
  
  // Close the database connection
  void close();
  
  // Check if connected
  bool connected() const;

  // Close cursor
  void closeCursor();

  // reconnect db
  bool reconnect();
};

#endif
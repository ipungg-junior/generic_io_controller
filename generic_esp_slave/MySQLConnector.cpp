#include "MySQLConnector.h"

MySQLConnector::MySQLConnector() : client(nullptr), connection(nullptr), currentCursor(nullptr), currentRow(nullptr), isConnected(false), lastQuery(0) {
  // Initialize the save fields
  saveUser[0] = '\0';
  savePassword[0] = '\0';
  saveDatabase[0] = '\0';
  savePort = 0;
}

MySQLConnector::~MySQLConnector() {
  close();
  if (currentCursor) {
    delete currentCursor;
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
  if (connection->connect(server, port, (char*)user, (char*)password, (char*)database)) {
    isConnected = true;
    // Copy the config values
    saveAddress = server;
    savePort = port;
    strncpy(saveUser, user, sizeof(saveUser) - 1);
    saveUser[sizeof(saveUser) - 1] = '\0';
    strncpy(savePassword, password, sizeof(savePassword) - 1);
    savePassword[sizeof(savePassword) - 1] = '\0';
    strncpy(saveDatabase, database, sizeof(saveDatabase) - 1);
    saveDatabase[sizeof(saveDatabase) - 1] = '\0';
    lastQuery = millis();
    return true;
  } else {
    isConnected = false;
    // Clean up on failure
    delete connection;
    connection = nullptr;
    delete client;
    client = nullptr;
    return false;
  }
}

bool MySQLConnector::reconnect() {
  // Clean up existing connection
  if (connection) {
    connection->close();
    delete connection;
    connection = nullptr;
  }
  
  if (client) {
    delete client;
    client = nullptr;
  }

  // Create a new EthernetClient
  client = new EthernetClient();
  
  // Create a new connection
  connection = new MySQL_Connection(client);
  
  // Connect to MySQL server
  if (connection->connect(saveAddress, savePort, saveUser, savePassword, saveDatabase)) {
    isConnected = true;
    return true;
  } else {
    isConnected = false;
    // Clean up on failure
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
  
  // Check if reconnection is needed (more than 1 minute since last query)
  if (millis() - lastQuery >= 60000) {
    Serial.println("Reconnecting to database...");
    if (!reconnect()) {
      Serial.println("Failed to reconnect to database!");
      return false;
    }
    Serial.println("Reconnected to database successfully");
  }
  
  // Update last query time
  lastQuery = millis();
  
  // Check if this is a SELECT query
  if (strncmp(sql, "SELECT", 6) == 0 || strncmp(sql, "select", 6) == 0) {
    // Clean up any existing cursor
    if (currentCursor) {
      delete currentCursor;
    }
    
    // Create a new cursor for SELECT queries
    currentCursor = new MySQL_Cursor(connection);
    currentRow = nullptr;
    
    // Execute the SELECT query
    if (!currentCursor->execute(sql)) {
      Serial.println("SELECT query execution failed");
      delete currentCursor;
      currentCursor = nullptr;
      return false;
    }
    
    // Get column information (required by MySQL library)
    column_names* cols = currentCursor->get_columns();
    
    // cols is now stored in the cursor and can be accessed later
    return true;
  } else {
    // For non-SELECT queries, execute directly
    MySQL_Cursor cur = MySQL_Cursor(connection);
    bool result = cur.execute(sql);
    
    if (!result) {
      Serial.println("Query execution failed");
    }
    
    return result;
  }
}

MySQL_Cursor* MySQLConnector::select(const char* sql) {
  if (!isConnected || !connection) {
    Serial.println("Not connected to database");
    return nullptr;
  }
  
  // Check if reconnection is needed (more than 1 minute since last query)
  if (millis() - lastQuery >= 60000) {
    Serial.println("Reconnecting to database...");
    if (!reconnect()) {
      Serial.println("Failed to reconnect to database!");
      return nullptr;
    }
    Serial.println("Reconnected to database successfully");
  }
  
  // Update last query time
  lastQuery = millis();
  
  if (currentCursor) {
      delete currentCursor;
    }
    
    // Create a new cursor for SELECT queries
    currentCursor = new MySQL_Cursor(connection);
    
    // Execute the SELECT query
    if (!currentCursor->execute(sql)) {
      Serial.println("SELECT query execution failed");
      delete currentCursor;
      currentRow = nullptr;
      return nullptr;
    }
  
  return currentCursor;
}

MySQL_Cursor* MySQLConnector::selectf(const char* format, ...) {
  if (!isConnected || !connection) {
    Serial.println("Not connected to database");
    return nullptr;
  }
  
  // Check if reconnection is needed (more than 1 minute since last query)
  if (millis() - lastQuery >= 60000) {
    Serial.println("Reconnecting to database...");
    if (!reconnect()) {
      Serial.println("Failed to reconnect to database!");
      return nullptr;
    }
    Serial.println("Reconnected to database successfully");
  }
  
  // Update last query time
  lastQuery = millis();
  
  // Create a buffer for the formatted query
  char queryBuffer[256];  // Adjust size as needed for your queries
  
  // Format the query with variables
  va_list args;
  va_start(args, format);
  vsnprintf(queryBuffer, sizeof(queryBuffer), format, args);
  va_end(args);
  
  // Execute the formatted SELECT query
  return select(queryBuffer);
}

bool MySQLConnector::fetchRow() {
  if (!currentCursor) {
    Serial.println("No active SELECT query, at least has a column!");
    return false;
  }
  
  currentRow = currentCursor->get_next_row();
  return (currentRow != nullptr);
}

int MySQLConnector::getInt(int columnIndex) {
  if (!currentRow || columnIndex < 0) {
    return 0;
  }
  
  // Check if column index is valid
  // Note: We're assuming the row has enough columns
  return atoi(currentRow->values[columnIndex]);
}

const char* MySQLConnector::getString(int columnIndex) {
  if (!currentRow || columnIndex < 0) {
    return "";
  }
  
  // Check if column index is valid
  // Note: We're assuming the row has enough columns
  return currentRow->values[columnIndex];
}

bool MySQLConnector::queryf(const char* format, ...) {
  if (!isConnected || !connection) {
    Serial.println("Not connected to database");
    return false;
  }
  
  // Check if reconnection is needed (more than 1 minute since last query)
  if (millis() - lastQuery >= 60000) {
    Serial.println("Reconnecting to database...");
    if (!reconnect()) {
      Serial.println("Failed to reconnect to database!");
      return false;
    }
    Serial.println("Reconnected to database successfully");
  }
  
  // Update last query time
  lastQuery = millis();
  
  // Create a buffer for the formatted query
  char queryBuffer[256];  // Adjust size as needed for your queries
  
  // Format the query with variables
  va_list args;
  va_start(args, format);
  vsnprintf(queryBuffer, sizeof(queryBuffer), format, args);
  va_end(args);
  
  // Check if this is a SELECT query
  if (strncmp(queryBuffer, "SELECT", 6) == 0 || strncmp(queryBuffer, "select", 6) == 0) {
    // Clean up any existing cursor
    if (currentCursor) {
      delete currentCursor;
    }
    
    // Create a new cursor for SELECT queries
    currentCursor = new MySQL_Cursor(connection);
    currentRow = nullptr;
    
    // Execute the SELECT query
    if (!currentCursor->execute(queryBuffer)) {
      Serial.println("SELECT query execution failed");
      delete currentCursor;
      currentCursor = nullptr;
      return false;
    }
    
    // Get column information (required by MySQL library)
    column_names* cols = currentCursor->get_columns();
    
    // cols is now stored in the cursor and can be accessed later
    return true;
  } else {
    // For non-SELECT queries, execute directly
    // Explicit cast to disambiguate between overloaded execute methods
    MySQL_Cursor cur = MySQL_Cursor(connection);
    bool result = cur.execute((const char*)queryBuffer);
    
    if (!result) {
      Serial.println("Query execution failed");
    }
    
    return result;
  }
}

bool MySQLConnector::selectQuery(const char* sql, QueryResult& result) {

  if (!isConnected || !connection) {
    Serial.println("Not connected to database");
    return false;
  }
  
  // Check if reconnection is needed (more than 1 minute since last query)
  if (millis() - lastQuery >= 60000) {
    Serial.println("Reconnecting to database...");
    if (!reconnect()) {
      Serial.println("Failed to reconnect to database!");
      return false;
    }
    Serial.println("Reconnected to database successfully");
  }
  
  // Update last query time
  lastQuery = millis();
  
  // Check if this is a SELECT query
  if (strncmp(sql, "SELECT", 6) != 0 && strncmp(sql, "select", 6) != 0) {
    Serial.println("This method is only for SELECT queries");
    return false;
  }
  
  // Clean up any existing cursor
  if (currentCursor) {
    delete currentCursor;
  }
  
  // Create a new cursor for SELECT queries
  currentCursor = new MySQL_Cursor(connection);
  currentRow = nullptr;
  
  // Execute the SELECT query
  if (!currentCursor->execute(sql)) {
    Serial.println("SELECT query execution failed");
    delete currentCursor;
    currentCursor = nullptr;
    return false;
  }
  
  // Get column information (required by MySQL library)
  column_names* cols = currentCursor->get_columns();
  if (!cols) {
    Serial.println("Failed to get column information");
    delete currentCursor;
    currentCursor = nullptr;
    return false;
  }
  
  // Clear the result vector
  result.clear();
  
  // Fetch all rows and add them to the result
  row_values* row;
  while ((row = currentCursor->get_next_row()) != nullptr) {
    RowData rowData;
    rowData.values.reserve(cols->num_fields);
    
    // Add each column value as a string
    for (int i = 0; i < cols->num_fields; i++) {
      if (row->values[i] != nullptr) {
        rowData.values.push_back(String(row->values[i]));
      } else {
        rowData.values.push_back(String(""));
      }
    }
    
    result.push_back(rowData);
  }
  
  // Clean up cursor
  delete currentCursor;
  currentCursor = nullptr;
  
  return true;
}

bool MySQLConnector::selectQueryf(QueryResult& result, const char* format, ...) {
  if (!isConnected || !connection) {
    Serial.println("Not connected to database");
    return false;
  }
  
  // Check if reconnection is needed (more than 1 minute since last query)
  if (millis() - lastQuery >= 60000) {
    Serial.println("Reconnecting to database...");
    if (!reconnect()) {
      Serial.println("Failed to reconnect to database!");
      return false;
    }
    Serial.println("Reconnected to database successfully");
  }
  
  // Update last query time
  lastQuery = millis();
  
  // Create a buffer for the formatted query
  char queryBuffer[256];  // Adjust size as needed for your queries
  
  // Format the query with variables
  va_list args;
  va_start(args, format);
  vsnprintf(queryBuffer, sizeof(queryBuffer), format, args);
  va_end(args);
  
  // Use the existing selectQuery method to execute the formatted query
  return selectQuery(queryBuffer, result);
}

MySQL_Connection* MySQLConnector::getConnection() {
  return connection;
}

void MySQLConnector::close() {
  if (connection) {
    connection->close();
    delete connection;
    connection = nullptr;
  }
  
  if (client) {
    delete client;
    client = nullptr;
  }
  
  isConnected = false;
}

bool MySQLConnector::connected() const {
  return isConnected && connection && connection->connected();
}

void MySQLConnector::closeCursor() {
  if (connection) {
    delete currentCursor;
    currentCursor = nullptr;
    delete currentRow;
    currentRow = nullptr;
  }
}
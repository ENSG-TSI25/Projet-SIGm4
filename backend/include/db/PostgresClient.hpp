#pragma once

#include <pqxx/pqxx>
#include "Config.hpp"
#include <memory>

/**
 * @class PostgresClient
 * @brief PostgreSQL database client for spatial data
 * 
 * Manages connection to PostgreSQL database with PostGIS extension.
 * Used to store and retrieve geospatial data from the application.
 */
class PostgresClient {
private:
    DBConfig config;                          ///< Connection configuration
    std::unique_ptr<pqxx::connection> conn;   ///< Database connection

public:
    /**
     * @brief Constructor with configuration
     * @param cfg Database configuration parameters
     */
    PostgresClient(const DBConfig& cfg);
    
    /**
     * @brief Destructor
     */
    ~PostgresClient() = default;
    
    /**
     * @brief Establishes database connection
     * 
     * Creates connection using stored configuration.
     * Called during application initialization.
     */
    void connect();
    
    /**
     * @brief Executes SQL query
     * @param sql SQL query string
     * @return Query result set
     */
    pqxx::result execute(const std::string& sql);
    
    /**
     * @brief Checks connection status
     * @return true if connected to database
     */
    bool isConnected() const;

    /**
     * @brief Initializes PostGIS extensions
     * 
     * Enables PostGIS and other required extensions
     * on the database connection.
     */
    void initExtensions();
};
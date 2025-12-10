#include <db/PostgresClient.hpp>
#include <stdexcept>
#include <iostream>

/**
 * @file PostgresClient.cpp
 * @brief PostgreSQL client implementation
 */

/**
 * @brief Constructor with database configuration
 * @param cfg Database configuration parameters
 */
PostgresClient::PostgresClient(const DBConfig& cfg)
    : config(cfg), conn(nullptr)
{}

/**
 * @brief Connects to PostgreSQL database
 * @throws std::runtime_error if connection fails
 * 
 * Builds connection string from configuration
 * and establishes connection using libpqxx.
 */
void PostgresClient::connect() {
    std::string connStr =
        "host=" + config.host +
        " port=" + std::to_string(config.port) +
        " user=" + config.user +
        " password=" + config.password +
        " dbname=" + config.dbname;
    
    conn = std::make_unique<pqxx::connection>(connStr);
    
    if (!conn->is_open())
        throw std::runtime_error("PostgreSQL connection failed");
    
    std::cout << "Connected to database: " << conn->dbname() << std::endl;
}

/**
 * @brief Executes SQL query
 * @param sql SQL query string
 * @return Query result set
 * @throws std::runtime_error if not connected
 * @throws pqxx::sql_error on query error
 */
pqxx::result PostgresClient::execute(const std::string& sql) {
    if (!conn || !conn->is_open())
        throw std::runtime_error("Connection not established");
    
    pqxx::work txn(*conn);
    pqxx::result r = txn.exec(sql);
    txn.commit();
    return r;
}

/**
 * @brief Checks connection status
 * @return true if connected to database
 */
bool PostgresClient::isConnected() const {
    return conn && conn->is_open();
}

/**
 * @brief Initializes PostGIS extensions
 * @throws std::runtime_error if not connected
 * 
 * Enables PostGIS and PostGIS Topology extensions
 * required for spatial operations.
 */
void PostgresClient::initExtensions() {
    if (!conn || !conn->is_open())
        throw std::runtime_error("Connection not established");
    
    pqxx::work txn(*conn);
    txn.exec("CREATE EXTENSION IF NOT EXISTS postgis;");
    txn.exec("CREATE EXTENSION IF NOT EXISTS postgis_topology;");
    txn.commit();
    
    std::cout << "PostGIS extensions initialized" << std::endl;
}
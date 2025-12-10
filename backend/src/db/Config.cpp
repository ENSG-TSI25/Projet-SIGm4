#include <db/Config.hpp>
#include <stdexcept>

/**
 * @file Config.cpp
 * @brief DBConfig implementation
 */

/**
 * @brief Loads database configuration from environment variables
 * @return DBConfig structure with connection parameters
 * @throws std::runtime_error if any required variable is missing
 * 
 * Reads the following environment variables:
 * - DB_HOST: Database server hostname
 * - DB_PORT: Database server port
 * - DB_USER: Database username
 * - DB_PASS: Database password
 * - DB_NAME: Database name
 * 
 * All variables are required. Used in Docker deployment.
 */
DBConfig DBConfig::loadFromEnv() {
    DBConfig cfg;
    
    const char* host = std::getenv("DB_HOST");
    const char* port = std::getenv("DB_PORT");
    const char* user = std::getenv("DB_USER");
    const char* pass = std::getenv("DB_PASS");
    const char* name = std::getenv("DB_NAME");
    
    if (!host || !port || !user || !pass || !name)
        throw std::runtime_error("Missing database environment variables");
    
    cfg.host = host;
    cfg.port = std::stoi(port);
    cfg.user = user;
    cfg.password = pass;
    cfg.dbname = name;
    
    return cfg;
}
#pragma once

#include <string>
#include <cstdlib>

/**
 * @struct DBConfig
 * @brief Database connection configuration
 * 
 * Contains parameters to connect to PostgreSQL database.
 * Values can be loaded from environment variables.
 */
struct DBConfig {
    std::string host;      ///< Database host address
    int port;              ///< Database port number
    std::string user;      ///< Database username
    std::string password;  ///< Database password
    std::string dbname;    ///< Database name

    /**
     * @brief Loads configuration from environment variables
     * @return DBConfig with values from environment
     * 
     * Reads DB_HOST, DB_PORT, DB_USER, DB_PASSWORD, DB_NAME
     * from environment. Used for Docker/container deployment.
     */
    static DBConfig loadFromEnv();
};